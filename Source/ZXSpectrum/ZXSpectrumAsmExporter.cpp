#include "CodeAnalyser/AssemblerExport.h"
#include "SpectrumEmu.h"

class FSpeccyAsmExporterBase : public FASMExporter
{
	public:
		void ProcessLabelsOutsideExportedRange(void) override
		{
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
		}
};

class FSJasmPlusExporter : public FSpeccyAsmExporterBase
{
public:
	FSJasmPlusExporter()
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
		Output("\tDEVICE ZXSPECTRUM48\n");	// only 48k spectrum asm dumps are supported atm
	}
};

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

// Agon exporter uses Spasm but needs to add some other stuff
class FAgonAsmExporter : public FEZ80AsmExporter
{
public:
	void	AddHeader(void) override
	{
		Output(".assume ADL=0\n");	// use standard z80 code
	}



};

bool InitZXSpectrumAsmExporters(FSpectrumEmu *pZXEmu)
{
	AddAssemblerExporter("SJasmPlus", new FSJasmPlusExporter);
	AddAssemblerExporter("Spasm", new FSpasmExporter);
	AddAssemblerExporter("Agon", new FAgonAsmExporter);
	AddAssemblerExporter("SpectrumNext", new FSpectrumNextExporter);
	return true;
}