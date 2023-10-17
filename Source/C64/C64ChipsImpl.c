#define CHIPS_IMPL

#include "chips/chips_common.h"
#include "chips/z80.h"
#include "chips/m6502.h"
#include "chips/m6526.h"
#include "chips/m6569.h"
#include "chips/m6581.h"
#include "chips/beeper.h"
#include "chips/kbd.h"
#include "chips/mem.h"
#include "chips/clk.h"
#include <chips/m6502.h>
#include <chips/m6522.h>
#include <chips/m6569.h>
#include <systems/c1530.h>
#include <systems/c1541.h>
#include <systems/c64.h>

#define CHIPS_UTIL_IMPL
#include "util/m6502dasm.h"
#include "util/z80dasm.h"


uint32_t C64ExecEmu(c64_t* sys, uint32_t micro_seconds) 
{
	CHIPS_ASSERT(sys && sys->valid);
	uint32_t num_ticks = clk_us_to_ticks(C64_FREQUENCY, micro_seconds);
	uint64_t pins = sys->pins;
	if (sys->debug.callback.func == NULL) 
	{
		// run without debug callback
		for (uint32_t ticks = 0; ticks < num_ticks; ticks++) 
		{
			pins = _c64_tick(sys, pins);
		}
	}
	else 
	{
		// run with debug callback
		for (uint32_t ticks = 0; (ticks < num_ticks) && !(*sys->debug.stopped); ticks++) 
		{
			pins = _c64_tick(sys, pins);
			sys->debug.callback.func(sys->debug.callback.user_data, pins);
		}
	}
	sys->pins = pins;
	kbd_update(&sys->kbd, micro_seconds);
	return num_ticks;
}