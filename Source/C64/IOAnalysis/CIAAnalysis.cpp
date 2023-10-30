#include "CIAAnalysis.h"
#include <vector>
#include <imgui.h>
#include <CodeAnalyser/CodeAnalyser.h>
#include <CodeAnalyser/CodeAnalysisPage.h>
#include "../C64Emulator.h"


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

FCIA1Analysis* pCIA1 = nullptr;
FCIA2Analysis* pCIA2 = nullptr;

FCIA1Analysis::FCIA1Analysis()
{
	Name = "CIA1";
	RegConfig = &g_CIA1RegDrawInfo;
	WriteEventType = (uint8_t)EC64Event::CIA1RegisterWrite;
	ReadEventType = (uint8_t)EC64Event::CIA1RegisterRead;
	pCIA1 = this;
}

FCIA2Analysis::FCIA2Analysis()
{
	Name = "CIA2";
	RegConfig = &g_CIA2RegDrawInfo;
	WriteEventType = (uint8_t)EC64Event::CIA2RegisterWrite;
	ReadEventType = (uint8_t)EC64Event::CIA2RegisterRead;
	pCIA2 = this;
}

void CIA1WriteEventShowAddress(FCodeAnalysisState& state, const FEvent& event)
{
	ImGui::Text("%s", g_CIA1RegDrawInfo[event.Address].Name);
}

void CIA1WriteEventShowValue(FCodeAnalysisState& state, const FEvent& event)
{
	g_CIA1RegDrawInfo[event.Address].UIDrawFunction(pCIA1, event.Value);
}

void CIA2WriteEventShowAddress(FCodeAnalysisState& state, const FEvent& event)
{
	ImGui::Text("%s", g_CIA2RegDrawInfo[event.Address].Name);
}

void CIA2WriteEventShowValue(FCodeAnalysisState& state, const FEvent& event)
{
	g_CIA2RegDrawInfo[event.Address].UIDrawFunction(pCIA2, event.Value);
}


void	FCIAAnalysis::Init(FC64Emulator* pEmulator)
{
	SetAnalyser(&pEmulator->GetCodeAnalysis());
	pCodeAnalyser->IOAnalyser.AddDevice(this);

	pCodeAnalyser->Debugger.RegisterEventType((uint8_t)EC64Event::CIA1RegisterWrite, "CIA1 Write", 0xff00ff00, CIA1WriteEventShowAddress, CIA1WriteEventShowValue);
	pCodeAnalyser->Debugger.RegisterEventType((uint8_t)EC64Event::CIA1RegisterRead, "CIA1 Read", 0xff007f00, CIA1WriteEventShowAddress, CIA1WriteEventShowValue);
	pCodeAnalyser->Debugger.RegisterEventType((uint8_t)EC64Event::CIA2RegisterWrite, "CIA2 Write", 0xff00ff00, CIA2WriteEventShowAddress, CIA2WriteEventShowValue);
	pCodeAnalyser->Debugger.RegisterEventType((uint8_t)EC64Event::CIA2RegisterRead, "CIA2 Read", 0xff007f00, CIA2WriteEventShowAddress, CIA2WriteEventShowValue);

	pC64Emu = pEmulator;
}

void FCIAAnalysis::Reset(void)
{
	for (int i = 0; i < kNoRegisters; i++)
		CIARegisters[i].Reset();
}

void	FCIAAnalysis::OnRegisterRead(uint8_t reg, FAddressRef pc)
{
	c64_t* pC64 = pC64Emu->GetEmu();
	const uint8_t val = M6526_GET_DATA(pC64->cia_1.pins);	// Not sure if this is correct
	pCodeAnalyser->Debugger.RegisterEvent(ReadEventType, pc, reg, val, pC64->vic.rs.v_count);

}
void	FCIAAnalysis::OnRegisterWrite(uint8_t reg, uint8_t val, FAddressRef pc)
{
	c64_t* pC64 = pC64Emu->GetEmu();
	FC64IORegisterInfo& ciaRegister = CIARegisters[reg];
	const uint8_t regChange = ciaRegister.LastVal ^ val;	// which bits have changed

	pCodeAnalyser->Debugger.RegisterEvent(WriteEventType, pc, reg, val, pC64->vic.rs.v_count);

	ciaRegister.Accesses[pc].WriteVals.insert(val);

	ciaRegister.LastVal = val;
}

void	FCIAAnalysis::DrawDetailsUI(void)
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
			DrawRegDetails(this, CIARegisters[SelectedRegister], RegConfig->at(SelectedRegister), pCodeAnalyser);
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