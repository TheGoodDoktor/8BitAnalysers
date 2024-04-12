#pragma once

#include "CodeAnalyser/CodeAnalyser.h"
#include "Misc/EmuBase.h"

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

#include "C64GamesList.h"
#include "C64Display.h"

#include "IOAnalysis/C64IOAnalysis.h"
#include "GraphicsViewer/C64GraphicsViewer.h"
#include "FileLoaders/CRTFile.h"

enum class EC64Event
{
	None = 0,

	VICRegisterWrite,
    VICScreenModeChar,
    VICScreenModeBmp,
    VICScreenModeMultiColour,
    VICScreenModeHiRes,
    
	SIDRegisterWrite,
	CIA1RegisterWrite,
	CIA1RegisterRead,
	CIA2RegisterWrite,
	CIA2RegisterRead,

};

enum class EC64FileType
{
	None,
	PRG,
	Tape,
	Disk,
	Cartridge
};

enum class EFileLoadPhase
{
	Idle,
	Reset,
	BasicReady,
	Loaded,
	Run,
	TapePlaying
};

struct FC64BankIds
{
	int16_t LowerRAM = -1;
	int16_t LowerRAM2 = -1;
	int16_t HighRAM = -1;
	int16_t IOArea = -1;
	int16_t BasicROM = -1;
	int16_t RAMBehindBasicROM = -1;
	int16_t KernelROM = -1;
	int16_t RAMBehindKernelROM = -1;
	int16_t CharacterROM = -1;
	int16_t RAMBehindCharROM = -1;
};

struct FC64Config;
struct FC64ProjectConfig;
class FC64Emulator;


struct FC64LaunchConfig : public FEmulatorLaunchConfig
{
};

class FC64Emulator : public FEmuBase
{
public:
	FC64Emulator() = default;

	bool    Init(const FEmulatorLaunchConfig& launchConfig) override;
	void    Shutdown() override;
	void	DrawEmulatorUI() override;
	void    Tick() override;
	void    Reset() override;

	void	FileMenuAdditions(void) override;
	void	SystemMenuAdditions(void) override;
	void	OptionsMenuAdditions(void) override;
	void	WindowsMenuAdditions(void) override;

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
		return CodeAnalysis.Debugger.GetPC();
	}

	uint16_t	GetSP(void) override
	{
		return m6502_s(&C64Emu.cpu) + 0x100;    // stack begins at 0x100
	}

	void* GetCPUEmulator(void) const override
	{
		return (void*)&C64Emu.cpu;
	}

	// End ICPUInterface interface implementation

	c64_desc_t GenerateC64Desc(c64_joystick_type_t joy_type);
	void SetupCodeAnalysisLabels(void);
	void UpdateCodeAnalysisPages(uint8_t cpuPort);
	FAddressRef	GetVICMemoryAddress(uint16_t vicAddress) const	// VIC address is 14bit (16K range)
	{
		const uint16_t physicalAddress = C64Emu.vic_bank_select + vicAddress;
		return FAddressRef(VICBankMapping[physicalAddress >> 12], physicalAddress);
	}
	bool IsAddressedByVIC(FAddressRef addr)
	{
		return VICBankMapping[addr.Address >> 12] == addr.BankId;
	}
	
	uint16_t GetIOAreaBankId() const { return BankIds.IOArea; }
	const FC64BankIds& GetBankIds(void) const { return BankIds; }

	FAddressRef	GetColourRAMAddress(uint16_t colRamAddress)	const // VIC address is 14bit (16K range)
	{
		return FAddressRef(BankIds.IOArea, colRamAddress + 0xD800);
	}

	bool	LoadEmulatorFile(const FEmulatorFile* pEmuFile) override;
	bool	NewProjectFromEmulatorFile(const FEmulatorFile& emuFile) override;
	bool	LoadProject(FProjectConfig *pConfig, bool bLoadGame) override;
	bool	SaveProject(void) override;

	// Cartridge
#if 0
	void	SetCartridgeHandler(FCartridgeHandler* pHandler) { delete pCartridgeHandler; pCartridgeHandler = pHandler;}
	void	ResetCartridgeBanks();
	FCartridgeSlot&	GetCartridgeSlot(ECartridgeSlot slot) { assert(slot!=ECartridgeSlot::Unknown); return CartridgeSlots[(int)slot]; }
	FCartridgeBank&	AddCartridgeBank(int bankNo, uint16_t address, uint32_t dataSize);
	void	SetCartridgeType(ECartridgeType type) { CartridgeType = type;}
	void	InitCartMapping(void);
	bool	MapCartridgeBank(ECartridgeSlot slot, int bankNo);
	void	UnMapCartridge(ECartridgeSlot slot);
#endif
	void ResetCodeAnalysis(void);
	bool LoadMachineState(const char* fname);
	bool SaveMachineState(const char* fname);

	// Emulator Event Handlers
	void    OnBoot(void);
	int     OnCPUTrap(uint16_t pc, int ticks, uint64_t pins);
	uint64_t    OnCPUTick(uint64_t pins);

	c64_t*	GetEmu() {return &C64Emu;}
	const FC64IOAnalysis&	GetC64IOAnalysis() { return IOAnalysis; }

	const FC64Config* GetC64GlobalConfig() const { return (const FC64Config*)pGlobalConfig; }
	FC64Config* GetC64GlobalConfig() { return (FC64Config*)pGlobalConfig; }

	void	SetLoadedFileType(EC64FileType type) { LoadedFileType = type;}
private:
	c64_t       C64Emu;
	double      ExecTime;

	EC64FileType	LoadedFileType = EC64FileType::None;
	EFileLoadPhase	FileLoadPhase = EFileLoadPhase::Idle;

	const FGameInfo*	CurrentGame = nullptr;

	FC64Display         Display;

	uint8_t				IOMemBuffer[0x1000];	// Buffer for IO memory

	uint8_t             LastMemPort = 0x7;  // Default startup
	uint16_t            PreviousPC = 0;

	FCartridgeManager	CartridgeManager;
	//FCartridgeHandler*	pCartridgeHandler = nullptr;
	//ECartridgeType		CartridgeType = ECartridgeType::None;
	//FCartridgeSlot		CartridgeSlots[(int)ECartridgeSlot::Max];
	//int16_t				FirstCartridgeBankId = -1;

	FC64IOAnalysis      IOAnalysis;
	std::set<FAddressRef>  InterruptHandlers;

	// Mapping status
	bool                bBasicROMMapped = true;
	bool                bKernelROMMapped = true;
	bool                bCharacterROMMapped = false;
	bool                bIOMapped = true;

	FC64BankIds			BankIds;
	uint16_t			VICBankMapping[16];

	FC64Emulator(const FC64Emulator&) = delete;                 // Prevent copy-construction
	FC64Emulator& operator=(const FC64Emulator&) = delete;      // Prevent assignment

};
