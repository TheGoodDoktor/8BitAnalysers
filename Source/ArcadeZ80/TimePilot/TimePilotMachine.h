#pragma once

#include "../ArcadeZ80Machine.h"

class FTimePilotMachine : public FArcadeZ80Machine
{
public:
	bool InitMachine(const FArcadeZ80MachineDesc& desc) override;
	void SetupCodeAnalysisForMachine() override;
	void SetupPalette();
	void UpdateScreen() override;

	void DrawDebugOverlays(float x, float y) override;

	void DrawCharMap(int priority);
	void DrawSprites();

	void DebugDrawCommandQueue();
	void DebugDrawString(uint16_t stringAddress);

	// Game ROMS
	uint8_t		ROM1[0x2000];	// 0x0000 - 0x1FFF
	uint8_t		ROM2[0x2000];	// 0x2000 - 0x3FFF
	uint8_t		ROM3[0x2000];	// 0x4000 - 0x5FFF

	uint8_t		AudioROM[0x1000];

	// GFX ROMS
	uint8_t		TilesROM[0x2000];
	uint8_t		SpriteROM[0x4000];

	// PROMS
	uint8_t		PalettePROM[0x40];
	uint8_t		SpriteLUTPROM[0x0100];
	uint8_t		CharLUTPROM[0x0100];

	// Bank Ids
	int16_t		ROM1BankId;
	int16_t		ROM2BankId;
	int16_t		ROM3BankId;
	int16_t		RAMBankId;

	int32_t		TicksPerFrame = 0;
	int32_t		FrameTicks = 0;

	uint8_t* pVideoRAM = nullptr;
	uint8_t* pColourRAM = nullptr;
	uint8_t* pSpriteRAM[2] = { nullptr, nullptr };

	uint32_t	Palette[32];
	uint32_t    TileColours[32][4];
	uint32_t	SpriteColours[64][4];

	bool		bSpriteDebug = false;
	bool		bRotateScreen = false;
};

void DrawCharacter8x8(FGraphicsView* pView, const uint8_t* pSrc, int xp, int yp, const uint32_t* cols, bool bFlipX, bool bFlipY, bool bRot90, bool bMask);
void DrawSprite(FGraphicsView* pView, const uint8_t* pSrc, int xp, int yp, const uint32_t* cols, bool bFlipX, bool bFlipY, bool bRot90);