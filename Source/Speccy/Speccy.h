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
	int			CurrentLayer = 0;	// layer ??

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

uint8_t* MemGetPtr(zx_t* zx, int layer, uint16_t addr);
uint8_t MemReadFunc(int layer, uint16_t addr, void* user_data);
void MemWriteFunc(int layer, uint16_t addr, uint8_t data, void* user_data);

// TODO: Replace these
inline uint8_t	ReadSpeccyByte(FSpeccy *pSpeccy, uint16_t address)
{
	return MemReadFunc(pSpeccy->CurrentLayer, address, &pSpeccy->CurrentState);
}

inline const uint8_t*	GetSpeccyMemPtr(FSpeccy *pSpeccy, uint16_t address)
{
	//return MemGetPtr(&pSpeccy->CurrentState, pSpeccy->CurrentLayer, address);

	const int bank = address >> 14;
	const int bankAddr = address & 0x3fff;

	if (bank == 0)
		return &pSpeccy->CurrentState.rom[0][bankAddr];
	else
		return &pSpeccy->CurrentState.ram[bank - 1][bankAddr];
	
}

inline void	WriteSpeccyByte(FSpeccy *pSpeccy, uint16_t address, uint8_t value)
{
	MemWriteFunc(pSpeccy->CurrentLayer, address, value, &pSpeccy->CurrentState);
}

uint16_t GetScreenPixMemoryAddress(int x, int y);
uint16_t GetScreenAttrMemoryAddress(int x, int y);

