
#pragma once

#include "Misc/EmuBase.h"
#include "huc6280.h"

class GeargrafxCore;
class Media;
class Memory;
struct FPCEConfig;
struct FPCEGameConfig;
class FPCECPUEmulator6502;
class FPCEViewer;
class FBatchGameLoadViewer;
class FSpriteViewer;
//class HuC6280_State;

struct FEmuDebugStats
{
	void Reset()
	{
	}
	std::map<std::string, int> GamesWithDupeBanks;
};

struct FPCELaunchConfig : public FEmulatorLaunchConfig
{
	void ParseCommandline(int argc, char** argv) override;
};

class FPCEEmu : public FEmuBase
{
public:
	FPCEEmu()
	{
	}

	// FEmuBase Begin
	bool	Init(const FEmulatorLaunchConfig& config) override;
	void	Shutdown() override;
	void	Tick() override;
	void	Reset() override;
	void	OnEnterEditMode(void) override;
	void	OnExitEditMode(void) override;
	bool	LoadEmulatorFile(const FEmulatorFile* pSnapshot) override;
	bool	NewProjectFromEmulatorFile(const FEmulatorFile& snapshot) override;
	bool	LoadProject(FProjectConfig* pGameConfig, bool bLoadGame) override;
	bool	SaveProject() override;

	void	FileMenuAdditions(void) override;
	void	SystemMenuAdditions(void)  override;
	void	OptionsMenuAdditions(void) override;
	void	ActionMenuAdditions(void) override;
	void	WindowsMenuAdditions(void)  override;
	void	AppFocusCallback(int focused) override;
	void	DrawEmulatorUI() override;
	// FEmuBase End

	// disable copy & assign because this class is big!
	FPCEEmu(const FPCEEmu&) = delete;
	FPCEEmu& operator= (const FPCEEmu&) = delete;

	//ICPUInterface Begin
	uint8_t		ReadByte(uint16_t address) const override;
	uint16_t	ReadWord(uint16_t address) const override;
	const uint8_t* GetMemPtr(uint16_t address) const override;
	void		WriteByte(uint16_t address, uint8_t value) override;

	FAddressRef	GetPC(void) override;
	uint16_t	GetSP(void) override;
	ICPUEmulator* GetCPUEmulator(void) const override;
	//ICPUInterface End

	void PostInstructionTick();

	const std::unordered_map<std::string, FGamesList>& GetGamesLists() const { return	GamesLists; }

	const FPCEConfig* GetPCEGlobalConfig() { return (const FPCEConfig*)pGlobalConfig; }

	GeargrafxCore* GetCore() const { return pCore; }
	Memory* GetMemory() const { return pMemory; }
	Media* GetMedia() const { return pMedia; }
	int GetVPos() const { return *pVPos; }
	HuC6280::HuC6280_State* Get6280State() const { return p6280State; }

	uint8_t* GetFrameBuffer() const { return pFrameBuffer; }

	FSpriteViewer* GetSpriteViewer() const { return pSpriteViewer; }

	int16_t GetBankForMprSlot(uint8_t bankIndex, uint8_t mprIndex);
	void MapMprBank(uint8_t mprIndex, uint8_t newBankIndex);

	void CheckDupeMprBankIds();

	static const int kNumBanks = 256;
	static const int kNumRomBanks = 128;
	static const int kNumMprSlots = 8;
	
	// The default number of banks in a bank set.
	// The number includes the primary bank and any extra banks for duplicates.
	// For example, a value of 4 means 1 primary and 3 duplicates.
	static const int kNumBankSetIds = 5;

	// A set of bank ids that all represent the same logical memory.
	// PCE games can map the same bank to different physical memory ranges.
	// Eg. ROM1 being mapped to 0x4000-0x6000 and 0x8000-0xa000.
	// This happens when the same bank index is present in >1 mpr slot.
	// It is technically possible to map the same bank across the entire physical memory range.
	// 8BA doesn't support a bank being mapped into >1 memory location at the same time, so 
	// we need a set of banks that all point to the same memory.
	struct FBankSet
	{
		void SetPrimaryMappedPage(FCodeAnalysisState& state, int bankSetIndex, uint16_t pageAddr);
		int16_t GetFreeBank(uint8_t mprSlot);
		void SetBankFreed(uint8_t mprSlot);
		void Reset();
		void AddBankId(int16_t bankId);
		int16_t GetBankId(int index) const;

		struct FBankSetEntry
		{
			int16_t BankId = -1;
			bool bMapped = false;
		};
		
		// Bank set index for each mpr slot. Only will be set to a bank id when a bank gets mapped.
		int SlotBankId[kNumMprSlots] = { -1, -1, -1, -1, -1, -1, -1, -1 };
	
		// List of all bank ids.
		// Entry 0 is the primary bank. It should always have a primary mapped page.
		// Entries >1 are the dupe banks and will get a primary mapped page if they get mapped in.
		std::vector<FBankSetEntry> Banks;
	};

	FBankSet* Banks[kNumBanks] = { nullptr };
	int MprBankSet[kNumMprSlots] = { -1, -1, -1, -1, -1, -1, -1, -1 };

	FEmuDebugStats DebugStats;

protected:
	bool LoadMachineState(const char* fname);
	bool SaveMachineState(const char* fname);


	void CheckPhysicalMemoryRangeIsMapped();
	void CheckMemoryMap();
	void ResetBanks();

protected:
	GeargrafxCore* pCore = nullptr;
	Media* pMedia = nullptr;
	Memory* pMemory = nullptr;
	uint8_t* pFrameBuffer = nullptr;
	int16_t* pAudioBuf = nullptr;
	int* pVPos = nullptr; // HuC6270 vertical position, cached for speed.
	HuC6280::HuC6280_State* p6280State = nullptr;

	FPCECPUEmulator6502* pPCE6502CPU;

	FPCEViewer* pPCEViewer = nullptr;
	FBatchGameLoadViewer* pBatchGameLoadViewer = nullptr;
	FSpriteViewer* pSpriteViewer = nullptr;

	bool bDoneInitialBankMapping = false;

	int16_t MprBankId[kNumMprSlots] = { -1, -1, -1, -1, -1, -1, -1, -1 };

	FBankSet BankSets[kNumBanks];
	int16_t NullBankId = -1;
};
