#include "SheilaAnalysis.h"
#include "../BBCEmulator.h"

static std::vector <FRegDisplayConfig>	g_SheilaRegDrawInfo =
{
	{0xFE00, "CRTC_AddrRegister"},
	{0xFE01, "CRTC_AddrWrite"},

	{0xFE08, "ACIA6850_CtrlStatusReg"},

	{0xFE10, "SerialULA_CtrlReg"},
	{0xFE20, "VideoULA_CtrlReg"},
	{0xFE21, "VideoULA_PaletteReg"},
	{0xFE30, "ROMSelect"},

	{0xFE40, "SystemVIA_RegB"},
	{0xFE41, "SystemVIA_RegA"},
	{0xFE42, "SystemVIA_DDRB"},
	{0xFE43, "SystemVIA_DDRA"},
	{0xFE44, "SystemVIA_Timer1CounterL"},
	{0xFE45, "SystemVIA_Timer1CounterH"},
	{0xFE46, "SystemVIA_Timer1LatchL"},
	{0xFE47, "SystemVIA_Timer1LatchH"},
	{0xFE48, "SystemVIA_Timer2CounterL"},
	{0xFE49, "SystemVIA_Timer2CounterH"},
	{0xFE4A, "SystemVIA_ShiftRegister"},
	{0xFE4B, "SystemVIA_ACR"},
	{0xFE4C, "SystemVIA_PCR"},
	{0xFE4D, "SystemVIA_IFR"},
	{0xFE4E, "SystemVIA_IER"},
	{0xFE4F, "SystemVIA_RegA_NoHandshake"},

	{0xFE60, "UserVIA_RegB"},
	{0xFE61, "UserVIA_RegA"},
	{0xFE62, "UserVIA_DDRB"},
	{0xFE63, "UserVIA_DDRA"},
	{0xFE64, "UserVIA_Timer1CounterL"},
	{0xFE65, "UserVIA_Timer1CounterH"},
	{0xFE66, "UserVIA_Timer1LatchL"},
	{0xFE67, "UserVIA_Timer1LatchH"},
	{0xFE68, "UserVIA_Timer2CounterL"},
	{0xFE69, "UserVIA_Timer2CounterH"},
	{0xFE6A, "UserVIA_ShiftRegister"},
	{0xFE6B, "UserVIA_ACR"},
	{0xFE6C, "UserVIA_PCR"},
	{0xFE6D, "UserVIA_IFR"},
	{0xFE6E, "UserVIA_IER"},
	{0xFE6F, "UserVIA_RegB_NoHandshake"},

	{0xFE80, "8271 FDD - Command/Status"},
	{0xFE81, "8271 FDD - Param/Result"},
	{0xFE84, "8271 FDD - Data Register"},

	{0xFEC0, "ADC_DSR_SCR"},
	{0xFEC1, "ADC_Data_H"},
	{0xFEC2, "ADC_Data_L"},

	{0xFEE0, "TubeULAStatusRegister"},
	{0xFEE1, "TubeULADataRegister"},
};

void FSheilaAnalysis::Init(FBBCEmulator* pEmulator)
{
	Name = "Sheila";
	SetAnalyser(&pEmulator->GetCodeAnalysis());

	pBBCEmu = pEmulator;
	pCodeAnalyser->IOAnalyser.AddDevice(this);
	pRegConfig = &g_SheilaRegDrawInfo;
}

void FSheilaAnalysis::Reset()
{
}

void AddSheilaRegisterLabels(FBBCEmulator* pEmulator)
{
	for (const auto& drawInfo : g_SheilaRegDrawInfo)
	{
		const FAddressRef regAddr(pEmulator->GetBankIds().OSROM, drawInfo.Address);

		FLabelInfo* pLabelInfo = GenerateLabelForAddress(pEmulator->GetCodeAnalysis(), regAddr, ELabelType::Data);
		const std::string labelName = std::string("Sheila_") + drawInfo.Name;
		pLabelInfo->ChangeName(labelName.c_str(), regAddr);
		FDataInfo* pDataInfo = pEmulator->GetCodeAnalysis().GetDataInfoForAddress(regAddr);
		pDataInfo->DisplayType = drawInfo.DisplayType;
	}
}
