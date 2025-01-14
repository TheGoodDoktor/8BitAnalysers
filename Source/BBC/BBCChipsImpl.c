#define CHIPS_IMPL
#include "BBCChipsImpl.h"

#define CHIPS_UTIL_IMPL
#include "util/m6502dasm.h"
#include "util/z80dasm.h"

uint64_t _bbc_tick(bbc_t* sys, uint64_t pins);

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

// BBC Implementation


void bbc_init(bbc_t* sys, const bbc_desc_t* desc) 
{
	CHIPS_ASSERT(sys && desc);
	if (desc->debug.callback.func) { CHIPS_ASSERT(desc->debug.stopped); }

	memset(sys, 0, sizeof(bbc_t));
	sys->valid = true;
	sys->debug = desc->debug;
	//sys->audio.callback = desc->audio.callback;
	//sys->audio.num_samples = _CPC_DEFAULT(desc->audio.num_samples, CPC_DEFAULT_AUDIO_SAMPLES);
	//CHIPS_ASSERT(sys->audio.num_samples <= CPC_MAX_AUDIO_SAMPLES);
	
	// initialize ROMS
	CHIPS_ASSERT(desc->roms.os.ptr && (desc->roms.os.size == 0x4000));
	CHIPS_ASSERT(desc->roms.basic.ptr && (desc->roms.basic.size == 0x4000));
	memcpy(sys->rom_os, desc->roms.os.ptr, 0x4000);
	memcpy(sys->rom_basic, desc->roms.basic.ptr, 0x4000);
	
	// initialize the hardware
	sys->pins = m6502_init(&sys->cpu,&desc->cpu);

	mc6845_init(&sys->crtc, MC6845_TYPE_UM6845R);
	mem_init(&sys->mem_cpu);

	// initial memory mapping
	mem_map_ram(&sys->mem_cpu, 0, 0x0000, 0x8000, sys->ram);
	mem_map_rom(&sys->mem_cpu, 0, 0x8000, 0x4000, sys->rom_basic);
	mem_map_rom(&sys->mem_cpu, 0, 0xC000, 0x4000, sys->rom_os);
}

void bbc_discard(bbc_t* sys) 
{
	CHIPS_ASSERT(sys && sys->valid);
	sys->valid = false;
}

void bbc_reset(bbc_t* sys) 
{
	CHIPS_ASSERT(sys && sys->valid);

	sys->pins |= M6502_RES;
	mc6845_reset(&sys->crtc);
}

// get display requirements and framebuffer content, may be called with nullptr
chips_display_info_t bbc_display_info(bbc_t* sys)
{
	const chips_display_info_t res = {
			.frame = {
				.dim = {
					.width = BBC_FRAMEBUFFER_WIDTH,
					.height = BBC_FRAMEBUFFER_HEIGHT,
				},
				.bytes_per_pixel = 1,
				.buffer = {
					.ptr = sys ? sys->fb : 0,
					.size = BBC_FRAMEBUFFER_SIZE_BYTES,
				}
			},
			.screen = {
				.x = 0,
				.y = 0,
				.width = BBC_DISPLAY_WIDTH,
				.height = BBC_DISPLAY_HEIGHT,
			},
			.palette = {
				.ptr = 0,
				.size = BBC_NUM_HWCOLORS * sizeof(uint32_t)
			}
	};
	CHIPS_ASSERT(((sys == 0) && (res.frame.buffer.ptr == 0)) || ((sys != 0) && (res.frame.buffer.ptr != 0)));
	CHIPS_ASSERT(((sys == 0) && (res.palette.ptr == 0)) || ((sys != 0) && (res.palette.ptr != 0)));
	return res;
}

// run BBC instance for given amount of micro_seconds, returns number of ticks executed
uint32_t bbc_exec(bbc_t* sys, uint32_t micro_seconds)
{
	CHIPS_ASSERT(sys && sys->valid);
	const uint32_t num_ticks = clk_us_to_ticks(BBC_FREQUENCY, micro_seconds);
	uint64_t pins = sys->pins;
	if (0 == sys->debug.callback.func) 
	{
		// run without debug hook
		for (uint32_t tick = 0; tick < num_ticks; tick++) 
		{
			pins = _bbc_tick(sys, pins);
		}
	}
	else 
	{
		// run with debug hook
		for (uint32_t tick = 0; (tick < num_ticks) && !(*sys->debug.stopped); tick++) 
		{
			pins = _bbc_tick(sys, pins);
			sys->debug.callback.func(sys->debug.callback.user_data, pins);
		}
	}
	sys->pins = pins;
	kbd_update(&sys->kbd, micro_seconds);
	return num_ticks;
}

// send a key down event
void bbc_key_down(bbc_t* bbc, int key_code)
{

}

// send a key up event
void bbc_key_up(bbc_t* bbc, int key_code)
{

}

uint64_t _bbc_tick(bbc_t* sys, uint64_t pins)
{
	CHIPS_ASSERT(sys && sys->valid);
	pins = m6502_tick(&sys->cpu, pins);

	// TODO: implement the rest of the tick

	return pins;
}