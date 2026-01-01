#pragma once

#include "Misc/EmuBase.h"

#include <chips/mem.h>

#include "ArcadeZ80Config.h"
#include "ArcadeZ80Machine.h"
#include <set>
#include <array>
#include <deque>

#include <imgui.h>
#include "Debug/DebugLog.h"

class FTubeCommand;


struct FArcadeZ80LaunchConfig : public FEmulatorLaunchConfig
{
    void ParseCommandline(int argc, char** argv) override
    {
        // Parse commandline arguments
		if (argc > 1)
		{
			//if(strcmp(argv[1],"-bbcbasic"))
				//bBBCBasic = true;
		}
    };

};

struct FArcadeZ80Debug
{
	bool	bDebugTubeComms = false;
	bool	bOSWORDDebug = false;
	bool	bLogVDUChars = false;
};


class FArcadeZ80: public FEmuBase
{
public:
    FArcadeZ80();

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
		return Machine.CPU.sp;
    }

    void* GetCPUEmulator(void) const override
    {
        return (void*)&Machine.CPU;
    }

    // End ICPUInterface interface implementation

	void	DebugBreak() { CodeAnalysis.Debugger.Break(); }
	void	ProcessTubeChar(uint8_t charVal);
	void	ProcessTubeCommandByte(uint8_t cmd);

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

	FArcadeZ80Machine& GetMachine() { return Machine; }
	const FArcadeZ80Debug& GetDebug() { return Debug;}

	void AddInputByte(uint8_t byte)
	{
		InputBuffer.push_back(byte);
		//LOGINFO("Added input byte: 0x%02X", byte);
	}

	bool GetInputByte(uint8_t& outByte)
	{
		if(InputBuffer.empty())
			return false;
		outByte = InputBuffer.front();
		//LOGINFO("Got input byte: 0x%02X", outByte);
		InputBuffer.pop_front();
		return true;
	}

	void FlushInputBuffer()
	{
		InputBuffer.clear();
		//LOGINFO("Input buffer flushed");
	}

	bool PopInputByte()
	{
		if (InputBuffer.empty())
			return false;
		InputBuffer.pop_front();
		return true;
	}

private:

	bool	LoadBinaries(void);
	FArcadeZ80Machine			Machine;
	FArcadeZ80LaunchConfig		LaunchConfig;
	FArcadeZ80Config*			pConfig = nullptr;
	FArcadeZ80Debug				Debug;

	FTubeCommand*				pCurrentCommand = nullptr; // current Tube command being processed

	std::deque<uint8_t>			InputBuffer;    
	int16_t						RamBankId = -1;    // RAM bank ID

	bool						bHasInterruptHandler = false;
	uint16_t					InterruptHandlerAddress = 0;
    std::set<FAddressRef>    InterruptHandlers;
    uint16_t                PreviousPC = 0;
    std::array<uint8_t, 3 * 256> IOMemBuffer;    // 3 pages

    static uint32_t        ColourPalette[16];

    const int            BasicROMSlot = 0;
    int16_t                CurrentROMBank = -1;

    FArcadeZ80(const FArcadeZ80&) = delete;                // Prevent copy-construction
    FArcadeZ80& operator=(const FArcadeZ80&) = delete;    // Prevent assignment
};


uint8_t BBCKeyFromImGuiKey(ImGuiKey key);
uint8_t GetPressedInternalKeyCode(void);
bool IsInternalKeyDown(uint8_t internalKeyCode);

