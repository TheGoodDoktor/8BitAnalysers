#include "TimePilotMachine.h"

#include <string.h>
#include <assert.h>
#include <vector>
#include "Debug/DebugLog.h"
#include "Util/FileUtil.h"
#include "CodeAnalyser/CodeAnalyser.h"
#include "Util/GraphicsView.h"
#include "imgui.h"
#include "ImGuiSupport/ImGuiScaling.h"
#include "TimePilotDebug.h"

// Time Pilot Specifics
bool FTimePilotMachine::InitMachine(const FArcadeZ80MachineDesc& desc)
{
	// TODO: Load ROMS etc.

	LoadBinaryFileToMem("Roms/TimePilot/tm1", ROM1, 0x2000);
	LoadBinaryFileToMem("Roms/TimePilot/tm2", ROM2, 0x2000);
	LoadBinaryFileToMem("Roms/TimePilot/tm3", ROM3, 0x2000);

	LoadBinaryFileToMem("Roms/TimePilot/tm4", SpriteROM, 0x2000);
	LoadBinaryFileToMem("Roms/TimePilot/tm5", SpriteROM + 0x2000, 0x2000);

	LoadBinaryFileToMem("Roms/TimePilot/tm6", TilesROM, 0x2000);

	// Code for Audio CPU
	LoadBinaryFileToMem("Roms/TimePilot/tm7", AudioROM, 0x1000);

	// PROMS
	LoadBinaryFileToMem("Roms/TimePilot/timeplt.b4", PalettePROM + 0x0000, 0x0020);
	LoadBinaryFileToMem("Roms/TimePilot/timeplt.b5", PalettePROM + 0x0020, 0x0020);
	LoadBinaryFileToMem("Roms/TimePilot/timeplt.e9", SpriteLUTPROM, 0x0100);
	LoadBinaryFileToMem("Roms/TimePilot/timeplt.e12", CharLUTPROM, 0x0100);

	SetupPalette();

	// Graphics
	pScreen = new FGraphicsView(256, 256);

	pVideoRAM = &RAM[0xa400 - 0x6000];
	pColourRAM = &RAM[0xa000 - 0x6000];
	pSpriteRAM[0] = &RAM[0xB000 - 0x6000];
	pSpriteRAM[1] = &RAM[0xB400 - 0x6000];

	// Dip switch 1
					// bits 0-1 : lives
					// bit	2	: Cocktail / Upright
					// bit  3	: Bonus life at 10k/50k or 20k/60k
					// bits 4-6 : difficulty 0-7, 0 = most difficult
					// bit	7	: Demo sounds on/off
	// DIP switches
	DSW1 = (3 << 0) |	// lives
		(0 << 2) |	// upright
		(0 << 3) |	// bonus life 10k/50k
		(4 << 4) |	// medium difficulty
		(1 << 7);	// demo sounds on 

	pDebug = CreateTimePilotDebug(this);

	return true;
}

/***************************************************************************

  Convert the color PROMs into a more useable format.

  Time Pilot has two 32x8 palette PROMs and two 256x4 lookup table PROMs
  (one for characters, one for sprites).
  The palette PROMs are connected to the RGB output this way:

  bit 7 -- 390 ohm resistor  -- BLUE
		-- 470 ohm resistor  -- BLUE
		-- 560 ohm resistor  -- BLUE
		-- 820 ohm resistor  -- BLUE
		-- 1.2kohm resistor  -- BLUE
		-- 390 ohm resistor  -- GREEN
		-- 470 ohm resistor  -- GREEN
  bit 0 -- 560 ohm resistor  -- GREEN

  bit 7 -- 820 ohm resistor  -- GREEN
		-- 1.2kohm resistor  -- GREEN
		-- 390 ohm resistor  -- RED
		-- 470 ohm resistor  -- RED
		-- 560 ohm resistor  -- RED
		-- 820 ohm resistor  -- RED
		-- 1.2kohm resistor  -- RED
  bit 0 -- not connected

***************************************************************************/
#define BIT(x,n) ((x)>>n) & 1
#define RGB(r,g,b) ((0xff<<24) | ((b)<<16) | ((g)<<8) | (r))

void FTimePilotMachine::SetupPalette()
{
	for (int i = 0; i < 32; i++)
	{
		int bit0, bit1, bit2, bit3, bit4;

		bit0 = BIT(PalettePROM[i + 1 * 32], 1);
		bit1 = BIT(PalettePROM[i + 1 * 32], 2);
		bit2 = BIT(PalettePROM[i + 1 * 32], 3);
		bit3 = BIT(PalettePROM[i + 1 * 32], 4);
		bit4 = BIT(PalettePROM[i + 1 * 32], 5);
		int const r = 0x19 * bit0 + 0x24 * bit1 + 0x35 * bit2 + 0x40 * bit3 + 0x4d * bit4;
		bit0 = BIT(PalettePROM[i + 1 * 32], 6);
		bit1 = BIT(PalettePROM[i + 1 * 32], 7);
		bit2 = BIT(PalettePROM[i + 0 * 32], 0);
		bit3 = BIT(PalettePROM[i + 0 * 32], 1);
		bit4 = BIT(PalettePROM[i + 0 * 32], 2);
		int const g = 0x19 * bit0 + 0x24 * bit1 + 0x35 * bit2 + 0x40 * bit3 + 0x4d * bit4;
		bit0 = BIT(PalettePROM[i + 0 * 32], 3);
		bit1 = BIT(PalettePROM[i + 0 * 32], 4);
		bit2 = BIT(PalettePROM[i + 0 * 32], 5);
		bit3 = BIT(PalettePROM[i + 0 * 32], 6);
		bit4 = BIT(PalettePROM[i + 0 * 32], 7);
		int const b = 0x19 * bit0 + 0x24 * bit1 + 0x35 * bit2 + 0x40 * bit3 + 0x4d * bit4;

		Palette[i] = RGB(r, g, b);
	}

	//colourPROM += 2 * 32;
	// color_prom now points to the beginning of the lookup table


	// sprites
	for (int i = 0; i < 64; i++)
	{
		SpriteColours[i][0] = Palette[SpriteLUTPROM[(i * 4) + 0] & 0x0f];
		SpriteColours[i][1] = Palette[SpriteLUTPROM[(i * 4) + 1] & 0x0f];
		SpriteColours[i][2] = Palette[SpriteLUTPROM[(i * 4) + 2] & 0x0f];
		SpriteColours[i][3] = Palette[SpriteLUTPROM[(i * 4) + 3] & 0x0f];
	}

	// characters
	for (int i = 0; i < 32; i++)
	{
		TileColours[i][0] = Palette[CharLUTPROM[(i * 4) + 0] & 0x0f];
		TileColours[i][1] = Palette[CharLUTPROM[(i * 4) + 1] & 0x0f];
		TileColours[i][2] = Palette[CharLUTPROM[(i * 4) + 2] & 0x0f];
		TileColours[i][3] = Palette[CharLUTPROM[(i * 4) + 3] & 0x0f];
	}
	//	palette.set_pen_color(32 * 4 + i, palette_val[*colourPROM++ & 0x0f]);

	// characters
	//for (int i = 0; i < 32 * 4; i++)
	//	palette.set_pen_color(i, palette_val[(*colourPROM++ & 0x0f) + 0x10]);
}


void FTimePilotMachine::SetupCodeAnalysisForMachine()
{
	FCodeAnalysisState& codeAnalysis = *pCodeAnalysis;

	// Set up memory banks
	ROM1BankId = codeAnalysis.CreateBank("ROM1", 8, ROM1, true, 0x0000, true);
	ROM2BankId = codeAnalysis.CreateBank("ROM2", 8, ROM2, true, 0x2000, true);
	ROM3BankId = codeAnalysis.CreateBank("ROM3", 8, ROM3, true, 0x4000, true);

	// map in banks
	codeAnalysis.MapBank(ROM1BankId, 0, EBankAccess::ReadWrite);
	codeAnalysis.MapBank(ROM2BankId, 8, EBankAccess::ReadWrite);
	codeAnalysis.MapBank(ROM3BankId, 16, EBankAccess::ReadWrite);
	mem_map_rom(&Memory, 0, 0x0000, 0x2000, ROM1);
	mem_map_rom(&Memory, 0, 0x2000, 0x2000, ROM2);
	mem_map_rom(&Memory, 0, 0x4000, 0x2000, ROM3);

	// RAM
	memset(&RAM, 0, sizeof(RAM));
	RAMBankId = codeAnalysis.CreateBank("RAM", 40, RAM, false, 0x6000, true);					// RAM - $6000 - $FFFF - pages 24-63 - 40K
	codeAnalysis.MapBank(RAMBankId, 24, EBankAccess::ReadWrite);
	mem_map_ram(&Memory, 0, 0x6000, 40 * 1024, RAM);
}

// byte 0, bits 7-4 : plane 0 for pixels 7-4
// byte 0, bits 3-0 : plane 1 for pixels 7-4

// byte 8, bits 7-4 : plane 0 for pixels 3-0
// byte 8, bits 3-0 : plane 1 for pixels 3-0

void DrawCharacter8x8(FGraphicsView* pView, const uint8_t* pSrc, int xp, int yp, const uint32_t* cols, bool bFlipX, bool bFlipY, bool bRot90, bool bMask)
{
	if (xp < 0 || yp < 0 || xp >(pView->GetWidth() - 8) || yp >(pView->GetHeight() - 8))	// primitive clipping
		return;

	uint32_t* pPixelBuffer = pView->GetPixelBuffer();
	uint32_t* pBase = pPixelBuffer + (xp + (yp * pView->GetWidth()));
	for (int y = 0; y < 8; y++)
	{
		const uint8_t charLine0 = pSrc[y];
		const uint8_t charLine1 = pSrc[y + 8];

		const uint8_t plane0 = (charLine0 & 0xf0) | (charLine1 & 0x0f);
		const uint8_t plane1 = ((charLine0 & 0x0f) << 4) | ((charLine1 & 0xf0) >> 4);

		for (int xpix = 0; xpix < 8; xpix++)
		{
			const int drawX = bFlipX ? (7 - xpix) : xpix;
			const int drawY = bFlipY ? (7 - y) : y;
			const bool bSet0 = (plane0 & (1 << (7 - xpix))) != 0;
			const bool bSet1 = (plane1 & (1 << (7 - xpix))) != 0;
			const int bBit0 = bSet0 ? 1 : 0;
			const int bBit1 = bSet1 ? 1 : 0;
			const uint32_t col = cols[bBit0 + (bBit1 << 1)];
			if (bMask && (col == cols[0]))
				continue;
			if (bRot90)
				*(pBase + drawY + (drawX * pView->GetWidth())) = col;
			else
				*(pBase + drawX + (drawY * pView->GetWidth())) = col;
		}
	}
}

// write nybble tp 4bpp ZXM image
void DrawZXNPixel(uint8_t* pZXNImage, int stride, int xpos, int ypos, uint8_t val)
{
	assert(val >= 0x0f);	// be sure it's a nybble

	uint8_t* pDest = pZXNImage + (ypos * stride) + (xpos>>1);

	if ((xpos & 1) == 0)	// even bits are high nybble
		*pDest = (*pDest & 0x0f) & (val << 4);
	else
		*pDest = (*pDest & 0xf0) & val;
}

void DrawCharacterToZXNImage(uint8_t* pZXNImage, int stride, const uint8_t* pSrc, int xpos,int ypos, bool bFlipX, bool bFlipY, bool bRot90)
{
	for (int y = 0; y < 8; y++)
	{
		const uint8_t charLine0 = pSrc[y];
		const uint8_t charLine1 = pSrc[y + 8];

		const uint8_t plane0 = (charLine0 & 0xf0) | (charLine1 & 0x0f);
		const uint8_t plane1 = ((charLine0 & 0x0f) << 4) | ((charLine1 & 0xf0) >> 4);

		for (int xpix = 0; xpix < 8; xpix++)
		{
			const int drawX = bFlipX ? (7 - xpix) : xpix;
			const int drawY = bFlipY ? (7 - y) : y;
			const bool bSet0 = (plane0 & (1 << (7 - xpix))) != 0;
			const bool bSet1 = (plane1 & (1 << (7 - xpix))) != 0;
			const int bBit0 = bSet0 ? 1 : 0;
			const int bBit1 = bSet1 ? 1 : 0;

			uint8_t col = bBit0 + (bBit1 << 1);

			/*
			const uint32_t col = cols[bBit0 + (bBit1 << 1)];
			if (bMask && (col == cols[0]))
				continue;*/
			if (bRot90)
				DrawZXNPixel(pZXNImage,stride, drawY,drawX,col);
			else
				DrawZXNPixel(pZXNImage, stride, drawX, drawY, col);				
		}
	}
}

// draw 16*16 sprite using 2 bit planes
void DrawSprite(FGraphicsView* pView, const uint8_t* pSrc, int xp, int yp, const uint32_t* cols, bool bFlipX, bool bFlipY, bool bRot90)
{
	for (int y = 0; y < 2; y++)
	{
		for (int x = 0; x < 2; x++)
		{
			const uint8_t* pChar = pSrc + (16 * (x + (y * 2)));

			int fx = bFlipX ? (1 - x) : x;
			int fy = bFlipY ? (1 - y) : y;

			int rx = bRot90 ? (1 - fy) : fx;
			int ry = bRot90 ? fx : fy;

			int drawX = xp + (rx * 8);
			int drawY = yp + (ry * 8);
			/*if (bFlipX)
				drawX = xp + ((1 - rx) * 8);
			if (bFlipY)
				drawY = yp + ((1 - ry) * 8);
				*/
			if (bRot90)
				DrawCharacter8x8(pView, pChar, drawX, drawY, cols, bFlipX, !bFlipY, bRot90, true);
			else
				DrawCharacter8x8(pView, pChar, drawX, drawY, cols, bFlipX, bFlipY, bRot90, true);
		}
	}
}

void DrawSpriteToZXNImage(uint8_t* pZXNImage, int stride, const uint8_t* pSrc, int xp, int yp)
{
	bool bFlipX = false;
	bool bFlipY = false;
	bool bRot90 = true;
	
	for (int y = 0; y < 2; y++)
	{
		for (int x = 0; x < 2; x++)
		{
			const uint8_t* pChar = pSrc + (16 * (x + (y * 2)));

			int fx = bFlipX ? (1 - x) : x;
			int fy = bFlipY ? (1 - y) : y;

			int rx = bRot90 ? (1 - fy) : fx;
			int ry = bRot90 ? fx : fy;

			int drawX = xp + (rx * 8);
			int drawY = yp + (ry * 8);
			/*if (bFlipX)
				drawX = xp + ((1 - rx) * 8);
			if (bFlipY)
				drawY = yp + ((1 - ry) * 8);
				*/
			if (bRot90)
				DrawCharacterToZXNImage(pZXNImage, stride, pChar, drawX, drawY, bFlipX, !bFlipY, bRot90);
			else
				DrawCharacterToZXNImage(pZXNImage, stride, pChar, drawX, drawY, bFlipX, bFlipY, bRot90);
		}
	}
}

void FTimePilotMachine::DrawCharMap(int priority)
{
	bool bRot = bRotateScreen;	// Rotation so screen is facing correct direction

	// TODO: update the screen image using the VideoRAM & ColourRAM
	for (int yc = 0; yc < 32; yc++)
	{
		for (int xc = 0; xc < 32; xc++)
		{
			const int charIndex = xc + (yc * 32);
			const uint8_t attr = pColourRAM[charIndex];
			const int vidChar = pVideoRAM[charIndex] + 8 * (attr & 0x20);
			const uint8_t colour = attr & 0x01f;
			const bool bFlipX = (attr & 0x40) != 0;
			const bool bFlipY = (attr & 0x80) != 0;
			const uint8_t category = (attr & 0x10) >> 4;	// ??
			if (priority == category)
			{
				const uint32_t* pColours = TileColours[colour];
				const uint8_t* pTile = &TilesROM[vidChar * 16];
				if (bRot)
					DrawCharacter8x8(pScreen, pTile, (31 - yc) * 8, xc * 8, pColours, bFlipX, !bFlipY, true, false);
				else
					DrawCharacter8x8(pScreen, pTile, xc * 8, yc * 8, pColours, bFlipX, bFlipY, false, false);
			}

		}
	}
}

void FTimePilotMachine::DrawSprites()
{
	bool bRot = bRotateScreen;	// Rotation so screen is facing correct direction

	for (int offs = 0x3e; offs >= 0x10; offs -= 2)
	{
		int const sx = pSpriteRAM[0][offs];
		int const sy = 241 - pSpriteRAM[1][offs + 1];

		int const code = pSpriteRAM[0][offs + 1];
		int const colour = pSpriteRAM[1][offs] & 0x3f;
		const bool bFlipx = (~pSpriteRAM[1][offs] & 0x40) != 0;
		const bool bFlipy = (pSpriteRAM[1][offs] & 0x80) != 0;

		const uint32_t* pSpriteColours = SpriteColours[colour];

		const int spriteByteSize = (4 * 16);
		const uint8_t* pSprite = &SpriteROM[code * spriteByteSize];
		if (bRot)
			DrawSprite(pScreen, pSprite, (256 - sy - 16), sx, pSpriteColours, bFlipx, bFlipy, true);
		else
			DrawSprite(pScreen, pSprite, sx, sy, pSpriteColours, bFlipx, bFlipy, false);
	}
}

static const char* kIN0Controls[] = {
	"Coin 1",
	"Coin 2",
	"Start 1",
	"Start 2",
	"Up",
	"Down",
	"Left",
	"Right"
}; 

static const char* kIN1Controls[] = {
	"Button 1",
	"Button 2",
	"Unused 2",
	"Unused 3",
	"Unused 4",
	"Unused 5",
	"Unused 6",
	"Unused 7"
};

static const char* kIn2Controls[] = {
	"Unused 0",
	"Unused 1",
	"Unused 2",
	"Unused 3",
	"Unused 4",
	"Unused 5",
	"Unused 6",
	"Unused 7"
};

static const char* kDSW0Controls[] = {
	"DSW0-0",
	"DSW0-1",
	"DSW0-2",
	"DSW0-3",
	"DSW0-4",
	"DSW0-5",
	"DSW0-6",
	"DSW0-7"
};

static const char* kDSW1Controls[] = {
	"DSW1-0",
	"DSW1-1",
	"DSW1-2",
	"DSW1-3",
	"DSW1-4",
	"DSW1-5",
	"DSW1-6",
	"DSW1-7"
};


void DrawBitfieldUI(const char* label, uint8_t& portValue, const char** controlNames)
{
	ImGui::Text("%s", label);
	for (int i = 0; i < 8; i++)
	{
		bool bSet = (portValue & (1 << i)) == 0;	// active low
		if (ImGui::Checkbox(controlNames[i], &bSet))
		{
			if (bSet)
				portValue &= ~(1 << i);	// active low
			else
				portValue |= (1 << i);
		}
		ImGui::SameLine();
	}
	ImGui::NewLine();
}




void FTimePilotMachine::DrawDebugOverlays(float x, float y)
{
	ImGui::Checkbox("Rotate Screen", &bRotateScreen);
	pDebug->DrawDebugOverlays(x, y);

	if (ImGui::CollapsingHeader("Controls/DIPs"))
	{
		// Inputs
		DrawBitfieldUI("IN0 Controls", IN0, kIN0Controls);
		DrawBitfieldUI("IN1 Controls", IN1, kIN1Controls);
		DrawBitfieldUI("IN2 Controls", IN2, kIn2Controls);

		// DIP Switches
		DrawBitfieldUI("DSW0", DSW0, kDSW0Controls);
		DrawBitfieldUI("DSW1", DSW1, kDSW1Controls);
	}
}

const uint8_t kInputBit_Player1Start = 0x08;
const uint8_t kInputBit_Player2Start = 0x10;

const uint8_t kInputBit_Coin = 0x01;

// Player Controls
const uint8_t kInputBit_Down	= 0x01;
const uint8_t kInputBit_Up		= 0x02;
const uint8_t kInputBit_Left	= 0x04;
const uint8_t kInputBit_Right	= 0x08;
const uint8_t kInputBit_Fire	= 0x10;

void SetInputBit(ImGuiKey key, ImGuiKey padKey, uint8_t& input, uint8_t bit)
{
	if (ImGui::IsKeyDown(key) || ImGui::IsKeyDown(padKey))
		input &= ~bit;	// active low
	else
		input |= bit;
}

void FTimePilotMachine::UpdateInput()
{
	SetInputBit(ImGuiKey_1, ImGuiKey_GamepadStart, IN0, kInputBit_Player1Start);	// player 1 start
	SetInputBit(ImGuiKey_2, ImGuiKey_GamepadL1, IN0, kInputBit_Player2Start);	// player 2 start
	SetInputBit(ImGuiKey_3, ImGuiKey_GamepadR1, IN0, kInputBit_Coin);			// coin

	if (bRotateScreen)
	{
		SetInputBit(ImGuiKey_W, ImGuiKey_GamepadDpadLeft, IN1, kInputBit_Down);
		SetInputBit(ImGuiKey_S, ImGuiKey_GamepadDpadRight, IN1, kInputBit_Up);
		SetInputBit(ImGuiKey_A, ImGuiKey_GamepadDpadUp, IN1, kInputBit_Left);
		SetInputBit(ImGuiKey_D, ImGuiKey_GamepadDpadDown, IN1, kInputBit_Right);
	}
	else
	{
		SetInputBit(ImGuiKey_W, ImGuiKey_GamepadDpadUp, IN1, kInputBit_Up);
		SetInputBit(ImGuiKey_S, ImGuiKey_GamepadDpadDown, IN1, kInputBit_Down);
		SetInputBit(ImGuiKey_A, ImGuiKey_GamepadDpadLeft, IN1, kInputBit_Left);
		SetInputBit(ImGuiKey_D, ImGuiKey_GamepadDpadRight, IN1, kInputBit_Right);
	}

	SetInputBit(ImGuiKey_M, ImGuiKey_GamepadFaceDown, IN1, kInputBit_Fire);
}

void FTimePilotMachine::UpdateScreen()
{
	DrawCharMap(0);
	DrawSprites();
	DrawCharMap(1);
}

// ZXN Exporting
void FTimePilotMachine::ExportZXNSprites()
{

}

void FTimePilotMachine::ExportZXNChars()
{

}

void FTimePilotMachine::ExportZXNPalettes()
{

}