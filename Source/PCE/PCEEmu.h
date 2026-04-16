
#pragma once

#include "Misc/EmuBase.h"
#include "huc6280.h"
#include "BankSet.h"
#include <vector>

#ifndef NDEBUG
#define ASSEMBLE_AFTER_ASM_EXPORT 1
#else
#define ASSEMBLE_AFTER_ASM_EXPORT 0
#endif

// Geargrafx fwd declares
class GeargrafxCore;
class Media;
class Memory;

struct FPCEConfig;
struct FPCEGameConfig;
class FPCECPUEmulator6502;
class FPCEViewer;
class FBatchGameLoadViewer;
class FSpriteViewer;
class FVRAMViewer;
class FPCEEmu;
struct FGameDbEntry;
struct FEmuDebugStats;
struct FGameDebugStats;
struct FAsmExportValidator;

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
	void	GlobalShortcuts(void) override;
	void	AppFocusCallback(int focused) override;
	void	DrawEmulatorUI() override;

	// Returns the primary bank ID for a given bank ID.
	// Each PCE ROM bank has up to kNumBankSetIds FCodeAnalysisBank entries (one primary
	// plus duplicates) so that the same logical bank can be simultaneously mapped to
	// multiple MPR slots. Only the primary (index 0) is exported. This function lets
	// shared code redirect any duplicate bank ID to the primary for label lookups.
	int16_t	GetCanonicalBankId(int16_t bankId) const override;
	void		BuildCanonicalBankIdLookup();
	void		BuildBankSetLookup();

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

	bool ExportAsmForCurrentGame();

	const std::unordered_map<std::string, FGamesList>& GetGamesLists() const { return	GamesLists; }
	const FPCEConfig* GetPCEGlobalConfig() { return (const FPCEConfig*)pGlobalConfig; }
	FGameDebugStats* GetGameDebugStats() const { return pGameDebugStats; }
	FGameDbEntry* GetGameDbEntry() const { return pGameDbEntry; }

	// Geargrafx helpers
	GeargrafxCore* GetCore() const { return pCore; }
	Memory* GetMemory() const { return pMemory; }
	Media* GetMedia() const { return pMedia; }
	int GetVPos() const { return *pVPos; }
	HuC6280::HuC6280_State* Get6280State() const { return p6280State; }
	HuC6270::HuC6270_State* Get6270State() const { return p6270State; }

	void OnInstructionExecuted(uint16_t pc);
	void OnVRAMWritten(uint16_t vramAddr, uint16_t value);

	uint8_t* GetFrameBuffer() const { return pFrameBuffer; }
	int16_t* GetAudioBuffer() const { return pAudioBuf; }

	FSpriteViewer* GetSpriteViewer() const { return pSpriteViewer; }
	FVRAMViewer* GetVRAMViewer() const { return pVRAMViewer; }
	FBatchGameLoadViewer* GetBatchGameLoadViewer() const { return pBatchGameLoadViewer;	}
	
	void EnableGeargrafxCallbacks(bool bEnabled);

	const FBankSet& GetBankSet(int index);

	// Get the PCE bank index (0-255) for a given bank id.
	uint8_t GetBankIndexForBankId(uint16_t bankId);

	int GetBankCount() const;
	void MapMprBank(uint8_t mprIndex, uint8_t newBankIndex);

	static constexpr int kNumBanks = 256;
	static constexpr int kNumRomBanks = 128;
	static constexpr int kNumMprSlots = 8;
	
	static constexpr int kFramebufferSize = 2048 * 512 * 4;

	FBankSet* Banks[kNumBanks] = { nullptr };
	
	// Lookup for which bank set is in each MPR slot
	int MprBankSet[kNumMprSlots] = { -1, -1, -1, -1, -1, -1, -1, -1 };

	FEmuDebugStats* pDebugStats = nullptr;

	uint16_t PrevPC = 0;
	int EmuFramesToRun = 1;

protected:

	bool LoadMachineState(const char* path, int index = -1);
	bool SaveMachineState(const char* path, int index = -1);

	void SaveGameDbEntry();

	void CheckPhysicalMemoryRangeIsMapped();
	void CheckMemoryMap();
	void CheckDupeMprBankIds();
	void UpdateDebugStats();
	void ResetBanks();
	void MapMprBanks();
	int16_t GetBankIdForMprSlot(uint8_t bankIndex, uint8_t mprIndex);
	
	void MapBankIdToMprSlot(uint8_t mprIndex, int16_t bankId);
	void RestoreMprBankMappings(const FPCEGameConfig* pConfig);

	void InitPalettes();
	void UpdatePalettes();
	void DetectDirtyBanks();

	void AddLabels();

protected:
	GeargrafxCore* pCore = nullptr;
	Media* pMedia = nullptr;
	Memory* pMemory = nullptr;
	uint8_t* pFrameBuffer = nullptr;
	int16_t* pAudioBuf = nullptr;
	int* pVPos = nullptr; // HuC6270 vertical position, cached for speed.
	HuC6280::HuC6280_State* p6280State = nullptr;
	HuC6270::HuC6270_State* p6270State = nullptr;

	FPCECPUEmulator6502* pPCE6502CPU;

	FPCEViewer* pPCEViewer = nullptr;
	FBatchGameLoadViewer* pBatchGameLoadViewer = nullptr;
	FSpriteViewer* pSpriteViewer = nullptr;
	FVRAMViewer* pVRAMViewer = nullptr;

	// used for #ifdef BANK_SWITCH_DEBUG
	bool bDoneInitialBankMapping = false;

	int16_t MprBankId[kNumMprSlots] = { -1, -1, -1, -1, -1, -1, -1, -1 };
	int16_t MprBankIdPrev[kNumMprSlots] = { -1, -1, -1, -1, -1, -1, -1, -1 };

	FBankSet BankSets[kNumBanks];
	//int16_t NullBankId = -1;

	// Fast lookup: maps each bankId to its canonical (primary) bankId.
	// Built once after all banks are created. Indexed directly by bankId.
	int16_t CanonicalBankIdLookup[FCodeAnalysisState::kMaxBanks];
	
	// Fast lookup to go from bankid to bankset
	FBankSet* BankSetLookup[FCodeAnalysisState::kMaxBanks];

	// cached for speed
	FGameDebugStats* pGameDebugStats = nullptr;
	FGameDbEntry* pGameDbEntry = nullptr;

	bool bCallbacksEnabled = true;

	FAsmExportValidator* pAsmExportValidator = nullptr;
};
