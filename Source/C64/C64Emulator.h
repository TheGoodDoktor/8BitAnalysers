#pragma once

#include "CodeAnalyser/CodeAnalyser.h"

//#define UI_DASM_USE_Z80
#define UI_DASM_USE_M6502
#define UI_DBG_USE_M6502
//#include "ui.h"
// Chips includes
#include <chips/chips_common.h>
#include <chips/m6502.h>
#include <chips/m6526.h>
#include <chips/m6569.h>
#include <chips/m6581.h>
#include <chips/kbd.h>
#include <chips/mem.h>
#include <chips/clk.h>
#include <systems/c1530.h>
#include <chips/m6522.h>
#include <systems/c1541.h>
#include <systems/c64.h>


#include <util/m6502dasm.h>
#include <util/z80dasm.h>

// Chips UI includes
#include <ui/ui_util.h>
#include <ui/ui_chip.h>
#include <ui/ui_util.h>
#include <ui/ui_m6502.h>
#include <ui/ui_m6526.h>
#include <ui/ui_m6569.h>
#include <ui/ui_m6581.h>
#include <ui/ui_audio.h>
#include <ui/ui_dasm.h>
#include <ui/ui_dbg.h>
#include <ui/ui_memedit.h>
#include <ui/ui_memmap.h>
#include <ui/ui_kbd.h>
#include <ui/ui_snapshot.h>
#include <ui/ui_c64.h>
#include <ui/ui_ay38910.h>

#include "C64GamesList.h"
#include "C64Display.h"

#include "IOAnalysis/C64IOAnalysis.h"
#include "GraphicsViewer/C64GraphicsViewer.h"

class FC64Emulator : public ICPUInterface
{
public:

	bool    Init();
	void    Shutdown();
	void    Tick();

	// Begin IInputEventHandler interface implementation
	void	OnKeyUp(int keyCode);
	void	OnKeyDown(int keyCode);
	void	OnChar(int charCode);
	void    OnGamepadUpdated(int mask);
	// End IInputEventHandler interface implementation

	// Begin ICPUInterface interface implementation
	uint8_t		ReadByte(uint16_t address) const override
	{
		return mem_rd(const_cast<mem_t*>(&C64Emu.mem_cpu), address);
	}
	uint16_t	ReadWord(uint16_t address) const override
	{
		return mem_rd16(const_cast<mem_t*>(&C64Emu.mem_cpu), address);
	}
	const uint8_t* GetMemPtr(uint16_t address) const override
	{
		return mem_readptr(const_cast<mem_t*>(&C64Emu.mem_cpu), address);
	}

	void WriteByte(uint16_t address, uint8_t value) override
	{
		mem_wr(&C64Emu.mem_cpu, address, value);
	}

	FAddressRef GetPC() override
	{
		uint16_t address = m6502_pc(&C64Emu.cpu);
		FAddressRef result(0, address);

		return result;
	}

	uint16_t	GetSP(void) override
	{
		return m6502_s(&C64Emu.cpu) + 0x100;    // stack begins at 0x100
	}

	// End ICPUInterface interface implementation

	c64_desc_t GenerateC64Desc(c64_joystick_type_t joy_type);
	void SetupCodeAnalysisLabels(void);
	void UpdateCodeAnalysisPages(uint8_t cpuPort);
	bool LoadGame(const FGameInfo* pGameInfo);
	void ResetCodeAnalysis(void);
	bool SaveCodeAnalysis(const FGameInfo* pGameInfo);
	bool LoadCodeAnalysis(const FGameInfo* pGameInfo);

	// Emulator Event Handlers
	void    OnBoot(void);
	int     OnCPUTrap(uint16_t pc, int ticks, uint64_t pins);
	uint64_t    OnCPUTick(uint64_t pins);

	c64_t*	GetEmu() {return &C64Emu;}
private:
	c64_t       C64Emu;
	ui_c64_t    C64UI;
	double      ExecTime;

	FC64GamesList       GamesList;
	const FGameInfo* CurrentGame = nullptr;

	FC64Display         Display;

	FCodeAnalysisState  CodeAnalysis;

	// Analysis pages
	FCodeAnalysisPage   KernelROM[8];       // 8K Kernel ROM
	FCodeAnalysisPage   BasicROM[8];        // 8K Basic ROM
	FCodeAnalysisPage   CharacterROM[4];    // 4K Character ROM
	FCodeAnalysisPage   IOSystem[4];        // 4K IO System
	FCodeAnalysisPage   RAM[64];            // 64K RAM

	uint8_t             LastMemPort = 0x7;  // Default startup
	uint16_t            LastPC = 0;

	FC64IOAnalysis      IOAnalysis;
	FC64GraphicsViewer  GraphicsViewer;
	std::set<uint16_t>  InterruptHandlers;

	// Mapping status
	bool                bBasicROMMapped = true;
	bool                bKernelROMMapped = true;
	bool                bCharacterROMMapped = false;
	bool                bIOMapped = true;

	// Bank Ids
	uint16_t            LowerRAMId = -1;
	uint16_t            HighRAMId = -1;
	uint16_t            IOAreaId = -1;
	uint16_t            BasicROMId = -1;
	uint16_t            RAMBehindBasicROMId = -1;
	uint16_t            KernelROMId = -1;
	uint16_t            RAMBehindKernelROMId = -1;
	uint16_t            CharacterROMId = -1;
	uint16_t            RAMBehindCharROMId = -1;
	uint16_t            ColourRAMId = -1;
};
