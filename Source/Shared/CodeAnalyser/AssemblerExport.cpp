#include "CodeAnalyser/CodeAnalyser.h"
#include "Util/Misc.h"
#include "Misc/EmuBase.h"
#include <util/z80dasm.h>
#include "Debug/DebugLog.h"

#include <string.h>
#include <stdarg.h>

#include <CodeAnalyser/Z80/Z80Disassembler.h>
#include "UI/CodeAnalyserUI.h"
#include "Disassembler.h"

struct FAssemblerConfig
{
	const char* DataBytePrefix = nullptr;
	const char* DataWordPrefix = nullptr;
	const char* DataTextPrefix = nullptr;
	const char*	ORGText = nullptr;
};

FAssemblerConfig g_DefaultAsmConfig = {
	"db",
	"dw",
	"ascii",
	"org",
};

FAssemblerConfig g_SpasmAsmConfig = {
	".db",
	".dw",
	".text",
	".org",
};

static const std::map<std::string, const FAssemblerConfig*> g_Configs = 
{
	{"Spasm",	&g_SpasmAsmConfig}
};

const FAssemblerConfig* GetAssemblerConfig(const char* pConfigName)
{
	auto findIt = g_Configs.find(pConfigName);
	if(findIt == g_Configs.end())
		return &g_DefaultAsmConfig;
	return findIt->second;
}

// Class to encapsulate ASM exporting
class FASMExporter
{
	public:
		FASMExporter(const char *pFilename, FCodeAnalysisState* pState);
		~FASMExporter();

		bool		Init();
		void		Output(const char* pFormat, ...);
		bool		ExportAddressRange(uint16_t startAddr, uint16_t endAddr);

		std::string		GenerateAddressLabelString(FAddressRef addr);
		void			ExportDataInfoASM(FAddressRef addr);

	private:
		void				OutputDataItemBytes(FAddressRef addr, const FDataInfo* pDataInfo);
		ENumberDisplayMode	GetNumberDisplayModeForDataItem(const FDataInfo* pDataInfo);

		bool			bInitialised = false;
		ENumberDisplayMode HexMode = ENumberDisplayMode::HexDollar;
		ENumberDisplayMode OldNumberMode;

		std::string		Filename;
		FILE*			FilePtr = nullptr;
		FCodeAnalysisState*	pCodeAnalyser = nullptr;

		const FAssemblerConfig*	pAssemblerConfig;
};

FASMExporter::FASMExporter(const char* pFilename, FCodeAnalysisState* pState) 
	: Filename(pFilename)
	, pCodeAnalyser(pState) 
{
	pAssemblerConfig = GetAssemblerConfig(pState->GetEmulator()->GetGlobalConfig()->ExportAssembler.c_str());
}

FASMExporter::~FASMExporter()
{
	if(FilePtr != nullptr)
		fclose(FilePtr);

	SetNumberDisplayMode(OldNumberMode);
}

bool FASMExporter::Init()
{
	FilePtr = fopen(Filename.c_str(), "wt");

	if (FilePtr == nullptr)
		return false;


	OldNumberMode = GetNumberDisplayMode();
	SetNumberDisplayMode(HexMode);
	bInitialised = true;
	return true;
}


void FASMExporter::Output(const char* pFormat, ...)
{
	va_list ap;
	va_start(ap, pFormat);
	vfprintf(FilePtr, pFormat, ap);
	va_end(ap);
}


// this might be a bit broken
std::string FASMExporter::GenerateAddressLabelString(FAddressRef addr)
{
	int labelOffset = 0;
	const char* pLabelString = nullptr;
	std::string labelStr;

	for (int addrVal = addr.Address; addrVal >= 0; addrVal--)
	{
		FLabelInfo* pLabelInfo = pCodeAnalyser->GetLabelForPhysicalAddress(addrVal);
		if (pLabelInfo != nullptr)
		{
			labelStr = "[" + std::string(pLabelInfo->GetName());
			break;
		}

		labelOffset++;
	}

	if (labelStr.empty() == false)
	{
		if (labelOffset > 0)	// add offset string
		{
            const int kOffsetStringSize = 16;
			char offsetString[kOffsetStringSize];
			snprintf(offsetString,kOffsetStringSize, " + %d]", labelOffset);
			labelStr += offsetString;
		}
		else
		{
			labelStr += "]";
		}
	}

	return labelStr;
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
	FCodeAnalysisState& state = *pCodeAnalyser;
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
	Output("%s %s", pAssemblerConfig->DataBytePrefix, textString.c_str());
}

void FASMExporter::ExportDataInfoASM(FAddressRef addr)
{
	const FDataInfo* pDataInfo = pCodeAnalyser->GetDataInfoForAddress(addr);
	FCodeAnalysisState& state = *pCodeAnalyser;

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
	/* {
		const uint8_t val = state.ReadByte(addr);
		Output("%s %s", pAssemblerConfig->DataBytePrefix, NumStr(val, dispMode));
	}
	break;*/
	case EDataType::ByteArray:
	{
		OutputDataItemBytes(addr,pDataInfo);
	/*
		std::string textString;
		FAddressRef byteAddress = addr;
		for (int i = 0; i < pDataInfo->ByteSize; i++)
		{
			const uint8_t val = state.ReadByte(byteAddress);
			char valTxt[16];
			snprintf(valTxt, 16, "%s%c", NumStr(val, dispMode), i < pDataInfo->ByteSize - 1 ? ',' : ' ');
			textString += valTxt;

			state.AdvanceAddressRef(byteAddress,1);
		}
		Output("%s %s", pAssemblerConfig->DataBytePrefix, textString.c_str());*/
	}
	break;
	case EDataType::Word:
	{
		const uint16_t val = state.ReadWord(addr);

		const FLabelInfo* pLabel = bOperandIsAddress ? state.GetLabelForPhysicalAddress(val) : nullptr;
		if (pLabel != nullptr)
		{
			Output("%s %s", pAssemblerConfig->DataWordPrefix, pLabel->GetName());
		}
		else
		{
			Output("%s %s", pAssemblerConfig->DataWordPrefix, NumStr(val, dispMode));
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
		std::string textString;
		FAddressRef charAddress = addr;
		for (int i = 0; i < pDataInfo->ByteSize; i++)
		{
			const char ch = state.ReadByte(charAddress);
			AppendCharToString(ch,textString);
			state.AdvanceAddressRef(charAddress,1);
		}
		Output("%s '%s'", pAssemblerConfig->DataTextPrefix, textString.c_str());
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
	FCodeAnalysisState& state = *pCodeAnalyser;
	// TODO: write screen memory regions

	// place an 'org' at the start
	Output("%s %s\n",pAssemblerConfig->ORGText, NumStr(startAddr));

	for (const FCodeAnalysisItem &item : pCodeAnalyser->ItemList)
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
			Output("%s:", pLabelInfo->GetName());
		}
		break;
		case EItemType::Code:
		{
			const FCodeInfo* pCodeInfo = static_cast<FCodeInfo*>(item.Item);

			WriteCodeInfoForAddress(state, addr);	// needed to refresh code info
			if (addr == g_DbgAddress)
				LOGINFO("DebugAddress");

			const std::string dasmString = GenerateDasmStringForAddress(state, addr, HexMode);

			Markup::SetCodeInfo(pCodeInfo);
			const std::string expString = Markup::ExpandString(dasmString.c_str());
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
			//const FDataInfo* pDataInfo = static_cast<FDataInfo*>(item.Item);
			ExportDataInfoASM(item.AddressRef);
		}
		break;
		case EItemType::CommentLine:
		{
			Output("; %s", item.Item->Comment.c_str());
		}
		break;
        default:
			LOGINFO("ASM export - unhandled type:%",(int)item.Item->Type);
        break;
		}

		// put comment on the end - not for comment lines
		if (item.Item->Type != EItemType::CommentLine && item.Item->Comment.empty() == false)
			Output("\t\t\t; %s", item.Item->Comment.c_str());
		Output("\n");
	}
	
	return true;
}


bool ExportAssembler(FCodeAnalysisState& state, const char* pTextFileName, uint16_t startAddr, uint16_t endAddr)
{
	FASMExporter exporter(pTextFileName,&state);
	if(exporter.Init() == false)
		return false;
		
	if(exporter.ExportAddressRange(startAddr,endAddr) == false)
		return false;

	return true;
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