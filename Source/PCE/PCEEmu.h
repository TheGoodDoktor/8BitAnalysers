
#pragma once

#include "Misc/EmuBase.h"

class GeargrafxCore;
class Media;
class Memory;
struct FPCEConfig;
struct FPCEGameConfig;
class FPCECPUEmulator6502;
class FPCEViewer;
class FBatchGameLoadViewer;

struct FEmuDebugStats
{
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

	const std::unordered_map<std::string, FGamesList>& GetGamesLists() const { return	GamesLists; }

	const FPCEConfig* GetPCEGlobalConfig() { return (const FPCEConfig*)pGlobalConfig; }

	GeargrafxCore* GetCore() const { return pCore; }
	Memory* GetMemory() const { return pMemory; }
	Media* GetMedia() const { return pMedia; }

	uint8_t* GetFrameBuffer() const { return pFrameBuffer; }

	int16_t GetBankForMprSlot(uint8_t bankIndex, uint8_t mprIndex);
	void MapMprBank(uint8_t mprIndex, uint8_t newBankIndex);

	void LogDupeMprBankIds();

	static const int kNumBanks = 256;
	static const int kNumRomBanks = 128;
	static const int kNumMprSlots = 8;
	static const int kNumBankSetIds = 4;

	// A set of bank ids that all represent the same logical memory.
	// PCE games can map the same bank to different physical memory ranges.
	// Eg. ROM1 being mapped to 0x4000-0x6000 and 0x8000-0xa000.
	// This happens when the same bank index is present in >1 mpr slot.
	// It is technically possible to map the same bank across the entire physical memory range.
	// 8BA doesn't support a bank being mapped into >1 memory location at the same time, so 
	// we need a set of banks that all point to the same memory.
	struct FBankSet
	{
		void SetupPrimaryBank(FCodeAnalysisState& state, uint16_t pageAddr)
		{
			FCodeAnalysisBank* pBank = state.GetBank(Banks[0].BankId);
			assert(pBank);
			pBank->PrimaryMappedPage = pageAddr;
		}
		int16_t GetNextFreeBank(uint8_t mprSlot)
		{
			for (int i = 0; i < Banks.size(); i++)
			{
				FBankSetEntry& entry = Banks[i];
				if (!entry.bMapped)
				{
					entry.bMapped = true;
					assert(SlotBankId[mprSlot] == -1);
					SlotBankId[mprSlot] = i;
					return entry.BankId;
				}
			}
			
			// todo return unused bank if we run out of banks
			return -1;
		}
		// todo: think of better name
		void UnmapCurrentBank(uint8_t mprSlot)
		{
			assert(SlotBankId[mprSlot] != -1);
			Banks[SlotBankId[mprSlot]].bMapped = false; 
			SlotBankId[mprSlot] = -1;
		}
		void Reset()
		{
			for (int i = 0; i < kNumMprSlots; i++)
				SlotBankId[i] = -1;
			for (int i = 0; i < Banks.size(); i++)
				Banks[i].bMapped = false;
		}
		void AddBankId(int16_t bankId)
		{
			Banks.push_back(FBankSetEntry({bankId, false}));
		}
		int16_t GetBankId(int index)
		{
			assert(!Banks.empty());
			if (index >= Banks.size())
				return -1;

			return Banks[index].BankId;
		}

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
	int16_t MprBankId[kNumMprSlots] = { -1, -1, -1, -1, -1, -1, -1, -1 };

	FBankSet BankSets[kNumBanks];

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

	FPCECPUEmulator6502* pPCE6502CPU;

	FPCEViewer* pPCEViewer = nullptr;
	FBatchGameLoadViewer* pBatchGameLoadViewer = nullptr;
};
