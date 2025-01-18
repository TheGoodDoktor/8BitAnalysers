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

#ifdef __cplusplus
extern "C" {
#endif

#define BBC_FREQUENCY (985248)              // clock frequency in Hz
#define BBC_DISPLAY_WIDTH (768)
#define BBC_DISPLAY_HEIGHT (272)
#define BBC_FRAMEBUFFER_WIDTH (1024)
#define BBC_FRAMEBUFFER_HEIGHT (312)
#define BBC_FRAMEBUFFER_SIZE_BYTES (BBC_FRAMEBUFFER_WIDTH * BBC_FRAMEBUFFER_HEIGHT)
#define BBC_NUM_HWCOLORS (32 + 32)  // 32 colors plus pure black plus debug visualization colors

#define BBC_SNAPSHOT_VERSION (1)

// configuration parameters for bbc_init()
typedef struct 
{
	m6502_desc_t		cpu;
	chips_debug_t		debug;
	chips_audio_desc_t	audio;

	// ROM images
	struct {
		chips_range_t os;
		chips_range_t basic;
	} roms;

} bbc_desc_t;

typedef struct
{
	uint8_t		ula_reg;
	int			screen_mode;
	bool		teletext;
	uint8_t		palette[16];
} bbc_video_ula_t;

// struct to hold the state of the BBC
typedef struct 
{
	m6502_t		cpu;
	mc6845_t	crtc;	
	uint64_t	pins;
	m6522_t		via_system;
	m6522_t		via_user;
	bbc_video_ula_t video_ula;
	kbd_t		kbd;		// keyboard matrix state
	mem_t		mem_cpu;	// cpu memory
	bool		valid;
	chips_debug_t debug;

	// memory
	uint8_t		ram[0x8000];		// 32K RAM
	uint8_t		rom_basic[0x4000];	// 16K BASIC ROM
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

#ifdef __cplusplus
} // extern "C"
#endif