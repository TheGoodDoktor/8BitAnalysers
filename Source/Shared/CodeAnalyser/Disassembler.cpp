#include "Disassembler.h"

#include "CodeAnalyser.h"
#include "6502/M6502Disassembler.h"
#include "Z80/Z80Disassembler.h"



void FAnalysisDasmState::OutputU8(uint8_t val, dasm_output_t outputCallback) 
{
	if (outputCallback != nullptr)
	{
		ENumberDisplayMode dispMode = GetNumberDisplayMode();

		if ((pCodeInfoItem->OperandType == EOperandType::Pointer || pCodeInfoItem->OperandType == EOperandType::JumpAddress)
			&& pCodeInfoItem->OperandAddress.IsValid())
		{
			char pointerMarkup[16];
			strcpy(pointerMarkup, "#OPERAND_ADDR#");
			for (int i = 0; i < strlen(pointerMarkup); i++)
				outputCallback(pointerMarkup[i], this);
			return;
		}

		if (pCodeInfoItem->OperandType == EOperandType::Decimal)
			dispMode = ENumberDisplayMode::Decimal;
		else if (pCodeInfoItem->OperandType == EOperandType::Hex)
			dispMode = ENumberDisplayMode::HexAitch;
		else if (pCodeInfoItem->OperandType == EOperandType::Binary)
			dispMode = ENumberDisplayMode::Binary;
		else if (pCodeInfoItem->OperandType == EOperandType::SignedNumber)
			dispMode = ENumberDisplayMode::Decimal;
		else if (pCodeInfoItem->OperandType == EOperandType::Ascii)
			dispMode = ENumberDisplayMode::Ascii;

		PushString("#IM:", outputCallback);
		PushString(NumStr(val, dispMode), outputCallback);
		PushString("#", outputCallback);
		//const char* outStr = NumStr(val, dispMode);
		//for (int i = 0; i < strlen(outStr); i++)
		//	outputCallback(outStr[i], this);
	}
}

void FAnalysisDasmState::OutputU16(uint16_t val, dasm_output_t outputCallback) 
{
	if (outputCallback)
	{
		ENumberDisplayMode dispMode = GetNumberDisplayMode();

		if (pCodeInfoItem->OperandType == EOperandType::Pointer || pCodeInfoItem->OperandType == EOperandType::JumpAddress)
		{
			char pointerMarkup[16];
			//snprintf(pointerMarkup,16,"#ADDR:0x%04X#", val);
			strcpy(pointerMarkup, "#OPERAND_ADDR#");
			for (int i = 0; i < strlen(pointerMarkup); i++)
				outputCallback(pointerMarkup[i], this);
			return;
		}

		if (pCodeInfoItem->OperandType == EOperandType::Decimal)
			dispMode = ENumberDisplayMode::Decimal;
		else if (pCodeInfoItem->OperandType == EOperandType::Hex)
			dispMode = ENumberDisplayMode::HexAitch;
		else if (pCodeInfoItem->OperandType == EOperandType::Binary)
			dispMode = ENumberDisplayMode::Binary;

		PushString("#IM:", outputCallback);
		PushString(NumStr(val, dispMode), outputCallback);
		PushString("#", outputCallback);

		//const char* outStr = NumStr(val, dispMode);
		//for (int i = 0; i < strlen(outStr); i++)
		//	outputCallback(outStr[i], this);
	}
}

void FAnalysisDasmState::OutputD8(int8_t val, dasm_output_t outputCallback) 
{
	if (outputCallback)
	{
		if (val < 0)
		{
			outputCallback('-', this);
			val = -val;
		}
		else
		{
			outputCallback('+', this);
		}

		PushString("#IM:", outputCallback);
		PushString(NumStr((uint8_t)val), outputCallback);
		PushString("#", outputCallback);

		//const char* outStr = NumStr((uint8_t)val);
		//for (int i = 0; i < strlen(outStr); i++)
		//	outputCallback(outStr[i], this);
	}
}

FAnalysisDasmState testA;

// disassembler callback to fetch the next instruction byte 
uint8_t AnalysisDasmInputCB(void* pUserData)
{
	FAnalysisDasmState* pDasmState = (FAnalysisDasmState*)pUserData;

	return pDasmState->CodeAnalysisState->ReadByte(pDasmState->CurrentAddress++);
}

// disassembler callback to output a character 
void AnalysisOutputCB(char c, void* pUserData)
{
	FAnalysisDasmState* pDasmState = (FAnalysisDasmState*)pUserData;

	// add character to string
	pDasmState->Text += c;
}

// For Aseembler exporter

void FExportDasmState::OutputU8(uint8_t val, dasm_output_t outputCallback) 
{
	if (outputCallback != nullptr)
	{
		ENumberDisplayMode dispMode = GetNumberDisplayMode();

		if (pCodeInfoItem->OperandType == EOperandType::Decimal)
			dispMode = ENumberDisplayMode::Decimal;
		if (pCodeInfoItem->OperandType == EOperandType::Hex)
			dispMode = HexDisplayMode;
		if (pCodeInfoItem->OperandType == EOperandType::Binary)
			dispMode = ENumberDisplayMode::Binary;

		const char* outStr = NumStr(val, dispMode);
		for (int i = 0; i < strlen(outStr); i++)
			outputCallback(outStr[i], this);
	}
}

void FExportDasmState::OutputU16(uint16_t val, dasm_output_t outputCallback) 
{
	if (outputCallback)
	{
		const bool bOperandIsAddress = (pCodeInfoItem->OperandType == EOperandType::JumpAddress || pCodeInfoItem->OperandType == EOperandType::Pointer);

		if (bOperandIsAddress)
		{
			int labelOffset = 0;

			for (int addrVal = val; addrVal >= 0; addrVal--)
			{
				const FLabelInfo* pLabel = CodeAnalysisState->GetLabelForPhysicalAddress(addrVal);
				if (pLabel != nullptr)
				{
					std::string labelName(pLabel->GetName());
					for (int i = 0; i < labelName.size(); i++)
					{
						outputCallback(labelName[i], this);
					}

					// add label offset
					if (labelOffset != 0)
					{
						char offsetString[32];
						sprintf(offsetString,"+%d",labelOffset);

						for (int i = 0; i < strlen(offsetString); i++)
							outputCallback(offsetString[i], this);
					}
					break;
				}
				else if (addrVal == 0)	// no label found
				{
					const char* outStr = NumStr(val, GetNumberDisplayMode());
					for (int i = 0; i < strlen(outStr); i++)
						outputCallback(outStr[i], this);
				}

				labelOffset++;
			}
		}
		else
		{
			ENumberDisplayMode dispMode = GetNumberDisplayMode();

			if (pCodeInfoItem->OperandType == EOperandType::Decimal)
				dispMode = ENumberDisplayMode::Decimal;
			if (pCodeInfoItem->OperandType == EOperandType::Hex)
				dispMode = HexDisplayMode;
			if (pCodeInfoItem->OperandType == EOperandType::Binary)
				dispMode = ENumberDisplayMode::Binary;

			const char* outStr = NumStr(val, dispMode);
			for (int i = 0; i < strlen(outStr); i++)
				outputCallback(outStr[i], this);
		}
	}
}

void FExportDasmState::OutputD8(int8_t val, dasm_output_t outputCallback) 
{
	if (outputCallback)
	{
		if (val < 0)
		{
			outputCallback('-', this);
			val = -val;
		}
		else
		{
			outputCallback('+', this);
		}
		const char* outStr = NumStr((uint8_t)val);
		for (int i = 0; i < strlen(outStr); i++)
			outputCallback(outStr[i], this);
	}
}

FExportDasmState testB;

/* disassembler callback to fetch the next instruction byte */
uint8_t ExportDasmInputCB(void* pUserData)
{
	FExportDasmState* pDasmState = (FExportDasmState*)pUserData;

	return pDasmState->CodeAnalysisState->CPUInterface->ReadByte(pDasmState->CurrentAddress++);
}

/* disassembler callback to output a character */
void ExportOutputCB(char c, void* pUserData)
{
	FExportDasmState* pDasmState = (FExportDasmState*)pUserData;

	// add character to string
	pDasmState->Text += c;
}


static IDasmNumberOutput* g_pNumberOutputObj = nullptr;
static IDasmNumberOutput* GetNumberOutput()
{
	return g_pNumberOutputObj;
}

void SetNumberOutput(IDasmNumberOutput* pNumberOutputObj)
{
	g_pNumberOutputObj = pNumberOutputObj;
}

// output an unsigned 8-bit value as hex string 
void DasmOutputU8(uint8_t val, dasm_output_t out_cb, void* user_data)
{
	IDasmNumberOutput* pNumberOutput = GetNumberOutput();
	if (pNumberOutput)
		pNumberOutput->OutputU8(val, out_cb);

}

// output an unsigned 16-bit value as hex string 
void DasmOutputU16(uint16_t val, dasm_output_t out_cb, void* user_data)
{
	IDasmNumberOutput* pNumberOutput = GetNumberOutput();
	if (pNumberOutput)
		pNumberOutput->OutputU16(val, out_cb);
}

// output a signed 8-bit offset as hex string 
void DasmOutputD8(int8_t val, dasm_output_t out_cb, void* user_data)
{
	IDasmNumberOutput* pNumberOutput = GetNumberOutput();
	if (pNumberOutput)
		pNumberOutput->OutputD8(val, out_cb);
}

std::string GenerateDasmStringForAddress(FCodeAnalysisState& state, uint16_t pc, ENumberDisplayMode hexMode)
{
	if(state.CPUInterface->CPUType == ECPUType::Z80)
		return Z80GenerateDasmStringForAddress(state,pc,hexMode);
	else
		return M6502GenerateDasmStringForAddress(state, pc, hexMode);
}