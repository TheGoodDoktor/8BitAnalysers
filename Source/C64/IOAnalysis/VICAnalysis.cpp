#include "VICAnalysis.h"
#include <CodeAnalyser/CodeAnalyser.h>
#include <chips/chips_common.h>

void FVICAnalysis::Init(FCodeAnalysisState* pAnalysis)
{
	Name = "VIC-II";
	SetAnalyser(pAnalysis);
	pAnalysis->IOAnalyser.AddDevice(this);
}

void FVICAnalysis::Reset(void)
{
	for (int i = 0; i < kNoRegisters; i++)
		VICRegisters[i].Reset();
}

void FVICAnalysis::OnRegisterRead(uint8_t reg, FAddressRef pc)
{

}

void FVICAnalysis::OnRegisterWrite(uint8_t reg, uint8_t val, FAddressRef pc)
{
	FC64IORegisterInfo& vicRegister = VICRegisters[reg];
	const uint8_t regChange = vicRegister.LastVal ^ val;	// which bits have changed

	vicRegister.Accesses[pc].WriteVals.insert(val);

	vicRegister.LastVal = val;
}

#include <imgui.h>
#include <CodeAnalyser/UI/CodeAnalyserUI.h>
#include <chips/m6569.h>
#include <vector>
#include <CodeAnalyser/CodeAnalysisPage.h>

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
	ImGui::Text("($%X) VScroll:%d, Height:%d, Scr:%s, %s, ExtBG:%s, RastMSB:%d",
		val,
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
	ImGui::Text("($%X) HScroll:%d, Width:%d, MultiColour:%s",
		val,
		val & 7,
		val & (1 << 3) ? 40 : 38,
		val & (1 << 4) ? "ON" : "OFF");

}

void DrawRegValueMemorySetup(uint8_t val)
{
	ImGui::Text("($%X) Char Addr: $%04X, Bitmap Address: $%04X, Screen Address: $%04X",
		val,
		((val >> 1) & 7) << 11,
		((val >> 3) & 1) << 13,
		(val >> 4) << 10
	);
	ImGui::Text("\t$%X", val);

}

static std::vector<FRegDisplayConfig>	g_VICRegDrawInfo = 
{
	{"VIC_Sprite0X",	DrawRegValueDecimal},	// 0x00
	{"VIC_Sprite0Y",	DrawRegValueDecimal}, 	// 0x01
	{"VIC_Sprite1X",	DrawRegValueDecimal}, 	// 0x02
	{"VIC_Sprite1Y",	DrawRegValueDecimal}, 	// 0x03
	{"VIC_Sprite2X",	DrawRegValueDecimal}, 	// 0x04
	{"VIC_Sprite2Y",	DrawRegValueDecimal}, 	// 0x05
	{"VIC_Sprite3X",	DrawRegValueDecimal}, 	// 0x06
	{"VIC_Sprite3Y",	DrawRegValueDecimal}, 	// 0x07
	{"VIC_Sprite4X",	DrawRegValueDecimal}, 	// 0x08
	{"VIC_Sprite4Y",	DrawRegValueDecimal}, 	// 0x09
	{"VIC_Sprite5X",	DrawRegValueDecimal}, 	// 0x0a
	{"VIC_Sprite5Y",	DrawRegValueDecimal}, 	// 0x0b
	{"VIC_Sprite6X",	DrawRegValueDecimal}, 	// 0x0c
	{"VIC_Sprite6Y",	DrawRegValueDecimal}, 	// 0x0d
	{"VIC_Sprite7X",	DrawRegValueDecimal}, 	// 0x0e
	{"VIC_Sprite7Y",	DrawRegValueDecimal}, 	// 0x0f
	{"VIC_SpriteXMSB",	DrawRegValueHex},	// 0x10
	{"VIC_ScrCtrl1",	DrawRegValueScreenControlReg1},	// 0x11
	{"VIC_RasterLine",	DrawRegValueDecimal},	// 0x12
	{"VIC_LightPenX",	DrawRegValueDecimal},	// 0x13
	{"VIC_LightPenY",	DrawRegValueDecimal},	// 0x14
	{"VIC_SpriteEnable",	DrawRegValueSpriteEnable},	// 0x15
	{"VIC_ScrCtrl2",		DrawRegValueScreenControlReg2},	// 0x16
	{"VIC_SpriteDblHeight",	DrawRegValueSpriteEnable},	// 0x17
	{"VIC_MemorySetup",		DrawRegValueMemorySetup},	// 0x18
	{"VIC_InterruptStatus",		DrawRegValueHex},	// 0x19
	{"VIC_InterruptControl",	DrawRegValueHex},// 0x1a
	{"VIC_SpritePriority",		DrawRegValueHex},// 0x1b
	{"VIC_SpriteMultiCol",		DrawRegValueSpriteEnable},// 0x1c
	{"VIC_SpriteDblWidth",		DrawRegValueSpriteEnable},// 0x1d
	{"VIC_Sprite-SpriteCol",		DrawRegValueSpriteEnable},// 0x1e
	{"VIC_Sprite-BackCol",	DrawRegValueSpriteEnable},	// 0x1f
	{"VIC_BorderColour",		DrawRegValueColour},// 0x20
	{"VIC_BackgroundColour",		DrawRegValueColour},// 0x21
	{"VIC_ExtraBackColour1",		DrawRegValueColour},// 0x22
	{"VIC_ExtraBackColour2",		DrawRegValueColour},// 0x23
	{"VIC_ExtraBackColour3",		DrawRegValueColour},// 0x24
	{"VIC_SpriteExtraColour1",		DrawRegValueColour},// 0x25
	{"VIC_SpriteExtraColour2",		DrawRegValueColour},// 0x26
	{"VIC_Sprite0Colour",		DrawRegValueColour},// 0x27
	{"VIC_Sprite1Colour",		DrawRegValueColour},// 0x28
	{"VIC_Sprite2Colour",		DrawRegValueColour},// 0x29
	{"VIC_Sprite3Colour",		DrawRegValueColour},// 0x2a
	{"VIC_Sprite4Colour",		DrawRegValueColour},// 0x2b
	{"VIC_Sprite5Colour",		DrawRegValueColour},// 0x2c
	{"VIC_Sprite6Colour",		DrawRegValueColour},// 0x2d
	{"VIC_Sprite7Colour",		DrawRegValueColour}// 0x2e
};

void FVICAnalysis::DrawDetailsUI(void)
{
	if (ImGui::BeginChild("VIC Reg Select", ImVec2(ImGui::GetWindowContentRegionWidth() * 0.5f, 0), true))
	{
		for (int i = 0; i < (int)g_VICRegDrawInfo.size(); i++)
		{
			char selectableTXT[32];
			snprintf(selectableTXT, sizeof(selectableTXT), "$%X %s", i, g_VICRegDrawInfo[i].Name);
			if (ImGui::Selectable(selectableTXT, SelectedRegister == i))
			//if (ImGui::Selectable(g_VICRegDrawInfo[i].Name, SelectedRegister == i))
			{
				SelectedRegister = i;
			}
		}
	}
	ImGui::EndChild();
	ImGui::SameLine();
	if (ImGui::BeginChild("VIC Reg Details"))
	{
		if (SelectedRegister != -1)
		{
			FC64IORegisterInfo& vicRegister = VICRegisters[SelectedRegister];
			const FRegDisplayConfig& regConfig = g_VICRegDrawInfo[SelectedRegister];

			if (ImGui::Button("Clear"))
			{
				vicRegister.LastVal = 0;
				vicRegister.Accesses.clear();
			}
			// move out into function?
			ImGui::Text("Last Val:");
			regConfig.UIDrawFunction(vicRegister.LastVal);
			ImGui::Text("Accesses:");
			for (auto& access : vicRegister.Accesses)
			{
				ImGui::Separator();

				DrawCodeAddress(*pCodeAnalyser, pCodeAnalyser->GetFocussedViewState(), access.first);

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

void AddVICRegisterLabels(FCodeAnalysisPage& IOPage)
{
	for(int reg=0;reg< (int)g_VICRegDrawInfo.size();reg++)
		IOPage.SetLabelAtAddress(g_VICRegDrawInfo[reg].Name, ELabelType::Data, reg);
}