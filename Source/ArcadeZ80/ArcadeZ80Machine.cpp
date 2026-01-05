#include "ArcadeZ80Machine.h"

#include <string.h>
#include <assert.h>
#include <vector>
#include "Debug/DebugLog.h"
#include "Util/FileUtil.h"
#include "CodeAnalyser/CodeAnalyser.h"
#include "Util/GraphicsView.h"
#include "imgui.h"
#include "ImGuiSupport/ImGuiScaling.h"

#define CHIPS_ASSERT(c) assert(c)

#define CPU_FREQUENCY (3000000)	// clock frequency in Hz 
#define VSYNC_PERIOD_4MHZ (CPU_FREQUENCY/60)
#define VBLANK_DURATION_4MHZ (((CPU_FREQUENCY/60)/525)*(525-483))

// initialize a new tube elite instance
bool FArcadeZ80Machine::Init(const FArcadeZ80MachineDesc& desc)
{
	if (desc.Debug.callback.func) 
	{ 
		CHIPS_ASSERT(desc.Debug.stopped); 
	}

	memset(&CPU, 0, sizeof(CPU));

	bValid = true;
	Debug = desc.Debug;

	// initialize the hardware
	Pins = z80_init(&CPU);
	mem_init(&Memory);

	// 64K RAM
	//mem_map_ram(&Memory, 0, 0x0000, 0x10000, RAM);

	// calculate number of ticks for VBlank (60Hz)
	//TicksPerFrame = clk_us_to_ticks(CPU_FREQUENCY, 16 * 1000);
	//FrameTicks = 0;
	
	if (InitMachine(desc) == false)
	{
		LOGERROR("Unable to init specific machine");
		return false;
	}

	return true;
}

// discard a tube elite instance
void FArcadeZ80Machine::Shutdown()
{
	CHIPS_ASSERT(bValid);
	bValid = false;
}

// reset a tube elite instance
void FArcadeZ80Machine::Reset()
{
	CHIPS_ASSERT(bValid);

	Pins |= Z80_RES;
}

void FArcadeZ80Machine::Update()
{
	UpdateScreen();
}

void FArcadeZ80Machine::DrawUI()
{
	ImVec2 pos = ImGui::GetCursorScreenPos();
	if (pScreen != nullptr)
	{
		pScreen->Draw();
	}
	DrawDebugOverlays(pos.x,pos.y);
}

//uint64_t _tube_elite_tick_cpu(tube_elite_t* sys, uint64_t pins)
void FArcadeZ80Machine::TickCPU()
{
	Pins = z80_tick(&CPU, Pins);

	// TODOL: handle memory accesses
	if (Pins & Z80_MREQ)
	{
		// TODO: Handle memory mapped ports
		
		// a memory request
		const uint16_t addr = Z80_GET_ADDR(Pins);
		if (Pins & Z80_RD) 
		{
			uint8_t value = 0;

			if (addr == 0xC000)	// video scan line
			{ 
				value = VCounter;	// Update value with VCounter for video scan line
			}
			
			else if (addr == 0xC300)	// IN0
			{
				value = 0xff;
			}
			else if (addr == 0xC320)	// IN1
			{
				value = 0xff;
			}
			else if (addr == 0xC340)	// IN2
			{
				value = 0xff;
			}
			else if (addr == 0xC360)	// DSW0
			{
				// Dip switch 0 - freeplay
				value = 0;
			}
			else if (addr == 0xC200)	// DSW1
			{
				// Dip switch 1
					// bits 0-1 : lives
					// bit	2	: Cocktail / Upright
					// bit  3	: Bonus life at 10k/50k or 20k/60k
					// bits 4-6 : difficulty 0-7, 0 = most difficult
					// bit	7	: Demo sounds on/off

				value = (3 << 0) |	// lives
					(0 << 2) |	// upright
					(0 << 3) |	// bonus life 10k/50k
					(4 << 4) |	// medium difficulty
					(1 << 7);	// demo sounds on 
			}
			else
			{ 
				value = mem_rd(&Memory, addr);
			}

			Z80_SET_DATA(Pins, value);
		}	
		else if (Pins & Z80_WR) 
		{
			if (addr == 0xC000)	// write audio command
			{

			}
			else if (addr == 0xC200)	// watchdog reset
			{
			}
			else if (addr == 0xC300)	// interrupt enable
			{
				NMIMask = (Z80_GET_DATA(Pins) & 0x01) ? true : false;
			}
			else if (addr == 0xC302)	// flip screen
			{
			}
			else if (addr == 0xC304)	// trigger audio interrupt
			{
			}
			else if (addr == 0xC308)	// video enable
			{
			}
			else if (addr == 0xC30A)	// coin counter 1
			{
			}
			else if (addr == 0xC30C)	// coin counter 2
			{
			}
			else
			{
				mem_wr(&Memory, addr, Z80_GET_DATA(Pins));
			}
		}
	}

	
}



void FArcadeZ80Machine::Tick()
{
	CHIPS_ASSERT(bValid);

	HCounter++;
	if (HCounter >= 342) 
	{
		HCounter = 0;
		VCounter++;
		if (VCounter >= 525) 
		{
			VCounter = 0;
		}
	}

	// activate NMI pin during VBLANK
	VSyncCount--;
	if (VSyncCount < 0) 
	{
		VSyncCount += VSYNC_PERIOD_4MHZ;
		VBlankCount = VBLANK_DURATION_4MHZ;
	}
	if (VBlankCount != 0) // in VBLANK
	{
		VBlankCount--;
		if (VBlankCount < 0) 
			VBlankCount = 0;
		VCounter = 0;
	}
	if (NMIMask && (VBlankCount > 0))
	{		
		Pins |= Z80_NMI;
	}
	else 
	{		
		Pins &= ~Z80_NMI;
	}

	TickCPU();

	TickCounter++;
}

// run arcade Z80 instance for given amount of micro_seconds, returns number of ticks executed
uint32_t FArcadeZ80Machine::Exec(uint32_t microSeconds)
{
	CHIPS_ASSERT(bValid);
	const uint32_t numTicks = clk_us_to_ticks(CPU_FREQUENCY, microSeconds);
	if (Debug.callback.func == nullptr)
	{
		// run without debug hook
		for (uint32_t tick = 0; tick < numTicks; tick++)
		{
			Tick();
			//pins = _tube_elite_tick(sys, pins);
		}
	}
	else
	{
		// run with debug hook
		for (uint32_t tick = 0; (tick < numTicks) && !(*Debug.stopped); tick++)
		{
			//pins = _tube_elite_tick(sys, pins);
			Tick();
			Debug.callback.func(Debug.callback.user_data, Pins);
		}
	}

	//sys->pins = pins;

	return numTicks;
}

uint32_t FArcadeZ80Machine::SaveSnapshot(FILE* fp) const
{
	CHIPS_ASSERT(fp != nullptr);
	// save a snapshot, patches pointers to zero and offsets, returns snapshot version
	chips_debug_t saveDebug = Debug;
	z80_t saveCPU = CPU;
	mem_t saveMemory = Memory;

	chips_debug_snapshot_onsave(&saveDebug);
	//z80_snapshot_onsave(&saveCPU);
	mem_snapshot_onsave(&saveMemory, (void*)this);

	fwrite(&saveDebug, sizeof(chips_debug_t), 1, fp);
	fwrite(&saveCPU, sizeof(z80_t), 1, fp);
	fwrite(&saveMemory, sizeof(mem_t), 1, fp);

	return ARCADEZ80_SNAPSHOT_VERSION;
}

bool FArcadeZ80Machine::LoadSnapshot(FILE* fp, uint32_t version)
{
	CHIPS_ASSERT(fp != nullptr);
	if (version != ARCADEZ80_SNAPSHOT_VERSION)
	{
		return false;
	}

	chips_debug_t loadDebug;
	z80_t loadCPU;
	mem_t loadMemory;
	fread(&loadDebug, sizeof(chips_debug_t), 1, fp);
	fread(&loadCPU, sizeof(z80_t), 1, fp);
	fread(&loadMemory, sizeof(mem_t), 1, fp);
	chips_debug_snapshot_onload(&loadDebug, &Debug);
	//z80_snapshot_onload(&loadCPU, &CPU);
	mem_snapshot_onload(&loadMemory, this);
	Debug = loadDebug;
	CPU = loadCPU;
	Memory = loadMemory;

	return true;
}


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
	pScreen = new FGraphicsView(256,256);

	pVideoRAM = &RAM[0xa400 - 0x6000];
	pColourRAM = &RAM[0xa000 - 0x6000];
	pSpriteRAM[0] = &RAM[0xB000 - 0x6000];
	pSpriteRAM[1] = &RAM[0xB400 - 0x6000];

	pSpriteView = new FGraphicsView(64, 64);

	
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


void FTimePilotMachine::SetupCodeAnalysisForMachine(FCodeAnalysisState& codeAnalysis)
{
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
	RAMBankId = codeAnalysis.CreateBank("RAM", 40, RAM, false, 0x6000, true);					// RAM - $6000 - $FFFF - pages 24-63 - 40K
	codeAnalysis.MapBank(RAMBankId, 24, EBankAccess::ReadWrite);
	mem_map_ram(&Memory, 0, 0x6000, 40 * 1024, RAM);


}

// byte 0, bits 7-4 : plane 0 for pixels 7-4
// byte 0, bits 3-0 : plane 1 for pixels 7-4

// byte 8, bits 7-4 : plane 0 for pixels 3-0
// byte 8, bits 3-0 : plane 1 for pixels 3-0

void DrawCharacter8x8(FGraphicsView* pView, const uint8_t* pSrc, int xp, int yp, const uint32_t* cols, bool bFlipX, bool bFlipY, bool bRot90)
{
	if(xp < 0 || yp < 0 || xp > (pView->GetWidth() - 8) || yp > (pView->GetHeight() - 8))	// primitive clipping
		return;

	uint32_t* pPixelBuffer = pView->GetPixelBuffer();
	uint32_t* pBase = pPixelBuffer + (xp + (yp * pView->GetWidth()));
	for (int y = 0; y < 8; y++)
	{
		const uint8_t charLine0 = pSrc[y];
		const uint8_t charLine1 = pSrc[y+8];

		const uint8_t plane0 = (charLine0 & 0xf0) | (charLine1 & 0x0f);
		const uint8_t plane1 = ((charLine0 & 0x0f)<<4) | ((charLine1 & 0xf0)>>4);

		for (int xpix = 0; xpix < 8; xpix++)
		{
			const int drawX = bFlipX ? (7 - xpix) : xpix;
			const int drawY = bFlipY ? (7 - y) : y;
			const bool bSet0 = (plane0 & (1 << (7 - xpix))) != 0;
			const bool bSet1 = (plane1 & (1 << (7 - xpix))) != 0;
			const int bBit0 = bSet0 ? 1 : 0;
			const int bBit1 = bSet1 ? 1 : 0;
			const uint32_t col = cols[bBit0 + (bBit1<<1)];
			if(bRot90)
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
				DrawCharacter8x8(pView, pChar, drawX, drawY, cols, bFlipY, !bFlipX, bRot90);
			else
				DrawCharacter8x8(pView, pChar, drawX, drawY, cols, bFlipX, bFlipY, bRot90);
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
			if(priority == category)
			{
				const uint32_t* pColours = TileColours[colour];
				const uint8_t* pTile = &TilesROM[vidChar * 16];
				if (bRot)
					DrawCharacter8x8(pScreen, pTile, (31 - yc) * 8, xc * 8, pColours, bFlipX, !bFlipY, true);
				else
					DrawCharacter8x8(pScreen, pTile, xc * 8, yc * 8, pColours, bFlipX, bFlipY, false);
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
		if(bRot)
			DrawSprite(pScreen, pSprite, sy, sx, pSpriteColours, bFlipx, bFlipy, true);
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
	const ImVec2 pos(x,y);
	const float scale = ImGui_GetScaling();
	ImDrawList* pDrawList = ImGui::GetWindowDrawList();

	for (int offs = 0x3e; offs >= 0x10; offs -= 2)
	{
		int const sx = pSpriteRAM[0][offs];
		int const sy = 241 - pSpriteRAM[1][offs + 1];

		int const code = pSpriteRAM[0][offs + 1];
		int const color = pSpriteRAM[1][offs] & 0x3f;
		int const flipx = ~pSpriteRAM[1][offs] & 0x40;
		int const flipy = pSpriteRAM[1][offs] & 0x80;


		pDrawList->AddRect(
			ImVec2(pos.x + sx * scale, pos.y + sy * scale),
			ImVec2(pos.x + (sx + 16) * scale, pos.y + (sy + 16) * scale),
			IM_COL32(255, 0, 0, 255));
		/*m_gfxdecode->gfx(1)->transpen(bitmap, cliprect,
			code,
			color,
			flipx, flipy,
			sx, sy, 0);
			*/
	}

	ImGui::Checkbox("Rotate Screen", &bRotateScreen);

	// Sprite Viewer
	static int SpriteNo = 0;
	static int SpriteColour = 0;
	static bool bFlipx = false;
	static bool bFlipy = false;
	static bool bRot = false;

	ImGui::InputInt("SpriteNo", &SpriteNo);
	//ImGui::SameLine();
	ImGui::InputInt("SpriteColour", &SpriteColour);
	ImGui::Checkbox("Flip X", &bFlipx);
	ImGui::SameLine();
	ImGui::Checkbox("Flip Y", &bFlipy);
	ImGui::SameLine();
	ImGui::Checkbox("Rotate 90", &bRot);

	const uint32_t* pSpriteColours = SpriteColours[SpriteColour];
	const int spriteByteSize = (4 * 16);
	const uint8_t* pSprite = &SpriteROM[SpriteNo * spriteByteSize];
	DrawSprite(pSpriteView, pSprite, 0, 0, pSpriteColours, bFlipx, bFlipy, bRot);

	/*uint32_t monoColours[2] = {0xff000000, 0xffffffff};
	pSpriteView->Draw1BppImageAt(pSprite, 0, 16, 32, 16, monoColours);
	pSpriteView->Draw1BppImageAt(pSprite, 0, 32, 16, 32, monoColours);

	DrawCharacter8x8(pSpriteView, pSprite, 16, 0, pSpriteColours, bFlipx, bFlipy, bRot);
	DrawCharacter8x8(pSpriteView, pSprite + 16, 16 + 8, 0, pSpriteColours, bFlipx, bFlipy, bRot);
	DrawCharacter8x8(pSpriteView, pSprite + 32, 16 + 0, 8, pSpriteColours, bFlipx, bFlipy, bRot);
	DrawCharacter8x8(pSpriteView, pSprite + 48, 16 + 8, 8, pSpriteColours, bFlipx, bFlipy, bRot);*/

	pSpriteView->Draw(true);
}

void FTimePilotMachine::UpdateScreen()
{
	DrawCharMap(0);
	DrawSprites();
	DrawCharMap(1);
}