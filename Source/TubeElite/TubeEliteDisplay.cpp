#include "TubeEliteDisplay.h"

#include "TubeElite.h"

#include <imgui.h>
#include "Debug/DebugLog.h"
#include "Debug/ImGuiLog.h"
#include <string>


ImGuiLog g_VDULog;

bool FTubeEliteDisplay::Init(FTubeElite* pSys)
{
	pTubeSys = pSys;
	return true;
}

bool FTubeEliteDisplay::ProcessVDUChar(uint8_t ch)
{
	return ProcessEliteChar(ch); // Elite display uses the same character processing as MOS VDU
}

// https://elite.bbcelite.com/6502sp/i_o_processor/subroutine/tt26.html
/*
	* 7 (beep)

	* 10 (line feed)

	* 11 (clear the top part of the screen and draw a
	border)

	* 12-13 (carriage return)

	* 32-95 (ASCII capital letters, numbers and
	punctuation)

	* 127 (delete the character to the left of the text
	cursor and move the cursor to the left)
*/
bool FTubeEliteDisplay::ProcessEliteChar(uint8_t ch)
{
	if(ch >=32 && ch < 127)
	{
		// printable character
		DrawCharAtCursor(ch);
		return true;
	}
	else
	{
		g_VDULog.AddLog("\n");
		bLastCharCtrl = true;

		switch (ch)
		{
			case 0:	// nothing
				g_VDULog.AddLog("<null>");
				return true;
			case 7:	// BEEP
				g_VDULog.AddLog("<beep>");
				return true;
			case 10:	// Line Feed
				CursorY++;
				if (CursorY >= kCharMapSizeY)
					CursorY = 0; // wrap around
				g_VDULog.AddLog("<lf>");
				return true;
			case 11: // Clear the top part of the screen and draw a border
				ClearTextScreen();
				g_VDULog.AddLog("<cls>");
				return true;
			case 12: // Carriage Return
			case 13:
				CursorX = 0; // move to start of line
				CursorY++;
				if (CursorY >= kCharMapSizeY)
					CursorY = 0; // wrap around
				g_VDULog.AddLog("<cr>");
				return true;
			case 127: // Delete the character to the left of the text cursor and move the cursor to the left
				if (CursorX > 0)
				{
					CursorX--; // move cursor back one position
					CharMap[CursorX][CursorY] = ' '; // clear the character at the cursor position
					g_VDULog.AddLog("<delete>");
					return true;
				}
				else
				{
					g_VDULog.AddLog("<delete at start of line>");
					return false; // nothing to delete
				}
			default:
				// ignore other control characters
				LOGWARNING("Tube Elite Display: Ignoring non-printable character %d(0x%02X)", ch, ch);
				pTubeSys->DebugBreak(); // break the execution
				g_VDULog.AddLog("<unhandled %d>", ch);
				return false; // unhandled character
		}
	}

	return false;
}

bool FTubeEliteDisplay::ProcessMOSVDUChar(uint8_t ch)
{
	if (CursorX < 0 || CursorX >= kCharMapSizeX || CursorY < 0 || CursorY >= kCharMapSizeY)
		return false;


	if (ch < 32 || ch > 127) // non-printable characters
	{
		g_VDULog.AddLog("\n");
		bLastCharCtrl = true;
		switch (ch)
		{
		case 0:	// nothing
			g_VDULog.AddLog("<null>");
			break;
		case 7: // BEEP
			g_VDULog.AddLog("<beep>");
			//pTubeSys->DebugBreak(); // break the execution
			break;
		case 8: // BS
			if (CursorX > 0)
				CursorX--; // move cursor back one position
			g_VDULog.AddLog("<back space>");
			break;
		case 9: // FS
			if (CursorX < kCharMapSizeX-1)
				CursorX++; // move cursor forward one position
			g_VDULog.AddLog("<forward space>");
			break;
		case 10: // move text cursor down 1 line
			CursorY++;
			if (CursorY >= kCharMapSizeY)
				CursorY = 0; // wrap around
			g_VDULog.AddLog("<cursor down>");
			break;
		case 11: // move text cursor up one line
			CursorY--;
			if(CursorY<0)
				CursorY = kCharMapSizeY - 1; //wrap around
			g_VDULog.AddLog("<cursor up>");
			break;
		case 12:	// CLS
			ClearTextScreen();
			pTubeSys->DebugBreak(); // break the execution
			g_VDULog.AddLog("<cls>");
			break;
		case 13: // CR
			CursorX = 0; // move to start of line
			g_VDULog.AddLog("<cr>");
			break;
		default:
			// ignore other control characters
			LOGWARNING("Tube Elite Display: Ignoring non-printable character %d(0x%02X)",ch, ch);
			pTubeSys->DebugBreak(); // break the execution
			g_VDULog.AddLog("<unhandled %d>",ch);
			return false;
		}
	}
	else
	{
		DrawCharAtCursor(ch);
	}
	return true;
}

void FTubeEliteDisplay::DrawCharAtCursor(uint8_t ch)
{
	// printable character
	CharMap[CursorX][CursorY] = ch;
	CursorX++;
	if (CursorX >= kCharMapSizeX)
	{
		CursorX = 0;
		CursorY++;
		if (CursorY >= kCharMapSizeY)
			CursorY = 0; // wrap around
	}

	if (bLastCharCtrl)
		g_VDULog.AddLog("\n");
	g_VDULog.AddLog("%c", ch); // log the character
	bLastCharCtrl = false;
}

void FTubeEliteDisplay::SetCursorX(int x) 
{ 
	g_VDULog.AddLog("\n<cursorX = %d>",x);
	bLastCharCtrl = true;
	CursorX = x; 
}

void FTubeEliteDisplay::SetCursorY(int y) 
{ 
	CursorY = y; g_VDULog.AddLog("\n<cursorY = %d>", y);
	bLastCharCtrl = true;
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

	drawList->AddRect(startPos, ImVec2(startPos.x + (kCharMapSizeX * charWidth),startPos.y + (kCharMapSizeY * charHeight)), IM_COL32(255, 255, 255, 128));
	
	bWindowFocused = ImGui::IsWindowFocused();

	ImGui::End();

	g_VDULog.Draw("VDU Log", &bShowLog);

}