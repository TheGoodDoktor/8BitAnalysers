#include "FredAnalysis.h"
#include "../BBCEmulator.h"



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

void FFredAnalysis::Init(FBBCEmulator* pEmulator)
{
	Name = "Fred";
	SetAnalyser(&pEmulator->GetCodeAnalysis());

	pBBCEmu = pEmulator;
	pCodeAnalyser->IOAnalyser.AddDevice(this);
	pRegConfig = &g_FredRegDrawInfo;
}

void FFredAnalysis::Reset()
{
}

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
