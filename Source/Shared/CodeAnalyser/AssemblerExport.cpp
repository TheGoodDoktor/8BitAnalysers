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
#include "FunctionAnalyser.h"


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
		const int kStringBufferSize = 1024;
		char stringBuffer[kStringBufferSize];
		int ret = vsnprintf(stringBuffer, kStringBufferSize, pFormat, ap);
		assert(ret < kStringBufferSize);	// increase kStrignBufferSize if this gets hit
		*OutputString += stringBuffer;
	}
	va_end(ap);
}

bool FASMExporter::IsLabelStubbed(const char* pLabelName) const
{
#if 0
	const FProjectConfig* pConfig = pEmulator->GetProjectConfig();
	const std::string labelName(pLabelName);
	for (const auto& stub : pConfig->StubOutFunctions)
	{
		if(stub == labelName)
			return true;
	} 
#endif
	FFunctionInfo* pFunc = pEmulator->GetCodeAnalysis().pFunctions->FindFunctionByName(pLabelName);
	if (pFunc != nullptr && pFunc->bStubbedOut)
		return true;

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

		const FLabelInfo* pLabel = (bOperandIsAddress && val != 0) ? state.GetLabelForPhysicalAddress(val) : nullptr;

		if (pLabel != nullptr)
		{
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

void FASMExporter::OutputFunctionDescription(const FFunctionInfo* pFunctionInfo)
{
	assert(pFunctionInfo != nullptr);
	const FLabelInfo* pLabelInfo = pEmulator->GetCodeAnalysis().GetLabelForAddress(pFunctionInfo->StartAddress);
	assert(pLabelInfo != nullptr);

	Output("; Function: %s\n", pLabelInfo->GetName());
	if (pFunctionInfo->Description.empty() == false)
	{
		Output("; Description: %s\n", pFunctionInfo->Description.c_str());
	}
	if (pFunctionInfo->Params.size() > 0)
	{
		Output("; Parameters:\n");
		for (const FFunctionParam& param : pFunctionInfo->Params)
		{
			Output(";   %s\n", param.GenerateDescription(pEmulator->GetCodeAnalysis()).c_str());
		}
	}
	if (pFunctionInfo->ReturnValues.size() > 0)
	{
		Output("; Returns:\n");
		for (const FFunctionParam& retVal : pFunctionInfo->ReturnValues)
		{
			Output(";   %s\n", retVal.GenerateDescription(pEmulator->GetCodeAnalysis()).c_str());
		}
	}
}

bool FASMExporter::ExportAddressRange(uint16_t startAddr , uint16_t endAddr)
{
	FCodeAnalysisState& state = pEmulator->GetCodeAnalysis();

	DasmState.ExportMin = startAddr;
	DasmState.ExportMax = endAddr;
	DasmState.pExporter = this;

	// place an 'org' at the start
	SetOutputToBody();

	Output("%s %s\n", Config.ORGText, NumStr(startAddr));

	// TODO: Set to start Addr?
	uint16_t nextAddr = state.ItemList[0].AddressRef.Address;

	for (const FCodeAnalysisItem &item : state.ItemList)
	{
		const uint16_t addr = item.AddressRef.Address;

		if (addr < startAddr)
			continue;

		if (addr > endAddr)
			break;

		if (addr != nextAddr)
		{
			LOGERROR("Asm Export - Overlap. Addr = 0x%04X, Expecting  0x%04X",addr,nextAddr);
		}

		//Output("; 0x%04X\n", addr);
		nextAddr = addr + item.Item->ByteSize;

		switch (item.Item->Type)
		{
		case EItemType::Label:
		{
			// Look for function info
			/*const FFunctionInfo* pFunc = state.pFunctions->GetFunctionAtAddress(item.AddressRef);
			if (pFunc != nullptr)
			{
				OutputFunctionDescription(pFunc);
			}*/
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

			WriteCodeInfoForAddress(state, addr);	// needed to refresh code info
			if (addr == g_DbgAddress)
				LOGINFO("DebugAddress");

			DasmState.CurrentAddress = addr;
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

bool ExportFunctionStubs(FEmuBase* pEmu, const char* pTextFileName)
{
	FCodeAnalysisState& state = pEmu->GetCodeAnalysis();
	FASMExporter* pExporter = GetAssemblerExporter(state.pGlobalConfig->ExportAssembler.c_str());
	if (pExporter == nullptr)
		return false;
	if (pExporter->Init(pTextFileName, pEmu) == false)
		return false;
	pExporter->SetOutputToHeader();
	pExporter->Output("; Function stubs\n");
	pExporter->Output("; Functions marked as 'Stubbed out' in the function analyser\n");
	pExporter->Output("; To implement the stub function, move to another file and mark function as 'Stub Implemented'\n");
	pExporter->Output("; This file is machine generated and should NOT be edited.\n\n");
	pExporter->AddHeader();
	pExporter->SetOutputToBody();

	// Export stubbed functions
	for (const auto& funcIt : state.pFunctions->GetFunctions())
	{
		const FFunctionInfo& func = funcIt.second;
		if (func.bStubbedOut && func.bStubImplemented == false)
		{
			const FLabelInfo* pLabelInfo = state.GetLabelForAddress(func.StartAddress);
			assert(pLabelInfo != nullptr);
			pExporter->Output("\n");
			pExporter->OutputFunctionDescription(&func);
			pExporter->Output("%s:\n", pLabelInfo->GetName());
			pExporter->Output("\tJP %s_Stubbed ; Call original function\n", pLabelInfo->GetName());
			//pExporter->Output("\t%s 0\n\n", pExporter->GetConfig().EQUText); // equ 0
		}
	}
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