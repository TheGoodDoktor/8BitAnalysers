#include "JimAnalysis.h"
#include "../BBCEmulator.h"

static std::vector <FRegDisplayConfig>	g_JimRegDrawInfo =
{
	{0xFD00, "PageStart" },
	{0xFDFE, "EntryPoint" },
	{0xFDFF, "PageEnd" }
};

void FJimAnalysis::Init(FBBCEmulator* pEmulator)
{
	Name = "Jim";
	SetAnalyser(&pEmulator->GetCodeAnalysis());

	pBBCEmu = pEmulator;
	pCodeAnalyser->IOAnalyser.AddDevice(this);
	pRegConfig = &g_JimRegDrawInfo;
}

void FJimAnalysis::Reset()
{
}



void AddJimRegisterLabels(FBBCEmulator* pEmulator)
{
	for (const auto& drawInfo : g_JimRegDrawInfo)
	{
		const FAddressRef regAddr(pEmulator->GetBankIds().OSROM, drawInfo.Address);
		FLabelInfo* pLabelInfo = GenerateLabelForAddress(pEmulator->GetCodeAnalysis(), regAddr, ELabelType::Data);
		const std::string labelName = std::string("Jim_") + drawInfo.Name;
		pLabelInfo->ChangeName(labelName.c_str(), regAddr);
		FDataInfo* pDataInfo = pEmulator->GetCodeAnalysis().GetDataInfoForAddress(regAddr);
		pDataInfo->DisplayType = drawInfo.DisplayType;
	}
}
