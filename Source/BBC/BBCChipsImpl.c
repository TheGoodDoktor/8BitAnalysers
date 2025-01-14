#define CHIPS_IMPL
#include "BBCChipsImpl.h"

#define CHIPS_UTIL_IMPL
#include "util/m6502dasm.h"
#include "util/z80dasm.h"


uint32_t BBCExecEmu(bbc_t * sys, uint32_t micro_seconds)
{
	CHIPS_ASSERT(sys && sys->valid);
	uint32_t num_ticks = clk_us_to_ticks(BBC_FREQUENCY, micro_seconds);
	uint64_t pins = sys->pins;
	if (sys->debug.callback.func == NULL)
	{
		// run without debug callback
		for (uint32_t ticks = 0; ticks < num_ticks; ticks++)
		{
			pins = _bbc_tick(sys, pins);
		}
	}
	else
	{
		// run with debug callback
		for (uint32_t ticks = 0; (ticks < num_ticks) && !(*sys->debug.stopped); ticks++)
		{
			pins = _bbc_tick(sys, pins);
			sys->debug.callback.func(sys->debug.callback.user_data, pins);
		}
	}
	sys->pins = pins;
	kbd_update(&sys->kbd, micro_seconds);
	return num_ticks;
}


uint64_t _bbc_tick(bbc_t* sys, uint64_t pins)
{
	CHIPS_ASSERT(sys && sys->valid);
	
	return pins;
}