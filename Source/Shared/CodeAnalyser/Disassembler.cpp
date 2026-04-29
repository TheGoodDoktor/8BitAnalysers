#include "Disassembler.h"

#include "CodeAnalyser.h"
#include "6502/M6502Disassembler.h"
#include "6502/HuC6280Disassembler.h"
#include "Z80/Z80Disassembler.h"

#include "AssemblerExport.h"

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

// todo: fallback to the old behaviour if we are just exporting physical memory.
// When exporting banks that may not be in physical memory we cannot use the same
// code for exporting physical memory. When we encounter a 16 bit value that is
// an operand address, it is ambiguous which bank this belongs to. There are potentially
// multiple banks that share the same physical address range.
void FExportDasmState::OutputU16(uint16_t val, dasm_output_t outputCallback) 
{
	if (outputCallback)
	{
		const bool bOperandIsAddress = (pCodeInfoItem->OperandType == EOperandType::JumpAddress || pCodeInfoItem->OperandType == EOperandType::Pointer);

		if (bOperandIsAddress)
		{
			FAddressRef labelAddress = FAddressRef::Invalid();
			const FLabelInfo* pLabel = nullptr;

			if (pCodeInfoItem->OperandAddress.IsValid())
			{
				pLabel = pExporter->ProcessOperandLabel(labelAddress, val, outputCallback);
			}
			else
			{
				// what to do here?
				const FCodeAnalysisBank* pCurBank = CodeAnalysisState->GetBank(CurrentAddress.GetBankId());
				pExporter->QueueWarning("'%s': 0x%04x. Found invalid operand address 0x%x. %s", pCurBank->Name.c_str(), CurrentAddress.GetAddress(), pCodeInfoItem->OperandAddress.GetAddress(), pCodeInfoItem->Text.c_str());
			}

			if (pLabel)
			{
				if (labelAddress.GetAddress() < ExportMin || labelAddress.GetAddress() > ExportMax)
				{
					LabelsOutsideRange.insert(labelAddress);
				}

				if (!CodeAnalysisState->IsBankIdCanonical(labelAddress.GetBankId()))
				{
					const FCodeAnalysisBank* pCurBank = CodeAnalysisState->GetBank(CurrentAddress.GetBankId());
					pExporter->QueueWarning("'%s': 0x%04x. Found non canonical bank label '%s' 0x%x. %s", pCurBank->Name.c_str(), CurrentAddress.GetAddress(), pLabel->GetName(), labelAddress.GetAddress(), pCodeInfoItem->Text.c_str());
				}
			}
			else
			{
				//const FCodeAnalysisBank* pBank = CodeAnalysisState->GetBank(CurrentAddress.GetBankId());
				//LOGWARNING("'%s': 0x%04x. No label found for address 0x%x", pBank->Name.c_str(), CurrentAddress.GetAddress(), pCodeInfoItem->OperandAddress.GetAddress());

				// Just output the address if we couldn't find a label.
				const char* outStr = NumStr(val, GetNumberDisplayMode());
				for (int i = 0; i < strlen(outStr); i++)
					outputCallback(outStr[i], this);
			}
#if 0
			int labelOffset = 0;
			uint16_t labelAddress = 0;
			std::vector<std::pair<FCodeAnalysisBank*, FLabelInfo*>> potentialLabels;
			
			const FLabelInfo* pLabel = nullptr;
			const FLabelInfo* pScopeLabel = nullptr;

			for (int addrVal = val; addrVal >= 0; addrVal--)
			{
				// sam. moved this to below to support labels at location 0x0000
				/*if (addrVal <= 0)	// no label found
				{
					const char* outStr = NumStr(val, GetNumberDisplayMode());
					for (int i = 0; i < strlen(outStr); i++)
						outputCallback(outStr[i], this);
					break;
				}*/

				// This address might not be in physical memory.
				// Go through all banks with a matching address range and find candidates.
				for (int b = 0; b < FCodeAnalysisState::BankCount; b++)
				{
					FCodeAnalysisBank& bank = CodeAnalysisState->GetBanks()[b];
					if (bank.bEverBeenMapped)
					{
						const uint16_t start = bank.GetMappedAddress();
						if (addrVal >= start && addrVal < start + bank.GetSizeBytes())
						{
							const FAddressRef addr(bank.Id, addrVal);
							if (FLabelInfo* pTmpLabel = CodeAnalysisState->GetLabelForAddress(addr))
							{
								if (!pLabel)
								{
									pLabel = pTmpLabel;
									pScopeLabel = CodeAnalysisState->GetScopeForAddress(addr);
								}
								potentialLabels.push_back(std::make_pair(&bank, pTmpLabel));
							}
						}
					}
				}

				if (potentialLabels.size() > 1)
				{
					const FCodeAnalysisBank* pBank = CodeAnalysisState->GetBank(CurrentAddress.GetBankId());
					LOGINFO("'%s': 0x%04x. Found %d labels for address 0x%x", pBank->Name.c_str(), CurrentAddress.GetAddress(), potentialLabels.size(), addrVal);
					for (auto pair : potentialLabels)
						LOGINFO("  %d '%s': '%s'", pair.first->Id, pair.first->Name.c_str(), pair.second->GetName());
				}
		
				if (pLabel != nullptr)
				{
					std::string labelName = pLabel->GetName();
					labelAddress = addrVal;

					if (pExporter->GetConfig().bUseLocalLabelPrefix)
					{
						// Local prefix
						if (pLabel->Global == false)
						{
							if (pScopeLabel != nullptr)
							{
								if(pScopeLabel != pCurrentScope)
								{
									std::string scopeLabelName = pScopeLabel->GetName();

									for (int i = 0; i < scopeLabelName.size(); i++)
										outputCallback(scopeLabelName[i], this);
							
									if (pExporter->IsLabelStubbed(scopeLabelName.c_str()))
									{
										const char* pStubbed = "_Stubbed";
										for (int i = 0; i < strlen(pStubbed); i++)
											outputCallback(pStubbed[i], this);
									}
								}

								const char* pLocalPrefix = pExporter->GetConfig().LocalLabelPrefix;
								for (int i = 0; i < strlen(pLocalPrefix); i++)
									outputCallback(pLocalPrefix[i], this);
							}
						}
					}

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
				else
				{
					// sam. moved this from above to support labels at location 0x0000
					if (addrVal == 0)	// no label found
					{
						const char* outStr = NumStr(val, GetNumberDisplayMode());
						for (int i = 0; i < strlen(outStr); i++)
							outputCallback(outStr[i], this);
						break;
					}
				}
			
				labelOffset++;
			}
			// referencing an address not in the disassembly but not null
			// sam. changed check for label address 0 to deal with a label at address 0.
			//if (/*labelAddress != 0 && */(labelAddress < ExportMin || labelAddress > ExportMax))
			if (pLabel && (labelAddress < ExportMin || labelAddress > ExportMax))
			{
				if (!potentialLabels.empty())
				{
					const FAddressRef addrRef(potentialLabels.front().first->Id, labelAddress);
					LabelsOutsideRange.insert(addrRef);
				}
			}
#endif
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

	const uint8_t val = pDasmState->CodeAnalysisState->ReadByte(pDasmState->CurrentAddress);

#ifndef NDEBUG
	const int16_t oldBankId = pDasmState->CurrentAddress.GetBankId();
#endif
	
	// sam. do we need to worry about ending up in another bank?
	pDasmState->CodeAnalysisState->AdvanceAddressRef(pDasmState->CurrentAddress);
	
#ifndef NDEBUG
	// sam. can we do this check somewhere else?
	const uint16_t addr = pDasmState->CurrentAddress.GetAddress();
	if (addr > pDasmState->ExportMax)
	{
		LOGERROR("Exporting past the export range. Current: 0x%x. Max: 0x%x", addr, pDasmState->ExportMax);
	}

	if (oldBankId != pDasmState->CurrentAddress.GetBankId())
	{
		LOGINFO("Advanced from bank %d to bank %d", oldBankId, pDasmState->CurrentAddress.GetBankId());
	}
#endif
	return val;
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
/*
std::string GenerateDasmStringForAddress(FCodeAnalysisState& state, uint16_t pc, ENumberDisplayMode hexMode)
{
	if(state.CPUInterface->CPUType == ECPUType::Z80)
		return Z80GenerateDasmStringForAddress(state,pc,hexMode);
	else
		return M6502GenerateDasmStringForAddress(state, pc, hexMode);
}*/

bool GenerateDasmExportString(FExportDasmState& exportState)
{

	switch (exportState.CodeAnalysisState->CPUInterface->CPUType)
	{
	case ECPUType::Z80:
		return Z80GenerateDasmExportString(exportState);
	case ECPUType::M6502:
		return M6502GenerateDasmExportString(exportState);
	case ECPUType::HuC6280:
		return HuC6280GenerateDasmExportString(exportState);
	default:
		return false;
	}
}