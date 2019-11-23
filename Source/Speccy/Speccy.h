#pragma once

#include "imgui.h"
#include <string>
#include <vector>

#define UI_DBG_USE_Z80
#define UI_DASM_USE_Z80
#include "chips/z80.h"
#include "chips/beeper.h"
#include "chips/ay38910.h"
#include "util/z80dasm.h"
#include "chips/mem.h"
#include "chips/kbd.h"
#include "chips/clk.h"
#include "systems/zx.h"
#include "chips/mem.h"
#include "ui/ui_util.h"
#include "ui/ui_chip.h"
#include "ui/ui_z80.h"
#include "ui/ui_ay38910.h"
#include "ui/ui_audio.h"
#include "ui/ui_kbd.h"
#include "ui/ui_dasm.h"
#include "ui/ui_dbg.h"
#include "ui/ui_memedit.h"
#include "ui/ui_memmap.h"
#include "ui/ui_zx.h"

enum class SpeccyModel
{
	Spectrum48K,
	Spectrum128K
};

struct FSpeccyConfig
{
	SpeccyModel	Model;
	int		NoStateBuffers = 0;
};

struct FSpeccy
{
	zx_t		CurrentState;	// Current Spectrum State

	unsigned char* FrameBuffer;	// pixel buffer to store emu output
	ImTextureID	Texture;		// texture 

	bool		ExecThisFrame = true; // Whether the emulator should execute this frame (controlled by UI)

	zx_t *		pStateBuffers;
	int			NoStateBuffers;
	int			CurrentStateBuffer = 0;
};

FSpeccy* InitSpeccy(const FSpeccyConfig& config);
void TickSpeccy(FSpeccy &speccyInstance);
void ShutdownSpeccy(FSpeccy*&pSpeccy);

const std::vector<std::string>& GetGameList();
bool LoadZ80File(FSpeccy &speccyInstance, const char *fName);

inline uint8_t	ReadySpeccyByte(const FSpeccy &speccy, uint16_t address)
{
	const int bank = address >> 14;
	const int bankAddr = address & 0x3fff;

	if(bank == 0)
		return speccy.CurrentState.rom[0][bankAddr];
	else
		return speccy.CurrentState.ram[bank-1][bankAddr];
}

inline const uint8_t*	GetSpeccyMemPtr(const FSpeccy &speccy, uint16_t address)
{
	const int bank = address >> 14;
	const int bankAddr = address & 0x3fff;

	if (bank == 0)
		return &speccy.CurrentState.rom[0][bankAddr];
	else
		return &speccy.CurrentState.ram[bank - 1][bankAddr];
}

inline void	WriteSpeccyByte(FSpeccy &speccy, uint16_t address, uint8_t value)
{
	const int bank = address >> 14;
	const int bankAddr = address & 0x3fff;

	if(bank > 0)
		speccy.CurrentState.ram[bank-1][bankAddr] = value;
}