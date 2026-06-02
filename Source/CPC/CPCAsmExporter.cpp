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

		static const char* blockComments[] = {
			"\n; Block 0 (0x0000-0x3fff) - RAM or OS ROM; screen memory possible but uncommon\n",
			"\n; Block 1 (0x4000-0x7fff) - RAM only (never ROM); commonly used as secondary screen buffer\n",
			"\n; Block 2 (0x8000-0xbfff) - RAM only (never ROM); screen memory possible\n",
			"\n; Block 3 (0xc000-0xffff) - RAM or BASIC/expansion ROM; default screen memory location\n",
		};

		int currentBlock = -1;
		for (auto labelAddr : DasmState.LabelsOutsideRange)
		{
			const FLabelInfo* pLabelInfo = state.GetLabelForPhysicalAddress(labelAddr);
			if (pLabelInfo == nullptr)
				continue;

			const int block = labelAddr >> 14;
			if (block != currentBlock)
			{
				Output("%s", blockComments[block]);
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
