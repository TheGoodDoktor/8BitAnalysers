#include "CRTCViewer.h"
#include "../CPCEmu.h"
#include <imgui.h>

void InputU8Decimal(const char* label, uint8_t& val)
{
	ImGui::PushItemWidth(ImGui::GetFontSize() * 5);
	const uint8_t u8_one = 1;
	ImGui::InputScalar(label, ImGuiDataType_U8, &val, &u8_one, NULL, "%u", ImGuiInputTextFlags_CharsDecimal);
	ImGui::PopItemWidth();
}

// Draw register value.
// Change text colour if register's value has been modified from the default value.
void DrawCrtcRegValue(uint8_t& reg, const char* text, uint8_t default_val, const std::string& description)
{
	bool bModified = false;
	bModified = reg != default_val;
	if (bModified)
		ImGui::PushStyleColor(ImGuiCol_Text, IM_COL32(255, 255, 0, 255));
	InputU8Decimal(text, reg);
	ImGui::NextColumn();
	if (bModified)
		ImGui::PopStyleColor();
	if (ImGui::IsItemHovered())
	{
		if (bModified || !description.empty())
		{
			ImGui::BeginTooltip();

			if (!description.empty())
				ImGui::Text("%s", description.c_str());

			if (bModified)
				ImGui::Text("Default = %d", default_val);
			ImGui::EndTooltip();
		}
	}
}

bool FCrtcViewer::Init(void)  
{ 
	return true; 
}

void FCrtcViewer::DrawUI(void)
{
	mc6845_t& mc = pCpcEmu->CpcEmuState.crtc;

	ImGui::Columns(2, "##regs", false);
	ImGui::SetColumnWidth(0, ImGui::GetFontSize() * 18);
	ImGui::SetColumnWidth(1, ImGui::GetFontSize() * 18);


	// todo apply min and max on these
	// see _mc6845_mask

	DrawCrtcRegValue(mc.h_total,			"R0 Horiz. Total",			63, std::string("Physical width of the screen, in characters.\nRange [0-255]"));
	DrawCrtcRegValue(mc.h_displayed,		"R1 Horiz. Displayed",		40, std::string("Range [0-255]"));
	DrawCrtcRegValue(mc.h_sync_pos,			"R2 Horiz. Sync Pos.",		46, std::string("Range [0-255]"));
	DrawCrtcRegValue(mc.sync_widths,		"R3 Sync Widths",			142, std::string(""));
	DrawCrtcRegValue(mc.v_total,			"R4 Vert. Total",			38, std::string("Range [0-127]"));
	DrawCrtcRegValue(mc.v_total_adjust,		"R5 Vert. Total Adjust",	0, std::string("Range [0-31]"));
	DrawCrtcRegValue(mc.v_displayed,		"R6 Vert. Displayed",		25, std::string("Range [0-127]"));
	DrawCrtcRegValue(mc.v_sync_pos,			"R7 Vert. Sync Pos.",		30, std::string("Range [0-127]"));
	DrawCrtcRegValue(mc.interlace_mode,		"R8 Interlace and Skew",	0, std::string("Range [0-3]"));
	DrawCrtcRegValue(mc.max_scanline_addr,	"R9 Max Scan lines",		7, std::string("Height of a character in scan lines. Height is R9+1. Default value is 7: 8x8 character.\nRange [0-7]"));
	DrawCrtcRegValue(mc.cursor_start,		"R10 Cursor Start",			0, std::string("Range [0-127]"));
	DrawCrtcRegValue(mc.cursor_end,			"R11 Cursor End",			0, std::string("Range [0-31]"));
	DrawCrtcRegValue(mc.start_addr_hi,		"R12 Disp. Start H",		48, std::string("Range [0-63]"));
	DrawCrtcRegValue(mc.start_addr_lo,		"R13 Disp. Start L",		0, std::string("Range [0-255]"));
	DrawCrtcRegValue(mc.cursor_hi,			"R14 Cursor H",				0, std::string("Range [0-63]"));
	DrawCrtcRegValue(mc.cursor_lo,			"R15 Cursor L",				0, std::string("Range [0-255]"));
	DrawCrtcRegValue(mc.lightpen_hi,		"R16 Light Pen H",			0, std::string("Range [0-63]"));
	DrawCrtcRegValue(mc.lightpen_lo,		"R17 Light Pen L",			0, std::string("Range [0-255]"));
	ImGui::Columns();
}
