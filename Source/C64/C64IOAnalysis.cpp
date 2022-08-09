#include "C64IOAnalysis.h"
#include "CodeAnalyser/CodeAnalyser.h"


void	FC64IOAnalysis::Init(FCodeAnalysisState* pAnalysis)
{
	pCodeAnalysis = pAnalysis;

	// TODO: set initial VIC register values
	for (int i = 0; i < 64; i++)
	{
		
	}
}

void	FC64IOAnalysis::RegisterIORead(uint16_t addr, uint16_t pc)
{

}

void	FC64IOAnalysis::RegisterIOWrite(uint16_t addr, uint8_t val, uint16_t pc)
{
	// VIC D000 - D3FFF
	if (addr >= 0xd000 && addr < 0xd400)
		RegisterVICWrite(addr & 0x3f, val, pc);
	// SID D400 - D7FFF
	if (addr >= 0xd400 && addr < 0xd800)
		RegisterSIDWrite(addr & 0x1f, val, pc);
}

void	FC64IOAnalysis::RegisterVICRead(uint8_t reg, uint16_t pc)
{

}

void	FC64IOAnalysis::RegisterVICWrite(uint8_t reg, uint8_t val, uint16_t pc)
{
	FC64IORegisterInfo& vicRegister = VICRegisters[reg];
	const uint8_t regChange = vicRegister.LastVal ^ val;	// which bits have changed

	vicRegister.Accesses[pc].WriteVals.insert(val);

	vicRegister.LastVal = val;
}

void	FC64IOAnalysis::RegisterSIDRead(uint8_t reg, uint16_t pc)
{

}

void	FC64IOAnalysis::RegisterSIDWrite(uint8_t reg, uint8_t val, uint16_t pc)
{
}

#include <imgui.h>
#include <CodeAnalyser/CodeAnalyserUI.h>

void DrawRegValueDefault(uint8_t val)
{
	ImGui::Text("\t$%X", val);
}

typedef void(*DrawRegValue)(uint8_t val);


struct FRegDisplayConfig
{
	const char* Name;
	void		(*UIDrawFunction)(uint8_t val);
};

static std::vector<FRegDisplayConfig>	g_VICRegDrawInfo = {

	{"$00 Sprite 0 X",	DrawRegValueDefault},	// 0x00
	{"$01 Sprite 0 Y",	DrawRegValueDefault}, 	// 0x01
	{"$02 Sprite 1 X",	DrawRegValueDefault}, 	// 0x02
	{"$03 Sprite 1 Y",	DrawRegValueDefault}, 	// 0x03
	{"$04 Sprite 2 X",	DrawRegValueDefault}, 	// 0x04
	{"$05 Sprite 2 Y",	DrawRegValueDefault}, 	// 0x05
	{"$06 Sprite 3 X",	DrawRegValueDefault}, 	// 0x06
	{"$07 Sprite 3 Y",	DrawRegValueDefault}, 	// 0x07
	{"$08 Sprite 4 X",	DrawRegValueDefault}, 	// 0x08
	{"$09 Sprite 4 Y",	DrawRegValueDefault}, 	// 0x09
	{"$0a Sprite 5 X",	DrawRegValueDefault}, 	// 0x0a
	{"$0b Sprite 5 Y",	DrawRegValueDefault}, 	// 0x0b
	{"$0c Sprite 6 X",	DrawRegValueDefault}, 	// 0x0c
	{"$0d Sprite 6 Y",	DrawRegValueDefault}, 	// 0x0d
	{"$0e Sprite 7 X",	DrawRegValueDefault}, 	// 0x0e
	{"$0f Sprite 7 Y",	DrawRegValueDefault}, 	// 0x0f
	{"$10 Sprite X MSB",	DrawRegValueDefault},	// 0x10
	{"$11 Screen Ctrl Reg 1",	DrawRegValueDefault},	// 0x11
	{"$12 Raster Line",	DrawRegValueDefault},	// 0x12
	{"$13 Light Pen X",	DrawRegValueDefault},	// 0x13
	{"$14 Light Pen Y",	DrawRegValueDefault},	// 0x14
	{"$15 Sprite Enable",	DrawRegValueDefault},	// 0x15
	{"$16 Screen Ctrl Reg 2",	DrawRegValueDefault},	// 0x16
	{"$17 Sprite Double Height",	DrawRegValueDefault},	// 0x17
	{"$18 Memory Setup",	DrawRegValueDefault},	// 0x18
	{"$19 Interrupt Status Register",	DrawRegValueDefault},	// 0x19
	{"$1a Interrupt Control Register",		DrawRegValueDefault},// 0x1a
	{"$1b Sprite Priority",		DrawRegValueDefault},// 0x1b
	{"$1c Sprite Multi-Colour",		DrawRegValueDefault},// 0x1c
	{"$1d Sprite Double Width",		DrawRegValueDefault},// 0x1d
	{"$1e Sprite-Sprite Collision",		DrawRegValueDefault},// 0x1e
	{"$1f Sprite-Background Collision",	DrawRegValueDefault},	// 0x1f
	{"$20 Border Colour",		DrawRegValueDefault},// 0x20
	{"$21 Background Colour",		DrawRegValueDefault},// 0x21
	{"$22 Extra Background Colour",		DrawRegValueDefault},// 0x22
	{"$23 Extra Background Colour",		DrawRegValueDefault},// 0x23
	{"$24 Extra Background Colour",		DrawRegValueDefault},// 0x24
	{"$25 Sprite Extra Colour 1",		DrawRegValueDefault},// 0x25
	{"$26 Sprite Extra Colour 2",		DrawRegValueDefault},// 0x26
	{"$27 Sprite 0 Colour",		DrawRegValueDefault},// 0x27
	{"$28 Sprite 1 Colour",		DrawRegValueDefault},// 0x28
	{"$29 Sprite 2 Colour",		DrawRegValueDefault},// 0x29
	{"$2a Sprite 3 Colour",		DrawRegValueDefault},// 0x2a
	{"$2b Sprite 4 Colour",		DrawRegValueDefault},// 0x2b
	{"$2c Sprite 5 Colour",		DrawRegValueDefault},// 0x2c
	{"$2d Sprite 6 Colour",		DrawRegValueDefault},// 0x2d
	{"$2e Sprite 7 Colour",		DrawRegValueDefault}// 0x2e

};


void	FC64IOAnalysis::DrawIOAnalysisUI(void)
{
	if (ImGui::BeginTabBar("IO Tab Bar"))
	{
		if (ImGui::BeginTabItem("VIC"))
		{
			if (ImGui::BeginChild("VIC Reg Select",ImVec2(ImGui::GetWindowContentRegionWidth() * 0.5f, 0), true))
			{
				for (int i = 0; i < (int)g_VICRegDrawInfo.size(); i++)
				{
					if (ImGui::Selectable(g_VICRegDrawInfo[i].Name, UIVICRegister == i))
					{
						UIVICRegister = i;
					}
				}
			}
			ImGui::EndChild();
			ImGui::SameLine();
			if (ImGui::BeginChild("VIC Reg Details"))
			{
				if (UIVICRegister != -1)
				{
					FC64IORegisterInfo& vicRegister = VICRegisters[UIVICRegister];
					ImGui::Text("Last Val:");
					g_VICRegDrawInfo[UIVICRegister].UIDrawFunction(vicRegister.LastVal);
					ImGui::Text("Accesses:");
					for (auto& access : vicRegister.Accesses)
					{
						ImGui::Text("$%x", access.first);
						DrawAddressLabel(*pCodeAnalysis, access.first);
						
						ImGui::Text("\tValues:");

						for (auto& val : access.second.WriteVals)
						{
							g_VICRegDrawInfo[UIVICRegister].UIDrawFunction(val);
						}

						//ImGui::Text("0x%x", access.first);
					}
				}
			}
			ImGui::EndChild();
			ImGui::EndTabItem();
		}

		if (ImGui::BeginTabItem("SID"))
		{
			ImGui::EndTabItem();
		}

		if (ImGui::BeginTabItem("CIA1"))
		{
			ImGui::EndTabItem();
		}

		if (ImGui::BeginTabItem("CIA2"))
		{
			ImGui::EndTabItem();
		}

		ImGui::EndTabBar();
	}
}