#include "JimAnalysis.h"
#include "../BBCEmulator.h"

void FJimAnalysis::Init(FBBCEmulator* pEmulator)
{
	pBBCEmu = pEmulator;
}

void FJimAnalysis::Reset()
{
}

void FJimAnalysis::OnRegisterRead(uint8_t reg, FAddressRef pc)
{
}

void FJimAnalysis::OnRegisterWrite(uint8_t reg, uint8_t val, FAddressRef pc)
{
}

void FJimAnalysis::DrawDetailsUI(void)
{
}

static std::vector <FRegDisplayConfig>	g_JimRegDrawInfo =
{
	{0xFD00, "PageStart" },
	{0xFDFE, "EntryPoint" },
	{0xFDFF, "PageEnd" }
};

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
