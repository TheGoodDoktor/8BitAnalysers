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

// Additional tick to support floating bus
static uint64_t FloatingBusTick(zx_t* sys, uint64_t pins)
{
	if (pins & Z80_IORQ && pins & Z80_RD)
	{
		if ((pins & Z80_A0) == 0)	// ULA
		{
			// TODOO: if we want to emulate tape then we could put something here
			// be careful not to lose the keyboard bits
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
				Z80_SET_DATA(pins, (uint64_t)sys->border_color);
			}
			else
			{
				const uint16_t yy = y - 32;
				const uint16_t clr_offset = 0x1800 + (((yy & ~0x7) << 2));
				const uint8_t clr = vidmem_bank[clr_offset];
				Z80_SET_DATA(pins, (uint64_t)clr);
			}

		}
	}
	return pins;
}


uint64_t ReadInputIOTick(uint64_t pins, GetIOInput ioInputCB, void* pUserData)
{
	if((pins & Z80_CTRL_PIN_MASK) == (Z80_IORQ | Z80_RD))
	{
		uint8_t inVal = 0;
		const uint16_t port = Z80_GET_ADDR(pins);

		if (ioInputCB(port, &inVal,pUserData))
		{
			Z80_SET_DATA(pins, (uint64_t)inVal);
		}
	}

	return pins;
}

uint32_t ZXExeEmu(zx_t* sys, uint32_t micro_seconds) 
{
	CHIPS_ASSERT(sys && sys->valid);
	const uint32_t num_ticks = clk_us_to_ticks(sys->freq_hz, micro_seconds);
	uint64_t pins = sys->pins;
	
	if (sys->debug.callback.func == NULL) 
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

uint32_t clk_ticks_to_us(uint64_t freq_hz, uint32_t ticks) 
{
	return (uint32_t)((ticks * 1000000) / freq_hz);
}

uint32_t ZXExeEmu_UseFetchCount(zx_t* sys, uint32_t noFetches, GetIOInput ioInputCB, void* pUserData)
{
	CHIPS_ASSERT(sys && sys->valid);
	uint64_t pins = sys->pins;
	uint32_t fetchCount = 0;
	uint32_t tickCount = 0;

	uint32_t rCounter = 0;

	if (sys->debug.callback.func == NULL)
	{
		// run without debug hook
		while(fetchCount < noFetches)
		{
			pins = _zx_tick(sys, pins);
			pins = FloatingBusTick(sys, pins);
			if (ioInputCB)
				pins = ReadInputIOTick(pins, ioInputCB, pUserData);

			if (z80_opdone(&sys->cpu))
			{
				const uint16_t pc = pins & 0xffff;
				const uint8_t opcode = mem_rd(&sys->mem, pc);
				if (opcode == 0xED || opcode == 0xCB)
					fetchCount++;
				else if (opcode == 0xDD || opcode == 0xFD)
				{
					fetchCount++;
					const uint8_t opcode2 = mem_rd(&sys->mem, pc + 1);
					if (opcode2 == 0xCB)
						fetchCount++;
				}

				fetchCount++;
			}
			tickCount++;
		}
	}
	else
	{
		// run with debug hook
		while (fetchCount < noFetches && !(*sys->debug.stopped))
		{
			/*switch (sys->cpu.step)
			{
			case 1:
			case 4:
			case 23:
			case 26:
			case 1471:
			case 1490:
			case 1507:
				fetchCount++;
			}*/

			pins = _zx_tick(sys, pins);
			pins = FloatingBusTick(sys, pins);
			if (ioInputCB)
				pins = ReadInputIOTick(pins, ioInputCB, pUserData);
			sys->debug.callback.func(sys->debug.callback.user_data, pins);
			if (z80_opdone(&sys->cpu))
			{
				const uint16_t pc = pins & 0xffff;
				const uint8_t opcode = mem_rd(&sys->mem, pc);
				if (opcode == 0xED || opcode == 0xCB)
					fetchCount++;
				else if (opcode == 0xDD || opcode == 0xFD)
				{
					fetchCount++;
					const uint8_t opcode2 = mem_rd(&sys->mem, pc+1);
					if(opcode2 == 0xCB)
						fetchCount++;
				}

				fetchCount++;
			}

			tickCount++;
		}
	}

	sys->pins = pins;
	kbd_update(&sys->kbd, clk_ticks_to_us(sys->freq_hz, tickCount));

	return fetchCount;
}