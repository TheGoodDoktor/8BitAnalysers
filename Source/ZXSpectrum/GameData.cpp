#include "GameData.h"

#include "Util/FileUtil.h"
#include "json.hpp"
//#include "magic_enum.hpp"
#include <iomanip>
#include <fstream>
#include <sstream>

#include "SpectrumEmu.h"
#include "GameViewers/GameViewer.h"
#include "Debug/DebugLog.h"
#include "Util/Misc.h"
#include <Util/GraphicsView.h>
#include "ZXSpectrumGameConfig.h"

static const int g_kBinaryFileVersionNo = 17;
static const int g_kBinaryFileMagic = 0xdeadface;

const char *GetLabelEnumString(ELabelType labelType)
{
	switch (labelType)
	{
	case ELabelType::Data:
		return "Data";
	case ELabelType::Function:
		return "Function";
	case ELabelType::Code:
		return "Code";
	case ELabelType::Text:
		return "Text";
	}

	return "Unknown";
};

ELabelType GetLabelEnumValue(const char* pString)
{
	for (int i = 0; i < (int)ELabelType::Max; i++)
	{
		if (strcmp(pString, GetLabelEnumString((ELabelType)i)) == 0)
			return (ELabelType)i;
	}

	return ELabelType::None;
}

const char* GetDataEnumString(EDataType dataType)
{
	switch (dataType)
	{
	case EDataType::Byte:
		return "Byte";
	case EDataType::ByteArray:
		return "ByteArray";
	case EDataType::Word:
		return "Word";
	case EDataType::WordArray:
		return "WordArray";
	case EDataType::Text:
		return "Text";
	case EDataType::Bitmap:
		return "Bitmap";
	case EDataType::CharacterMap:
		return "CharacterMap";
	case EDataType::ScreenPixels:
		return "ScreenPixels";
	case EDataType::Image:
		return "Image";
	case EDataType::Blob:
		return "Blob";
	case EDataType::ColAttr:
		return "ColAttr";
	}
	return "Unknown";
};

EDataType GetDataEnumValue(const char* pString)
{
	for (int i = 0; i < (int)EDataType::Max; i++)
	{
		if (strcmp(pString, GetDataEnumString((EDataType)i)) == 0)
			return (EDataType)i;
	}

	return EDataType::None;
}

// Labels
#if 0
void SaveLabelsBin(const FCodeAnalysisState& state, FILE* fp, uint16_t startAddress, uint16_t endAddress)
{
	int recordCount = 0;
	for (int i = startAddress; i <= endAddress; i++)
	{
		if (state.GetLabelForPhysicalAddress(i) != nullptr)
			recordCount++;
	}

	fwrite(&recordCount, sizeof(int), 1, fp);

	for (int i = startAddress; i <= endAddress; i++)
	{

		const FLabelInfo* pLabel = state.GetLabelForPhysicalAddress(i);
		if (pLabel != nullptr)
		{
			const uint16_t addr = (uint16_t)i;
			const std::string funcName = GetLabelEnumString(pLabel->LabelType);
			//const std::string meName = std::string(magic_enum::enum_name(pLabel->LabelType));
			//assert(meName == funcName);
			WriteStringToFile(funcName, fp);
			fwrite(&addr, sizeof(addr), 1, fp);
			fwrite(&pLabel->ByteSize, sizeof(pLabel->ByteSize), 1, fp);
			WriteStringToFile(pLabel->Name, fp);
			WriteStringToFile(pLabel->Comment, fp);
			fwrite(&pLabel->Global, sizeof(bool), 1, fp);

			// References?
			long refCountPos = ftell(fp);
			const auto& referenceList = pLabel->References.GetReferences();
			int noReferences = (int)referenceList.size();
			fwrite(&noReferences, sizeof(int), 1, fp);
			noReferences = 0;
			for (const auto& ref : referenceList)
			{
				const uint16_t refAddr = ref.Address;
				if (refAddr >= startAddress && refAddr <= endAddress)	// only add references from region we are saving
				{
					fwrite(&refAddr, sizeof(refAddr), 1, fp);
					noReferences++;
				}
			}

			// fix up reference count
			fseek(fp, refCountPos, SEEK_SET);
			fwrite(&noReferences, sizeof(int), 1, fp);	// new reference count
			fseek(fp, 0, SEEK_END);	// point back to end of file
		}
	}
}
#endif
void LoadLabelsBin(FCodeAnalysisState& state, FILE* fp, int versionNo, uint16_t startAddress, uint16_t endAddress)
{
	int recordCount = 0;

	state.ResetLabelNames();

	fread(&recordCount, sizeof(int), 1, fp);

	for (int i = 0; i < recordCount; i++)
	{
		FLabelInfo* pLabel = FLabelInfo::Allocate();

		std::string enumVal;
		ReadStringFromFile(enumVal, fp);
		const ELabelType funcType = GetLabelEnumValue(enumVal.c_str());
		//const ELabelType meType = magic_enum::enum_cast<ELabelType>(enumVal).value();
		//assert(meType == funcType);
		pLabel->LabelType = funcType;
		uint16_t addr;
		fread(&addr, sizeof(addr), 1, fp);
		fread(&pLabel->ByteSize, sizeof(pLabel->ByteSize), 1, fp);
		ReadStringFromFile(pLabel->Name, fp);
		ReadStringFromFile(pLabel->Comment, fp);

		if (versionNo > 2)
			fread(&pLabel->Global, sizeof(bool), 1, fp);

		// References?
		if (versionNo > 1)
		{
			int noReferences = 0;
			fread(&noReferences, sizeof(int), 1, fp);
			for (int i = 0; i < noReferences; i++)
			{
				uint16_t refAddr;
				fread(&refAddr, sizeof(refAddr), 1, fp);
				if (refAddr >= startAddress && refAddr <= endAddress)
				{
					pLabel->References.RegisterAccess(state.AddressRefFromPhysicalAddress(refAddr));
				}
				else
				{
					LOGWARNING("Label reference address %x outside of range", refAddr);
				}
			}
		}

		state.SetLabelForPhysicalAddress(addr, pLabel);
	}
}

// Code Info

void SaveCodeInfoBin(const FCodeAnalysisState& state, FILE* fp, uint16_t startAddress, uint16_t endAddress)
{
	int recordCount = 0;
	for (int i = startAddress; i <= endAddress; i++)
	{
		if (state.GetCodeInfoForAddress(i) != nullptr)
			recordCount++;
	}

	fwrite(&recordCount, sizeof(int), 1, fp);

	for (int i = startAddress; i <= endAddress; i++)
	{
		const FCodeInfo* pCodeInfo = state.GetCodeInfoForAddress(i);
		if (pCodeInfo != nullptr)
		{
			const uint16_t addr = (uint16_t)i;
			fwrite(&pCodeInfo->OperandType, sizeof(pCodeInfo->OperandType), 1, fp);
			fwrite(&pCodeInfo->Flags, sizeof(pCodeInfo->Flags), 1, fp);
			fwrite(&addr, sizeof(addr), 1, fp);
			fwrite(&pCodeInfo->ByteSize, sizeof(pCodeInfo->ByteSize), 1, fp);
			WriteStringToFile(pCodeInfo->Comment, fp);
		}
	}
}

void LoadCodeInfoBin(FCodeAnalysisState& state, FILE* fp, int versionNo, uint16_t startAddress, uint16_t endAddress)
{
	int recordCount;
	fread(&recordCount, sizeof(int), 1, fp);

	for (int i = 0; i < recordCount; i++)
	{
		FCodeInfo* pCodeInfo = FCodeInfo::Allocate();

		if (versionNo > 8)
			fread(&pCodeInfo->OperandType, sizeof(pCodeInfo->OperandType), 1, fp);

		if (versionNo >= 4)
			fread(&pCodeInfo->Flags, sizeof(pCodeInfo->Flags), 1, fp);

		uint16_t addr;
		fread(&addr, sizeof(addr), 1, fp);
		fread(&pCodeInfo->ByteSize, sizeof(pCodeInfo->ByteSize), 1, fp);
		if (versionNo < 10)
		{
			fread(&pCodeInfo->JumpAddress, sizeof(pCodeInfo->JumpAddress), 1, fp);
			fread(&pCodeInfo->PointerAddress, sizeof(pCodeInfo->PointerAddress), 1, fp);
			std::string tmp;
			ReadStringFromFile(tmp, fp);
		}
		//ReadStringFromFile(pCodeInfo->Text, fp);
		ReadStringFromFile(pCodeInfo->Comment, fp);
		state.SetCodeInfoForAddress(addr, pCodeInfo);
		for (int codeByte = 1; codeByte < pCodeInfo->ByteSize; codeByte++)	
		{
			FDataInfo* pOperandData = state.GetReadDataInfoForAddress(addr + codeByte);
			pOperandData->DataType = EDataType::InstructionOperand;
			pOperandData->ByteSize = 1;
			pOperandData->InstructionAddress = state.AddressRefFromPhysicalAddress(addr);
		}
	}
}

// Data Info

void SaveDataInfoBin(const FCodeAnalysisState& state, FILE* fp, uint16_t startAddress, uint16_t endAddress)
{
	int recordCount = 0;
	for (int i = startAddress; i <= endAddress; i++)
	{
		if (state.GetReadDataInfoForAddress(i) != nullptr)
			recordCount++;
	}

	fwrite(&recordCount, sizeof(int), 1, fp);

	for (int i = startAddress; i <= endAddress; i++)
	{
		const FDataInfo* pDataInfo = state.GetReadDataInfoForAddress(i);
		if (pDataInfo != nullptr)
		{
			const std::string funcName = GetDataEnumString(pDataInfo->DataType);
			//const std::string meName = std::string(magic_enum::enum_name(pDataInfo->DataType));
			//assert(meName == funcName);
			WriteStringToFile(funcName, fp);
			uint16_t addr = (uint16_t)i;
			fwrite(&addr, sizeof(addr), 1, fp);
			fwrite(&pDataInfo->ByteSize, sizeof(pDataInfo->ByteSize), 1, fp);
			fwrite(&pDataInfo->Flags, sizeof(pDataInfo->Flags), 1, fp);
			fwrite(&pDataInfo->CharSetAddress, sizeof(pDataInfo->CharSetAddress), 1, fp);
			fwrite(&pDataInfo->DisplayType, sizeof(pDataInfo->DisplayType), 1, fp);
			fwrite(&pDataInfo->EmptyCharNo, sizeof(pDataInfo->EmptyCharNo), 1, fp);
			WriteStringToFile(pDataInfo->Comment, fp);

			// Reads & Writes?
			int noReads = 0;
			const long noReadsFilePos = ftell(fp);
			fwrite(&noReads, sizeof(int), 1, fp);
			for (const auto& ref : pDataInfo->Reads.GetReferences())
			{
				const uint16_t refAddr = ref.Address;
				if (refAddr >= startAddress && refAddr <= endAddress)
				{
					fwrite(&refAddr, sizeof(refAddr), 1, fp);
					noReads++;
				}
			}

			// patch number of reads
			fseek(fp, noReadsFilePos, SEEK_SET);
			fwrite(&noReads, sizeof(int), 1, fp);
			fseek(fp, 0, SEEK_END);


			int noWrites = 0;
			const long noWritesFilePos = ftell(fp);
			fwrite(&noWrites, sizeof(int), 1, fp);
			for (const auto& ref : pDataInfo->Writes.GetReferences())
			{
				const uint16_t refAddr = ref.Address;
				if (refAddr >= startAddress && refAddr <= endAddress)
				{
					fwrite(&refAddr, sizeof(refAddr), 1, fp);
					noWrites++;
				}
			}

			// patch number of writes
			fseek(fp, noWritesFilePos, SEEK_SET);
			fwrite(&noWrites, sizeof(int), 1, fp);
			fseek(fp, 0, SEEK_END);
		}
	}


}

void LoadDataInfoBin(FCodeAnalysisState& state, FILE* fp, int versionNo, uint16_t startAddress, uint16_t endAddress)
{
	int recordCount;
	fread(&recordCount, sizeof(int), 1, fp);

	for (int i = 0; i < recordCount; i++)
	{
		std::string enumVal;
		ReadStringFromFile(enumVal, fp);
		uint16_t address = 0;
		fread(&address, sizeof(address), 1, fp);

		FDataInfo* pDataInfo = state.GetReadDataInfoForAddress(address);
		//pDataInfo->Address = address;
		const EDataType funcType = GetDataEnumValue(enumVal.c_str());
		//const EDataType meType = magic_enum::enum_cast<EDataType>(enumVal).value();
		//assert(funcType == meType);
		pDataInfo->DataType = funcType;
		fread(&pDataInfo->ByteSize, sizeof(pDataInfo->ByteSize), 1, fp);
		if (versionNo > 5)
		{
			fread(&pDataInfo->Flags, sizeof(pDataInfo->Flags), 1, fp);
			if (pDataInfo->bShowCharMap)
			{
				pDataInfo->DataType = EDataType::CharacterMap;
				pDataInfo->bShowCharMap = false;
			}

			if (pDataInfo->bShowBinary)
			{
				pDataInfo->DataType = EDataType::Bitmap;
				pDataInfo->bShowBinary = false;
			}
		}

		if (versionNo > 10)
		{
			fread(&pDataInfo->CharSetAddress, sizeof(pDataInfo->CharSetAddress), 1, fp);
			fread(&pDataInfo->DisplayType, sizeof(pDataInfo->DisplayType), 1, fp);
		}

		if (versionNo > 12)
		{
			fread(&pDataInfo->EmptyCharNo, sizeof(pDataInfo->EmptyCharNo), 1, fp);
		}

		ReadStringFromFile(pDataInfo->Comment, fp);

		// References?
		if (versionNo > 1)
		{
			int noReads;
			fread(&noReads, sizeof(int), 1, fp);
			for (int i = 0; i < noReads; i++)
			{
				uint16_t dataAddr;
				fread(&dataAddr, sizeof(uint16_t), 1, fp);
				if (dataAddr >= startAddress && dataAddr <= endAddress)
					pDataInfo->Reads.RegisterAccess(state.AddressRefFromPhysicalAddress(dataAddr));
				else
					LOGWARNING("LoadDataInfoBin: Address %x outside of range", dataAddr);
			}
		}
		if (versionNo > 2)
		{
			int noWrites;
			fread(&noWrites, sizeof(int), 1, fp);
			for (int i = 0; i < noWrites; i++)
			{
				uint16_t dataAddr;
				fread(&dataAddr, sizeof(uint16_t), 1, fp);
				if (dataAddr >= startAddress && dataAddr <= endAddress)
					pDataInfo->Writes.RegisterAccess(state.AddressRefFromPhysicalAddress(dataAddr));
				else
					LOGWARNING("LoadDataInfoBin: Address %x outside of range", dataAddr);
			}
		}

		//state.SetReadDataInfoForAddress(pDataInfo->Address, pDataInfo);
		//state.SetWriteDataInfoForAddress(pDataInfo->Address, pDataInfo);
	}
}

#if 0
void SaveCommentBlocksBin(const FCodeAnalysisState& state, FILE* fp, uint16_t startAddress, uint16_t endAddress)
{
	int recordCount = 0;
	for (int i = startAddress; i <= endAddress; i++)
	{
		if (state.GetCommentBlockForAddress(state.AddressRefFromPhysicalAddress(i)) != nullptr)
			recordCount++;
	}

	fwrite(&recordCount, sizeof(int), 1, fp);

	for (int i = startAddress; i <= endAddress; i++)
	{
		const FCommentBlock* pCommentBlock = state.GetCommentBlockForAddress(state.AddressRefFromPhysicalAddress(i));
		if (pCommentBlock != nullptr)
		{
			uint16_t addr = (uint16_t)i;
			fwrite(&addr, sizeof(addr), 1, fp);
			WriteStringToFile(pCommentBlock->Comment, fp);
		}
	}
}
#endif
void LoadCommentBlocksBin(FCodeAnalysisState& state, FILE* fp, int versionNo, uint16_t startAddress, uint16_t endAddress)
{
	int recordCount;
	fread(&recordCount, sizeof(int), 1, fp);

	for (int i = 0; i < recordCount; i++)
	{
		FCommentBlock* pCommentBlock = FCommentBlock::Allocate();
		uint16_t address;
		fread(&address, sizeof(address), 1, fp);
		ReadStringFromFile(pCommentBlock->Comment, fp);
		state.SetCommentBlockForAddress(state.AddressRefFromPhysicalAddress(address), pCommentBlock);
	}
}


#if 0
// Binary save
void SaveGameDataBin(const FCodeAnalysisState& state, FILE *fp, uint16_t addrStart, uint16_t addrEnd)
{
	fwrite(&g_kBinaryFileMagic, sizeof(int), 1, fp);
	fwrite(&g_kBinaryFileVersionNo, sizeof(int), 1, fp);

	fwrite(&addrStart, sizeof(uint16_t), 1, fp);	// write memory range of this block
	fwrite(&addrEnd, sizeof(uint16_t), 1, fp);

	for (int i = addrStart; i <= addrEnd; i++)
	{
		const uint16_t invalid = 0;
		const uint16_t addr = state.GetLastWriterForAddress(i).Address;
		if (addr >= addrStart && addr <= addrEnd)
			fwrite(&addr, sizeof(uint16_t), 1, fp);
		else
			fwrite(&invalid, sizeof(uint16_t), 1, fp);
	}

	SaveLabelsBin(state, fp, addrStart, addrEnd);
	SaveCodeInfoBin(state, fp, addrStart, addrEnd);
	SaveDataInfoBin(state, fp, addrStart, addrEnd);
	SaveCommentBlocksBin(state, fp, addrStart, addrEnd);

	// Save Watches
	int noWatches = 0;// (int)state.GetWatches().size();
	const long noWatchPos = ftell(fp);
	fwrite(&noWatches, sizeof(noWatches), 1, fp);

	for (const auto& watch : state.GetWatches())
	{
		if (watch.Address >= addrStart && watch.Address <= addrEnd)
		{
			fwrite(&watch, sizeof(uint16_t), 1, fp);
			noWatches++;
		}
	}

	// patch up watch count
	fseek(fp, noWatchPos, SEEK_SET);
	fwrite(&noWatches, sizeof(noWatches), 1, fp);
	fseek(fp, 0, SEEK_END);

	// Save char sets
	{
		int noCharSets = 0;
		const long noCharSetsPos = ftell(fp);
		fwrite(&noCharSets, sizeof(noCharSets), 1, fp);

		for (int i = 0; i < GetNoCharacterSets(); i++)
		{
			const FCharacterSet* pCharSet = GetCharacterSetFromIndex(i);
			const uint16_t addr = pCharSet->Params.Address.Address;
			if (addr >= addrStart && addr <= addrEnd)
			{
				EColourInfo		ColourInfo = EColourInfo::None;
				fwrite(&pCharSet->Params.Address, sizeof(pCharSet->Params.Address), 1, fp);
				fwrite(&pCharSet->Params.AttribsAddress, sizeof(pCharSet->Params.AttribsAddress), 1, fp);
				fwrite(&pCharSet->Params.MaskInfo, sizeof(pCharSet->Params.MaskInfo), 1, fp);
				fwrite(&pCharSet->Params.ColourInfo, sizeof(pCharSet->Params.ColourInfo), 1, fp);
				fwrite(&pCharSet->Params.bDynamic, sizeof(pCharSet->Params.bDynamic), 1, fp);
				noCharSets++;
			}
		}

		// patch up char set count
		fseek(fp, noCharSetsPos, SEEK_SET);
		fwrite(&noCharSets, sizeof(noCharSets), 1, fp);
		fseek(fp, 0, SEEK_END);
	}

	// Save char maps
	{
		int noCharMaps = 0;
		const long noCharMapsPos = ftell(fp);
		fwrite(&noCharMaps, sizeof(noCharMaps), 1, fp);

		for (int i = 0; i < GetNoCharacterMaps(); i++)
		{
			const FCharacterMap* pCharMap = GetCharacterMapFromIndex(i);
			const uint16_t addr = pCharMap->Params.Address.Address;
			if (addr >= addrStart && addr <= addrEnd)
			{
				fwrite(&pCharMap->Params.Address, sizeof(pCharMap->Params.Address), 1, fp);
				fwrite(&pCharMap->Params.Width, sizeof(pCharMap->Params.Width), 1, fp);
				fwrite(&pCharMap->Params.Height, sizeof(pCharMap->Params.Height), 1, fp);
				fwrite(&pCharMap->Params.CharacterSet, sizeof(pCharMap->Params.CharacterSet), 1, fp);
				fwrite(&pCharMap->Params.IgnoreCharacter, sizeof(pCharMap->Params.IgnoreCharacter), 1, fp);
				noCharMaps++;
			}
		}

		// patch up char set count
		fseek(fp, noCharMapsPos, SEEK_SET);
		fwrite(&noCharMaps, sizeof(noCharMaps), 1, fp);
		fseek(fp, 0, SEEK_END);
	}
}
#endif

// Binary load
// TODO: retire
void LoadGameDataBin(FCodeAnalysisState& state, FILE *fp, int versionNo, uint16_t addrStart, uint16_t addrEnd)
{
	if (versionNo >= 8)
	{
		fread(&addrStart, sizeof(uint16_t), 1, fp);
		fread(&addrEnd, sizeof(uint16_t), 1, fp);

		for (int i = addrStart; i <= addrEnd; i++)
		{
			uint16_t lastWriter;
			fread(&lastWriter, sizeof(uint16_t), 1, fp);
			state.SetLastWriterForAddress(i, state.AddressRefFromPhysicalAddress(lastWriter));
		}
	}
	else if (versionNo >= 4)
	{
		for (int i = 0; i < (1 << 16); i++)
		{
			uint16_t lastWriter;
			fread(&lastWriter, sizeof(uint16_t), 1, fp);
			state.SetLastWriterForAddress(i, state.AddressRefFromPhysicalAddress(lastWriter));
		}
	}

	LoadLabelsBin(state, fp, versionNo, addrStart, addrEnd);
	LoadCodeInfoBin(state, fp, versionNo, addrStart, addrEnd);
	LoadDataInfoBin(state, fp, versionNo, addrStart, addrEnd);
	if (versionNo >= 5)
		LoadCommentBlocksBin(state, fp, versionNo, addrStart, addrEnd);

	// watches 
	if (versionNo >= 7)
	{
		int noWatches;
		fread(&noWatches, sizeof(noWatches), 1, fp);

		for (int i = 0; i < noWatches; i++)
		{
			uint16_t watchAddress;
			fread(&watchAddress, sizeof(uint16_t), 1, fp);
			if (watchAddress >= addrStart && watchAddress <= addrEnd)
				state.Debugger.AddWatch({ state.GetBankFromAddress(watchAddress), watchAddress });
		}
	}

	// char sets
	if (versionNo > 10)
	{
		int noCharSets;
		fread(&noCharSets, sizeof(noCharSets), 1, fp);

		for (int i = 0; i < noCharSets; i++)
		{
			FCharSetCreateParams params;
			fread(&params.Address, sizeof(params.Address), 1, fp);
			if (versionNo > 11)
			{
				fread(&params.AttribsAddress, sizeof(params.AttribsAddress), 1, fp);
				fread(&params.MaskInfo, sizeof(params.MaskInfo), 1, fp);
				fread(&params.ColourInfo, sizeof(params.ColourInfo), 1, fp);
			}
			if (versionNo > 14)
			{
				fread(&params.bDynamic, sizeof(params.bDynamic), 1, fp);
			}
			params.ColourLUT = state.Config.CharacterColourLUT;
			CreateCharacterSetAt(state, params);
		}
	}

	// char maps
	if (versionNo > 13)
	{
		int noCharMaps;
		fread(&noCharMaps, sizeof(noCharMaps), 1, fp);

		for (int i = 0; i < noCharMaps; i++)
		{
			FCharMapCreateParams params;
			fread(&params.Address, sizeof(params.Address), 1, fp);
			fread(&params.Width, sizeof(params.Width), 1, fp);
			fread(&params.Height, sizeof(params.Height), 1, fp);
			fread(&params.CharacterSet, sizeof(params.CharacterSet), 1, fp);
			fread(&params.IgnoreCharacter, sizeof(params.IgnoreCharacter), 1, fp);
			CreateCharacterMap(state, params);
		}
	}
}

const uint32_t kMachineStateMagic = 0xFaceCafe;
const uint32_t kMachineStateVersion = 4;

static zx_t g_SaveSlot;

void SaveMachineState(FSpectrumEmu* pSpectrumEmu, FILE *fp)
{
	FCodeAnalysisState& state = pSpectrumEmu->CodeAnalysis;
	FZXSpectrumGameConfig& config = *(FZXSpectrumGameConfig*)pSpectrumEmu->pActiveGame->pConfig;

	// revert cheats
	for (FCheat& cheat : config.Cheats)
	{
		for (auto& entry : cheat.Entries)
		{
			if (cheat.bEnabled)	// cheat activated so revert
			{
				pSpectrumEmu->WriteByte(entry.Address, entry.OldValue);
				cheat.bEnabled = false;
			}
		}
	}

	// revert NOPs
	for (int addr = 0x4000; addr <= 0xffff; addr++)
	{
		FCodeInfo* pCodeInfo = state.GetCodeInfoForAddress(addr);
		if (pCodeInfo && pCodeInfo->bNOPped)
		{
			// Restore
			for (int i = 0; i < pCodeInfo->ByteSize; i++)
				state.CPUInterface->WriteByte(addr + i, pCodeInfo->OpcodeBkp[i]);

			pCodeInfo->bNOPped = false;
		}
	}

	// write magic
	fwrite(&kMachineStateMagic, sizeof(kMachineStateMagic), 1, fp);
	fwrite(&kMachineStateVersion, sizeof(kMachineStateVersion), 1, fp);

	// just save the whole thing out
	zx_t& dst =  g_SaveSlot;
	dst = pSpectrumEmu->ZXEmuState;	// copy to save slot
	chips_debug_snapshot_onsave(&dst.debug);
	chips_audio_callback_snapshot_onsave(&dst.audio.callback);
	ay38910_snapshot_onsave(&dst.ay);
	mem_snapshot_onsave(&dst.mem, &pSpectrumEmu->ZXEmuState);

	fwrite(&dst, sizeof(zx_t), 1, fp);
	return;
}

bool LoadMachineState(FSpectrumEmu* pSpectrumEmu, FILE* fp)
{
	uint32_t magicVal;
	fread(&magicVal, sizeof(magicVal), 1, fp);
	if (magicVal != kMachineStateMagic)
		return false;

	// version
	uint32_t fileVersion = 0;
	fread(&fileVersion, sizeof(fileVersion), 1, fp);

	if (fileVersion != kMachineStateVersion)	// since machine state is not that important different file version numbers get rejected
		return false;

	// load the entire state
	zx_t* sys = &pSpectrumEmu->ZXEmuState;
	zx_t& im = g_SaveSlot;

	fread(&im, sizeof(zx_t), 1, fp);	// load into save slot

	// fixup pointers & callbacks
	chips_debug_snapshot_onload(&im.debug, &sys->debug);
	chips_audio_callback_snapshot_onload(&im.audio.callback, &sys->audio.callback);
	ay38910_snapshot_onload(&im.ay, &sys->ay);
	mem_snapshot_onload(&im.mem, sys);
	*sys = im;	// copy across new state

	// Set code analysis banks
	if (sys->type == ZX_TYPE_128)
	{
		const uint8_t memConfig = pSpectrumEmu->ZXEmuState.last_mem_config;
		pSpectrumEmu->SetROMBank(memConfig & (1 << 4) ? 1 : 0);
		pSpectrumEmu->SetRAMBank(3, memConfig & 0x7);
		pSpectrumEmu->CodeAnalysis.SetAllBanksDirty();
	}
	return true;
}

bool SaveGameState(FSpectrumEmu* pSpectrumEmu, const char* fname)
{
	FILE* fp = fopen(fname, "wb");
	if (fp == NULL)
		return false;
	SaveMachineState(pSpectrumEmu, fp);
	fclose(fp);

	return true;
}

bool LoadGameState(FSpectrumEmu* pSpectrumEmu, const char* fname)
{
	FILE* fp = fopen(fname, "rb");
	if (fp == NULL)
		return false;

	LoadMachineState(pSpectrumEmu, fp);
	fclose(fp);

	return true;
}
#if 0
bool SaveGameData(FSpectrumEmu* pSpectrumEmu, const char* fname)
{
	FCodeAnalysisState& state = pSpectrumEmu->CodeAnalysis;
	FGameConfig& config = *pSpectrumEmu->pActiveGame->pConfig;

	FILE* fp = fopen(fname, "wb");
	if (fp == NULL)
		return false;

	SaveGameDataBin(state, fp, 0x4000, 0xffff);

	fwrite(&state.StackMin, sizeof(uint16_t), 1, fp);
	fwrite(&state.StackMax, sizeof(uint16_t), 1, fp);

	const uint8_t hasSnapshot = config.WriteSnapshot ? 1 : 0;

	fwrite(&hasSnapshot, sizeof(uint8_t), 1, fp);

	if (hasSnapshot == 1)
	{
		SaveMachineState(pSpectrumEmu, fp);
	}

	fclose(fp);

	return true;
}

bool SaveROMData(const FCodeAnalysisState& state, const char* fname)
{
	FILE* fp = fopen(fname, "wb");
	if (fp == NULL)
		return false;

	SaveGameDataBin(state, fp, 0x0000, 0x3fff);

	return true;

}
#endif
bool LoadGameData(FSpectrumEmu* pSpectrumEmu, const char* fname)
{
	FCodeAnalysisState& state = pSpectrumEmu->CodeAnalysis;

	FILE* fp = fopen(fname, "rb");
	if (fp == NULL)
		return false;

	int magic, versionNo;
	fread(&magic, sizeof(int), 1, fp);
	if (magic != g_kBinaryFileMagic)
	{
		fclose(fp);
		return false;
	}

	fread(&versionNo, sizeof(int), 1, fp);

	LoadGameDataBin(state, fp, versionNo, 0x4000, 0xffff);

	if (versionNo > 16)
	{
		uint16_t	stackMin, stackMax;
		fread(&stackMin, sizeof(uint16_t), 1, fp);
		fread(&stackMax, sizeof(uint16_t), 1, fp);

		// should we just discard them since the binary
	}

	if (versionNo > 15)
	{
		uint8_t hasSnapshot = 0;

		fread(&hasSnapshot, sizeof(uint8_t), 1, fp);

		if (hasSnapshot == 1)
		{
			LoadMachineState(pSpectrumEmu, fp);
		}
	}
	fclose(fp);
	return true;
}

bool LoadROMData(FCodeAnalysisState& state, const char* fname)
{
	FILE* fp = fopen(fname, "rb");
	if (fp == NULL)
		return false;

	int magic, versionNo;
	fread(&magic, sizeof(int), 1, fp);
	if (magic != g_kBinaryFileMagic)
	{
		fclose(fp);
		return false;
	}

	fread(&versionNo, sizeof(int), 1, fp);
	LoadGameDataBin(state, fp, versionNo, 0x0000, 0x3fff);

	fclose(fp);
	return true;
}
