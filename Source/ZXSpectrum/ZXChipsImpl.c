#define CHIPS_IMPL
#include "ZXChipsImpl.h"

#define CHIPS_UTIL_IMPL
#include "util/z80dasm.h"
#include "util/m6502dasm.h"

// decode whole screen
void ZXDecodeScreen(zx_t* pZX)
{
	const int oldScanlineVal = pZX->scanline_y;
	pZX->scanline_y = 0;
	for (int i = 0; i < pZX->frame_scan_lines; i++)
	{
		_zx_decode_scanline(pZX);
	}
	pZX->scanline_y = oldScanlineVal;
}

static uint64_t FloatingBusTick(zx_t* sys, uint64_t pins)
{
	if (pins & Z80_IORQ && pins & Z80_RD)
	{
		if ((pins & Z80_A0) == 0)	// ULA
		{

		}
		else if ((pins & (Z80_A7 | Z80_A6 | Z80_A5)) == 0)	// Kempston
		{
		}
		else if (pins & 0xff)   // floating bus
		{
			const int top_decode_line = sys->top_border_scanlines - 32;
			const uint16_t y = sys->scanline_y - top_decode_line;
			const uint8_t* vidmem_bank = sys->ram[sys->display_ram_bank];

			if ((y < 32) || (y >= 224))
			{
				Z80_SET_DATA(pins, sys->border_color);
			}
			else
			{
				const uint16_t yy = y - 32;
				const uint16_t clr_offset = 0x1800 + (((yy & ~0x7) << 2));
				const uint8_t clr = vidmem_bank[clr_offset];
				Z80_SET_DATA(pins, clr);
			}

		}
	}
	return pins;
}

uint32_t ZXExeEmu(zx_t* sys, uint32_t micro_seconds) 
{
	CHIPS_ASSERT(sys && sys->valid);
	const uint32_t num_ticks = clk_us_to_ticks(sys->freq_hz, micro_seconds);
	uint64_t pins = sys->pins;
	
	if (0 == sys->debug.callback.func) 
	{
		// run without debug hook
		for (uint32_t tick = 0; tick < num_ticks; tick++) 
		{
			pins = _zx_tick(sys, pins);
			pins = FloatingBusTick(sys, pins);
		}
	}
	else 
	{
		// run with debug hook
		for (uint32_t tick = 0; (tick < num_ticks) && !(*sys->debug.stopped); tick++) 
		{
			pins = _zx_tick(sys, pins);
			pins = FloatingBusTick(sys, pins);
			sys->debug.callback.func(sys->debug.callback.user_data, pins);
		}
	}
	sys->pins = pins;
	kbd_update(&sys->kbd, micro_seconds);
	return num_ticks;
}