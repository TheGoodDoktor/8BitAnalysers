#include "TubeEliteDisplay.h"

#include "TubeElite.h"

#include <imgui.h>
#include "Debug/DebugLog.h"
#include "Debug/ImGuiLog.h"
#include <string>
#include "TubeFrameBuffer.h"

ImGuiLog g_VDULog;


uint32_t g_Palette[4][4] =
{
	// palette 0 = Yellow, red, cyan palette (space view)
	{
		IM_COL32(0, 0, 0, 255),	// black
		IM_COL32(255, 255, 0, 255),	// yellow
		IM_COL32(255, 0, 0, 255),	// red
		IM_COL32(0, 255, 255, 255),	// cyan
	},
	// palette 1 = Yellow, red, white palette (charts)
	{
		IM_COL32(0, 0, 0, 255),			// black
		IM_COL32(255, 255, 0, 255),		// yellow
		IM_COL32(255, 0, 0, 255),		// red
		IM_COL32(255, 255, 255, 255),	// white
	},
	// palette 2 = Yellow, white, cyan palette (title screen)
	{
		IM_COL32(0, 0, 0, 255),			// black
		IM_COL32(255, 255, 0, 255),		// yellow
		IM_COL32(255, 255, 255, 255),	// white
		IM_COL32(0, 255, 255, 255),		// cyan
	},
	// palette 3 = Yellow, magenta, white palette (trading)
	{
		IM_COL32(0, 0, 0, 255),			// black
		IM_COL32(255, 255, 0, 255),		// yellow
		IM_COL32(255, 0, 255, 255),		// magenta
		IM_COL32(255, 255, 255, 255),	// white
	}
};

void SetElitePalette(int paletteNo)
{
	switch (paletteNo)
	{
	case 0:
		// Set palette 0 - yellow, red, cyan
		Display::SetPalette(1, 255, 255, 0);	// Yellow
		Display::SetPalette(2, 255, 0, 0);	// Red
		Display::SetPalette(3, 0, 255, 255);	// Cyan
		break;
	case 1:
		// Set palette 1 - yellow, red, white
		Display::SetPalette(1, 255, 255, 0);	// Yellow
		Display::SetPalette(2, 255, 0, 0);	// Red
		Display::SetPalette(3, 255, 255, 255);	// White
		break;
	case 2:
		// Set palette 2 - yellow, white, cyan
		Display::SetPalette(1, 255, 255, 0);	// Yellow
		Display::SetPalette(2, 255, 255, 255);	// White
		Display::SetPalette(3, 0, 255, 255);	// Cyan
		break;
	case 3:
		// Set palette 3 - yellow, magenta, white
		Display::SetPalette(1, 255, 255, 0);	// Yellow
		Display::SetPalette(2, 255, 0, 255);	// Magenta
		Display::SetPalette(3, 255, 255, 255);	// White
		break;
	}
}

bool FTubeEliteDisplay::Init(FTubeElite* pSys)
{
	pTubeSys = pSys;
	Display::Init(); // Initialize the display system
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
		CurrentColour = ((cmdByte >> 3) & 2)  | (cmdByte & 1); // set the current colour
		ProcessingCommand = 0; // command completed
		if (debug.bLogVDUChars)
		{
			g_VDULog.AddLog("\n");
			bLastCharCtrl = true;
			g_VDULog.AddLog("<set current colour %d>", cmdByte);
		}
		return true;
	// https://elite.bbcelite.com/6502sp/i_o_processor/subroutine/setvdu19.html
	case kEliteVDUCode_ChangeColourPalette:
		ColourPalette = cmdByte >> 4; // change the colour palette
		SetElitePalette(cmdByte >> 4); // set the palette
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
			//g_VDULog.AddLog("\n");
			bLastCharCtrl = true;
		}

		switch (ch)
		{
			case kEliteVDUCode_Null:	// nothing
				if (debug.bLogVDUChars)
					g_VDULog.AddLog("\n<null>");
				return true;
			case kEliteVDUCode_Beep:	// BEEP
				if (debug.bLogVDUChars)
					g_VDULog.AddLog("\n<beep>");
				return true;
			case kEliteVDUCode_LF:	// Line Feed
				CursorY++;
				if (CursorY >= kCharMapSizeY)
					CursorY = 0; // wrap around
				if (debug.bLogVDUChars)
					g_VDULog.AddLog("\n<lf>");
				return true;
			case kEliteVDUCode_CLS: // Clear the top part of the screen and draw a border
			{
				Display::ClearScreen(0); // clear the display
				// draw the border
				uint8_t borderCol = 1;
				Display::DrawLine(0, 0, 255, 0, borderCol);
				Display::DrawLine(1,0,1,191,borderCol);
				Display::DrawLine(0, 0, 0, 191, borderCol);
				Display::DrawLine(255, 0, 255, 191, borderCol);
				Display::DrawLine(254, 0, 254, 191, borderCol);
				if (debug.bLogVDUChars)
					g_VDULog.AddLog("\n<cls>");
				return true;
			}
			case kEliteVDUCode_CR: // Carriage Return
				CursorX = 1; // move to start of line as x = 1 see https://elite.bbcelite.com/6502sp/i_o_processor/subroutine/tt26.html

				CursorY++;
				if (CursorY >= kCharMapSizeY)
					CursorY = 0; // wrap around
				if (debug.bLogVDUChars)
					g_VDULog.AddLog("\n<crlf>");
				return true;
			case kEliteVDUCode_CR2: // Carriage Rturn - no line feed?
				CursorX = 1; // move to start of line as x = 1 see https://elite.bbcelite.com/6502sp/i_o_processor/subroutine/tt26.html
				
				if (debug.bLogVDUChars)
					g_VDULog.AddLog("\n<cr>");
				return true;
			case kEliteVDUCode_Delete: // Delete the character to the left of the text cursor and move the cursor to the left
				if (CursorX > 0)
				{
					CursorX--; // move cursor back one position
					Display::DrawChar8x8(CursorX, CursorY, ' ', CurrentColour); // draw a space character
					if (debug.bLogVDUChars)
						g_VDULog.AddLog("\n<delete>");
					return true;
				}
				else
				{
					if (debug.bLogVDUChars)
						g_VDULog.AddLog("\n<delete at start of line>");
					return false; // nothing to delete
				}

				// Elite specific commands
			case kEliteVDUCode_DrawLines: // Draw Lines command
				ProcessingCommand = kEliteVDUCode_DrawLines;
				NumLineBytesToRead = -1; // to indicate we are waiting for the amount
				NoCommandBytesRead = 0; // reset the number of bytes read
				NewLine.colour = g_Palette[ColourPalette][CurrentColour];	// get from palette & line colour
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
			Display::ClearScreen();
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
	Display::DrawChar8x8(CursorX * 8, CursorY * 8, ch, CurrentColour);

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

void FTubeEliteDisplay::ClearScreenBottom(void)
{
	//Display::ClearScreenFromYpos(20 * 8, 0);
	SetCursorX(1);
	SetCursorY(20);
}

bool FTubeEliteDisplay::AddLine(const FLine& newLine)
{
	Display::DrawLineEOR(newLine.x1, newLine.y1, newLine.x2, newLine.y2, CurrentColour);
	return true;
}

// https://elite.bbcelite.com/6502sp/i_o_processor/subroutine/pixel.html
void FTubeEliteDisplay::ReceivePixelData(const uint8_t* pPixelData)
{
	const uint8_t noPixelBytes = pPixelData[0];
	const int noPixels = (noPixelBytes - 2) / 3; // each pixel is 3 bytes (dist, x, y)
	const uint8_t* pData = pPixelData + 2; // skip the first two bytes

	for (int i = 0; i < noPixels; i++)
	{
		const uint8_t dist = pData[0];
		const uint8_t x = pData[1];
		const uint8_t y = pData[2];

		pData += 3; // move to the next pixel

		uint8_t pixelColour = 7;// default colour is white

		if((dist & 7) == 0)
		{ 
			Display::DrawPixelEOR(x, y, 7); // draw the pixel in white
		}
		else
		{
			const uint8_t colLUT[8] = { 3, 1, 1, 2, 2, 1, 2, 1 }; // colour lookup table
			pixelColour = colLUT[dist & 7]; // get the colour from the lookup table
			// TODO: square or dash depending on distance
			if (dist < 80)	// square for close pixels
			{
				Display::DrawPixelEOR(x, y, pixelColour); // draw the pixel
				Display::DrawPixelEOR(x+1, y, pixelColour); // draw the pixel
				Display::DrawPixelEOR(x+1, y+1, pixelColour); // draw the pixel
				Display::DrawPixelEOR(x, y+1, pixelColour); // draw the pixel
			}
			else if (dist < 128) // dash for medium distance pixels
			{
				Display::DrawPixelEOR(x, y, pixelColour); // draw the pixel
				Display::DrawPixelEOR(x + 1, y, pixelColour); // draw the pixel
			}
			else // far away pixels
			{
				Display::DrawPixelEOR(x, y, pixelColour); // draw the pixel
			}
		}
		
	}
}

// https://elite.bbcelite.com/6502sp/i_o_processor/subroutine/hloin.html
void FTubeEliteDisplay::ReceiveSunLineData(const uint8_t* pLineData)
{
	const uint8_t noLineBytes = pLineData[0];
	const int noLines = (noLineBytes - 2) / 3; // each line is 3 bytes (X1,X2,Y)
	const uint8_t* pData = pLineData + 2; // skip the first two bytes

	for (int i = 0; i < noLines; i++)
	{
		const uint8_t x1 = pData[0];
		const uint8_t x2 = pData[1];
		const uint8_t y = pData[2];
		pData += 3; // move to the next line
		// TODO: draw sun in orange

		Display::DrawHLineEOR(x1,x2, y, 7); // draw the line in white
	}

}


bool FTubeEliteDisplay::UpdateKeyboardBuffer(uint8_t* pBuffer)
{
	if (bWindowFocused == false)
	{
		for (int i = 2; i < 14; i++)
			pBuffer[i] = 0; // clear the buffer

		pBuffer[14] = 0x10; // Joystick 1 fire button not pressed
		return false;
	}
		

	// 	*Byte #2: If a non - primary flight control key is
	// 		being pressed, its internal key number is put here
	pBuffer[2] = GetPressedInternalKeyCode(); // no non-primary flight control key pressed

	// * Byte #3: "?" is being pressed(0 = no, &FF = yes)
	pBuffer[3] = ImGui::IsKeyDown(ImGuiKey_Slash) ? 0xff : 0xff; // "?" key pressed ?

	// * Byte #4: Space is being pressed(0 = no, &FF = yes)
	pBuffer[4] = ImGui::IsKeyDown(ImGuiKey_Space) ? 0xff : 0x00; // Space key pressed

	// * Byte #5: "<" is being pressed(0 = no, &FF = yes)
	pBuffer[5] = ImGui::IsKeyDown(ImGuiKey_Comma) ? 0xff : 0x00; // "<" key pressed

	// * Byte #6: ">" is being pressed(0 = no, &FF = yes)
	pBuffer[6] = ImGui::IsKeyDown(ImGuiKey_Period) ? 0xff : 0x00; // ">" key pressed

	// * Byte #7: "X" is being pressed(0 = no, &FF = yes)
	pBuffer[7] = ImGui::IsKeyDown(ImGuiKey_X) ? 0xff : 0x00; // "X" key pressed

	// * Byte #8: "S" is being pressed(0 = no, &FF = yes)
	pBuffer[8] = ImGui::IsKeyDown(ImGuiKey_S) ? 0xff : 0x00; // "S" key pressed

	// * Byte #9: "A" is being pressed(0 = no, &FF = yes)
	pBuffer[9] = ImGui::IsKeyDown(ImGuiKey_A) ? 0xff : 0x00; // "A" key pressed

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
	for (int key = ImGuiKey_NamedKey_BEGIN; key < ImGuiKey_COUNT; key++)
	{
		if (ImGui::IsKeyPressed((ImGuiKey)key))
		{
			if (bWindowFocused)
			{
				const int bbcKey = BBCKeyFromImGuiKey((ImGuiKey)key);
				if (bbcKey != 0 && bWindowFocused)
				{
					//bbc_key_down(&pBBCEmu->GetBBC(), bbcKey);
					//BBCKeyDown[bbcKey] = true; // mark the key as pressed
					//TODO: send to key buffer
					pTubeSys->AddInputByte(bbcKey);

				}
			}
			
		}
		else if (ImGui::IsKeyReleased((ImGuiKey)key))
		{
			const int bbcKey = BBCKeyFromImGuiKey((ImGuiKey)key);
			if (bbcKey != 0)
			{
				//BBCKeyDown[bbcKey] = false; // mark the key as released
				//bbc_key_up(&pBBCEmu->GetBBC(), bbcKey);
			}
		}
	}

	pTubeSys->GetCodeAnalysis().Debugger.SetDebugKeysEnabled(!bWindowFocused);
}

void FTubeEliteDisplay::DrawUI(void)
{
	ImGui::Begin("Tube Elite Display");
	ImGui::Text("Tube Elite Display");
	Display::RenderFrame();

	// bounding rect
	ImDrawList* drawList = ImGui::GetWindowDrawList();
	//drawList->AddRect(startPos, ImVec2(startPos.x + (kCharMapSizeX * charWidth), startPos.y + (kCharMapSizeY * charHeight)), IM_COL32(255, 255, 255, 128));
	// Show DashboardParams
	ImGui::Text("Dashboard Params:");
	ImGui::Text("Speed: %d", DashboardParams.Speed);
	ImGui::Text("Altitude: %d", DashboardParams.Altitude);
	ImGui::Text("Fuel: %d", DashboardParams.Fuel);
	ImGui::Text("Energy: %d", DashboardParams.Energy);
	ImGui::Text("For Shield: %d", DashboardParams.ForwardShield);
	ImGui::Text("Aft Shield: %d", DashboardParams.AftShield);
	ImGui::Text("Laser Temp: %d", DashboardParams.LaserTemp);
	ImGui::Text("Cabin Temp: %d", DashboardParams.CabinTemp);
	bWindowFocused = ImGui::IsWindowFocused();

	ImGui::End();

	g_VDULog.Draw("VDU Log", &bShowLog);

}