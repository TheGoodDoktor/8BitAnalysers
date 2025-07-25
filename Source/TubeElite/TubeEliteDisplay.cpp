#include "TubeEliteDisplay.h"

#include "TubeElite.h"

#include <imgui.h>
#include "Debug/DebugLog.h"
#include <string>

bool FTubeEliteDisplay::Init(FTubeElite* pSys)
{
	pTubeSys = pSys;
	return true;
}

bool FTubeEliteDisplay::ProcessVDUChar(uint8_t ch)
{
	if (CursorX < 0 || CursorX >= kCharMapSizeX || CursorY < 0 || CursorY >= kCharMapSizeY)
		return false;

	if (ch < 32 || ch > 127) // non-printable characters
	{
		switch (ch)
		{
		case 0:	// nothing
			break;
		case 8: // BS
			if (CursorX > 0)
				CursorX--; // move cursor back one position
			break;
		case 9: // FS
			if (CursorX < kCharMapSizeX-1)
				CursorX++; // move cursor forward one position
			break;
		case 10: // move text cursor down 1 line
			CursorY++;
			if (CursorY >= kCharMapSizeY)
				CursorY = 0; // wrap around
			break;
		case 11: // move text cursor up one line
			CursorY--;
			if(CursorY<0)
				CursorY = kCharMapSizeY - 1; //wrap around
			break;
		case 12:	// CLS
			ClearTextScreen();
			break;
		case 13: // CR
			CursorX = 0; // move to start of line
			break;
		default:
			// ignore other control characters
			LOGWARNING("Tube Elite Display: Ignoring non-printable character %d(0x%02X)",ch, ch);
			pTubeSys->DebugBreak(); // break the execution
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

void FTubeEliteDisplay::ClearTextScreen(uint8_t clearChar)
{
	for (int i = 0; i < kCharMapSizeX * kCharMapSizeY; i++)
	{
		CharMap[i % kCharMapSizeX][i / kCharMapSizeX] = clearChar;
	}
}

void FTubeEliteDisplay::ClearTextScreenFromRow(uint8_t rowNo, uint8_t clearChar)
{
	for (int clearY = rowNo; clearY < kCharMapSizeY; clearY++)
	{
		for(int x=0;x<kCharMapSizeX;x++)
			CharMap[x][rowNo] = clearChar;
	}
}


void FTubeEliteDisplay::Tick(void)
{
	for (int key = ImGuiKey_NamedKey_BEGIN; key < ImGuiKey_COUNT; key++)
	{
		if (ImGui::IsKeyPressed((ImGuiKey)key, false))
		{
			const int bbcKey = BBCKeyFromImGuiKey((ImGuiKey)key);
			if (bbcKey != 0 && bWindowFocused)
			{
				//bbc_key_down(&pBBCEmu->GetBBC(), bbcKey);
				//TODO: send to key buffer
				pTubeSys->AddInputByte(bbcKey);

			}
		}
		else if (ImGui::IsKeyReleased((ImGuiKey)key))
		{
			const int bbcKey = BBCKeyFromImGuiKey((ImGuiKey)key);
			if (bbcKey != 0)
			{
				//bbc_key_up(&pBBCEmu->GetBBC(), bbcKey);
			}
		}
	}
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
	
	bWindowFocused = ImGui::IsWindowFocused();

	ImGui::End();


}