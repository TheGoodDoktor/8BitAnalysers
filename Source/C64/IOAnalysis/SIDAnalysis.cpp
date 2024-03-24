#include "SIDAnalysis.h"
#include "CodeAnalyser/CodeAnalyser.h"
#include "../C64Emulator.h"

void SIDWriteEventShowAddress(FCodeAnalysisState& state, const FEvent& event);
void SIDWriteEventShowValue(FCodeAnalysisState& state, const FEvent& event);

FSIDAnalysis* pSID = nullptr; // hack


void	FSIDAnalysis::Init(FC64Emulator* pEmulator)
{
	Name = "SID";
	SetAnalyser(&pEmulator->GetCodeAnalysis());
	pCodeAnalyser->IOAnalyser.AddDevice(this);
	pC64Emu = pEmulator;

	pCodeAnalyser->Debugger.RegisterEventType((uint8_t)EC64Event::SIDRegisterWrite, "SID Write", 0xffff0000, SIDWriteEventShowAddress, SIDWriteEventShowValue);

	pSID = this;
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
	c64_t* pC64 = pC64Emu->GetEmu();
	FC64IORegisterInfo& sidRegister = SIDRegisters[reg];
	const uint8_t regChange = sidRegister.LastVal ^ val;	// which bits have changed

	pCodeAnalyser->Debugger.RegisterEvent((uint8_t)EC64Event::SIDRegisterWrite, pc, reg, val, pC64->vic.rs.v_count);
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
	{"Voice 1 Freq L",			DrawRegValueDecimal},	// 0x00
	{"Voice 1 Freq H",			DrawRegValueDecimal},	// 0x01
	{"Voice 1 Pulse Width L",	DrawRegValueDecimal},	// 0x02
	{"Voice 1 Pulse Width H",	DrawRegValueDecimal},	// 0x03
	{"Voice 1 Control",		DrawRegValueDecimal},	// 0x04
	{"Voice 1 AD",			DrawRegValueDecimal},	// 0x05
	{"Voice 1 SR",			DrawRegValueDecimal},	// 0x06
	{"Voice 2 Freq L",			DrawRegValueDecimal},	// 0x07
	{"Voice 2 Freq H",			DrawRegValueDecimal},	// 0x08
	{"Voice 2 Pulse Width L",	DrawRegValueDecimal},	// 0x09
	{"Voice 2 Pulse Width H",	DrawRegValueDecimal},	// 0x0a
	{"Voice 2 Control",		DrawRegValueDecimal},	// 0x0b
	{"Voice 2 AD",			DrawRegValueDecimal},	// 0x0c
	{"Voice 2 SR",			DrawRegValueDecimal},	// 0x0d
	{"Voice 3 Freq L",			DrawRegValueDecimal},	// 0x0e
	{"Voice 3 Freq H",			DrawRegValueDecimal},	// 0x0f
	{"Voice 3 Pulse Width L",	DrawRegValueDecimal},	// 0x10
	{"Voice 3 Pulse Width H",	DrawRegValueDecimal},	// 0x11
	{"Voice 3 Control",		DrawRegValueDecimal},	// 0x12
	{"Voice 3 AD",			DrawRegValueDecimal},	// 0x13
	{"Voice 3 SR",			DrawRegValueDecimal},	// 0x14
	{"Filter Cut Off L",		DrawRegValueDecimal},	// 0x15
	{"Filter Cut Off H",		DrawRegValueDecimal},	// 0x16
	{"Filter Control",		DrawRegValueDecimal},	// 0x17
	{"Vol And Filter Modes",	DrawRegValueDecimal},	// 0x18
	{"Paddle X",				DrawRegValueDecimal},	// 0x19
	{"Paddle Y",				DrawRegValueDecimal},	// 0x1a
	{"Voice 3 Waveform",		DrawRegValueDecimal},	// 0x1b
	{"Voice 3 ADSR Out",		DrawRegValueDecimal},	// 0x1c
	{"Unused1",				DrawRegValueDecimal},	// 0x1d
	{"Unused2",				DrawRegValueDecimal},	// 0x1e
	{"Unused3",				DrawRegValueDecimal},	// 0x1f
};

void SIDWriteEventShowAddress(FCodeAnalysisState& state, const FEvent& event)
{
	ImGui::Text("%s", g_SIDRegDrawInfo[event.Address].Name);
}

void SIDWriteEventShowValue(FCodeAnalysisState& state, const FEvent& event)
{

	g_SIDRegDrawInfo[event.Address].UIDrawFunction(pSID, event.Value);
}


void	FSIDAnalysis::DrawDetailsUI(void)
{
	if (ImGui::BeginChild("SID Reg Select", ImVec2(ImGui::GetContentRegionAvail().x * 0.5f, 0), true))
	{
		SelectedRegister = DrawRegSelectList(g_SIDRegDrawInfo, SelectedRegister);
	}
	ImGui::EndChild();
	ImGui::SameLine();
	if (ImGui::BeginChild("SID Reg Details"))
	{
		if (SelectedRegister != -1)
		{
			DrawRegDetails(this, SIDRegisters[SelectedRegister], g_SIDRegDrawInfo[SelectedRegister], pCodeAnalyser);
		}
	}
	ImGui::EndChild();
}

std::string GetSIDLabelName(int regNo)
{
	const int reg = regNo & 0x1f;
	const int mirrorNo = regNo >> 8;
	const std::string displayName = g_SIDRegDrawInfo[reg].Name;
	std::string labelName = "SID_";

	if(mirrorNo > 0)
	{
		labelName += "Mirror";
		labelName += '0' + mirrorNo;
		labelName += '_';
	}

	for (auto ch : displayName)
	{
		if (ch != ' ')
			labelName += ch;
	}

	

	return labelName;
}

void AddSIDRegisterLabels(FCodeAnalysisPage& IOPage)
{
	std::vector<FRegDisplayConfig>& regList = g_SIDRegDrawInfo;

	for(int mirrorNo =0;mirrorNo<4;mirrorNo++)
	{
		const int mirrorOffset = mirrorNo * 0x100;
		
		for (int reg = 0; reg < (int)regList.size(); reg++)
		{
			const int addr = reg + mirrorOffset;
			IOPage.SetLabelAtAddress(GetSIDLabelName(addr).c_str(), ELabelType::Data, addr, true);
			IOPage.DataInfo[addr].DisplayType = regList[reg].DisplayType;
		}
	}

}