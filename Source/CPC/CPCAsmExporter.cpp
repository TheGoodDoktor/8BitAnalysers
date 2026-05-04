#include "CodeAnalyser/AssemblerExport.h"
#include "CPCEmu.h"

class FCPCAsmExporterBase : public FASMExporter
{
public:
	void ProcessLabelsOutsideExportedRange(void) override
	{
		FCodeAnalysisState& state = pEmulator->GetCodeAnalysis();

		SetOutputToHeader();

		Output("\n; RAM Labels\n");

		for (auto labelAddr : DasmState.LabelsOutsideRange)
		{
			const FLabelInfo* pLabelInfo = state.GetLabelForPhysicalAddress(labelAddr);
			if (labelAddr >= 0x0000)
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
	return true;
}
