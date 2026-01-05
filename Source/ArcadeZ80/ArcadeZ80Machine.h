#pragma once

#include "chips/chips_common.h"
#include "chips/z80.h"
#include "chips/beeper.h"
#include "chips/kbd.h"
#include "chips/mem.h"
#include "chips/clk.h"

#include <cstring>
#include <stdio.h>

#define ARCADEZ80_SNAPSHOT_VERSION (1)

class FCodeAnalysisState;
class FGraphicsView;

struct FArcadeZ80MachineDesc
{
	chips_debug_t	Debug;
};

class FArcadeZ80Machine
{
public:
	bool Init(const FArcadeZ80MachineDesc& desc);
	virtual bool InitMachine(const FArcadeZ80MachineDesc& desc) = 0;
	virtual void SetupCodeAnalysisForMachine(FCodeAnalysisState& codeAnalysis) = 0;
	virtual void UpdateScreen() {}
	virtual void DrawDebugOverlays(float x, float y) {}
	void Update();
	void DrawUI();
	void Shutdown();
	void Reset();
	uint32_t Exec(uint32_t microSeconds);

	uint32_t SaveSnapshot(FILE* fp) const;
	bool LoadSnapshot(FILE* fp, uint32_t version);
private:
	void TickCPU();
	void Tick();

public:
	z80_t			CPU;
	uint64_t		Pins = 0;
	mem_t			Memory;	// cpu memory
	chips_debug_t	Debug;
	uint32_t		TickCounter;
	int			VSyncCount = 0;
	int			VBlankCount = 0;
	int			VCounter = 0;
	int			HCounter = 0;
	bool		NMIMask = false;

	static const int kTubePollInterval = 100;	
	uint32_t		TubePollCounter = 0;

	bool		bValid = false;

	// memory
	uint8_t		RAM[0x10000];		// 64K RAM

	FGraphicsView*	pScreen = nullptr;

};

class FTimePilotMachine : public FArcadeZ80Machine
{
public:
	bool InitMachine(const FArcadeZ80MachineDesc& desc) override;
	void SetupCodeAnalysisForMachine(FCodeAnalysisState& codeAnalysis) override;
	void SetupPalette();
	void UpdateScreen() override;

	void DrawDebugOverlays(float x,float y) override;
	void DrawCharMap(int priority);
	void DrawSprites();

	// Game ROMS
	uint8_t		ROM1[0x2000];	// 0x0000 - 0x1FFF
	uint8_t		ROM2[0x2000];	// 0x2000 - 0x3FFF
	uint8_t		ROM3[0x2000];	// 0x4000 - 0x5FFF

	uint8_t		AudioROM[0x1000];	

	// GFX ROMS
	uint8_t		TilesROM[0x2000];
	uint8_t		SpriteROM[0x4000];

	// PROMS
	//uint8_t		PROMs[0x0240];
	uint8_t		PalettePROM[0x40];
	//uint8_t		Palette2PROM[0x20];
	uint8_t		SpriteLUTPROM[0x0100];
	uint8_t		CharLUTPROM[0x0100];

	// RAM
	//uint8_t		VideoRAM[0x800];
	//uint8_t		ColourRAM[0x400];
	//uint8_t		SpriteRAM[0x800];

	// Bank Ids
	int16_t		ROM1BankId;
	int16_t		ROM2BankId;
	int16_t		ROM3BankId;
	int16_t		RAMBankId;

	//int16_t		TileROMBankId;
	//int16_t		SpriteROM1BankId;
	//int16_t		SpriteROM2BankId;

	//int16_t		VideoRAMBankId;
	//int16_t		SpriteRAMBankId;

	int32_t		TicksPerFrame = 0;
	int32_t		FrameTicks = 0;

	uint8_t*	pVideoRAM = nullptr;
	uint8_t*	pColourRAM = nullptr;
	uint8_t*	pSpriteRAM[2] = {nullptr, nullptr};

	uint32_t	Palette[32];
	uint32_t    TileColours[32][4];
	uint32_t	SpriteColours[64][4];


	FGraphicsView* pSpriteView = nullptr;
};
