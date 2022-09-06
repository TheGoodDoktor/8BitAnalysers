#include "GameConfig.h"

#include "Util/FileUtil.h"
#include "json.hpp"
#include "magic_enum.hpp"
#include <iomanip>
#include <fstream>

#include "SpeccyUI.h"
#include "GameViewers/GameViewer.h"
#include "SnapshotLoaders/GamesList.h"

using json = nlohmann::json;
static std::vector< FGameConfig *>	g_GameConfigs;

bool AddGameConfig(FGameConfig *pConfig)
{
	g_GameConfigs.push_back(pConfig);
	return true;
}

const std::vector< FGameConfig *>& GetGameConfigs()
{
	return g_GameConfigs;
}

FGameConfig * CreateNewGameConfigFromSnapshot(const FGameSnapshot& snapshot)
{
	FGameConfig *pNewConfig = new FGameConfig;

	pNewConfig->Name = snapshot.DisplayName;
	pNewConfig->SnapshotFile = snapshot.FileName;
	pNewConfig->pViewerConfig = GetViewConfigForGame(pNewConfig->Name.c_str());

	return pNewConfig;
}



bool SaveGameConfigToFile(const FGameConfig &config, const char *fname)
{
	json jsonConfigFile;
	jsonConfigFile["Name"] = config.Name;
	jsonConfigFile["Z80File"] = config.SnapshotFile;

	for (const auto&sprConfigIt : config.SpriteConfigs)
	{
		json spriteConfig;
		const FSpriteDefConfig& spriteDefConfig = sprConfigIt.second;
		
		spriteConfig["Name"] = sprConfigIt.first;
		spriteConfig["BaseAddress"] = MakeHexString(spriteDefConfig.BaseAddress);
		spriteConfig["Count"] = spriteDefConfig.Count;
		spriteConfig["Width"] = spriteDefConfig.Width;
		spriteConfig["Height"] = spriteDefConfig.Height;

		jsonConfigFile["SpriteConfigs"].push_back(spriteConfig);
	}

	for (const FCheat& cheat : config.Cheats)
	{
		json cheatJson;
		cheatJson["Description"] = cheat.Description;
		for (const FCheatMemoryEntry& entry : cheat.Entries)
		{
			json cheatEntryJson;
			cheatEntryJson["Address"] = entry.Address;
			cheatEntryJson["Value"] = entry.Value;
			cheatJson["Entries"].push_back(cheatEntryJson);
		}

		jsonConfigFile["Cheats"].push_back(cheatJson);
	}

	std::ofstream outFileStream(fname);
	if (outFileStream.is_open())
	{
		outFileStream << std::setw(4) << jsonConfigFile << std::endl;
		return true;
	}

	return false;
}


bool LoadGameConfigFromFile(FGameConfig &config, const char *fname)
{
	std::ifstream inFileStream(fname);
	if (inFileStream.is_open() == false)
		return false;

	json jsonConfigFile;

	inFileStream >> jsonConfigFile;
	inFileStream.close();

	config.Name = jsonConfigFile["Name"].get<std::string>();
	config.SnapshotFile = jsonConfigFile["Z80File"].get<std::string>();
	config.pViewerConfig = GetViewConfigForGame(config.Name.c_str());

	for(const auto & jsonSprConfig : jsonConfigFile["SpriteConfigs"])
	{
		const std::string &name = jsonSprConfig["Name"].get<std::string>();
		
		FSpriteDefConfig &sprConfig = config.SpriteConfigs[name];
		sprConfig.BaseAddress = ParseHexString16bit(jsonSprConfig["BaseAddress"].get<std::string>());
		sprConfig.Count = jsonSprConfig["Count"].get<int>();
		sprConfig.Width = jsonSprConfig["Width"].get<int>();
		sprConfig.Height = jsonSprConfig["Height"].get<int>();
	}

	for (const auto& cheatJson : jsonConfigFile["Cheats"])
	{
		FCheat cheat;
		cheat.Description = cheatJson["Description"].get<std::string>();

		for (const auto& cheatEntryJson : cheatJson["Entries"])
		{
			FCheatMemoryEntry entry;
			entry.Address = cheatEntryJson["Address"].get<int>();
			entry.Value = cheatEntryJson["Value"].get<int>();
			cheat.Entries.push_back(entry);
		}
		config.Cheats.push_back(cheat);
	}
	return true;
}

bool LoadPOKFile(FGameConfig &config, const char *fname)
{
	std::ifstream inFileStream(fname);
	if (inFileStream.is_open() == false)
		return false;

	return true;
}

// Labels

void SaveLabelsBin(const FCodeAnalysisState &state, FILE *fp, uint16_t startAddress, uint16_t endAddress)
{
	int recordCount = 0;
	for (int i = startAddress; i <= endAddress; i++)
	{
		if (state.GetLabelForAddress(i) != nullptr)
			recordCount++;
	}

	fwrite(&recordCount, sizeof(int), 1, fp);

	for (int i = startAddress; i <= endAddress; i++)
	{
		const FLabelInfo *pLabel = state.GetLabelForAddress(i);
		if (pLabel != nullptr)
		{
			WriteStringToFile(std::string(magic_enum::enum_name(pLabel->LabelType)), fp);
			fwrite(&pLabel->Address, sizeof(pLabel->Address), 1, fp);
			fwrite(&pLabel->ByteSize, sizeof(pLabel->ByteSize), 1, fp);
			WriteStringToFile(pLabel->Name, fp);
			WriteStringToFile(pLabel->Comment, fp);
			fwrite(&pLabel->Global, sizeof(bool), 1, fp);

			// References?
			int noRefences = (int)pLabel->References.size();
			fwrite(&noRefences, sizeof(int), 1, fp);
			for (const auto &ref : pLabel->References)
			{
				uint16_t refAddr = ref.first;
				fwrite(&refAddr, sizeof(refAddr), 1, fp);
			}
		}
	}
}

void LoadLabelsBin(FCodeAnalysisState &state, FILE *fp,int versionNo)
{
	int recordCount = 0;

	state.ResetLabelNames();

	fread(&recordCount, sizeof(int), 1, fp);

	for (int i = 0; i < recordCount; i++)
	{
		FLabelInfo *pLabel = new FLabelInfo;
		
		std::string enumVal;
		ReadStringFromFile(enumVal, fp);
		pLabel->LabelType = magic_enum::enum_cast<LabelType>(enumVal).value();
		fread(&pLabel->Address, sizeof(pLabel->Address), 1, fp);
		fread(&pLabel->ByteSize, sizeof(pLabel->ByteSize), 1, fp);
		ReadStringFromFile(pLabel->Name, fp);
		ReadStringFromFile(pLabel->Comment, fp);

		if (versionNo > 2)
			fread(&pLabel->Global, sizeof(bool), 1, fp);
		
		// References?
		if(versionNo > 1)
		{
			int noReferences;// = (int)pLabel->References.size();
			fread(&noReferences, sizeof(int), 1, fp);
			for (int i=0;i< noReferences;i++)
			{
				uint16_t refAddr;
				fread(&refAddr, sizeof(refAddr), 1, fp);
				pLabel->References[refAddr] = 1;
			}
		}

		state.SetLabelForAddress(pLabel->Address,pLabel);
	}
}

// Code Info

void SaveCodeInfoBin(const FCodeAnalysisState &state, FILE *fp, uint16_t startAddress, uint16_t endAddress)
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
		const FCodeInfo *pCodeInfo = state.GetCodeInfoForAddress(i);
		if (pCodeInfo != nullptr)
		{
			fwrite(&pCodeInfo->Flags, sizeof(pCodeInfo->Flags), 1, fp);
			fwrite(&pCodeInfo->Address, sizeof(pCodeInfo->Address), 1, fp);
			fwrite(&pCodeInfo->ByteSize, sizeof(pCodeInfo->ByteSize), 1, fp);
			fwrite(&pCodeInfo->JumpAddress, sizeof(pCodeInfo->JumpAddress), 1, fp);
			fwrite(&pCodeInfo->PointerAddress, sizeof(pCodeInfo->PointerAddress), 1, fp);
			WriteStringToFile(pCodeInfo->Text, fp);
			WriteStringToFile(pCodeInfo->Comment, fp);
		}
	}
}

void LoadCodeInfoBin(FCodeAnalysisState &state, FILE *fp, int versionNo)
{
	int recordCount;
	fread(&recordCount, sizeof(int), 1, fp);

	for (int i = 0; i < recordCount; i++)
	{
		FCodeInfo *pCodeInfo = FCodeInfo::Allocate();

		if(versionNo >= 4)
			fread(&pCodeInfo->Flags, sizeof(pCodeInfo->Flags), 1, fp);

		fread(&pCodeInfo->Address, sizeof(pCodeInfo->Address), 1, fp);
		fread(&pCodeInfo->ByteSize, sizeof(pCodeInfo->ByteSize), 1, fp);
		fread(&pCodeInfo->JumpAddress, sizeof(pCodeInfo->JumpAddress), 1, fp);
		fread(&pCodeInfo->PointerAddress, sizeof(pCodeInfo->PointerAddress), 1, fp);
		ReadStringFromFile(pCodeInfo->Text, fp);
		ReadStringFromFile(pCodeInfo->Comment, fp);
		for(int codeByte = 0;codeByte < pCodeInfo->ByteSize;codeByte++)	// set for whole instruction address range
			state.SetCodeInfoForAddress(pCodeInfo->Address + codeByte, pCodeInfo);
	}
}

// Data Info

void SaveDataInfoBin(const FCodeAnalysisState& state, FILE *fp, uint16_t startAddress, uint16_t endAddress)
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
		const FDataInfo *pDataInfo = state.GetReadDataInfoForAddress(i);
		if (pDataInfo != nullptr)
		{
			WriteStringToFile(std::string(magic_enum::enum_name(pDataInfo->DataType)), fp);
			fwrite(&pDataInfo->Address, sizeof(pDataInfo->Address), 1, fp);
			fwrite(&pDataInfo->ByteSize, sizeof(pDataInfo->ByteSize), 1, fp);
			WriteStringToFile(pDataInfo->Comment, fp);

			// Reads & Writes?
			int noReads = (int)pDataInfo->Reads.size();
			fwrite(&noReads, sizeof(int), 1, fp);
			for (const auto &ref : pDataInfo->Reads)
			{
				uint16_t refAddr = ref.first;
				fwrite(&refAddr, sizeof(refAddr), 1, fp);
			}

			int noWrites = (int)pDataInfo->Writes.size();
			fwrite(&noWrites, sizeof(int), 1, fp);
			for (const auto &ref : pDataInfo->Writes)
			{
				uint16_t refAddr = ref.first;
				fwrite(&refAddr, sizeof(refAddr), 1, fp);
			}
		}
	}
	
	
}

void LoadDataInfoBin(FCodeAnalysisState& state, FILE *fp, int versionNo)
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
		pDataInfo->Address = address;
		pDataInfo->DataType = magic_enum::enum_cast<DataType>(enumVal).value();
		fread(&pDataInfo->ByteSize, sizeof(pDataInfo->ByteSize), 1, fp);
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
				pDataInfo->Reads[dataAddr] = 1;
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
				pDataInfo->Writes[dataAddr] = 1;
			}
		}

		//state.SetReadDataInfoForAddress(pDataInfo->Address, pDataInfo);
		//state.SetWriteDataInfoForAddress(pDataInfo->Address, pDataInfo);
	}
}

static const int g_kBinaryFileVersionNo = 4;
static const int g_kBinaryFileMagic = 0xdeadface;

// Binary save
bool SaveGameDataBin(const FCodeAnalysisState& state, const char *fname, uint16_t addrStart, uint16_t addrEnd)
{
	FILE *fp = fopen(fname, "wb");
	if (fp == NULL)
		return false;

	fwrite(&g_kBinaryFileMagic, sizeof(int), 1, fp);
	fwrite(&g_kBinaryFileVersionNo, sizeof(int), 1, fp);
	for (int i = 0; i < 1 << 16; i++)
	{
		uint16_t addr = state.GetLastWriterForAddress(i);
		fwrite(&addr, sizeof(uint16_t), 1, fp);
	}
	//fwrite(&state.LastWriter, sizeof(uint16_t), 1 << 16, fp);	// write whole address range
	SaveLabelsBin(state, fp, addrStart, addrEnd);
	SaveCodeInfoBin(state, fp, addrStart, addrEnd);
	SaveDataInfoBin(state, fp, addrStart, addrEnd);

	fclose(fp);
	return true;
}

// Binary load
bool LoadGameDataBin(FCodeAnalysisState& state, const char *fname, uint16_t addrStart, uint16_t addrEnd)
{
	FILE *fp = fopen(fname, "rb");
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

	// loop across address range
	// clear what we're replacing
	/*/for (int i = addrStart; i <= addrEnd; i++)
	{
		FLabelInfo* pLabel = state.GetLabelForAddress(i);
		delete pLabel;
		state.SetLabelForAddress(i, nullptr);
	}*/

	if (versionNo >= 4)
	{
		for (int i = 0; i < (1 << 16); i++)
		{
			uint16_t lastWriter;
			fread(&lastWriter, sizeof(uint16_t), 1, fp);
			state.SetLastWriterForAddress(i, lastWriter);
		}
	}

	LoadLabelsBin(state, fp, versionNo);
	LoadCodeInfoBin(state, fp, versionNo);
	LoadDataInfoBin(state, fp, versionNo);

	fclose(fp);
	return true;
}

bool SaveGameData(const FCodeAnalysisState& state, const char *fname)
{
	return SaveGameDataBin(state, fname,0x4000, 0xffff);
}

bool SaveROMData(const FCodeAnalysisState& state, const char *fname)
{
	return SaveGameDataBin(state, fname, 0x0000, 0x3fff);
}

bool LoadGameData(FCodeAnalysisState& state, const char *fname)
{
	return LoadGameDataBin(state,fname,0x4000,0xffff);
}

bool LoadROMData(FCodeAnalysisState& state, const char *fname)
{
	return LoadGameDataBin(state, fname,0x0000,0x3fff);
}

bool LoadGameConfigs(FSpectrumEmu *pEmu)
{
	FDirFileList listing;

	if (EnumerateDirectory("Configs", listing) == false)
		return false;

	for (const auto &file : listing)
	{
		const std::string &fn = "configs/" + file.FileName;
		if ((fn.substr(fn.find_last_of(".") + 1) == "json"))
		{
			FGameConfig *pNewConfig = new FGameConfig;
			if (LoadGameConfigFromFile(*pNewConfig, fn.c_str()))
			{
				AddGameConfig(pNewConfig);
			}
			else
			{
				delete pNewConfig;
			}
		}
	}
	return true;
}