#include "CodeAnalyser/AssemblerExport.h"
#include "CodeAnalyser/UI/CodeAnalyserUI.h"
#include "CPCEmu.h"

class FCPCAsmExporterBase : public FASMExporter
{
public:

	void ProcessLabelsOutsideExportedRange(void) override
	{
		FCodeAnalysisState& state = pEmulator->GetCodeAnalysis();

		SetOutputToHeader();

		Output("\n; RAM Labels\n");

		static const char* blockRanges[] = {
			"Block 0 (0x0000-0x3fff)",
			"Block 1 (0x4000-0x7fff)",
			"Block 2 (0x8000-0xbfff)",
			"Block 3 (0xc000-0xffff)",
		};

		int currentBlock = -1;
		FCPCEmu* pCPCEmu = static_cast<FCPCEmu*>(pEmulator);
		for (auto labelAddr : DasmState.LabelsOutsideRange)
		{
			// TODO: GetLabelForPhysicalAddress uses the read mapping, so labels in RAM hidden
			// behind an active ROM overlay (e.g. RAM 3 behind ROM BASIC) are silently dropped.
			// Fix: fall back to the write bank and use a write-side label lookup.
			const FLabelInfo* pLabelInfo = state.GetLabelForPhysicalAddress(labelAddr);
			if (pLabelInfo == nullptr)
				continue;

			const int block = labelAddr >> 14;
			if (block != currentBlock)
			{
				const FAddressRef addrRef = state.AddressRefFromPhysicalAddress(labelAddr);
				const FCodeAnalysisBank* pBank = state.GetBank(addrRef.BankId);

				std::string desc = pBank ? pBank->Name : "unknown";

				if (pBank && !pBank->bMachineROM && pCPCEmu)
				{
					const uint16_t screenStart = pCPCEmu->Screen.GetScreenAddrStart();
					const uint16_t blockStart = (uint16_t)(block * 0x4000);
					const uint16_t blockEnd = blockStart + 0x3FFF;
					if (screenStart >= blockStart && screenStart <= blockEnd)
						desc += " (screen memory)";
				}

				Output("\n; %s - %s\n", blockRanges[block], desc.c_str());
				currentBlock = block;
			}
			Output("%s: \t%s %s\n", pLabelInfo->GetName(), Config.EQUText, NumStr(labelAddr));
		}
		Output("\n");
	}
};

class FSJasmPlusCPCExporter : public FCPCAsmExporterBase
{
public:
	FSJasmPlusCPCExporter()
	{
		Config.DataBytePrefix = "db";
		Config.DataWordPrefix = "dw";
		Config.DataTextPrefix = "db";
		Config.ORGText = "\torg";
		Config.EQUText = "equ";
		Config.LocalLabelPrefix = ".";
	}

	void AddHeader(void) override
	{
		FCPCEmu* pCPCEmu = static_cast<FCPCEmu*>(pEmulator);
		if (pCPCEmu && pCPCEmu->CPCEmuState.type == CPC_TYPE_6128)
			Output("\tDEVICE AMSTRADCPC6128\n");
		else
			Output("\tDEVICE AMSTRADCPC464\n");
	}
};

class FMaxamExporter : public FCPCAsmExporterBase
{
public:
	FMaxamExporter()
	{
		Config.DataBytePrefix = "defb";
		Config.DataWordPrefix = "defw";
		Config.DataTextPrefix = "defm";
		Config.ORGText = "org";
		Config.EQUText = "equ";
		Config.LocalLabelPrefix = ".";
	}
};

bool InitCPCAsmExporters(FCPCEmu* pCPCEmu)
{
	AddAssemblerExporter("SJasmPlus", new FSJasmPlusCPCExporter);
	AddAssemblerExporter("Maxam", new FMaxamExporter);

	FGlobalConfig* pConfig = pCPCEmu->GetCodeAnalysis().pGlobalConfig;
	if (pConfig != nullptr && pConfig->ExportAssembler.empty())
		pConfig->ExportAssembler = "SJasmPlus";

	return true;
}
