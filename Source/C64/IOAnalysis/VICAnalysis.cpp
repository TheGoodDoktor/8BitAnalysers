#include "VICAnalysis.h"

void FVICAnalysis::Init(FCodeAnalysisState* pAnalysis)
{
	pCodeAnalysis = pAnalysis;

}

void FVICAnalysis::OnRegisterRead(uint8_t reg, uint16_t pc)
{

}

void FVICAnalysis::OnRegisterWrite(uint8_t reg, uint8_t val, uint16_t pc)
{
	FC64IORegisterInfo& vicRegister = VICRegisters[reg];
	const uint8_t regChange = vicRegister.LastVal ^ val;	// which bits have changed

	vicRegister.Accesses[pc].WriteVals.insert(val);

	vicRegister.LastVal = val;
}

#include <imgui.h>
#include <CodeAnalyser/CodeAnalyserUI.h>
#include <chips/m6569.h>
#include <vector>

void DrawRegValueSpriteEnable(uint8_t val)
{
	for (int i = 0; i < 8; i++)
	{
		ImGui::Text("%d:%s ", i, ((val >> i) & 1) ? "Y" : "N");
		if (i < 7)
			ImGui::SameLine();
	}
}

void DrawRegValueColour(uint8_t val)
{
	ImVec4 c;
	const ImVec2 size(18, 18);
	c = ImColor(m6569_color(val & 15));
	ImGui::ColorButton("##hw_color", c, ImGuiColorEditFlags_NoAlpha, size);
}

/*
Screen control register #1. Bits:

Bits #0-#2: Vertical raster scroll.

Bit #3: Screen height; 0 = 24 rows; 1 = 25 rows.

Bit #4: 0 = Screen off, complete screen is covered by border; 1 = Screen on, normal screen contents are visible.

Bit #5: 0 = Text mode; 1 = Bitmap mode.

Bit #6: 1 = Extended background mode on.

Bit #7: Read: Current raster line (bit #8).
Write: Raster line to generate interrupt at (bit #8).
*/

void DrawRegValueScreenControlReg1(uint8_t val)
{
	ImGui::Text("VScroll:%d, Height:%d, Scr:%s, %s, ExtBG:%s, RastMSB:%d",
		val & 7,
		val & (1 << 3) ? 25 : 24,
		val & (1 << 4) ? "ON" : "OFF",
		val & (1 << 5) ? "BMP" : "TXT",
		val & (1 << 6) ? "ON" : "OFF",
		val & (1 << 7) ? 1 : 0);
}

/*
Screen control register #2. Bits:

Bits #0-#2: Horizontal raster scroll.

Bit #3: Screen width; 0 = 38 columns; 1 = 40 columns.

Bit #4: 1 = Multicolor mode on.
*/

void DrawRegValueScreenControlReg2(uint8_t val)
{
	ImGui::Text("HScroll:%d, Width:%d, MultiColour:%s",
		val & 7,
		val & (1 << 3) ? 40 : 38,
		val & (1 << 4) ? "ON" : "OFF");

}

void DrawRegValueMemorySetup(uint8_t val)
{
	ImGui::Text("\t$%X", val);

}

static std::vector<FRegDisplayConfig>	g_VICRegDrawInfo = {

	{"Sprite 0 X",	DrawRegValueDecimal},	// 0x00
	{"Sprite 0 Y",	DrawRegValueDecimal}, 	// 0x01
	{"Sprite 1 X",	DrawRegValueDecimal}, 	// 0x02
	{"Sprite 1 Y",	DrawRegValueDecimal}, 	// 0x03
	{"Sprite 2 X",	DrawRegValueDecimal}, 	// 0x04
	{"Sprite 2 Y",	DrawRegValueDecimal}, 	// 0x05
	{"Sprite 3 X",	DrawRegValueDecimal}, 	// 0x06
	{"Sprite 3 Y",	DrawRegValueDecimal}, 	// 0x07
	{"Sprite 4 X",	DrawRegValueDecimal}, 	// 0x08
	{"Sprite 4 Y",	DrawRegValueDecimal}, 	// 0x09
	{"Sprite 5 X",	DrawRegValueDecimal}, 	// 0x0a
	{"Sprite 5 Y",	DrawRegValueDecimal}, 	// 0x0b
	{"Sprite 6 X",	DrawRegValueDecimal}, 	// 0x0c
	{"Sprite 6 Y",	DrawRegValueDecimal}, 	// 0x0d
	{"Sprite 7 X",	DrawRegValueDecimal}, 	// 0x0e
	{"Sprite 7 Y",	DrawRegValueDecimal}, 	// 0x0f
	{"Sprite X MSB",	DrawRegValueDefault},	// 0x10
	{"Screen Ctrl Reg 1",	DrawRegValueScreenControlReg1},	// 0x11
	{"Raster Line",	DrawRegValueDecimal},	// 0x12
	{"Light Pen X",	DrawRegValueDecimal},	// 0x13
	{"Light Pen Y",	DrawRegValueDecimal},	// 0x14
	{"Sprite Enable",	DrawRegValueSpriteEnable},	// 0x15
	{"Screen Ctrl Reg 2",	DrawRegValueScreenControlReg2},	// 0x16
	{"Sprite Double Height",	DrawRegValueSpriteEnable},	// 0x17
	{"Memory Setup",	DrawRegValueMemorySetup},	// 0x18
	{"Interrupt Status Register",	DrawRegValueDefault},	// 0x19
	{"Interrupt Control Register",		DrawRegValueDefault},// 0x1a
	{"Sprite Priority",		DrawRegValueDefault},// 0x1b
	{"Sprite Multi-Colour",		DrawRegValueSpriteEnable},// 0x1c
	{"Sprite Double Width",		DrawRegValueSpriteEnable},// 0x1d
	{"Sprite-Sprite Collision",		DrawRegValueSpriteEnable},// 0x1e
	{"Sprite-Background Collision",	DrawRegValueSpriteEnable},	// 0x1f
	{"Border Colour",		DrawRegValueColour},// 0x20
	{"Background Colour",		DrawRegValueColour},// 0x21
	{"Extra Background Colour",		DrawRegValueColour},// 0x22
	{"Extra Background Colour",		DrawRegValueColour},// 0x23
	{"Extra Background Colour",		DrawRegValueColour},// 0x24
	{"Sprite Extra Colour 1",		DrawRegValueColour},// 0x25
	{"Sprite Extra Colour 2",		DrawRegValueColour},// 0x26
	{"Sprite 0 Colour",		DrawRegValueColour},// 0x27
	{"Sprite 1 Colour",		DrawRegValueColour},// 0x28
	{"Sprite 2 Colour",		DrawRegValueColour},// 0x29
	{"Sprite 3 Colour",		DrawRegValueColour},// 0x2a
	{"Sprite 4 Colour",		DrawRegValueColour},// 0x2b
	{"Sprite 5 Colour",		DrawRegValueColour},// 0x2c
	{"Sprite 6 Colour",		DrawRegValueColour},// 0x2d
	{"Sprite 7 Colour",		DrawRegValueColour}// 0x2e

};

void FVICAnalysis::DrawUI(void)
{
	if (ImGui::BeginChild("VIC Reg Select", ImVec2(ImGui::GetWindowContentRegionWidth() * 0.5f, 0), true))
	{
		for (int i = 0; i < (int)g_VICRegDrawInfo.size(); i++)
		{
			char selectableTXT[32];
			sprintf_s(selectableTXT, "$%X %s", i, g_VICRegDrawInfo[i].Name);
			if (ImGui::Selectable(selectableTXT, UIVICRegister == i))
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
			const FC64IORegisterInfo& vicRegister = VICRegisters[UIVICRegister];
			const FRegDisplayConfig& regConfig = g_VICRegDrawInfo[UIVICRegister];

			// move out into function?
			ImGui::Text("Last Val:");
			regConfig.UIDrawFunction(vicRegister.LastVal);
			ImGui::Text("Accesses:");
			for (auto& access : vicRegister.Accesses)
			{
				ImGui::Separator();
				ImGui::Text("Code at: $%X", access.first);
				DrawAddressLabel(*pCodeAnalysis, access.first);

				ImGui::Text("Values:");

				for (auto& val : access.second.WriteVals)
				{
					regConfig.UIDrawFunction(val);
				}
			}
		}
	}
	ImGui::EndChild();
}
