#include "CodeAnalyser/AssemblerExport.h"

class FPCEAsmExporterBase : public FASMExporter
{
	public:
		void ProcessLabelsOutsideExportedRange(void) override
		{
			#if 0
			FCodeAnalysisState& state = pEmulator->GetCodeAnalysis();

			SetOutputToHeader();

			Output("\n\n; ROM Labels\n");

			for (auto labelAddr : DasmState.LabelsOutsideRange)
			{
				const FLabelInfo* pLabelInfo = state.GetLabelForPhysicalAddress(labelAddr);
				if (labelAddr < 0x4000)
					Output("%s: \t%s %s\n", pLabelInfo->GetName(),Config.EQUText, NumStr(labelAddr));
			}

			Output("\n; Screen Memory Labels\n");

			for (auto labelAddr : DasmState.LabelsOutsideRange)
			{
				const FLabelInfo* pLabelInfo = state.GetLabelForPhysicalAddress(labelAddr);
				if (labelAddr >= 0x4000)
					Output("%s: \t%s %s\n", pLabelInfo->GetName(), Config.EQUText, NumStr(labelAddr));
			}

			Output("\n");
			#endif
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
		Config.ORGText = "\torg";
		Config.EQUText = "equ";
		Config.LocalLabelPrefix = ".";
	}

	void AddHeader(void) override
	{
		//Output("\tDEVICE ZXSPECTRUM48\n");	// only 48k spectrum asm dumps are supported atm
	}
};

bool InitPCEAsmExporters()
{
	AddAssemblerExporter("PCEAS", new FPCEASAssemblerExporter);
	return true;
}