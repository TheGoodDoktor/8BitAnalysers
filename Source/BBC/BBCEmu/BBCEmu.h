#pragma once

#include "chips/chips_common.h"
#include "chips/z80.h"
#include "chips/m6502.h"
#include "chips/mc6845.h"
#include "chips/beeper.h"
#include "chips/kbd.h"
#include "chips/mem.h"
#include "chips/clk.h"
#include <chips/m6522.h>

#include "FDC8271.h"

#define BBC_FREQUENCY (2000000)	// clock frequency in Hz 
#define BBC_DISPLAY_WIDTH (768)
#define BBC_DISPLAY_HEIGHT (272)
#define BBC_FRAMEBUFFER_WIDTH (1024)
#define BBC_FRAMEBUFFER_HEIGHT (312)
#define BBC_FRAMEBUFFER_SIZE_BYTES (BBC_FRAMEBUFFER_WIDTH * BBC_FRAMEBUFFER_HEIGHT)
#define BBC_NUM_HWCOLORS (32 + 32)  // 32 colors plus pure black plus debug visualization colors
#define BBC_NUM_ROM_SLOTS (16)
#define BBC_SNAPSHOT_VERSION (1)

	// IC32 Latch pins
#define IC32_LATCH_SOUNDGEN_WR		(1<<0)
#define IC32_LATCH_READ_SPEECH		(1<<1)
#define IC32_LATCH_WRITE_SPEECH		(1<<2)
#define IC32_LATCH_KEYBOARD_WR		(1<<3)
#define IC32_LATCH_SCREENADDR_B0	(1<<4)
#define IC32_LATCH_SCREENADDR_B1	(1<<5)
#define IC32_LATCH_SCREENADDR_MASK	(0x30)
#define IC32_LATCH_CAPS_LOCK_LED	(1<<6)
#define IC32_LATCH_SHIFT_LOCK_LED	(1<<7)

// Key Codes
#define BBC_KEYCODE_CURSOR_LEFT	0x08
#define BBC_KEYCODE_CURSOR_RIGHT	0x09
#define BBC_KEYCODE_CURSOR_DOWN	0x0A
#define BBC_KEYCODE_CURSOR_UP	0x0B

#define BBC_KEYCODE_BACKSPACE	0x0C
#define BBC_KEYCODE_ENTER		0x0D
#define BBC_KEYCODE_SHIFT		0x0E
#define BBC_KEYCODE_CTRL		0x0F
#define BBC_KEYCODE_CAPS_LOCK	0x10
#define BBC_KEYCODE_SHIFT_LOCK	0x11
#define BBC_KEYCODE_SPACE		0x20
#define BBC_KEYCODE_F0			0xF0
#define BBC_KEYCODE_F1			0xF1
#define BBC_KEYCODE_F2			0xF2
#define BBC_KEYCODE_F3			0xF3
#define BBC_KEYCODE_F4			0xF4
#define BBC_KEYCODE_F5			0xF5
#define BBC_KEYCODE_F6			0xF6
#define BBC_KEYCODE_F7			0xF7
#define BBC_KEYCODE_F8			0xF8
#define BBC_KEYCODE_F9			0xF9


// configuration parameters for bbc_init()
typedef struct
{
	m6502_desc_t		cpu;
	chips_debug_t		debug;
	chips_audio_desc_t	audio;
	chips_range_t		os_rom;
	uint8_t*			roms[16];

} bbc_desc_t;

typedef struct
{
	uint8_t		ula_reg;	// raw register value
	bool		flash;
	bool		teletext;
	int			num_chars_per_line;
	bool		mc6845_high_freq;
	int			cursor_width_bytes;

	uint8_t		palette[16];
} bbc_video_ula_t;

// struct to hold the state of the 6850 ACIA
#define M6850_PIN_RW	(24)
#define M6850_PIN_CS1	(40)
#define M6850_CS1       (1ULL<<M6850_PIN_CS1)
#define M6850_RW        (1ULL<<M6850_PIN_RW)

typedef struct
{
	uint8_t		controlreg;
	uint8_t		statusreg;
	uint16_t	data;
} mc6850_t;



// struct to hold the state of the BBC
typedef struct
{
	m6502_t		cpu;
	mc6845_t	crtc;
	uint64_t	pins;
	m6522_t		via_system;
	m6522_t		via_user;
	mc6850_t	acia;
	fdc8271_t	fdc8271;
	bbc_video_ula_t video_ula;
	uint8_t		ic32;		// IC32 Latch
	kbd_t		kbd;		// keyboard matrix state
	bool		key_pressed;	// to trigger interrupt
	int			key_scan_column;
	mem_t		mem_cpu;	// cpu memory
	uint8_t		rom_select;
	bool		valid;
	chips_debug_t debug;
	uint32_t	tick_counter;

	// memory
	uint8_t		ram[0x8000];		// 32K RAM
	const uint8_t* rom_slots[16];		// pointer to selected ROM slot
	uint8_t		rom_os[0x4000];		// 16K OS ROM

	//alignas(64) 
	uint8_t fb[BBC_FRAMEBUFFER_SIZE_BYTES];


}bbc_t;

// initialize a new BBC instance
void bbc_init(bbc_t* bbc, const bbc_desc_t* desc);
// discard a BBC instance
void bbc_discard(bbc_t* bbc);
// reset a BBC instance
void bbc_reset(bbc_t* bbc);
// get display requirements and framebuffer content, may be called with nullptr
chips_display_info_t bbc_display_info(bbc_t* bbc);
// run BBC instance for given amount of micro_seconds, returns number of ticks executed
uint32_t bbc_exec(bbc_t* bbc, uint32_t micro_seconds);
// send a key down event
void bbc_key_down(bbc_t* bbc, int key_code);
// send a key up event
void bbc_key_up(bbc_t* bbc, int key_code);

// save a snapshot, patches pointers to zero and offsets, returns snapshot version
uint32_t bbc_save_snapshot(bbc_t* sys, bbc_t* dst);
// load a snapshot, returns false if snapshot versions don't match
bool bbc_load_snapshot(bbc_t* sys, uint32_t version, bbc_t* src);


// video ULA
void bbc_video_ula_init(bbc_video_ula_t* ula);
void bbc_video_ula_io_write(bbc_video_ula_t* ula, uint8_t reg, uint8_t data);

// 6805 ACIA
void mc6850_init(mc6850_t* acia);
void mc6850_reset(mc6850_t* acia);
uint64_t mc6850_tick(mc6850_t* acia, uint64_t pins);


// Misc stuff here to support disc emulation

#include <limits.h>

enum NMI_Nums {
	nmi_floppy,
	nmi_econet,
};

extern int TotalCycles;

extern unsigned char NMIStatus;

#define CycleCountTMax INT_MAX
#define CycleCountWrap (INT_MAX / 2)

#define SetTrigger(after, var) var = TotalCycles + (after)
#define IncTrigger(after, var) var += (after)

#define ClearTrigger(var) var=CycleCountTMax;

#define AdjustTrigger(var) if (var!=CycleCountTMax) var-=CycleCountWrap;

// Audio

#define SAMPLE_RELAY_ON         0
#define SAMPLE_RELAY_OFF        1
#define SAMPLE_DRIVE_MOTOR      2
#define SAMPLE_HEAD_LOAD        3
#define SAMPLE_HEAD_UNLOAD      4
#define SAMPLE_HEAD_SEEK        5
#define SAMPLE_HEAD_STEP        6

constexpr int SAMPLE_HEAD_SEEK_CYCLES_PER_TRACK = 48333; // 0.02415s per track in the sound file
constexpr int SAMPLE_HEAD_STEP_CYCLES = 100000; // 0.05s sound file
constexpr int SAMPLE_HEAD_LOAD_CYCLES = 400000; // 0.2s sound file

void PlaySoundSample(int sample, bool repeat);
void StopSoundSample(int sample);

// LED displays

struct LEDType
{
	bool ShiftLock;
	bool CapsLock;
	bool Motor;
	bool FloppyDisc[2];
	bool HDisc[4];
	bool ShowDisc;
	bool ShowKB;
};

extern LEDType LEDs;

