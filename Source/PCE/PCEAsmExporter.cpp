#include "CodeAnalyser/AssemblerExport.h"
#include "CodeAnalyser/6502/HuC6280Disassembler.h"
#include "PCEEmu.h"

#define ASM_EXPORT_LABEL_DEBUG 0
#define ASM_EXPORT_NEW_LABELS_METHOD 1

class FPCEAsmExporterBase : public FASMExporter
{
	public:
		void ProcessLabelsOutsideExportedRange() override
		{
			// We need to deal with games like Bikkuriman World (Japan) (Alt 1).
			// It exports ROM 1 that has the address range [0x0000 - 0x1fff],
			// but this means we dont include the HW PAGE labels because they are not technically out of range.
			// Hence we are looking at bank id instead of address range.

			// do we need to deal with the situation where we are exporting a range that does not span an entire bank?

#if ASM_EXPORT_NEW_LABELS_METHOD
			std::unordered_set<int16_t> exportedBankIds;
			for (auto pBank : ExportBanks)
			{
				exportedBankIds.insert(pBank->Id);
			}
#endif
			if (!DasmState.LabelsOutsideRange.empty())
			{
				FCodeAnalysisState& state = pEmulator->GetCodeAnalysis();

				std::set<FAddressRef> labels;
				
#if ASM_EXPORT_LABEL_DEBUG
				LOGINFO("Labels outside range:");
#endif
				for (auto labelAddrRef : DasmState.LabelsOutsideRange)
				{
#if ASM_EXPORT_LABEL_DEBUG
					FCodeAnalysisBank* pBank = state.GetBank(labelAddrRef.GetBankId());
					const FLabelInfo* pLabelInfo = state.GetLabelForAddress(labelAddrRef);
					LOGINFO("  Label: %s 0x%x [%s]", pLabelInfo ? pLabelInfo->GetName() : "Unknown label", labelAddrRef.GetAddress(), pBank ? pBank->Name.c_str() : "Unknown bank");
#endif
#if ASM_EXPORT_NEW_LABELS_METHOD
					// See if this label is in the exported banks.
					// If it isn't then we want to declare it.
					auto it = exportedBankIds.find(labelAddrRef.GetBankId());
					if (it == exportedBankIds.end())
						labels.insert(labelAddrRef);
#else
					bool bInRange = false;
					for (const FExportRange& range : ExportRanges)
					{
						const uint16_t labelAddr = labelAddrRef.GetAddress();
						if (labelAddr >= range.Min && labelAddr <= range.Max)
						{
							bInRange = true;
							break;
						}
					}
					if (!bInRange)
					{
						labels.insert(labelAddrRef);
					}
#endif
				}

				SetOutputToHeader();

				if (labels.size())
				{
					Output("\n; Labels\n");

#if ASM_EXPORT_LABEL_DEBUG
					LOGINFO("Writing %d Labels", labels.size());
#endif
					for (auto labelAddr : labels)
					{
						const FLabelInfo* pLabelInfo = state.GetLabelForAddress(labelAddr);
						if (pLabelInfo)
						{
							Output("%s: \t%s %s\n", pLabelInfo->GetName(), Config.EQUText, NumStr(labelAddr.GetAddress()));
							FCodeAnalysisBank* pBank = state.GetBank(labelAddr.GetBankId());
#if ASM_EXPORT_LABEL_DEBUG
							LOGINFO("  Label: %s 0x%x [%s]", pLabelInfo->GetName(), labelAddr.GetAddress(), pBank ? pBank->Name.c_str() : "Unknown bank");
#endif
						}
						else
							LOGINFO("Can't get label for address 0x%x", labelAddr.GetAddress());
					}

					Output("\n");
				}
			}
		}
		virtual FLabelInfo* ProcessOperandLabel(FAddressRef& labelAddress, uint16_t val, dasm_output_t outputCallback) override
		{
			FCodeAnalysisState& state = pEmulator->GetCodeAnalysis();
			FLabelInfo* pLabel = state.GetLabelForAddress(DasmState.pCodeInfoItem->OperandAddress);

			if (pLabel != nullptr)
			{
				const uint16_t operandAddr = DasmState.pCodeInfoItem->OperandAddress.GetAddress();
				const FDataInfo* pDataInfo = state.GetDataInfoForAddress(DasmState.pCodeInfoItem->OperandAddress);

				if (operandAddr != val)
				{
					// todo check if bank has been mapped to val
					// if not return null?
					// if so, output the label and then the offset below

					const std::string labelName = pLabel->GetName();
					labelAddress = DasmState.pCodeInfoItem->OperandAddress;

					for (int i = 0; i < labelName.size(); i++)
					{
						outputCallback(labelName[i], &DasmState);
					}

					const int32_t offset = (int32_t)val - (int32_t)operandAddr;
					char offsetStr[16];
					if (offset > 0)
						snprintf(offsetStr, sizeof(offsetStr), "+$%X", offset);
					else
						snprintf(offsetStr, sizeof(offsetStr), "-$%X", -offset);
					for (const char* p = offsetStr; *p; p++)
						outputCallback(*p, &DasmState);

					QueueWarning("Fixed up label %s with offset %s$%x", pLabel->GetName(), offset > 0 ? "+" : "-", abs(offset));
				}
				else if (pDataInfo != nullptr && pDataInfo->DataType == EDataType::InstructionOperand)
				{
					// The operand address points inside the instruction's own bytes (e.g. a jump
					// table like JMP [data_ROM_02_7A4B,X] where 7A4B is the 2nd byte of the
					// instruction at 7A4A). Emitting the label for 7A4B directly would place it
					// after the instruction in the output, producing unassemblable code.
					//
					// Instead, find the label at the start of the parent instruction and emit
					// it as label+offset, e.g. label_ROM_02_7A4A+1.
					const FAddressRef instrAddr = pDataInfo->InstructionAddress;
					const FLabelInfo* pInstrLabel = state.GetLabelForAddress(instrAddr);
					if (pInstrLabel != nullptr)
					{
						const int offset = DasmState.pCodeInfoItem->OperandAddress.GetAddress() - instrAddr.GetAddress();
						std::string labelExpr = pInstrLabel->GetName();
						if (offset > 0)
						{
							char offsetStr[16];
							snprintf(offsetStr, sizeof(offsetStr), "+%d", offset);
							labelExpr += offsetStr;
						}
						labelAddress = instrAddr;
						for (char c : labelExpr)
							outputCallback(c, &DasmState);
					}
					else
					{
						const FCodeAnalysisBank* pCurBank = state.GetBank(DasmState.CurrentAddress.GetBankId());
						QueueWarning("'%s': 0x%04x. Label '%s' (0x%04x) is inside the instruction bytes and no instruction label found. Outputting raw value.", pCurBank->Name.c_str(), DasmState.CurrentAddress.GetAddress(), pLabel->GetName(), DasmState.pCodeInfoItem->OperandAddress.GetAddress());
						DasmState.NumRawValuesOutput++;
						return nullptr;
					}
				}
				else
				{
					const std::string labelName = pLabel->GetName();
					labelAddress = DasmState.pCodeInfoItem->OperandAddress;

					for (int i = 0; i < labelName.size(); i++)
					{
						outputCallback(labelName[i], &DasmState);
					}
				}
			}
			return pLabel;
		}

		/*const FLabelInfo* ResolveOperandLabel(const FLabelInfo* pLabel, const FAddressRef& operandAddress, uint16_t val, FAddressRef& outResolvedAddress) override
		{
			if (operandAddress.GetAddress() == val)
			{
				outResolvedAddress = operandAddress;
				return pLabel;
			}

			FPCEEmu* pPCEEmu = static_cast<FPCEEmu*>(pEmulator);
			FCodeAnalysisState& state = pEmulator->GetCodeAnalysis();

			const FBankSet* pBankSet = pPCEEmu->GetBankSetForBankId(operandAddress.GetBankId());
			if (pBankSet == nullptr)
				return nullptr;

			// Check if this bank set has ever been mapped to the slot containing val
			const int neededSlot = val / 0x2000;
			if (!(pBankSet->MappedSlotsMask & (1 << neededSlot)))
				return nullptr;

			// Return the label at its primary address. The caller will emit an offset
			// (e.g. "label + $2000") so the expression assembles to val correctly.
			outResolvedAddress = operandAddress;
			return pLabel;
		}*/

		void	ExportDidEnd() override
		{
			if (DasmState.NumRawValuesOutput)
				LOGINFO("Output %d raw values instead of labels", DasmState.NumRawValuesOutput);

			// Reset disassembler back to default settings
			FHuC6280DisassemblerConfig& config = GetHuC6280DisassemblerConfig();
			config = GetHuC6280DisassemblerDefaultConfig();

			// Update every code item to refresh the disassembly back to the code analysis disassembly.
			FCodeAnalysisState& state = pEmulator->GetCodeAnalysis();
			int addr = 0x2000;
			while (addr <= 0xffff)
			{
				if (const FCodeInfo* pCodeInfo = state.GetCodeInfoForPhysicalAddress(addr))
				{
					UpdateCodeInfoForAddress(state, addr);
					addr += pCodeInfo->ByteSize;
				}
				else
				{
					addr++;
				}
			}
		}
};

class FPCEASAssemblerExporter : public FPCEAsmExporterBase
{
public:
	FPCEASAssemblerExporter()
	{
		Config.DataBytePrefix = "db";
		Config.DataWordPrefix = "dw";
		Config.DataTextPrefix = "db";
		Config.ORGText = "\t.org";
		Config.EQUText = ".equ";

		// PCEAS doesn't support labels with the function as a prefix, eg. EntryPoint.loop
		// Where EntryPoint is a function and loop is a local label in the EntryPoint function.
		Config.LocalLabelPrefix = "";
		Config.bUseLocalLabelPrefix = false;
	}

	void	ExportDidBegin() override
	{
		FHuC6280DisassemblerConfig& config = GetHuC6280DisassemblerConfig();
		config.BrOp = '[';
		config.BrCl = ']';
		config.ZpPr = ZeroPagePrefix.c_str();
		config.MprIndexMode = true;
	}

	void AddHeader(void) override
	{
		//Output("\t.cpu 6280\n");
		Output("\t.list\n");
	}
	void AddBankSection(const FCodeAnalysisBank* pBank) override
	{
		// todo: if this bank has a dynamic address range then dont use the current mapped address
		// as the org value because it could be incorrect. try to infer a deterministic address. 
		// try either:
		// - using the first address we ever saw this bank mapped to
		// - the most common address for this bank
		FASMExporter::AddBankSection(pBank);

		// todo: output which slots/addresses this bank has been mapped to
		FPCEEmu* pPCEEmu = static_cast<FPCEEmu*>(pEmulator);
		const uint8_t bankIndex = pPCEEmu->GetBankIndexForBankId(pBank->Id);
		if (!pBank->bEverBeenMapped)
			Output("\t; Note: this bank has never been mapped so the correct org address is unknown.\n", bankIndex);
		Output("\t.bank %d\n", bankIndex);

		if (bankIndex == 0xff)
			LOGERROR("Could not lookup bank index for bank id %d", pBank->Id);
	}

protected:
	std::string ZeroPagePrefix = "<";
};

bool InitPCEAsmExporters()
{
	AddAssemblerExporter("PCEAS", new FPCEASAssemblerExporter);
	return true;
}