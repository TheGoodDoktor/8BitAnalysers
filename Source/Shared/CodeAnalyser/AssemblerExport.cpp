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
	DasmState.CodeAnalysisState = &pEmu->GetCodeAnalysis();
	DasmState.HexDisplayMode = HexMode;

	OldNumberMode = GetNumberDisplayMode();
	SetNumberDisplayMode(HexMode);
	bInitialised = true;
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
	return true;
}



void FASMExporter::Output(const char* pFormat, ...)
{
	va_list ap;
	va_start(ap, pFormat);
	//vfprintf(FilePtr, pFormat, ap);

	if (OutputString != nullptr)
	{
		const int kStringBufferSize = 256;
		char stringBuffer[kStringBufferSize];
		int ret = vsnprintf(stringBuffer, kStringBufferSize, pFormat, ap);
		assert(ret < kStringBufferSize);	// increase kStrignBufferSize if this gets hit
		*OutputString += stringBuffer;
	}
	va_end(ap);
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

uint16_t g_DbgAddress = 0xEA71;

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
		snprintf(valTxt, 16, "%s%c", NumStr(val, GetNumberDisplayModeForDataItem(pDataInfo)), i < pDataInfo->ByteSize - 1 ? ',' : ' ');
		textString += valTxt;

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

		const FLabelInfo* pLabel = bOperandIsAddress ? state.GetLabelForPhysicalAddress(val) : nullptr;
		if (pLabel != nullptr)
		{
			Output("%s %s", Config.DataWordPrefix, pLabel->GetName());
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

bool FASMExporter::ExportAddressRange(uint16_t startAddr , uint16_t endAddr)
{
	FCodeAnalysisState& state = pEmulator->GetCodeAnalysis();

	DasmState.ExportMin = startAddr;
	DasmState.ExportMax = endAddr;
	
	// place an 'org' at the start
	SetOutputToBody();

	Output("%s %s\n", Config.ORGText, NumStr(startAddr));

	for (const FCodeAnalysisItem &item : state.ItemList)
	{
		const uint16_t addr = item.AddressRef.Address;

		if (addr < startAddr)
			continue;

		if (addr > endAddr)
			break;

		switch (item.Item->Type)
		{
		case EItemType::Label:
		{
			const FLabelInfo* pLabelInfo = static_cast<FLabelInfo*>(item.Item);
			if(IsLabelStubbed(pLabelInfo->GetName()))
				Output("%s_Stubbed:", pLabelInfo->GetName());
			else
				Output("%s:", pLabelInfo->GetName());
		}
		break;
		case EItemType::Code:
		{
			const FCodeInfo* pCodeInfo = static_cast<FCodeInfo*>(item.Item);

			WriteCodeInfoForAddress(state, addr);	// needed to refresh code info
			if (addr == g_DbgAddress)
				LOGINFO("DebugAddress");

			DasmState.CurrentAddress = addr;
			DasmState.pCodeInfoItem = pCodeInfo;
			DasmState.Text.clear();

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
		if (item.Item->Type != EItemType::CommentLine && item.Item->Comment.empty() == false)
		{
			const std::string expString = Markup::ExpandString(state, item.Item->Comment.c_str());
			Output("\t\t\t; %s", expString.c_str());
		}
		Output("\n");
	}

	ProcessLabelsOutsideExportedRange();
	
	
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
		
	const bool bSuccess = pExporter->ExportAddressRange(startAddr,endAddr);

	pExporter->Finish();
	return bSuccess;
}

// Util functions

// There's probably a better way of doing this
void AppendCharToString(char ch, std::string& outString)
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