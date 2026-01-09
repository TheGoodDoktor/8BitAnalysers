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
			DrawSprite(pScreen, pSprite, (256 - sy), sx, pSpriteColours, bFlipx, bFlipy, true);
		else
			DrawSprite(pScreen, pSprite, sx, sy, pSpriteColours, bFlipx, bFlipy, false);

		/*m_gfxdecode->gfx(1)->transpen(bitmap, cliprect,
			code,
			color,
			flipx, flipy,
			sx, sy, 0);
			*/
	}
}




void FTimePilotMachine::DrawDebugOverlays(float x, float y)
{
	const ImVec2 pos(x, y);
	const float scale = ImGui_GetScaling();
	ImDrawList* pDrawList = ImGui::GetWindowDrawList();
	const float sprSize = 16.0f * scale;

	if (bSpriteDebug)
	{
		for (int offs = 0x3e; offs >= 0x10; offs -= 2)
		{
			int const sx = pSpriteRAM[0][offs];
			int const sy = 241 - pSpriteRAM[1][offs + 1];

			int const code = pSpriteRAM[0][offs + 1];
			int const color = pSpriteRAM[1][offs] & 0x3f;
			int const flipx = ~pSpriteRAM[1][offs] & 0x40;
			int const flipy = pSpriteRAM[1][offs] & 0x80;

			int sprNo = (offs - 0x10) / 2;
			char sprNoText[8];
			snprintf(sprNoText,8,"%d",sprNo);

			if (bRotateScreen)
			{
				ImVec2 scrPos(pos.x + (256 - sy) * scale, pos.y + sx * scale);

				pDrawList->AddRect(
					scrPos,
					ImVec2(pos.x + ((256 - sy) + 16) * scale, pos.y + (sx + 16) * scale),
					IM_COL32(255, 0, 0, 255));
				pDrawList->AddText(scrPos, 0xffffffff, sprNoText);
			}
			else
			{
				ImVec2 scrPos(pos.x + sx * scale, pos.y + sy * scale);

				pDrawList->AddRect(
					scrPos,
					ImVec2(pos.x + (sx + 16) * scale, pos.y + (sy + 16) * scale),
					IM_COL32(255, 0, 0, 255));

				pDrawList->AddText(scrPos,0xffffffff,sprNoText);
			}
		}
	}

	ImGui::Checkbox("Rotate Screen", &bRotateScreen);
	ImGui::SameLine();
	ImGui::Checkbox("Sprite Debug", &bSpriteDebug);


}



void FTimePilotMachine::UpdateScreen()
{
	DrawCharMap(0);
	DrawSprites();
	DrawCharMap(1);
}