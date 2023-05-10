#include "CIAAnalysis.h"
#include <vector>
#include <imgui.h>
#include <CodeAnalyser/CodeAnalysisPage.h>

static std::vector<FRegDisplayConfig>	g_CIA1RegDrawInfo =
{
	{"CIA1_KeyBJoy2",			DrawRegValueHex},	// 0x00
	{"CIA1_KeyBJoy1",			DrawRegValueHex},	// 0x01
	{"CIA1_PortA_DDR",			DrawRegValueHex},	// 0x02
	{"CIA1_PortB_DDR",			DrawRegValueHex},	// 0x03
	{"CIA1_TimerA_L",			DrawRegValueHex},	// 0x04
	{"CIA1_TimerA_H",			DrawRegValueHex},	// 0x05
	{"CIA1_TimerB_L",			DrawRegValueHex},	// 0x06
	{"CIA1_TimerB_H",			DrawRegValueHex},	// 0x07
	{"CIA1_TOD_dsecs",			DrawRegValueHex},	// 0x08
	{"CIA1_TOD_secs",			DrawRegValueHex},	// 0x09
	{"CIA1_TOD_mins",			DrawRegValueHex},	// 0x0a
	{"CIA1_TOD_hours",			DrawRegValueHex},	// 0x0b
	{"CIA1_SerialShift",		DrawRegValueHex},	// 0x0c
	{"CIA1_InterruptCS",		DrawRegValueHex},	// 0x0d
	{"CIA1_TimerA_CR",			DrawRegValueHex},	// 0x0e
	{"CIA1_TimerB_CR",			DrawRegValueHex},	// 0x0f
};

static std::vector<FRegDisplayConfig>	g_CIA2RegDrawInfo =
{
	{"CIA2_PortA_Serial",		DrawRegValueHex},	// 0x00
	{"CIA2_PortB_RS232",		DrawRegValueHex},	// 0x01
	{"CIA2_PortA_DDR",			DrawRegValueHex},	// 0x02
	{"CIA2_PortB_DDR",			DrawRegValueHex},	// 0x03
	{"CIA2_TimerA_L",			DrawRegValueHex},	// 0x04
	{"CIA2_TimerA_H",			DrawRegValueHex},	// 0x05
	{"CIA2_TimerB_L",			DrawRegValueHex},	// 0x06
	{"CIA2_TimerB_H",			DrawRegValueHex},	// 0x07
	{"CIA2_TOD_dsecs",			DrawRegValueHex},	// 0x08
	{"CIA2_TOD_secs",			DrawRegValueHex},	// 0x09
	{"CIA2_TOD_mins",			DrawRegValueHex},	// 0x0a
	{"CIA2_TOD_hours",			DrawRegValueHex},	// 0x0b
	{"CIA2_SerialShift",		DrawRegValueHex},	// 0x0c
	{"CIA2_InterruptCS",		DrawRegValueHex},	// 0x0d
	{"CIA2_TimerA_Control",		DrawRegValueHex},	// 0x0e
	{"CIA2_TimerB_Control",		DrawRegValueHex},	// 0x0f
};

FCIA1Analysis::FCIA1Analysis()
{
	RegConfig = &g_CIA1RegDrawInfo;
}

FCIA2Analysis::FCIA2Analysis()
{
	RegConfig = &g_CIA2RegDrawInfo;

}

void	FCIAAnalysis::Init(FCodeAnalysisState* pAnalysis)
{
	pCodeAnalysis = pAnalysis;
}

void FCIAAnalysis::Reset(void)
{
	for (int i = 0; i < kNoRegisters; i++)
		CIARegisters[i].Reset();
}

void	FCIAAnalysis::OnRegisterRead(uint8_t reg, uint16_t pc)
{

}
void	FCIAAnalysis::OnRegisterWrite(uint8_t reg, uint8_t val, uint16_t pc)
{
	FC64IORegisterInfo& ciaRegister = CIARegisters[reg];
	const uint8_t regChange = ciaRegister.LastVal ^ val;	// which bits have changed

	ciaRegister.Accesses[pc].WriteVals.insert(val);

	ciaRegister.LastVal = val;
}

void	FCIAAnalysis::DrawUI(void)
{
	if (ImGui::BeginChild("CIA Reg Select", ImVec2(ImGui::GetWindowContentRegionWidth() * 0.5f, 0), true))
	{
		SelectedRegister = DrawRegSelectList(*RegConfig, SelectedRegister);
	}
	ImGui::EndChild();
	ImGui::SameLine();
	if (ImGui::BeginChild("VIC Reg Details"))
	{
		if (SelectedRegister != -1)
		{
			DrawRegDetails(CIARegisters[SelectedRegister], RegConfig->at(SelectedRegister), pCodeAnalysis);
		}
	}
	ImGui::EndChild();
}

void AddCIARegisterLabels(FCodeAnalysisPage& IOPage)
{
	// CIA 1 -$DC00 - $DC0F
	std::vector<FRegDisplayConfig>& CIA1RegList = g_CIA1RegDrawInfo;

	for (int reg = 0; reg < (int)CIA1RegList.size(); reg++)
		IOPage.SetLabelAtAddress(CIA1RegList[reg].Name, ELabelType::Data, reg);

	// CIA 2 -$DD00 - $DD0F
	std::vector<FRegDisplayConfig>& CIA2RegList = g_CIA1RegDrawInfo;

	for (int reg = 0; reg < (int)CIA2RegList.size(); reg++)
		IOPage.SetLabelAtAddress(CIA2RegList[reg].Name, ELabelType::Data, reg + 0x100);	// offset by 256 bytes

}