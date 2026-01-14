#include "CodeAnalyser/AssemblerExport.h"
//#include "PCEEmu.h"

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

class FKickAssemblerExporter : public FPCEAsmExporterBase
{
public:
	FKickAssemblerExporter()
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

#if 0
class FSpectrumNextExporter : public FSJasmPlusExporter
{
public:
	FSpectrumNextExporter() : FSJasmPlusExporter()
	{

	}

	void AddHeader(void) override
	{
		Output("\tDEVICE ZXSPECTRUMNEXT\n");	
	}
};

class FSpasmExporter : public FSpeccyAsmExporterBase
{
public:
	FSpasmExporter()
	{
		Config.DataBytePrefix = ".db";
		Config.DataWordPrefix = ".dw";
		Config.DataTextPrefix = ".text";
		Config.ORGText = ".org";
		Config.EQUText = ".equ";
		Config.LocalLabelPrefix = "@";
	}
};

class FEZ80AsmExporter : public FSpeccyAsmExporterBase
{
public:
	FEZ80AsmExporter()
	{
		Config.DataBytePrefix = ".db";
		Config.DataWordPrefix = ".dw";
		Config.DataTextPrefix = ".db";
		Config.ORGText = ".org";
		Config.EQUText = ".equ";
		Config.LocalLabelPrefix = "@";

	}
};
#endif

bool InitPCEAsmExporters()
{
	AddAssemblerExporter("KickAssembler", new FKickAssemblerExporter);
	return true;
}