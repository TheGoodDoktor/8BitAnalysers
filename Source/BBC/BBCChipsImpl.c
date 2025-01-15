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
	const uint16_t addr = M6502_GET_ADDR(pins);
	
	uint64_t system_via_pins = pins & M6502_PIN_MASK;
	uint64_t user_via_pins = pins & M6502_PIN_MASK;

	if (addr >= 0xFC00 && addr < 0xFF00)	// IO Registers
	{
		// Read IO

		// System VIA
		if ((addr & ~0xf) == 0xfe40 || (addr & ~0xf) == 0xfe50)
		{
			system_via_pins |= M6522_CS1;
		}

		// User VIA
		if ((addr & ~0xf) == 0xfe60 || (addr & ~0xf) == 0xfe70)
		{
			user_via_pins |= M6522_CS1;
		}

		// CRTC
		if ((addr & ~7) == 0xFE00)
		{
			// 6845 in/out
			uint64_t crtc_pins = (pins & M6502_PIN_MASK) | MC6845_CS;
			if (pins & M6502_RW)
				crtc_pins |= MC6845_RW; 
			if ((addr &1) == 0)	
				crtc_pins |= MC6845_RS;	// register select

			pins = mc6845_iorq(&sys->crtc, crtc_pins) & M6502_PIN_MASK;
		}

		// ACIA read status
		if (addr == 0xfe08)
		{
			//LOGINFO("ACIA read status at pc:0x%x", sys->cpu.PC);
			//data = acia_rd(&sys->acia, addr & 0xf);
		}

		// ACIA read data
		if (addr == 0xfe09)
		{
			//LOGINFO("ACIA read data at pc:0x%x", sys->cpu.PC);
			//data = acia_rd(&sys->acia, addr & 0xf);
		}

		// Serial ULA read
		if (addr == 0xfe10)
		{
			//LOGINFO("Serial ULA read reg:%d at pc:0x%x", addr & 0xf, sys->cpu.PC);
			//data = serial_ula_rd(&sys->serial_ula, addr & 0xf);
		}

		// Video ULA read
		if ((addr & ~3) == 0xfe20)
		{
			//LOGINFO("Video ULA read reg:%d at pc:0x%x", addr & 0x3, sys->cpu.PC);
			//data = video_ula_rd(&sys->video_ula, addr & 0x3);
		}

		// Tube read
		if ((addr & ~0x1f) == 0xfee0)
		{
			//LOGINFO("Tube read reg:%d at pc:0x%x", addr & 0x7, sys->cpu.PC);
			//return ReadTubeFromHostSide(Address & 7); // Read From Tube
		}

		// teletext
		if ((addr & ~0x3) == 0xfc10)
		{
			//LOGINFO("Teletext read reg:%d at pc:0x%x",addr&3,sys->cpu.PC);
			//return(TeletextRead(Address & 0x3));
		}
	}
	else
	{
		// regular memory access
		if (pins & M6502_RW)
		{
			M6502_SET_DATA(pins, mem_rd(&sys->mem_cpu, addr));
		}
		else
		{
			mem_wr(&sys->mem_cpu, addr, M6502_GET_DATA(pins));
		}
	}

	// Tick System VIA
	system_via_pins = m6522_tick(&sys->via_system, system_via_pins);
	if (system_via_pins & M6522_IRQ) 
		pins |= M6502_NMI;
		
	if ((system_via_pins & (M6522_CS1 | M6522_RW)) == (M6522_CS1 | M6522_RW)) 
		pins = M6502_COPY_DATA(pins, system_via_pins);
		

	// Tick User VIA
	user_via_pins = m6522_tick(&sys->via_user, user_via_pins);
	if (user_via_pins & M6522_IRQ) 
		pins |= M6502_NMI;
		
	if ((user_via_pins & (M6522_CS1 | M6522_RW)) == (M6522_CS1 | M6522_RW)) 
		pins = M6502_COPY_DATA(pins, user_via_pins);

	// Tick CRTC
	uint64_t crtc_pins = mc6845_tick(&sys->crtc);

	// TODO: implement the rest of the tick

	

	return pins;
}

// Snapshots
// TODO: Add to as functionality gets added

uint32_t bbc_save_snapshot(bbc_t* sys, bbc_t* dst)
{
	CHIPS_ASSERT(sys && dst);
	*dst = *sys;
	chips_debug_snapshot_onsave(&dst->debug);
	//chips_audio_callback_snapshot_onsave(&dst->audio.callback);
	m6502_snapshot_onsave(&dst->cpu);
	//m6569_snapshot_onsave(&dst->vic);
	mem_snapshot_onsave(&dst->mem_cpu, sys);
	return BBC_SNAPSHOT_VERSION;
}

bool bbc_load_snapshot(bbc_t* sys, uint32_t version, bbc_t* src) 
{
	CHIPS_ASSERT(sys && src);
	if (version != BBC_SNAPSHOT_VERSION) 
	{
		return false;
	}
	static bbc_t im;
	im = *src;
	chips_debug_snapshot_onload(&im.debug, &sys->debug);
	//chips_audio_callback_snapshot_onload(&im.audio.callback, &sys->audio.callback);
	m6502_snapshot_onload(&im.cpu, &sys->cpu);
	mem_snapshot_onload(&im.mem_cpu, sys);
	*sys = im;
	return true;
}