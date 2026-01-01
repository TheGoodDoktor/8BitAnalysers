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


struct FArcadeZ80MachineDesc
{
	chips_debug_t	Debug;
};

class FArcadeZ80Machine
{
public:
	bool Init(const FArcadeZ80MachineDesc& desc);
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

	static const int kTubePollInterval = 100;	
	uint32_t		TubePollCounter = 0;

	bool		bValid = false;

	// memory
	uint8_t		RAM[0x10000];		// 64K RAM
};
