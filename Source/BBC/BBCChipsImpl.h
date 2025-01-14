#pragma once

#include "chips/chips_common.h"
#include "chips/z80.h"
#include "chips/m6502.h"
#include "chips/m6526.h"
#include "chips/beeper.h"
#include "chips/kbd.h"
#include "chips/mem.h"
#include "chips/clk.h"
#include <chips/m6522.h>

#define BBC_FREQUENCY (985248)              // clock frequency in Hz

// struct to hold the state of the BBC
typedef struct 
{
	m6502_t		cpu;
	uint64_t	pins;

	kbd_t		kbd;		// keyboard matrix state
	mem_t		mem_cpu;	// cpu memory
	bool		valid;
	chips_debug_t debug;

	// memory
	uint8_t		ram[0x8000];		// 32K RAM
	uint8_t		rom_basic[0x4000];	// 16K BASIC ROM
	uint8_t		rom_os[0x4000];		// 16K OS ROM

}bbc_t;

uint64_t _bbc_tick(bbc_t* sys, uint64_t pins);