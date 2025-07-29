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
const uint8_t kEliteVDUCode_Null = 0;	// null character
const uint8_t kEliteVDUCode_Beep = 7;	// beep
const uint8_t kEliteVDUCode_LF = 10;	// line feed
const uint8_t kEliteVDUCode_CLS = 11;	// clear the top part of the screen and draw a border
const uint8_t kEliteVDUCode_CR = 12;	// carriage return
const uint8_t kEliteVDUCode_CR2 = 13;	// carriage return (alternate code, same as 12)
const uint8_t kEliteVDUCode_Delete = 127;	// delete the character to the left of the text cursor and move the cursor to the left

// special commands
const uint8_t kEliteVDUCode_DrawLines = 129;	// draw lines command
const uint8_t kEliteVDUCode_ShowEnergyBombEffect = 131;	// show energy bomb effect
const uint8_t kEliteVDUCode_ShowHyperspaceColours = 132;	// show hyperspace colours
const uint8_t kEliteVDUCode_SetCursorX = 133;	// set cursor X position
const uint8_t kEliteVDUCode_SetCursorY = 134;	// set cursor Y position
const uint8_t kEliteVDUCode_ClearScreenBottom = 135;	// clear the bottom part of the screen
const uint8_t kEliteVDUCode_UpdateDashboard = 136;	// update dashboard
const uint8_t kEliteVDUCode_ShowHideDashboard = 138;	// show/hide dashboard
const uint8_t kEliteVDUCode_Set6522SystemVIAIER = 139;	// set 6522 system VIA IER
const uint8_t kEliteVDUCode_ToggleDashboardBulb = 140;	// toggle dashboard bulb
const uint8_t kEliteVDUCode_SetDiscCatalogueFlag = 141;	// set disc catalogue flag
const uint8_t kEliteVDUCode_SetCurrentColour = 142;	// set current colour
const uint8_t kEliteVDUCode_ChangeColourPalette = 143;	// change colour palette
const uint8_t kEliteVDUCode_SetFileSavingFlag = 144;	// set file saving flag
const uint8_t kEliteVDUCode_ExecuteBRKInstruction = 145;	// execute BRK instruction
const uint8_t kEliteVDUCode_WriteToPrinterScreen = 146;	// write to printer screen
const uint8_t kEliteVDUCode_BlankLineOnPrinter = 147;	// blank line on printer

bool FTubeEliteDisplay::ProcessEliteCommandByte(uint8_t cmdByte)
{
	const FTubeEliteDebug& debug = pTubeSys->GetDebug();

	switch (ProcessingCommand)
	{
	case kEliteVDUCode_DrawLines: // Draw Lines command
		if (NumLineBytesToRead == -1)
		{
			NumLineBytesToRead = cmdByte; // first byte is the number of bytes to read
		}
		else
		{
			const uint8_t lineByteNo = NoCommandBytesRead & 3;
			NewLine.bytes[lineByteNo] = cmdByte; // store the byte in the line
			if (lineByteNo == 3)
			{
				AddLine(NewLine); // add the line to the heap
			}
			NoCommandBytesRead++;
			if (NoCommandBytesRead == NumLineBytesToRead - 1)
				ProcessingCommand = 0; // command completed
		}
		return true;

	case kEliteVDUCode_ShowEnergyBombEffect: // Show energy bomb effect
		if (debug.bLogVDUChars)
		{
			g_VDULog.AddLog("\n");
			bLastCharCtrl = true;
			g_VDULog.AddLog("<energy bomb %d>", cmdByte);
		}
		// TODO: Implement the energy bomb effect
		ProcessingCommand = 0; // command completed
		return true;

	case kEliteVDUCode_ShowHyperspaceColours: // Show hyperspace colours
		if (debug.bLogVDUChars)
		{
			g_VDULog.AddLog("\n");
			bLastCharCtrl = true;
			g_VDULog.AddLog("<hyperspace colours %d>", cmdByte);
		}
		// TODO: Implement the hyperspace colours effect
		ProcessingCommand = 0; // command completed
		return true;

	case kEliteVDUCode_SetCursorX: // Set cursor X position
		SetCursorX(cmdByte); // set the cursor X position
		if (debug.bLogVDUChars)
		{
			g_VDULog.AddLog("\n");
			bLastCharCtrl = true;
			g_VDULog.AddLog("<cursorX X %d>", cmdByte);
		}
		ProcessingCommand = 0; // command completed
		return true;

	case kEliteVDUCode_SetCursorY: // Set cursor Y position
		SetCursorY(cmdByte); // set the cursor Y position
		if (debug.bLogVDUChars)
		{
			g_VDULog.AddLog("\n");
			bLastCharCtrl = true;
			g_VDULog.AddLog("<cursorY %d>", cmdByte);
		}
		ProcessingCommand = 0; // command completed
		return true;

	case kEliteVDUCode_UpdateDashboard:
		if (NoCommandBytesRead == 15)
		{
			ProcessingCommand = 0; // command completed
		}
		else
		{
			DashboardParams.Bytes[NoCommandBytesRead] = cmdByte; // store the byte in the dashboard parameters
			NoCommandBytesRead++;
		}
		return true;

	case kEliteVDUCode_ShowHideDashboard:
		ProcessingCommand = 0; // command completed
		if (debug.bLogVDUChars)
		{
			g_VDULog.AddLog("\n");
			bLastCharCtrl = true;
			g_VDULog.AddLog("<show/hide dashboard %d>", cmdByte);
		}
		return true; // show/hide dashboard command, not implemented yet

	case kEliteVDUCode_Set6522SystemVIAIER:
		ProcessingCommand = 0; // command completed
		if (debug.bLogVDUChars)
		{
			g_VDULog.AddLog("\n");
			bLastCharCtrl = true;
			g_VDULog.AddLog("<set 6522 system VIA IER %d>", cmdByte);
		}
		return true;

	case kEliteVDUCode_ToggleDashboardBulb:
		ProcessingCommand = 0; // command completed
		if (debug.bLogVDUChars)
		{
			g_VDULog.AddLog("\n");
			bLastCharCtrl = true;
			g_VDULog.AddLog("<toggle dashboard bulb %d>", cmdByte);
		}
		return true;

	case kEliteVDUCode_SetDiscCatalogueFlag:
		ProcessingCommand = 0; // command completed
		if (debug.bLogVDUChars)
		{
			g_VDULog.AddLog("\n");
			bLastCharCtrl = true;
			g_VDULog.AddLog("<set disc catalogue flag %d>", cmdByte);
		}
		return true;

	case kEliteVDUCode_SetCurrentColour:
		CurrentColour = cmdByte; // set the current colour
		ProcessingCommand = 0; // command completed
		if (debug.bLogVDUChars)
		{
			g_VDULog.AddLog("\n");
			bLastCharCtrl = true;
			g_VDULog.AddLog("<set current colour %d>", cmdByte);
		}
		return true;
	case kEliteVDUCode_ChangeColourPalette:
		ColourPalette = cmdByte; // change the colour palette
		ProcessingCommand = 0; // command completed
		if (debug.bLogVDUChars)
		{
			g_VDULog.AddLog("\n");
			bLastCharCtrl = true;
			g_VDULog.AddLog("<change colour palette %d>", cmdByte);
		}
		return true;

	case kEliteVDUCode_SetFileSavingFlag:
		ProcessingCommand = 0; // command completed
		if (debug.bLogVDUChars)
		{
			g_VDULog.AddLog("\n");
			bLastCharCtrl = true;
			g_VDULog.AddLog("<set file saving flag %d>", cmdByte);
		}
		return true;

	case kEliteVDUCode_WriteToPrinterScreen:
		if (debug.bLogVDUChars)
		{
			g_VDULog.AddLog("\n");
			bLastCharCtrl = true;
			g_VDULog.AddLog("<write to printer screen %d>", cmdByte);
		}
		DrawCharAtCursor(cmdByte); // write the character to the printer screen
		return true;
		
	default:
		LOGERROR("Unhandled Elite command: 0x%02X(%d)", ProcessingCommand, ProcessingCommand);
		assert(0); // unhandled command
		return false;
		break;
	}
}

bool FTubeEliteDisplay::ProcessEliteChar(uint8_t ch)
{
	const FTubeEliteDebug& debug = pTubeSys->GetDebug();

	// are we already processing a command?
	if (ProcessingCommand != 0)
	{
		return ProcessEliteCommandByte(ch); // process the command
	}

	if(ch >=32 && ch < 127)
	{
		// printable character
		DrawCharAtCursor(ch);
		return true;
	}
	else
	{
		if(debug.bLogVDUChars)
		{
			g_VDULog.AddLog("\n");
			bLastCharCtrl = true;
		}

		switch (ch)
		{
			case kEliteVDUCode_Null:	// nothing
				if (debug.bLogVDUChars)
					g_VDULog.AddLog("<null>");
				return true;
			case kEliteVDUCode_Beep:	// BEEP
				if (debug.bLogVDUChars)
					g_VDULog.AddLog("<beep>");
				return true;
			case kEliteVDUCode_LF:	// Line Feed
				CursorY++;
				if (CursorY >= kCharMapSizeY)
					CursorY = 0; // wrap around
				if (debug.bLogVDUChars)
					g_VDULog.AddLog("<lf>");
				return true;
			case kEliteVDUCode_CLS: // Clear the top part of the screen and draw a border
				ClearTextScreen();
				if (debug.bLogVDUChars)
					g_VDULog.AddLog("<cls>");
				NoLines = 0;	// clear line heap
				NoPixels = 0;	// clear pixel heap
				return true;
			case kEliteVDUCode_CR: // Carriage Return
			case kEliteVDUCode_CR2:
				CursorX = 0; // move to start of line
				CursorY++;
				if (CursorY >= kCharMapSizeY)
					CursorY = 0; // wrap around
				if (debug.bLogVDUChars)
					g_VDULog.AddLog("<cr>");
				return true;
			case kEliteVDUCode_Delete: // Delete the character to the left of the text cursor and move the cursor to the left
				if (CursorX > 0)
				{
					CursorX--; // move cursor back one position
					CharMap[CursorX][CursorY] = ' '; // clear the character at the cursor position
					if (debug.bLogVDUChars)
						g_VDULog.AddLog("<delete>");
					return true;
				}
				else
				{
					if (debug.bLogVDUChars)
						g_VDULog.AddLog("<delete at start of line>");
					return false; // nothing to delete
				}

				// Elite specific commands
			case kEliteVDUCode_DrawLines: // Draw Lines command
				ProcessingCommand = kEliteVDUCode_DrawLines;
				NumLineBytesToRead = -1; // to indicate we are waiting for the amount
				NoCommandBytesRead = 0; // reset the number of bytes read
				return true; // command processed

			// single param commands can go under the same case
			case kEliteVDUCode_ShowEnergyBombEffect: // Show energy bomb effect
			case kEliteVDUCode_ShowHyperspaceColours: // Show hyperspace colours
			case kEliteVDUCode_SetCursorX: // Set cursor X position
			case kEliteVDUCode_SetCursorY: // Set cursor Y position
			case kEliteVDUCode_ShowHideDashboard: // Show/Hide dashboard
			case kEliteVDUCode_Set6522SystemVIAIER: // Set 6522 system VIA IER
			case kEliteVDUCode_ToggleDashboardBulb: // Toggle dashboard bulb
			case kEliteVDUCode_SetDiscCatalogueFlag: // Set disc catalogue flag
			case kEliteVDUCode_SetCurrentColour: // Set current colour
			case kEliteVDUCode_ChangeColourPalette: // Change colour palette
			case kEliteVDUCode_SetFileSavingFlag: // Set file saving flag
			case kEliteVDUCode_WriteToPrinterScreen: // Write to printer screen
				ProcessingCommand = ch;
				return true; // command processed

			case kEliteVDUCode_ClearScreenBottom: // Clear the bottom part of the screen
				ClearScreenBottom();
				if (debug.bLogVDUChars)
					g_VDULog.AddLog("<clear screen bottom>");
				return true; // command processed

			case kEliteVDUCode_UpdateDashboard: // Update dashboard
				ProcessingCommand = kEliteVDUCode_UpdateDashboard;
				NoCommandBytesRead = 0; // reset the number of bytes read
				return true; // command processed

			case kEliteVDUCode_BlankLineOnPrinter: // Blank line on printer
				if (debug.bLogVDUChars)
				{
					g_VDULog.AddLog("\n");
					bLastCharCtrl = true;
					g_VDULog.AddLog("<blank line on printer>");
				}
				return true; // command processed

			case kEliteVDUCode_ExecuteBRKInstruction: // Execute BRK instruction - not used
				if (debug.bLogVDUChars)
				{
					g_VDULog.AddLog("\n");
					bLastCharCtrl = true;
					g_VDULog.AddLog("<execute BRK instruction>");
				}
				return true; // command processed

			default:
				// ignore other control characters
				//LOGWARNING("Tube Elite Display: Ignoring non-printable character %d(0x%02X)", ch, ch);
				//pTubeSys->DebugBreak(); // break the execution
				//g_VDULog.AddLog("<unhandled %d>", ch);
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
			NoLines = 0;
			NoPixels = 0; // clear line and pixel heaps
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

	const FTubeEliteDebug& debug = pTubeSys->GetDebug();
	if (debug.bLogVDUChars)
	{
		if (bLastCharCtrl)
			g_VDULog.AddLog("\n");
		g_VDULog.AddLog("%c", ch); // log the character
		bLastCharCtrl = false;
	}
}

void FTubeEliteDisplay::SetCursorX(int x) 
{ 
	CursorX = x; 
}

void FTubeEliteDisplay::SetCursorY(int y) 
{ 
	CursorY = y; 
}

void FTubeEliteDisplay::ClearTextScreen(uint8_t clearChar)
{
	for (int i = 0; i < kCharMapSizeX * kCharMapSizeY; i++)
	{
		CharMap[i % kCharMapSizeX][i / kCharMapSizeX] = clearChar;
	}
}

void FTubeEliteDisplay::ClearScreenBottom(void)
{
	ClearTextScreenFromRow(20, 0);
	SetCursorY(20);
}

void FTubeEliteDisplay::ClearTextScreenFromRow(uint8_t rowNo, uint8_t clearChar)
{
	for (int clearY = rowNo; clearY < kCharMapSizeY; clearY++)
	{
		for(int x=0;x<kCharMapSizeX;x++)
			CharMap[x][rowNo] = clearChar;
	}
}

bool FTubeEliteDisplay::AddLine(const FLine& newLine)
{
	if (NoLines == kMaxLines)
		NoLines = 0;//hack

	// check if the line already exists in the heap
	// this is because they were drawn using EOR on the BBC so adding the same line twice is removing it
	for (int i = 0; i < NoLines; i++)
	{
		if (LineHeap[i].val == newLine.val)
		{
			// line already exists, remove it
			for (int j = i; j < NoLines - 1; j++)
			{
				LineHeap[j] = LineHeap[j + 1]; // shift the lines down
			}
			NoLines--; // reduce the number of lines
			return true; // line removed
		}
	}

	// it's a new line so add it
	LineHeap[NoLines++] = newLine;
	return true;
}

// https://elite.bbcelite.com/6502sp/i_o_processor/subroutine/pixel.html
void FTubeEliteDisplay::ReceivePixelData(const uint8_t* pPixelData)
{
	const uint8_t noPixelBytes = pPixelData[0];
	const int noPixels = (noPixelBytes - 2) / 3; // each pixel is 3 bytes (dist, x, y)
	const uint8_t* pData = pPixelData + 2; // skip the first two bytes
	//NoPixels = 0;	// Hack - there seems to be a problem with removing old pixels, so we reset the pixel count here

	for (int i = 0; i < noPixels && NoPixels < kMaxPixels; i++)
	{
		bool bAddPixel = true;
		FPixel pixel;
		pixel.dist = pData[0];
		pixel.x = pData[1];
		pixel.y = pData[2];
		pData += 3; // move to the next pixel

		// check if pixel already exists in the heap
		for (int j = 0; j < NoPixels; j++)
		{
			if (PixelHeap[j].val == pixel.val)
			{
				// pixel already exists, remove it
				for (int k = j; k < NoPixels - 1; k++)
				{
					PixelHeap[k] = PixelHeap[k + 1]; // shift the pixels down
				}
				NoPixels--; // reduce the number of pixels
				bAddPixel = false; // don't add the pixel, it was removed
				break; // jump of the pixel check loop
			}
		}

		// it's a new pixel so add it
		if(bAddPixel)
			PixelHeap[NoPixels++] = pixel;
	}
}

bool FTubeEliteDisplay::UpdateKeyboardBuffer(uint8_t* pBuffer)
{
	if (bWindowFocused = false)
	{
		for (int i = 0; i < 14; i++)
			pBuffer[i] = 0; // clear the buffer

		pBuffer[14] = 0x10; // Joystick 1 fire button not pressed
		return false;
	}
		

	// 	*Byte #2: If a non - primary flight control key is
	// 		being pressed, its internal key number is put here
	pBuffer[2] = LastInternalKeyCode; // no non-primary flight control key pressed

	// * Byte #3: "?" is being pressed(0 = no, &FF = yes)
	pBuffer[3] = ImGui::IsKeyPressed(ImGuiKey_Slash) ? 0xff : 0xff; // "?" key pressed ?

	// * Byte #4: Space is being pressed(0 = no, &FF = yes)
	pBuffer[4] = ImGui::IsKeyPressed(ImGuiKey_Space) ? 0xff : 0x00; // Space key pressed

	// * Byte #5: "<" is being pressed(0 = no, &FF = yes)
	pBuffer[5] = ImGui::IsKeyPressed(ImGuiKey_Comma) ? 0xff : 0x00; // "<" key pressed

	// * Byte #6: ">" is being pressed(0 = no, &FF = yes)
	pBuffer[6] = ImGui::IsKeyPressed(ImGuiKey_Period) ? 0xff : 0x00; // ">" key pressed

	// * Byte #7: "X" is being pressed(0 = no, &FF = yes)
	pBuffer[7] = ImGui::IsKeyPressed(ImGuiKey_X) ? 0xff : 0x00; // "X" key pressed

	// * Byte #8: "S" is being pressed(0 = no, &FF = yes)
	pBuffer[8] = ImGui::IsKeyPressed(ImGuiKey_S) ? 0xff : 0x00; // "S" key pressed

	// * Byte #9: "A" is being pressed(0 = no, &FF = yes)
	pBuffer[9] = ImGui::IsKeyPressed(ImGuiKey_A) ? 0xff : 0x00; // "A" key pressed

	// * Byte #10: Joystick X value(high byte)
	pBuffer[10] = 0; // Joystick X value not pressed, set to 0

	// * Byte #11: Joystick Y value(high byte)
	pBuffer[11] = 0; // Joystick Y value not pressed, set to 0

	// * Byte #12: Bitstik rotation value(high byte)
	pBuffer[12] = 0; // Bitstik rotation value not pressed, set to 0

	// * Byte #14: Joystick 1 fire button is being pressed
	// (Bit 4 set = no, Bit 4 clear = yes)
	pBuffer[14] = 0x10; // Joystick 1 fire button not pressed, set to 0x10

	return true;
}


void FTubeEliteDisplay::Tick(void)
{
	LastInternalKeyCode = 0; // reset last key code

	for (int key = ImGuiKey_NamedKey_BEGIN; key < ImGuiKey_COUNT; key++)
	{
		if (ImGui::IsKeyPressed((ImGuiKey)key, false))
		{
			if (bWindowFocused)
			{
				const int bbcKey = BBCKeyFromImGuiKey((ImGuiKey)key);
				if (bbcKey != 0 && bWindowFocused)
				{
					//bbc_key_down(&pBBCEmu->GetBBC(), bbcKey);
					BBCKeyDown[bbcKey] = true; // mark the key as pressed
					//TODO: send to key buffer
					pTubeSys->AddInputByte(bbcKey);

				}

				const uint8_t internalKey = IntenalBBCKeyFromImGuiKey((ImGuiKey)key);
				if(internalKey != 0xff)
					LastInternalKeyCode = internalKey; // store the last key code pressed
			}
			
		}
		else if (ImGui::IsKeyReleased((ImGuiKey)key))
		{
			const int bbcKey = BBCKeyFromImGuiKey((ImGuiKey)key);
			if (bbcKey != 0)
			{
				BBCKeyDown[bbcKey] = false; // mark the key as released
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
	float charWidth = 8;//ImGui::GetFontSize() * 0.6f; // approximate character width
	float charHeight = 8;//ImGui::GetFontSize(); // approximate character height
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

	// Draw Line Heap
	for (int i = 0; i < NoLines; i++)
	{
		const FLine& line = LineHeap[i];
		drawList->AddLine(ImVec2(startPos.x + line.x1, startPos.y + line.y1),
			ImVec2(startPos.x + line.x2, startPos.y + line.y2), IM_COL32(255, 255, 255, 255));
	}

	// Draw Pixel Heap
	for (int i = 0; i < NoPixels; i++)
	{
		const FPixel& pixel = PixelHeap[i];
		const float pixelsize = 2.0f; // size of the pixel circle, TODO: calculate based on dist
		const ImColor pixelColor = ImColor(255, 255, 255, 255); // TODO: use pixel.dist for color
		drawList->AddCircleFilled(ImVec2(startPos.x + pixel.x, startPos.y + pixel.y), pixelsize, pixelColor);
	}

	drawList->AddRect(startPos, ImVec2(startPos.x + (kCharMapSizeX * charWidth),startPos.y + (kCharMapSizeY * charHeight)), IM_COL32(255, 255, 255, 128));
	
	bWindowFocused = ImGui::IsWindowFocused();

	ImGui::End();

	g_VDULog.Draw("VDU Log", &bShowLog);

}