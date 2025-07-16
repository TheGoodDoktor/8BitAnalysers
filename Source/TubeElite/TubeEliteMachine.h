#pragma once

#include "chips/chips_common.h"
#include "chips/z80.h"
#include "chips/m65C02.h"
#include "chips/mc6845.h"
#include "chips/beeper.h"
#include "chips/kbd.h"
#include "chips/mem.h"
#include "chips/clk.h"

typedef struct
{
	m65C02_desc_t		cpu;
	chips_debug_t		debug;
	chips_audio_desc_t	audio;

} tube_elite_desc_t;

typedef struct
{
	m65C02_t		cpu;
	uint64_t	pins;
	mem_t		mem_cpu;	// cpu memory
	chips_debug_t	debug;
	uint32_t	tick_counter;

	bool		valid;
	// memory
	uint8_t		ram[0x10000];		// 64K RAM
}tube_elite_t;

// initialize a new tube elite instance
void tube_elite_init(tube_elite_t* sys, const tube_elite_desc_t* desc);
// discard a tube elite instance
void tube_elite_discard(tube_elite_t* sys);
// reset a tube elite instance
void tube_elite_reset(tube_elite_t* sys);
// run tube elite instance for given amount of micro_seconds, returns number of ticks executed
uint32_t tube_elite_exec(tube_elite_t* sys, uint32_t micro_seconds);

// save a snapshot, patches pointers to zero and offsets, returns snapshot version
uint32_t tube_elite_save_snapshot(tube_elite_t* sys, tube_elite_t* dst);
// load a snapshot, returns false if snapshot versions don't match
bool tube_elite_load_snapshot(tube_elite_t* sys, uint32_t version, tube_elite_t* src);