#include "SIDAnalysis.h"
#include "CodeAnalyser/CodeAnalyser.h"
void	FSIDAnalysis::Init(FCodeAnalysisState* pAnalysis)
{
	Name = "SID";
	SetAnalyser(pAnalysis);
	pAnalysis->IOAnalyser.AddDevice(this);
}

void FSIDAnalysis::Reset(void)
{
	for (int i = 0; i < kNoRegisters; i++)
		SIDRegisters[i].Reset();
}

void	FSIDAnalysis::OnRegisterRead(uint8_t reg, FAddressRef pc)
{

}
void	FSIDAnalysis::OnRegisterWrite(uint8_t reg, uint8_t val, FAddressRef pc)
{
	FC64IORegisterInfo& sidRegister = SIDRegisters[reg];
	const uint8_t regChange = sidRegister.LastVal ^ val;	// which bits have changed

	sidRegister.Accesses[pc].WriteVals.insert(val);

	sidRegister.LastVal = val;
}

#include <imgui.h>
#include <CodeAnalyser/UI/CodeAnalyserUI.h>
#include <chips/chips_common.h>
#include <chips/m6569.h>
#include <vector>
#include <CodeAnalyser/CodeAnalysisPage.h>

static std::vector<FRegDisplayConfig>	g_SIDRegDrawInfo =
{
	{"SID_Voice1FreqL",			DrawRegValueDecimal},	// 0x00
	{"SID_Voice1FreqH",			DrawRegValueDecimal},	// 0x01
	{"SID_Voice1PulseWidthL",	DrawRegValueDecimal},	// 0x02
	{"SID_Voice1PulseWidthH",	DrawRegValueDecimal},	// 0x03
	{"SID_Voice1Control",		DrawRegValueDecimal},	// 0x04
	{"SID_Voice1AD",			DrawRegValueDecimal},	// 0x05
	{"SID_Voice1SR",			DrawRegValueDecimal},	// 0x06
	{"SID_Voice2FreqL",			DrawRegValueDecimal},	// 0x07
	{"SID_Voice2FreqH",			DrawRegValueDecimal},	// 0x08
	{"SID_Voice2PulseWidthL",	DrawRegValueDecimal},	// 0x09
	{"SID_Voice2PulseWidthH",	DrawRegValueDecimal},	// 0x0a
	{"SID_Voice2Control",		DrawRegValueDecimal},	// 0x0b
	{"SID_Voice2AD",			DrawRegValueDecimal},	// 0x0c
	{"SID_Voice2SR",			DrawRegValueDecimal},	// 0x0d
	{"SID_Voice3FreqL",			DrawRegValueDecimal},	// 0x0e
	{"SID_Voice3FreqH",			DrawRegValueDecimal},	// 0x0f
	{"SID_Voice3PulseWidthL",	DrawRegValueDecimal},	// 0x10
	{"SID_Voice3PulseWidthH",	DrawRegValueDecimal},	// 0x11
	{"SID_Voice3Control",		DrawRegValueDecimal},	// 0x12
	{"SID_Voice3AD",			DrawRegValueDecimal},	// 0x13
	{"SID_Voice3SR",			DrawRegValueDecimal},	// 0x14
	{"SID_FilterCutOffL",		DrawRegValueDecimal},	// 0x15
	{"SID_FilterCutOffH",		DrawRegValueDecimal},	// 0x16
	{"SID_FilterControl",		DrawRegValueDecimal},	// 0x17
	{"SID_VolAndFilterModes",	DrawRegValueDecimal},	// 0x18
	{"SID_PaddleX",				DrawRegValueDecimal},	// 0x19
	{"SID_PaddleY",				DrawRegValueDecimal},	// 0x1a
	{"SID_Voice3Waveform",		DrawRegValueDecimal},	// 0x1b
	{"SID_Voice3ADSROut",		DrawRegValueDecimal},	// 0x1c
	{"SID_Unused1",				DrawRegValueDecimal},	// 0x1d
	{"SID_Unused2",				DrawRegValueDecimal},	// 0x1e
	{"SID_Unused3",				DrawRegValueDecimal},	// 0x1f
};


void	FSIDAnalysis::DrawDetailsUI(void)
{
	if (ImGui::BeginChild("VIC Reg Select", ImVec2(ImGui::GetWindowContentRegionWidth() * 0.5f, 0), true))
	{
		SelectedRegister = DrawRegSelectList(g_SIDRegDrawInfo, SelectedRegister);
	}
	ImGui::EndChild();
	ImGui::SameLine();
	if (ImGui::BeginChild("VIC Reg Details"))
	{
		if (SelectedRegister != -1)
		{
			DrawRegDetails(SIDRegisters[SelectedRegister], g_SIDRegDrawInfo[SelectedRegister], pCodeAnalyser);
		}
	}
	ImGui::EndChild();
}

void AddSIDRegisterLabels(FCodeAnalysisPage& IOPage)
{
	std::vector<FRegDisplayConfig>& regList = g_SIDRegDrawInfo;

	for (int reg = 0; reg < (int)regList.size(); reg++)
		IOPage.SetLabelAtAddress(regList[reg].Name, ELabelType::Data, reg);

}