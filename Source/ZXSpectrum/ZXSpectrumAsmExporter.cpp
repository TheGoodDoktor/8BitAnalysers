#include "CodeAnalyser/AssemblerExport.h"
#include "SpectrumEmu.h"

class FSJasmPlusExporter : public FASMExporter
{
public:
	FSJasmPlusExporter()
	{
		Config.DataBytePrefix = "db";
		Config.DataWordPrefix = "dw";
		Config.DataTextPrefix = "ascii";
		Config.ORGText = "\torg";
	}

	void AddHeader(void) override
	{
		Output("\tdevice zxspectrum48\n");	// only 48k spectrum asm dumps are supported atm
	}

	void ProcessLabelsOutsideExportedRange(void) override
	{
		FCodeAnalysisState& state = *pCodeAnalyser;

		SetOutputToHeader();

		Output("\n\n; ROM Labels\n");

		for (auto labelAddr : DasmState.LabelsOutsideRange)
		{
			const FLabelInfo* pLabelInfo = state.GetLabelForPhysicalAddress(labelAddr);
			if(labelAddr < 0x4000)
				Output("%s: \tequ %s\n", pLabelInfo->GetName(), NumStr(labelAddr));
		}

		Output("\n; Screen Memory Labels\n");

		for (auto labelAddr : DasmState.LabelsOutsideRange)
		{
			const FLabelInfo* pLabelInfo = state.GetLabelForPhysicalAddress(labelAddr);
			if (labelAddr >= 0x4000)
				Output("%s: \tequ %s\n", pLabelInfo->GetName(), NumStr(labelAddr));
		}

		Output("\n");
	}

};

class FSpasmExporter : public FASMExporter
{
public:
	FSpasmExporter()
	{
		Config.DataBytePrefix = ".db";
		Config.DataWordPrefix = ".dw";
		Config.DataTextPrefix = ".text";
		Config.ORGText = ".org";
	}
};

// Agon exporter uses Spasm but needs to add some other stuff
class FAgonAsmExporter : public FSpasmExporter
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
	return true;
}