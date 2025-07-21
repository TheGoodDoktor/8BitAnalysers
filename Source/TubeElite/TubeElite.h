#pragma once

#include "Misc/EmuBase.h"

#include <chips/mem.h>

#include "TubeEliteConfig.h"
#include "TubeEliteMachine.h"
#include "TubeEliteDisplay.h"
#include <set>
#include <array>
#include <deque>

#include <imgui.h>

class FTubeCommand;

struct FTubeEliteLaunchConfig : public FEmulatorLaunchConfig
{
    void ParseCommandline(int argc, char** argv) override
    {
        // Parse commandline arguments
		if (argc > 1)
		{
			if(strcmp(argv[1],"-bbcbasic"))
				bBBCBasic = true;
		}
    };

	bool	bBBCBasic = false;
};

class FTubeElite : public FEmuBase, public ITubeDataHandler
{
public:
    FTubeElite();

    bool    Init(const FEmulatorLaunchConfig& launchConfig) override;
    void    Shutdown() override;
    void    DrawEmulatorUI() override;
    void    Tick() override;
    void    Reset() override;
    void    FixupAddressRefs();

    void    FileMenuAdditions(void) override;
    void    SystemMenuAdditions(void) override;
    void    OptionsMenuAdditions(void) override;
    void    WindowsMenuAdditions(void) override;

    // Begin IInputEventHandler interface implementation
    void    OnKeyUp(int keyCode);
    void    OnKeyDown(int keyCode);
    void    OnChar(int charCode);
    void    OnGamepadUpdated(int mask);
    // End IInputEventHandler interface implementation

    // Begin ICPUInterface interface implementation
    uint8_t        ReadByte(uint16_t address) const override
    {
        return mem_rd(const_cast<mem_t*>(&Machine.Memory), address);
    }
    uint16_t    ReadWord(uint16_t address) const override
    {
        return mem_rd16(const_cast<mem_t*>(&Machine.Memory), address);
    }
    const uint8_t* GetMemPtr(uint16_t address) const override
    {
        return mem_readptr(const_cast<mem_t*>(&Machine.Memory), address);
    }

    void WriteByte(uint16_t address, uint8_t value) override
    {
        mem_wr(&Machine.Memory, address, value);
    }

    FAddressRef GetPC() override
    {
        return CodeAnalysis.Debugger.GetPC();
    }

    uint16_t    GetSP(void) override
    {
        return m65C02_s(&Machine.CPU) + 0x100;    // stack begins at 0x100
    }

    void* GetCPUEmulator(void) const override
    {
        return (void*)&Machine.CPU;
    }

    // End ICPUInterface interface implementation

	// Begin ITubeDataHandler interface implementation
	bool HandleIncomingByte(ETubeRegister reg, uint8_t val) override;
	void PollTubeCommand(void) override;
	// End ITubeDataHandler interface implementation

	void	ProcessTubeCommandByte(uint8_t cmd);

	void	OSWORD(uint8_t command,const uint8_t* pParamBlock,std::vector<uint8_t> outBlock);

    void    SetupCodeAnalysisLabels();

    bool    SaveMachineState(const char* fname);
    bool    LoadMachineState(const char* fname);


    bool    LoadEmulatorFile(const FEmulatorFile* pEmuFile) override;
    bool    NewProjectFromEmulatorFile(const FEmulatorFile& emuFile) override;
    bool    LoadProject(FProjectConfig* pConfig, bool bLoadGame) override;
    bool    SaveProject(void) override;

    bool    LoadROM(const char* pFileName, int slot);
    void    SetROMSlot(int slotNo);

    bool    LoadDiscImage(const char* pFileName);

    //const FBBCBankIds&    GetBankIds() const { return BankIds; }

    uint64_t    OnCPUTick(uint64_t pins);

	FTubeEliteMachine& GetMachine() { return Machine; }
	FTubeEliteDisplay& GetDisplay() { return Display; }

	void AddInputByte(uint8_t byte)
	{
		InputBuffer.push_back(byte);
	}

	bool PopInputByte(uint8_t& outByte)
	{
		if(InputBuffer.empty())
			return false;
		outByte = InputBuffer.front();
		InputBuffer.pop_front();
		return true;
	}

private:

	bool	LoadBinaries(void);
	FTubeEliteMachine			Machine;
	FTubeEliteDisplay			Display;
    FTubeEliteLaunchConfig		LaunchConfig;
	FTubeEliteConfig*			pConfig = nullptr;

	FTubeCommand*				pCurrentCommand = nullptr; // current Tube command being processed

	std::deque<uint8_t>			InputBuffer;    
    //FBBCBankIds            BankIds;
	int16_t						RamBankId = -1;    // RAM bank ID

    //FBBCDisplay            Display;

    std::set<FAddressRef>    InterruptHandlers;
    uint16_t                PreviousPC = 0;
    //FBBCIOAnalysis            IOAnalysis;
    std::array<uint8_t, 3 * 256> IOMemBuffer;    // 3 pages

    static uint32_t        ColourPalette[16];

    const int            BasicROMSlot = 0;
   // FROMSlot            ROMSlots[BBC_NUM_ROM_SLOTS];
    int16_t                CurrentROMBank = -1;

    FTubeElite(const FTubeElite&) = delete;                // Prevent copy-construction
    FTubeElite& operator=(const FTubeElite&) = delete;    // Prevent assignment
};


int BBCKeyFromImGuiKey(ImGuiKey key);