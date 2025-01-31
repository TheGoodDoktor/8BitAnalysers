#include "BBCEmu.h"

#include <string.h>
#include <assert.h>

#include "Disc8271.h"

#define CHIPS_ASSERT(c) assert(c)

uint64_t _bbc_tick(bbc_t* sys, uint64_t pins);

uint32_t BBCExecEmu(bbc_t* sys, uint32_t micro_seconds)
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
void bbc_init_key_map(bbc_t* sys);


void bbc_init(bbc_t* sys, const bbc_desc_t* desc)
{
	CHIPS_ASSERT(sys && desc);
	if (desc->debug.callback.func) { CHIPS_ASSERT(desc->debug.stopped); }

	memset(sys, 0, sizeof(bbc_t));
	sys->valid = true;
	sys->debug = desc->debug;
	sys->rom_select = 0;
	//sys->audio.callback = desc->audio.callback;
	//sys->audio.num_samples = _CPC_DEFAULT(desc->audio.num_samples, CPC_DEFAULT_AUDIO_SAMPLES);
	//CHIPS_ASSERT(sys->audio.num_samples <= CPC_MAX_AUDIO_SAMPLES);

	// initialize ROMS
	CHIPS_ASSERT(desc->os_rom.ptr && (desc->os_rom.size == 0x4000));
	memcpy(sys->rom_os, desc->os_rom.ptr, 0x4000);

	// initialize the hardware
	sys->pins = m6502_init(&sys->cpu, &desc->cpu);

	m6522_init(&sys->via_system);
	m6522_init(&sys->via_user);
	mc6845_init(&sys->crtc, MC6845_TYPE_UM6845R);
	mc6850_init(&sys->acia);
	fdc8271_init(&sys->fdc8271);
	mem_init(&sys->mem_cpu);
	sys->ic32 = 0x00;	// TODO: determiine initial value

	bbc_video_ula_init(&sys->video_ula);

	bbc_init_key_map(sys);

	// initial memory mapping

	// 32K RAM
	mem_map_ram(&sys->mem_cpu, 0, 0x0000, 0x8000, sys->ram);

	// Map in first ROM slot
	for (int slot = 0; slot < BBC_NUM_ROM_SLOTS; slot++)
		sys->rom_slots[slot] = desc->roms[slot];
	sys->rom_select = 0xf;
	mem_map_rom(&sys->mem_cpu, 0, 0x8000, 0x4000, sys->rom_slots[sys->rom_select]);

	// map in OS rom
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
	mc6850_reset(&sys->acia);
	m6522_reset(&sys->via_system);
}

#if 0
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
#endif
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

uint64_t _bbc_tick_cpu(bbc_t* sys, uint64_t pins)
{
	pins = m6502_tick(&sys->cpu, pins);

	// the IRQ and NMI pins will be set by the HW each tick
	pins &= ~(M6502_IRQ | M6502_NMI);

	const uint16_t addr = M6502_GET_ADDR(pins);

	uint64_t system_via_pins = pins & M6502_PIN_MASK;
	uint64_t user_via_pins = pins & M6502_PIN_MASK;
	uint64_t acia_pins = pins & M6502_PIN_MASK;

	if (addr >= 0xFC00 && addr < 0xFF00)	// IO Registers
	{
		// Read IO

		// System VIA
		if ((addr & ~0xf) == 0xfe40 || (addr & ~0xf) == 0xfe50)
			system_via_pins |= M6522_CS1;

		// User VIA
		if ((addr & ~0xf) == 0xfe60 || (addr & ~0xf) == 0xfe70)
			user_via_pins |= M6522_CS1;

		// CRTC
		if ((addr & ~7) == 0xFE00)
		{
			// 6845 in/out
			uint64_t crtc_pins = (pins & M6502_PIN_MASK) | MC6845_CS;
			if (pins & M6502_RW)
				crtc_pins |= MC6845_RW;
			if ((addr & 1) == 1)
				crtc_pins |= MC6845_RS;	// register select

			pins = mc6845_iorq(&sys->crtc, crtc_pins) & M6502_PIN_MASK;
		}

		// ACIA
		if (addr == 0xfe08 || addr == 0xfe09)
			acia_pins |= M6850_CS1;

		// Serial ULA 
		if (addr == 0xfe10)
		{
			//LOGINFO("Serial ULA read reg:%d at pc:0x%x", addr & 0xf, sys->cpu.PC);
			//data = serial_ula_rd(&sys->serial_ula, addr & 0xf);
		}

		// Video ULA 
		// https://beebwiki.mdfs.net/Video_ULA
		if ((addr & ~3) == 0xfe20)
		{
			//LOGINFO("Video ULA read reg:%d at pc:0x%x", addr & 0x3, sys->cpu.PC);
			//data = video_ula_rd(&sys->video_ula, addr & 0x3);
			if (pins & M6502_RW)
			{
				//data = video_ula_rd(&sys->video_ula, addr & 0x3);
			}
			else
			{
				bbc_video_ula_io_write(&sys->video_ula, addr & 0x3, M6502_GET_DATA(pins));
			}
		}

		// ROM Select
		if (addr == 0xfe30)
		{
			if (pins & M6502_RW)
			{
				M6502_SET_DATA(pins, sys->rom_select);
			}
			else
			{
				const uint8_t newRomSelect = M6502_GET_DATA(pins);
				if (sys->rom_select != newRomSelect)
				{
					sys->rom_select = newRomSelect;
					mem_map_rom(&sys->mem_cpu, 0, 0x8000, 0x4000, sys->rom_slots[sys->rom_select]);
				}
			}
		}

		// FDD
		if ((addr & ~7) == 0xFE80)
		{
			if (pins & M6502_RW)	// read
			{
				M6502_SET_DATA(pins, Disc8271Read( addr & 7));
			}
			else // write
			{
				Disc8271Write( addr & 7, M6502_GET_DATA(pins));
			}
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
	// Set CA1 on vsync from 6845
	if (sys->crtc.vs)
		system_via_pins |= M6522_CA1;

	// Read keyboard
	// TODO: move this to a separate function?
	uint16_t row_mask = 0xff;

	if ((sys->ic32 & IC32_LATCH_KEYBOARD_WR) == 0)	// auto scan mode disabled
	{
		uint8_t sys_port_a = sys->via_system.pa.pins;

		// bits 0-6 are key we want to read
		// bit 7 should be set if key is down
		uint8_t key_code = sys_port_a & 0x7f;
		uint8_t key_row = (key_code >> 4) & 7;
		uint8_t key_col = key_code & 0xf;

		sys->key_scan_column = key_col;

		kbd_set_active_columns(&sys->kbd, 1 << key_col);
		if (kbd_scan_lines(&sys->kbd) & (1 << key_row))
			sys_port_a |= 0x80;	// set bit 7
		else
			sys_port_a &= 0x7f;	// clear bit 7

		// update port A
		M6522_SET_PA(system_via_pins, sys_port_a);
	}
	else
	{
		sys->key_scan_column = (sys->key_scan_column + 1) & 0xf;
		// skip scanning column 0
		//if (sys->key_scan_column == 0)
		//	sys->key_scan_column = 1;
	}


	kbd_set_active_columns(&sys->kbd, 1 << sys->key_scan_column);
	if (kbd_scan_lines(&sys->kbd))
	{
		system_via_pins |= M6522_CA2;
	}
	// end keyboard read

	system_via_pins = m6522_tick(&sys->via_system, system_via_pins);

	if ((system_via_pins & (M6522_CS1 | M6522_RW)) == M6522_CS1)
	{
		// Update IC32 Latch
		uint8_t sys_port_b = M6522_GET_PB(system_via_pins);
		if (sys_port_b & (1 << 3))
			sys->ic32 |= 1 << (sys_port_b & 0x7);	// set latch bit
		else
			sys->ic32 &= ~(1 << (sys_port_b & 0x7));	// clear latch bit
	}

	if (system_via_pins & M6522_IRQ)
		pins |= M6502_IRQ;

	// read?
	if ((system_via_pins & (M6522_CS1 | M6522_RW)) == (M6522_CS1 | M6522_RW))
		pins = M6502_COPY_DATA(pins, system_via_pins);


	// Tick User VIA
	user_via_pins = m6522_tick(&sys->via_user, user_via_pins);
	if (user_via_pins & M6522_IRQ)
		pins |= M6502_IRQ;

	if ((user_via_pins & (M6522_CS1 | M6522_RW)) == (M6522_CS1 | M6522_RW))
		pins = M6502_COPY_DATA(pins, user_via_pins);



	// Tick ACIA
	acia_pins = mc6850_tick(&sys->acia, acia_pins);
	if ((acia_pins & (M6850_CS1 | M6522_RW)) == (M6850_CS1 | M6522_RW))
		pins = M6502_COPY_DATA(pins, acia_pins);

	// TODO: implement the rest of the tick

	// Tick FDC
	Disc8271Poll();

	if (NMIStatus != 0)
	{
		pins |= M6502_NMI;
	}

	TotalCycles++;
	if (TotalCycles > CycleCountWrap)
	{
		TotalCycles -= CycleCountWrap;
		AdjustTrigger(Disc8271Trigger);
	}

	return pins;
}

void _bbc_tick_crtc(bbc_t* sys)
{
	// Tick CRTC
	uint64_t crtc_pins = mc6845_tick(&sys->crtc);
}

uint64_t _bbc_tick(bbc_t* sys, uint64_t pins)
{
	CHIPS_ASSERT(sys && sys->valid);

	// tick the CPU & CRTC at alternate cycles
	// not sure if we need this and it causes problems with stepping in the debugger
	//if ((sys->tick_counter & 1) == 0)
	{
		pins = _bbc_tick_cpu(sys, pins);
	}
	//if ((sys->tick_counter & 1) == 1)
	{
		_bbc_tick_crtc(sys);
	}

	sys->tick_counter++;
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

// Keyboard
// keyboard ref: https://beebwiki.mdfs.net/Keyboard


void bbc_init_key_map(bbc_t* sys)
{
	kbd_init(&sys->kbd, 1);

	// TODO: modify this to match the BBC keyboard

	/* shift key is entire line 7 */
	//const int shift = (1 << 0); kbd_register_modifier_line(&sys->kbd, 0, 7);
	/* ctrl key is entire line 6 */
	//const int ctrl = (1 << 1); kbd_register_modifier_line(&sys->kbd, 1, 6);

	// shift is column 0, line 0
	kbd_register_modifier(&sys->kbd, 0, 0, 0);
	// ctrl is column 1, line 0
	//kbd_register_modifier(&sys->kbd, 1, 1, 0);

	const int kColumns = 10;
	const int kRows = 8;

	/* alpha-numeric keys */
	// each string is a row of the keyboard matrix
	const char* keymap_char =
		/* no shift */
	//   0123456789 (col)
		"          "	// row 0
		"q345 8 -^ "	// row 1
		" wet7i90_ "	// row 2
		"12dr6uop[ "	// row 3
		" axfyjk@: "	// row 4
		" scghnl;] "	// row 5
		" z vbm,./ "	// row 6
		"          "	// row 7

		/* shift */
	//   0123456789 (col)
		"          "	// row 0
		"Q#$% ( =~ "	// row 1
		" WET&I) £ "	// row 2
		"!\"DR&UOP{ "	// row 3
		" AXFYJK * "	// row 4
		" SCGHNL+} "	// row 5
		" Z VBM<>? "	// row 6
		"          ";	// row 7

	CHIPS_ASSERT(strlen(keymap_char) == (kRows * kColumns * 2));

	const uint8_t* keymap = (const uint8_t *)keymap_char;

	for (int shift = 0; shift < 2; shift++)
	{
		for (int column = 0; column < kColumns; column++)
		{
			for (int row = 0; row < kRows; row++)
			{
				int c = keymap[(shift * (kColumns * kRows)) + (row * kColumns) + column];
				if (c != 0x20)
				{
					kbd_register_key(&sys->kbd, (int)c, column, row, shift ? (1 << 0) : 0);
				}
			}
		}
	}

	// special keys
	kbd_register_key(&sys->kbd, BBC_KEYCODE_SPACE, 2, 6, 0);    // space
	kbd_register_key(&sys->kbd, BBC_KEYCODE_CURSOR_LEFT, 9, 1, 1);    // cursor left
	kbd_register_key(&sys->kbd, BBC_KEYCODE_CURSOR_RIGHT, 9, 7, 0);    // cursor right
	kbd_register_key(&sys->kbd, BBC_KEYCODE_CURSOR_DOWN, 9, 2, 0);    // cursor down
	kbd_register_key(&sys->kbd, BBC_KEYCODE_CURSOR_UP, 9, 3, 1);    // cursor up
	kbd_register_key(&sys->kbd, BBC_KEYCODE_BACKSPACE, 9, 5, 0);    // backspace -> delete
	kbd_register_key(&sys->kbd, BBC_KEYCODE_ENTER, 9, 4, 0);    // return
	kbd_register_key(&sys->kbd, BBC_KEYCODE_SHIFT, 0, 0, 0);    // shift
	kbd_register_key(&sys->kbd, BBC_KEYCODE_CTRL, 1, 0, 0);    // ctrl
	kbd_register_key(&sys->kbd, BBC_KEYCODE_CAPS_LOCK, 0, 4, 0);    // caps lock
}

// send a key down event
void bbc_key_down(bbc_t* sys, int key_code)
{
	sys->key_pressed = true;
	kbd_key_down(&sys->kbd, key_code);
}

// send a key up event
void bbc_key_up(bbc_t* sys, int key_code)
{
	kbd_key_up(&sys->kbd, key_code);
}


// Video ULA

void bbc_video_ula_init(bbc_video_ula_t* ula)
{
	ula->ula_reg = 0;
	ula->teletext = false;
	ula->num_chars_per_line = 80;
}

void bbc_video_ula_io_write(bbc_video_ula_t* ula, uint8_t reg, uint8_t data)
{
	if (reg == 0)	// write to UL reg
	{
		int noCharsPerLine[] = { 10,20,40,80 };

		ula->ula_reg = data;
		ula->flash = (data & 0x1) != 0;
		ula->teletext = (data & 0x2) != 0;
		ula->num_chars_per_line = noCharsPerLine[(data >> 2) & 0x3];
		ula->mc6845_high_freq = (data & 0x10) != 0;
		ula->cursor_width_bytes = (data >> 5);
	}
	else if (reg == 1)
	{
		// Write palette
		ula->palette[data >> 4] = data & 0xf;
	}
}

// ACIA
void mc6850_init(mc6850_t* acia)
{
	acia->controlreg = 0;
}

void mc6850_reset(mc6850_t* acia)
{
	acia->controlreg = 0;
}

uint64_t mc6850_tick(mc6850_t* acia, uint64_t pins)
{
	if (pins & M6850_CS1)
	{
		if ((pins & M6502_A0) == 0)	// Control/Status register
		{
			if (pins & M6502_RW)
			{
				M6502_SET_DATA(pins, acia->statusreg);
			}
			else
			{
				acia->controlreg = M6502_GET_DATA(pins);
			}
		}
		else	// data register
		{
			if (pins & M6502_RW)
			{
				// Read data
				M6502_SET_DATA(pins, acia->data);
			}
			else
			{
				// Write data
				acia->data = M6502_GET_DATA(pins);
			}
		}
	}

	// TODO: tick behaviour

	return pins;
}

// Misc stuff here to support disc emulation
int TotalCycles = 0;
unsigned char NMIStatus = 0;

void PlaySoundSample(int sample, bool repeat)
{

}

void StopSoundSample(int sample)
{

}

LEDType LEDs;