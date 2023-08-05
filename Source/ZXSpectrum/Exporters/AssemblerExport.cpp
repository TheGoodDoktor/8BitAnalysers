#include "CodeAnalyser/CodeAnalyser.h"
#include "../SpectrumConstants.h"
#include "Util/Misc.h"
#include <util/z80dasm.h>
#include "Debug/DebugLog.h"

#include <string.h>
#include <CodeAnalyser/Z80/Z80Disassembler.h>

// this might be a bit broken
std::string GenerateAddressLabelString(FCodeAnalysisState& state, FAddressRef addr)
{
	int labelOffset = 0;
	const char* pLabelString = nullptr;
	std::string labelStr;

	for (int addrVal = addr.Address; addrVal >= 0; addrVal--)
	{
		FLabelInfo* pLabelInfo = state.GetLabelForPhysicalAddress(addrVal);
		if (pLabelInfo != nullptr)
		{
			labelStr = "[" + pLabelInfo->Name;
			break;
		}

		labelOffset++;
	}

	if (labelStr.empty() == false)
	{
		if (labelOffset > 0)	// add offset string
		{
			char offsetString[16];
			sprintf(offsetString, " + %d]", labelOffset);
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

bool ExportAssembler(FCodeAnalysisState& state, const char* pTextFileName, uint16_t startAddr /* = kScreenAttrMemEnd + 1*/, uint16_t endAddr /* = 0xffff */)
{
	FILE* fp =fopen(pTextFileName, "wt");

	if (fp == nullptr)
		return false;

	ENumberDisplayMode hexMode = ENumberDisplayMode::HexDollar;

	ENumberDisplayMode oldMode = GetNumberDisplayMode();
	SetNumberDisplayMode(hexMode);

	// TODO: write screen memory regions

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
			fprintf(fp, "%s:", pLabelInfo->Name.c_str());
		}
		break;
		case EItemType::Code:
		{
			const FCodeInfo* pCodeInfo = static_cast<FCodeInfo*>(item.Item);

			WriteCodeInfoForAddress(state, addr);	// needed to refresh code info
			if (addr == g_DbgAddress)
				LOGINFO("DebugAddress");

			const std::string dasmString = Z80GenerateDasmStringForAddress(state, addr, hexMode);
			fprintf(fp, "\t%s", dasmString.c_str());

			if (pCodeInfo->JumpAddress.IsValid())
			{
				const std::string labelStr = GenerateAddressLabelString(state, pCodeInfo->JumpAddress);
				if (labelStr.empty() == false)
					fprintf(fp, "\t;%s", labelStr.c_str());

			}
			else if (pCodeInfo->PointerAddress.IsValid())
			{
				const std::string labelStr = GenerateAddressLabelString(state, pCodeInfo->PointerAddress);
				if (labelStr.empty() == false)
					fprintf(fp, "\t;%s", labelStr.c_str());
			}
		}

		break;
		case EItemType::Data:
		{
			const FDataInfo* pDataInfo = static_cast<FDataInfo*>(item.Item);
			ENumberDisplayMode dispMode = GetNumberDisplayMode();

			if (pDataInfo->OperandType == EOperandType::Decimal)
				dispMode = ENumberDisplayMode::Decimal;
			if (pDataInfo->OperandType == EOperandType::Hex)
				dispMode = hexMode;
			if (pDataInfo->OperandType == EOperandType::Binary)
				dispMode = ENumberDisplayMode::Binary;

			const bool bOperandIsAddress = (pDataInfo->OperandType == EOperandType::JumpAddress || pDataInfo->OperandType == EOperandType::Pointer);


			fprintf(fp, "\t");
			switch (pDataInfo->DataType)
			{
			case EDataType::Byte:
			{
				const uint8_t val = state.CPUInterface->ReadByte(addr);
				fprintf(fp, "db %s", NumStr(val, dispMode));
			}
			break;
			case EDataType::ByteArray:
			{
				std::string textString;
				for (int i = 0; i < pDataInfo->ByteSize; i++)
				{
					const uint8_t val = state.CPUInterface->ReadByte(addr + i);
					char valTxt[16];
					sprintf(valTxt, "%s%c", NumStr(val, dispMode), i < pDataInfo->ByteSize - 1 ? ',' : ' ');
					textString += valTxt;
				}
				fprintf(fp, "db %s", textString.c_str());
			}
			break;
			case EDataType::Word:
			{
				const uint16_t val = state.CPUInterface->ReadWord(addr);

				const FLabelInfo* pLabel = bOperandIsAddress ? state.GetLabelForPhysicalAddress(val) : nullptr;
				if (pLabel != nullptr)
				{
					fprintf(fp, "dw %s", pLabel->Name.c_str());
				}
				else
				{
					fprintf(fp, "dw %s", NumStr(val, dispMode));
				}
			}
			break;
			case EDataType::WordArray:
			{
				const int wordSize = pDataInfo->ByteSize / 2;
				std::string textString;
				for (int i = 0; i < wordSize; i++)
				{
					const uint16_t val = state.CPUInterface->ReadWord(addr + (i * 2));
					char valTxt[16];
					sprintf(valTxt, "%s%c", NumStr(val), i < wordSize - 1 ? ',' : ' ');
					textString += valTxt;
				}
				fprintf(fp, "dw %s", textString.c_str());
			}
			break;
			case EDataType::Text:
			{
				std::string textString;
				for (int i = 0; i < pDataInfo->ByteSize; i++)
				{
					const char ch = state.CPUInterface->ReadByte(addr + i);
					if (ch == '\n')
						textString += "<cr>";
					if (pDataInfo->bBit7Terminator && ch & (1 << 7))	// check bit 7 terminator flag
						textString += ch & ~(1 << 7);	// remove bit 7
					else
						textString += ch;
				}
				fprintf(fp, "ascii '%s'", textString.c_str());
			}
			break;

			case EDataType::ScreenPixels:
			case EDataType::Blob:
			default:
				fprintf(fp, "%d Bytes", pDataInfo->ByteSize);
				break;
			}
		}
		break;
		}

		// put comment on the end
		if (item.Item->Comment.empty() == false)
			fprintf(fp, "\t;%s", item.Item->Comment.c_str());
		fprintf(fp, "\n");
	}

	fclose(fp);


	SetNumberDisplayMode(oldMode);
	return true;
}
