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
	FCodeAnalysisState* pCodeAnalysis = nullptr;
	chips_debug_t	Debug;
};

class FMachineDebug
{
public:
	virtual void DrawDebugUI() = 0;
	virtual void DrawDebugOverlays(float x, float y) = 0;

};

class FArcadeZ80Machine
{
public:
	bool Init(const FArcadeZ80MachineDesc& desc);
	virtual bool InitMachine(const FArcadeZ80MachineDesc& desc) = 0;
	virtual void SetupCodeAnalysisForMachine() = 0;
	virtual void UpdateScreen() {}
	virtual void UpdateInput() {}
	virtual void DrawDebugOverlays(float x, float y) {}
	void DrawDebugUI();
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

	uint8_t		IN0 = 0xff;	// input port 0
	uint8_t		IN1 = 0xff;	// input port 1
	uint8_t		IN2 = 0xff;	// input port 2

	uint8_t		DSW0 = 0;	// dip switch 0
	uint8_t		DSW1 = 0;	// dip switch 1

	static const int kTubePollInterval = 100;	
	uint32_t		TubePollCounter = 0;

	bool		bValid = false;

	// memory
	uint8_t		RAM[0x10000];		// 64K RAM

	FGraphicsView*	pScreen = nullptr;

	FCodeAnalysisState* pCodeAnalysis = nullptr;
	FMachineDebug*		pDebug = nullptr;
};



