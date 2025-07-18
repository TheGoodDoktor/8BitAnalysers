#include "TubeEliteDisplay.h"

#include <imgui.h>
#include "Debug/DebugLog.h"
#include <string>

bool FTubeEliteDisplay::ProcessVDUChar(uint8_t ch)
{
	if (CursorX < 0 || CursorX >= kCharMapSizeX || CursorY < 0 || CursorY >= kCharMapSizeY)
		return false;

	if (ch < 32 || ch > 127) // non-printable characters
	{
		switch (ch)
		{
		case 0x0D: // CR
			CursorX = 0; // move to start of line
			break;
		case 0x0A: // LF
			CursorY++;
			if (CursorY >= kCharMapSizeY)
				CursorY = 0; // wrap around
			break;
		case 0x08: // BS
			if (CursorX > 0)
				CursorX--; // move cursor back one position
			break;
		default:
			// ignore other control characters
			LOGWARNING("Tube Elite Display: Ignoring non-printable character 0x%02X", ch);
			return false;
		}
	}
	else
	{
		CharMap[CursorX][CursorY] = ch;
		CursorX++;
		if (CursorX >= kCharMapSizeX)
		{
			CursorX = 0;
			CursorY++;
			if (CursorY >= kCharMapSizeY)
				CursorY = 0; // wrap around
		}
	}
	return true;
}

void FTubeEliteDisplay::DrawUI(void)
{
	ImGui::Begin("Tube Elite Display");
	ImGui::Text("Tube Elite Display");

	// Draw the character map
	float charWidth = ImGui::GetFontSize() * 0.6f; // approximate character width
	float charHeight = ImGui::GetFontSize(); // approximate character height
	ImDrawList* drawList = ImGui::GetWindowDrawList();
	ImVec2 startPos = ImGui::GetCursorScreenPos();
	for (int y = 0; y < kCharMapSizeY; y++)
	{
		for (int x = 0; x < kCharMapSizeX; x++)
		{
			const uint8_t ch = CharMap[x][y];
			if (ch != 0)
			{
				//ImGui::Text("%c", ch);
				drawList->AddText(ImVec2(startPos.x + x * charWidth, startPos.y + y * charHeight), IM_COL32(255, 255, 255, 255), std::string(1, ch).c_str());
			}
			else
			{
				//ImGui::Text(" "); // empty space
			}
		}
		ImGui::NewLine();
	}
	ImGui::End();
}