#pragma once

#include "Misc/EmuBase.h"
#include "BBCChipsImpl.h"
#include "IOAnalysis/BBCIOAnalysis.h"
#include <chips/mem.h>
#include <set>
#include <array>

struct FBBCLaunchConfig : public FEmulatorLaunchConfig
{
	void ParseCommandline(int argc, char** argv) override
	{
		// Parse commandline arguments
	};
};

struct FBBCBankIds
{
	int16_t RAM = -1;
	int16_t OSROM = -1;
	int16_t BasicROM = -1;
};

class FBBCEmulator : public FEmuBase
{
public:
	FBBCEmulator();

	bool	Init(const FEmulatorLaunchConfig& launchConfig) override;
	void    Shutdown() override;
	void	DrawEmulatorUI() override;
	void    Tick() override;
	void    Reset() override;
	void	FixupAddressRefs();

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
		return mem_rd(const_cast<mem_t*>(&BBCEmu.mem_cpu), address);
	}
	uint16_t	ReadWord(uint16_t address) const override
	{
		return mem_rd16(const_cast<mem_t*>(&BBCEmu.mem_cpu), address);
	}
	const uint8_t* GetMemPtr(uint16_t address) const override
	{
		return mem_readptr(const_cast<mem_t*>(&BBCEmu.mem_cpu), address);
	}

	void WriteByte(uint16_t address, uint8_t value) override
	{
		mem_wr(&BBCEmu.mem_cpu, address, value);
	}

	FAddressRef GetPC() override
	{
		return CodeAnalysis.Debugger.GetPC();
	}

	uint16_t	GetSP(void) override
	{
		return m6502_s(&BBCEmu.cpu) + 0x100;    // stack begins at 0x100
	}

	void* GetCPUEmulator(void) const override
	{
		return (void*)&BBCEmu.cpu;
	}

	// End ICPUInterface interface implementation

	void	SetupCodeAnalysisLabels();

	bool	SaveMachineState(const char* fname);
	bool	LoadMachineState(const char* fname);


	bool	LoadEmulatorFile(const FEmulatorFile* pEmuFile) override;
	bool	NewProjectFromEmulatorFile(const FEmulatorFile& emuFile) override;
	bool	LoadProject(FProjectConfig* pConfig, bool bLoadGame) override;
	bool	SaveProject(void) override;

	const FBBCBankIds&	GetBankIds() const { return BankIds; }

	uint64_t	OnCPUTick(uint64_t pins);

	bbc_t& GetBBC() { return BBCEmu; }

private:
	bbc_t				BBCEmu;
	FBBCLaunchConfig	LaunchConfig;

	FBBCBankIds			BankIds;	

	std::set<FAddressRef>	InterruptHandlers;
	uint16_t				PreviousPC = 0;
	FBBCIOAnalysis			IOAnalysis;
	std::array<uint8_t, 3 * 256> IOMemBuffer;	// 3 pages

	FBBCEmulator(const FBBCEmulator&) = delete;				// Prevent copy-construction
	FBBCEmulator& operator=(const FBBCEmulator&) = delete;	// Prevent assignment
};
