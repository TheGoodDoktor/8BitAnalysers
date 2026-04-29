#include "AssemblerExport.h"

#include "CodeAnalyser/CodeAnalyser.h"
#include "Util/Misc.h"
#include "Misc/EmuBase.h"
#include <util/z80dasm.h>
#include "Debug/DebugLog.h"
#include "Misc/GameConfig.h"

#include <string.h>
#include <stdarg.h>

#include <CodeAnalyser/Z80/Z80Disassembler.h>
#include "UI/CodeAnalyserUI.h"
#include "Disassembler.h"


FAssemblerConfig g_DefaultConfig = {
	"db",
	"dw",
	"ascii",
	"org",
};

static std::map<std::string, FASMExporter*> g_AssemblerExporters;

const std::map<std::string, FASMExporter*>& GetAssemblerExporters()
{
	return g_AssemblerExporters;
}

bool AddAssemblerExporter(const char* pName, FASMExporter* pExporter)
{
	auto res = g_AssemblerExporters.insert({pName,pExporter });

	return res.second;
}

FASMExporter* GetAssemblerExporter(const char* pConfigName)
{
	auto findIt = g_AssemblerExporters.find(pConfigName);
	if(findIt == g_AssemblerExporters.end())
		return nullptr;
	return findIt->second;
}


bool FASMExporter::Init(const char* pFilename, FEmuBase* pEmu)
{
	pEmulator = pEmu;
	FilePtr = fopen(pFilename, "wt");

	if (FilePtr == nullptr)
		return false;

	HeaderText.clear();
	BodyText.clear();
	ExportRanges.clear();
	DasmState.CodeAnalysisState = &pEmu->GetCodeAnalysis();
	DasmState.HexDisplayMode = HexMode;
	DasmState.LabelsOutsideRange.clear();
	DasmState.NumRawValuesOutput = 0;

	OldNumberMode = GetNumberDisplayMode();
	SetNumberDisplayMode(HexMode);
	bInitialised = true;

	ExportDidBegin();

	return true;
}

bool FASMExporter::Finish()
{
	if (FilePtr != nullptr)
	{
		fwrite(HeaderText.c_str(), HeaderText.size(), 1, FilePtr);
		fwrite(BodyText.c_str(), BodyText.size(),1,FilePtr);
		fclose(FilePtr);
	}

	SetNumberDisplayMode(OldNumberMode);
	ExportDidEnd();

	return true;
}



void FASMExporter::Output(const char* pFormat, ...)
{
	va_list ap;
	va_start(ap, pFormat);
	//vfprintf(FilePtr, pFormat, ap);

	if (OutputString != nullptr)
	{
		const int kStringBufferSize = 1024;
		char stringBuffer[kStringBufferSize];
		int ret = vsnprintf(stringBuffer, kStringBufferSize, pFormat, ap);
		assert(ret < kStringBufferSize);	// increase kStrignBufferSize if this gets hit
		*OutputString += stringBuffer;
	}
	va_end(ap);
}

void FASMExporter::AddBankSection(const FCodeAnalysisBank* pBank)
{
	SetOutputToBody();

	Output("\n;--------------------------------------------------------------------------------\n");
	Output("; Bank name: %s\n", pBank->Name.c_str());
	if (!pBank->Description.empty())
		Output("; Bank description: %s\n", pBank->Description.c_str());
	Output(";--------------------------------------------------------------------------------\n");
}

bool FASMExporter::IsLabelStubbed(const char* pLabelName) const
{
	const FProjectConfig* pConfig = pEmulator->GetProjectConfig();
	const std::string labelName(pLabelName);
	for (const auto& stub : pConfig->StubOutFunctions)
	{
		if(stub == labelName)
			return true;
	}

	return false;
}

int16_t g_DbgBank = 76;
uint16_t g_DbgAddress = 0xe62f;

void AppendCharToString(char ch, std::string& outString);

ENumberDisplayMode FASMExporter::GetNumberDisplayModeForDataItem(const FDataInfo* pDataInfo)
{
	ENumberDisplayMode dispMode = GetNumberDisplayMode();

	if (pDataInfo->DisplayType == EDataItemDisplayType::Decimal)
		dispMode = ENumberDisplayMode::Decimal;
	if (pDataInfo->DisplayType == EDataItemDisplayType::Hex)
		dispMode = HexMode;
	if (pDataInfo->DisplayType == EDataItemDisplayType::Binary)
		dispMode = ENumberDisplayMode::Binary;
	if (pDataInfo->DisplayType == EDataItemDisplayType::Ascii)
		dispMode = ENumberDisplayMode::Ascii;

	return dispMode;
}

void FASMExporter::OutputDataItemBytes(FAddressRef addr, const FDataInfo* pDataInfo)
{
	FCodeAnalysisState& state = pEmulator->GetCodeAnalysis();
	std::string textString;
	FAddressRef byteAddress = addr;
	for (int i = 0; i < pDataInfo->ByteSize; i++)
	{
		const uint8_t val = state.ReadByte(byteAddress);
		char valTxt[16];

		ENumberDisplayMode dispMode = GetNumberDisplayModeForDataItem(pDataInfo);
		if (dispMode == ENumberDisplayMode::Ascii)
		{
			if(val < 0x20 || val > 0x7E)
				dispMode = HexMode;
		}
		
		snprintf(valTxt, 16, "%s", NumStr(val, dispMode));

		textString += valTxt;
		if (i < pDataInfo->ByteSize - 1)
			textString += ',';

		state.AdvanceAddressRef(byteAddress, 1);
	}
	Output("%s %s", Config.DataBytePrefix, textString.c_str());
}

void FASMExporter::ExportDataInfoASM(FAddressRef addr)
{
	FCodeAnalysisState& state = pEmulator->GetCodeAnalysis();
	const FDataInfo* pDataInfo = state.GetDataInfoForAddress(addr);

	const ENumberDisplayMode dispMode = GetNumberDisplayModeForDataItem(pDataInfo);

	const bool bOperandIsAddress = (pDataInfo->DisplayType == EDataItemDisplayType::JumpAddress || pDataInfo->DisplayType == EDataItemDisplayType::Pointer);

	EDataType outputDataType;

	// TODO: redirect other types (char maps, bitmaps etc.) to byte arrays etc.

	switch (pDataInfo->DataType)
	{
	case EDataType::CharacterMap:
	case EDataType::Bitmap:
	case EDataType::ColAttr:
			outputDataType = EDataType::ByteArray;
			break;
		default:
			outputDataType = pDataInfo->DataType;
	}

	Output("\t");
	switch (outputDataType)
	{
	case EDataType::Byte:
	case EDataType::ByteArray:
		OutputDataItemBytes(addr,pDataInfo);
	break;
	case EDataType::Word:
	{
		const uint16_t val = state.ReadWord(addr);

		const FLabelInfo* pLabel = (bOperandIsAddress && val != 0) ? state.GetLabelForAddress(addr) : nullptr;

		if (pLabel != nullptr)
		{
			// todo: replace this physical address call
			const FLabelInfo* pScopeLabel = pLabel->Global == false ? state.GetScopeLabelForPhysicalAddress(val) : nullptr;
			const FLabelInfo* pCurrentScope = state.GetScopeForAddress(addr);

			if (pScopeLabel != nullptr && pScopeLabel != pCurrentScope)
			{
				std::string scopeLabelName = pScopeLabel->GetName();
				Output("%s %s.%s", Config.DataWordPrefix, pScopeLabel->GetName(),pLabel->GetName());
			}
			else
			{
				Output("%s %s", Config.DataWordPrefix, pLabel->GetName());
			}
		}
		else
		{
			Output("%s %s", Config.DataWordPrefix, NumStr(val, dispMode));
		}
	}
	break;
	case EDataType::WordArray:
	{
		const int wordSize = pDataInfo->ByteSize / 2;
		std::string textString;
		FAddressRef wordAddr = addr;
		for (int i = 0; i < wordSize; i++)
		{
			const uint16_t val = state.ReadWord(wordAddr);
			char valTxt[16];
			snprintf(valTxt, 16, "%s%c", NumStr(val), i < wordSize - 1 ? ',' : ' ');
			textString += valTxt;
			state.AdvanceAddressRef(wordAddr,2);
		}
		Output("dw %s", textString.c_str());
	}
	break;
	case EDataType::Text:
	{
		//OutputDataItemBytes(addr,pDataInfo);
		// This old text export doesn't really work
		
		std::string textString;
		FAddressRef charAddress = addr;
		for (int i = 0; i < pDataInfo->ByteSize; i++)
		{
			const char ch = state.ReadByte(charAddress);

			// handle chars that need to be escaped
			if(ch == '\"')
				AppendCharToString('\\', textString);

			AppendCharToString(ch,textString);
			state.AdvanceAddressRef(charAddress,1);
		}
		Output("%s \"%s\"", Config.DataTextPrefix, textString.c_str());
		
	}
	break;

	case EDataType::ScreenPixels:
	case EDataType::Blob:
	default:
		OutputDataItemBytes(addr, pDataInfo);
		break;
	}
}

bool FASMExporter::TryExportByteRun(FCodeAnalysisState& state, const std::vector<FCodeAnalysisItem>& itemList, int& itemIdx, uint16_t endAddr, uint16_t& nextAddr)
{
	const FCodeAnalysisItem& item = itemList[itemIdx];
	if (item.Item->Type != EItemType::Data || !item.Item->Comment.empty())
		return false;

	const FDataInfo* pDataInfo = state.GetDataInfoForAddress(item.AddressRef);
	if (!pDataInfo || pDataInfo->DataType != EDataType::Byte || pDataInfo->ByteSize != 1 || pDataInfo->DisplayType != EDataItemDisplayType::Unknown)
		return false;

	Output("\t%s ", Config.DataBytePrefix);
	int batchCount = 0;
	
	static constexpr int kMaxBytesPerLine = 16;

	while (itemIdx < (int)itemList.size() && batchCount < kMaxBytesPerLine)
	{
		const FCodeAnalysisItem& item = itemList[itemIdx];
		const uint16_t addr = item.AddressRef.GetAddress();

		if (addr > endAddr) break;
		if (item.Item->Type != EItemType::Data) break;
		if (!item.Item->Comment.empty()) break;

		const FDataInfo* pData = state.GetDataInfoForAddress(item.AddressRef);
		if (!pData || pData->DataType != EDataType::Byte || pData->ByteSize != 1)
			break;

		if (batchCount > 0) Output(",");
		Output("%s", NumStr(state.ReadByte(item.AddressRef), GetNumberDisplayModeForDataItem(pData)));

		nextAddr = addr + 1;
		batchCount++;
		itemIdx++;

		// Stop before a label so the label appears on its own line
		if (itemIdx < (int)itemList.size() && itemList[itemIdx].Item->Type == EItemType::Label)
			break;
	}

	Output("\n");
	itemIdx--; // counteract the for-loop increment in the caller
	return true;
}

bool FASMExporter::ExportAddressRange(const std::vector<FCodeAnalysisItem>& itemList, uint16_t startAddr , uint16_t endAddr, bool bIsPhysicalMem)
{
	if (itemList.empty())
	{
		LOGWARNING("ASM Exporter: Cannot export address range. Item list is empty.");
		return false;
	}

	FCodeAnalysisState& state = pEmulator->GetCodeAnalysis();

	DasmState.ExportMin = startAddr;
	DasmState.ExportMax = endAddr;
	DasmState.pExporter = this;

	// todo: extend the existing range instead of having multiple ranges?
	ExportRanges.insert({ startAddr, endAddr });

	// place an 'org' at the start
	SetOutputToBody();

	Output("%s %s\n", Config.ORGText, NumStr(startAddr));

	uint16_t nextAddr = itemList[0].AddressRef.GetAddress();

	for (int itemIdx = 0; itemIdx < (int)itemList.size(); itemIdx++)
	{
		const FCodeAnalysisItem& item = itemList[itemIdx];
		const uint16_t addr = item.AddressRef.GetAddress();

		if (addr < startAddr)
			continue;

		if (addr > endAddr)
			break;

		// Skip labels whose address falls within the byte range of the preceding instruction.
		// They cannot be placed correctly in the output and the raw value is already used in the operand.
		if (item.Item->Type == EItemType::Label && addr < nextAddr)
			continue;

		if (addr != nextAddr)
		{
			const FCodeAnalysisBank* pBank = state.GetBank(item.AddressRef.GetBankId());
			LOGERROR("'%s': 0x%04x. Asm Export - Overlap. Expecting 0x%04X", pBank->Name.c_str(), item.AddressRef.GetAddress(), nextAddr);
		}

		//Output("; 0x%04X\n", addr);
		nextAddr = addr + item.Item->ByteSize;

		if (TryExportByteRun(state, itemList, itemIdx, endAddr, nextAddr))
			continue;

		switch (item.Item->Type)
		{
		case EItemType::Label:
		{
			const FLabelInfo* pLabelInfo = static_cast<FLabelInfo*>(item.Item);
			if(pLabelInfo->Global == false)
				Output("%s",Config.LocalLabelPrefix);

			if(IsLabelStubbed(pLabelInfo->GetName()))
				Output("%s_Stubbed:", pLabelInfo->GetName());
			else
				Output("%s:", pLabelInfo->GetName());
		}
		break;
		case EItemType::Code:
		{
			const FCodeInfo* pCodeInfo = static_cast<FCodeInfo*>(item.Item);

			if (bIsPhysicalMem)
			{
				// Sam. This breaks if we call it on a bank that is not in physical memory.
				// It will set the wrong bank's memory as code.
				// I looked at rewriting it using address refs but came to the conclusion
				// it is not possible.
				WriteCodeInfoForAddress(state, addr);	// needed to refresh code info
			}

			//if (item.AddressRef.GetBankId() == 0 && addr == 0xf9b2)
			if (addr == g_DbgAddress)
				LOGINFO("DebugAddress");

			DasmState.CurrentAddress = item.AddressRef;
			DasmState.pCodeInfoItem = pCodeInfo;
			DasmState.Text.clear();
			DasmState.pCurrentScope = state.GetScopeForAddress(item.AddressRef);

			GenerateDasmExportString(DasmState);
			//const std::string dasmString = GenerateDasmStringForAddress(state, addr, HexMode);

			Markup::SetCodeInfo(pCodeInfo);
			const std::string expString = Markup::ExpandString(state,DasmState.Text.c_str());
			Output("\t%s", expString.c_str());

			/*if (pCodeInfo->OperandAddress.IsValid())
			{
				const std::string labelStr = GenerateAddressLabelString(pCodeInfo->OperandAddress);
				if (labelStr.empty() == false)
					fprintf(fp, "\t;%s", labelStr.c_str());

			}*/
		}

		break;
		case EItemType::Data:
		{
			ExportDataInfoASM(item.AddressRef);
		}
		break;
		case EItemType::CommentLine:
		case EItemType::FunctionDescLine:
		{
			const std::string expString = Markup::ExpandString(state, item.Item->Comment.c_str());

			Output("; %s", expString.c_str());
		}
		break;
        default:
			LOGINFO("ASM export - unhandled type:%",(int)item.Item->Type);
        break;
		}

		// put comment on the end - not for comment lines
		if (item.Item->Type != EItemType::CommentLine && item.Item->Type != EItemType::FunctionDescLine && item.Item->Comment.empty() == false)
		{
			const std::string expString = Markup::ExpandString(state, item.Item->Comment.c_str());
			Output("\t\t\t; %s", expString.c_str());
		}
		Output("\n");
	}

	return true;
}


bool ExportAssembler(FEmuBase* pEmu, const char* pTextFileName, uint16_t startAddr, uint16_t endAddr)
{
	FCodeAnalysisState& state = pEmu->GetCodeAnalysis();
	FASMExporter* pExporter = GetAssemblerExporter(state.pGlobalConfig->ExportAssembler.c_str());
	if(pExporter == nullptr)
		return false;

	if(pExporter->Init(pTextFileName, pEmu) == false)
		return false;

	pExporter->SetOutputToHeader();
	pExporter->AddHeader();
		
	const bool bSuccess = pExporter->ExportAddressRange(state.ItemList, startAddr,endAddr, true);

	pExporter->ProcessLabelsOutsideExportedRange();
	pExporter->Finish();
	return bSuccess;
}

bool ExportAssemblerForBanks(class FEmuBase* pEmu, const char* pTextFileName, const std::vector<int16_t>& bankList)
{
	FCodeAnalysisState& state = pEmu->GetCodeAnalysis();
	FASMExporter* pExporter = GetAssemblerExporter(state.pGlobalConfig->ExportAssembler.c_str());
	if (pExporter == nullptr)
	{
		LOGWARNING("Failed to export '%s': No exporter is set.", pTextFileName);
		return false;
	}

	if (pExporter->Init(pTextFileName, pEmu) == false)
		return false;

	pExporter->SetOutputToHeader();
	pExporter->AddHeader();

	// Build bank list
	pExporter->ExportBanks.clear();
	if (bankList.empty())
	{
		// Add all banks
		for (int b = 0; b < FCodeAnalysisState::BankCount; b++)
		{
			FCodeAnalysisBank& bank = state.GetBanks()[b];
			pExporter->ExportBanks.emplace_back(&bank);
		}
	}
	else
	{
		for (int16_t bankId : bankList)
		{
			if (const FCodeAnalysisBank* pBank = state.GetBank(bankId))
			{
				pExporter->ExportBanks.emplace_back(pBank);
			}
		}
	}

	for (const FCodeAnalysisBank* pBank : pExporter->ExportBanks)
	{
		if (pBank->PrimaryMappedPage != -1 && !pBank->bMachineROM/* && pBank->bEverBeenMapped*/)
		{
			pExporter->AddBankSection(pBank);
			
			const uint16_t mappedAddr = pBank->GetMappedAddress();
			const uint16_t endAddr = mappedAddr + pBank->GetSizeBytes() - 1;

			uint16_t firstItemAddr = mappedAddr;
			if (!pBank->ItemList.empty())
			{
				firstItemAddr = pBank->ItemList.front().AddressRef.GetAddress();
				if (firstItemAddr != mappedAddr)
					LOGINFO("First item starts at 0x%x even though mapped address is 0x%x", firstItemAddr, mappedAddr);
			}

			LOGINFO("Exporting bank %03d '%s' [0x%04x - 0x%04x]", pBank->Id, pBank->Name.c_str(), firstItemAddr, endAddr);
			pExporter->ExportAddressRange(pBank->ItemList, firstItemAddr, endAddr, false);
		}
	}
	
	pExporter->ProcessLabelsOutsideExportedRange();

	pExporter->Finish();
	return true;
}

// Util functions

// There's probably a better way of doing this
void AppendCharToString(char ch, std::string& outString)
{
	bool bOutputEscapeCodes = false;

	if(bOutputEscapeCodes)
	{
		switch (ch)
		{
		case '\'':
			outString+= "\\'";
			break;

		case '\"':
			outString += "\\\"";
			break;

		case '\?':
			outString += "\\?";
			break;

		case '\\':
			outString += "\\\\";
			break;

		case '\a':
			outString += "\\a";
			break;

		case '\b':
			outString += "\\b";
			break;

		case '\f':
			outString += "\\f";
			break;

		case '\n':
			outString += "\\n";
			break;

		case '\r':
			outString += "\\r";
			break;

		case '\t':
			outString += "\\t";
			break;

		case '\v':
			outString += "\\v";
			break;

		default:
			outString += ch;
		}
	}
	else
	{
		outString += ch;
	}
}