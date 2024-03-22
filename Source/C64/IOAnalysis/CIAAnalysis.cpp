#include "CIAAnalysis.h"
#include <vector>
#include <imgui.h>
#include <CodeAnalyser/CodeAnalyser.h>
#include <CodeAnalyser/CodeAnalysisPage.h>
#include "../C64Emulator.h"


static std::vector<FRegDisplayConfig>	g_CIA1RegDrawInfo =
{
	{"CIA1 PortA KeyB/Joy2",			DrawRegValueHex},	// 0x00
	{"CIA1 PortB KeyB/Joy1",			DrawRegValueHex},	// 0x01
	{"CIA1 PortA DDR",			DrawRegValueHex},	// 0x02
	{"CIA1 PortB DDR",			DrawRegValueHex},	// 0x03
	{"CIA1 TimerA L",			DrawRegValueHex},	// 0x04
	{"CIA1 TimerA H",			DrawRegValueHex},	// 0x05
	{"CIA1 TimerB L",			DrawRegValueHex},	// 0x06
	{"CIA1 TimerB H",			DrawRegValueHex},	// 0x07
	{"CIA1 TOD Dsecs",			DrawRegValueHex},	// 0x08
	{"CIA1 TOD Secs",			DrawRegValueHex},	// 0x09
	{"CIA1 TOD Mins",			DrawRegValueHex},	// 0x0a
	{"CIA1 TOD Hours",			DrawRegValueHex},	// 0x0b
	{"CIA1 SerialShift",		DrawRegValueHex},	// 0x0c
	{"CIA1 InterruptCS",		DrawRegValueHex},	// 0x0d
	{"CIA1 TimerA Control",			DrawRegValueHex},	// 0x0e
	{"CIA1 TimerB Control",			DrawRegValueHex},	// 0x0f
};

static const char* CIA1PortABits[]
{
	"Keyboard Matrix Column 0",
	"Keyboard Matrix Column 1",
	"Keyboard Matrix Column 2",
	"Keyboard Matrix Column 3",
	"Keyboard Matrix Column 4",
	"Keyboard Matrix Column 5",
	"Keyboard Matrix Column 6",
	"Keyboard Matrix Column 7",
};

static const char* CIA1PortBBits[]
{
	"Keyboard Matrix Row 0",
	"Keyboard Matrix Row 1",
	"Keyboard Matrix Row 2",
	"Keyboard Matrix Row 3",
	"Keyboard Matrix Row 4",
	"Keyboard Matrix Row 5",
	"Keyboard Matrix Row 6",
	"Keyboard Matrix Row 7",
};

static std::vector<FRegDisplayConfig>	g_CIA2RegDrawInfo =
{
	{"CIA2 PortA Serial",		DrawRegValueHex},	// 0x00
	{"CIA2 PortB RS232",		DrawRegValueHex},	// 0x01
	{"CIA2 PortA DDR",			DrawRegValueHex},	// 0x02
	{"CIA2 PortB DDR",			DrawRegValueHex},	// 0x03
	{"CIA2 TimerA L",			DrawRegValueHex},	// 0x04
	{"CIA2 TimerA H",			DrawRegValueHex},	// 0x05
	{"CIA2 TimerB L",			DrawRegValueHex},	// 0x06
	{"CIA2 TimerB H",			DrawRegValueHex},	// 0x07
	{"CIA2 TOD dsecs",			DrawRegValueHex},	// 0x08
	{"CIA2 TOD secs",			DrawRegValueHex},	// 0x09
	{"CIA2 TOD mins",			DrawRegValueHex},	// 0x0a
	{"CIA2 TOD hours",			DrawRegValueHex},	// 0x0b
	{"CIA2 Serial Shift",		DrawRegValueHex},	// 0x0c
	{"CIA2 Interrupt CS",		DrawRegValueHex},	// 0x0d
	{"CIA2 TimerA Control",		DrawRegValueHex},	// 0x0e
	{"CIA2 TimerB Control",		DrawRegValueHex},	// 0x0f
};

static const char* CIA2PortABits[]
{
	"VIC Bank Select bit 0",
	"VIC Bank Select bit 1",
	"RS232 TXD Output",
	"Serial ATN Out",
	"Serial Clock Out",
	"Serial Data Out",
	"Serial Clock In",
	"Serial Data In",
};

static const char* CIA2PortBBits[]
{
	"User Port 0/RS232: RXD",
	"User Port 1/RS232: RTS",
	"User Port 2/RS232: DTR",
	"User Port 3/RS232: RI",
	"User Port 4/RS232: DCD",
	"User Port 5",
	"User Port 6/RS232: CTS",
	"User Port 7/RS232: DSR",
};

FCIA1Analysis* pCIA1 = nullptr;
FCIA2Analysis* pCIA2 = nullptr;

FCIA1Analysis::FCIA1Analysis()
{
	Name = "CIA1";
	RegConfig = &g_CIA1RegDrawInfo;
	WriteEventType = (uint8_t)EC64Event::CIA1RegisterWrite;
	ReadEventType = (uint8_t)EC64Event::CIA1RegisterRead;
	PortABitNames = CIA1PortABits;
	PortBBitNames = CIA1PortBBits;
	pCIA1 = this;
}

const m6526_t* FCIA1Analysis::GetCIA()
{
	c64_t* pC64 = pC64Emu->GetEmu();
	return &pC64->cia_1;
}

FCIA2Analysis::FCIA2Analysis()
{
	Name = "CIA2";
	RegConfig = &g_CIA2RegDrawInfo;
	WriteEventType = (uint8_t)EC64Event::CIA2RegisterWrite;
	ReadEventType = (uint8_t)EC64Event::CIA2RegisterRead;
	PortABitNames = CIA2PortABits;
	PortBBitNames = CIA2PortBBits;
	pCIA2 = this;
}

const m6526_t* FCIA2Analysis::GetCIA()
{
	c64_t* pC64 = pC64Emu->GetEmu();
	return &pC64->cia_2;
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

void DrawPortState(const m6526_port_t& port, const char** portNames)
{
	ImDrawList* dl = ImGui::GetWindowDrawList();
	const float lineHeight = ImGui::GetTextLineHeight();
	const float rectSize = lineHeight;

	for (int portBit = 0; portBit < 8; portBit++)
	{
		ImVec2 pos = ImGui::GetCursorScreenPos();
		ImVec2 rectMin(pos.x, pos.y + 3);
		ImVec2 rectMax(pos.x + rectSize, pos.y + rectSize + 3);
		
		// Data direction register
		const uint32_t ddCol = (port.ddr & (1<< portBit)) ? 0xff0000ff : 0xff00ff00;
		dl->AddRectFilled(rectMin, rectMax, ddCol);
		rectMin.x += rectSize + 4;
		rectMax.x += rectSize + 4;

		// port pins
		if(port.pins & (1 << portBit))
			dl->AddRectFilled(rectMin, rectMax, 0xffffffff);
		else
			dl->AddRect(rectMin, rectMax, 0xffffffff);
		rectMin.x += rectSize + 4;
		rectMax.x += rectSize + 4;

		// input latch
		if (port.inp & (1 << portBit))
			dl->AddRectFilled(rectMin, rectMax, 0xffffffff);
		else
			dl->AddRect(rectMin, rectMax, 0xffffffff);
		rectMin.x += rectSize + 4;
		rectMax.x += rectSize + 4;

		// reg
		if (port.reg & (1 << portBit))
			dl->AddRectFilled(rectMin, rectMax, 0xffffffff);
		else
			dl->AddRect(rectMin, rectMax, 0xffffffff);
		rectMin.x += rectSize + 4;
		rectMax.x += rectSize + 4;

		// port bit title
		ImGui::SetCursorScreenPos(rectMin);
		ImGui::Text("%s",portNames[portBit]);

	}
}

void	FCIAAnalysis::DrawDetailsUI(void)
{
	c64_t* pC64 = pC64Emu->GetEmu();

	const m6526_t* pCIA = GetCIA();
	const float lineHeight = ImGui::GetTextLineHeight();

	//ImVec2 pos = ImGui::GetCursorScreenPos();
	if(ImGui::BeginChild("CIA Ports", ImVec2(0, lineHeight * 15)))
	{
		if(ImGui::BeginChild("PortAChild", ImVec2(ImGui::GetContentRegionAvail().x * 0.5f, 0)))
		{
			ImGui::Text("Port A");
			DrawPortState(pCIA->pa, PortABitNames);
		}
		ImGui::EndChild();
		ImGui::SameLine();
		if (ImGui::BeginChild("PortBChild"))
		{
			ImGui::Text("Port B");
			DrawPortState(pCIA->pb, PortBBitNames);
		}
		ImGui::EndChild();
	}
	ImGui::EndChild();

#if 0
	const ENumberDisplayMode numberMode = GetNumberDisplayMode();
	SetNumberDisplayMode(ENumberDisplayMode::Binary);
	ImGui::Text("Port A");
	ImGui::Text("DDR:  \t %s", NumStr(pCIA->pa.ddr));
	ImGui::Text("Pins: \t %s", NumStr(pCIA->pa.pins));
	ImGui::Text("Reg:  \t %s", NumStr(pCIA->pa.reg));
	ImGui::Text("Latch:\t %s",NumStr(pCIA->pa.inp));
	ImGui::Text("Port B");
	ImGui::Text("DDR:  \t %s", NumStr(pCIA->pb.ddr));
	ImGui::Text("Pins: \t %s", NumStr(pCIA->pb.pins));
	ImGui::Text("Reg:  \t %s", NumStr(pCIA->pb.reg));
	ImGui::Text("Latch:\t %s", NumStr(pCIA->pb.inp));
	SetNumberDisplayMode(numberMode);
#endif
	if (ImGui::BeginChild("CIA Reg Select", ImVec2(ImGui::GetContentRegionAvail().x * 0.5f, 0), true))
	{
		SelectedRegister = DrawRegSelectList(*RegConfig, SelectedRegister);
	}
	ImGui::EndChild();
	ImGui::SameLine();
	if (ImGui::BeginChild("CIA Reg Details"))
	{
		if (SelectedRegister != -1)
		{
			DrawRegDetails(this, CIARegisters[SelectedRegister], RegConfig->at(SelectedRegister), pCodeAnalyser);
		}
	}
	ImGui::EndChild();
}

std::string GetCIALabelName(int ciaNo, int reg)
{
	std::vector<FRegDisplayConfig>& regList = (ciaNo == 1) ? g_CIA1RegDrawInfo : g_CIA2RegDrawInfo;
	std::string displayName = regList[reg].Name;
	std::string labelName;

	for (auto ch : displayName)
	{
		if (ch != ' ')
			labelName += ch;
	}

	return labelName;
}

void AddCIARegisterLabels(FCodeAnalysisPage& IOPage)
{
	// CIA 1 -$DC00 - $DC0F
	std::vector<FRegDisplayConfig>& CIA1RegList = g_CIA1RegDrawInfo;

	for (int reg = 0; reg < (int)CIA1RegList.size(); reg++)
		IOPage.SetLabelAtAddress(GetCIALabelName(1,reg).c_str(), ELabelType::Data, reg, true);

	// CIA 2 -$DD00 - $DD0F
	std::vector<FRegDisplayConfig>& CIA2RegList = g_CIA1RegDrawInfo;

	for (int reg = 0; reg < (int)CIA2RegList.size(); reg++)
		IOPage.SetLabelAtAddress(GetCIALabelName(2, reg).c_str(), ELabelType::Data, reg + 0x100, true);	// offset by 256 bytes

}