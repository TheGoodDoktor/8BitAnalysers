#include "FredAnalysis.h"
#include "../BBCEmulator.h"

void FFredAnalysis::Init(FBBCEmulator* pEmulator)
{
	pBBCEmu = pEmulator;
}

void FFredAnalysis::Reset()
{
}

void FFredAnalysis::OnRegisterRead(uint8_t reg, FAddressRef pc)
{
}

void FFredAnalysis::OnRegisterWrite(uint8_t reg, uint8_t val, FAddressRef pc)
{
}

void FFredAnalysis::DrawDetailsUI(void)
{
}

static std::vector <FRegDisplayConfig>	g_FredRegDrawInfo = 
{
	{0xFC00, "Test_Hardware"},
	{0xFC10, "Teletext"},
	{0xFC14, "Prestel"},
	{0xFC20, "IEEE_488_Interface"},
	{0xFC30, "Cambridge_Ring_Interface"},
	{0xFC40, "Winchester_Disc"},
	{0xFC48, "Reserved"},
	{0xFC60, "Serial_Expansion"},
	{0xFC80, "Test_Hardware"},
	{0xFCFE, "Jim_Paging_Register_MSB"},
	{0xFCFF, "Jim_Paging_Register_LSB"}
};

void AddFredRegisterLabels(FBBCEmulator* pEmulator)
{
	for (const auto& drawInfo : g_FredRegDrawInfo)
	{
		const FAddressRef regAddr(pEmulator->GetBankIds().OSROM, drawInfo.Address);
		
		FLabelInfo* pLabelInfo = GenerateLabelForAddress(pEmulator->GetCodeAnalysis(), regAddr, ELabelType::Data);
		const std::string labelName = std::string("Fred_") + drawInfo.Name;
		pLabelInfo->ChangeName(labelName.c_str(), regAddr);

		FDataInfo* pDataInfo = pEmulator->GetCodeAnalysis().GetDataInfoForAddress(regAddr);
		pDataInfo->DisplayType = drawInfo.DisplayType;
	}
}
