#include "TubeEliteMachine.h"

#include <string.h>
#include <assert.h>

#define CHIPS_ASSERT(c) assert(c)

#define TUBE_FREQUENCY (3000000)	// clock frequency in Hz 
#define TUBE_ELITE_SNAPSHOT_VERSION (1)


// initialize a new tube elite instance
void tube_elite_init(tube_elite_t* sys, const tube_elite_desc_t* desc)
{
	CHIPS_ASSERT(sys && desc);
	if (desc->debug.callback.func) { CHIPS_ASSERT(desc->debug.stopped); }

	memset(sys, 0, sizeof(tube_elite_t));
	sys->valid = true;
	sys->debug = desc->debug;

	// initialize the hardware
	sys->pins = m65C02_init(&sys->cpu, &desc->cpu);
	mem_init(&sys->mem_cpu);

	// 64K RAM
	mem_map_ram(&sys->mem_cpu, 0, 0x0000, 0x10000, sys->ram);
}

// discard a tube elite instance
void tube_elite_discard(tube_elite_t* sys)
{
	CHIPS_ASSERT(sys && sys->valid);
	sys->valid = false;
}

// reset a tube elite instance
void tube_elite_reset(tube_elite_t* sys)
{
	CHIPS_ASSERT(sys && sys->valid);

	sys->pins |= M6502_RES;
}

uint64_t _tube_elite_tick_cpu(tube_elite_t* sys, uint64_t pins)
{
	pins = m65C02_tick(&sys->cpu, pins);

	// the IRQ and NMI pins will be set by the HW each tick
	pins &= ~(M6502_IRQ | M6502_NMI);

	const uint16_t addr = M6502_GET_ADDR(pins);

	// regular memory access
	if (pins & M6502_RW)
	{
		M6502_SET_DATA(pins, mem_rd(&sys->mem_cpu, addr));
	}
	else
	{
		mem_wr(&sys->mem_cpu, addr, M6502_GET_DATA(pins));
	}

	return pins;
}

uint64_t _tube_elite_tick(tube_elite_t* sys, uint64_t pins)
{
	CHIPS_ASSERT(sys && sys->valid);

	// tick the CPU & CRTC at alternate cycles
	// not sure if we need this and it causes problems with stepping in the debugger
	//if ((sys->tick_counter & 1) == 0)
	{
		pins = _tube_elite_tick_cpu(sys, pins);
	}
	
	sys->tick_counter++;
	return pins;
}

// run tube elite instance for given amount of micro_seconds, returns number of ticks executed
uint32_t tube_elite_exec(tube_elite_t* sys, uint32_t micro_seconds)
{
	CHIPS_ASSERT(sys && sys->valid);
	const uint32_t num_ticks = clk_us_to_ticks(TUBE_FREQUENCY, micro_seconds);
	uint64_t pins = sys->pins;
	if (0 == sys->debug.callback.func)
	{
		// run without debug hook
		for (uint32_t tick = 0; tick < num_ticks; tick++)
		{
			pins = _tube_elite_tick(sys, pins);
		}
	}
	else
	{
		// run with debug hook
		for (uint32_t tick = 0; (tick < num_ticks) && !(*sys->debug.stopped); tick++)
		{
			pins = _tube_elite_tick(sys, pins);
			sys->debug.callback.func(sys->debug.callback.user_data, pins);
		}
	}
	sys->pins = pins;

	return num_ticks;
}


// save a snapshot, patches pointers to zero and offsets, returns snapshot version
uint32_t tube_elite_save_snapshot(tube_elite_t* sys, tube_elite_t* dst)
{
	CHIPS_ASSERT(sys && dst);
	*dst = *sys;
	chips_debug_snapshot_onsave(&dst->debug);
	m65C02_snapshot_onsave(&dst->cpu);
	mem_snapshot_onsave(&dst->mem_cpu, sys);
	return TUBE_ELITE_SNAPSHOT_VERSION;
}

// load a snapshot, returns false if snapshot versions don't match
bool tube_elite_load_snapshot(tube_elite_t* sys, uint32_t version, tube_elite_t* src)
{
	CHIPS_ASSERT(sys && src);
	if (version != TUBE_ELITE_SNAPSHOT_VERSION)
	{
		return false;
	}
	static tube_elite_t im;
	im = *src;
	chips_debug_snapshot_onload(&im.debug, &sys->debug);
	m65C02_snapshot_onload(&im.cpu, &sys->cpu);
	mem_snapshot_onload(&im.mem_cpu, sys);
	*sys = im;
	return true;
}