
#pragma once

#include "Misc/EmuBase.h"
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
class FPCECPUEmulator6502;
class FPCEViewer;
class FBatchGameLoadViewer;
class FSpriteViewer;
class FVRAMViewer;
class FVRAMAnalysisState;
class FCDROMViewer;
class FPCEEmu;
struct FGameDbEntry;
struct FEmuDebugStats;
struct FGameDebugStats;
struct FAsmExportValidator;
class FCDROMAnalyser;
class FRecentMemoryAccess;

struct HuC6280_State;
struct HuC6270_State;

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

	// Platform-specific analysis JSON hooks (FEmuBase)
	void	ExportPlatformAnalysisJson(nlohmann::json& jsonDoc) override;
	bool	ImportPlatformAnalysisJson(const nlohmann::json& jsonDoc) override;
	bool	MprBankIdsAreValid() const;

	// Returns the primary bank ID for a given bank ID.
	// This allows shared code to redirect any duplicate bank ID to the primary for label lookups.
	int16_t	GetCanonicalBankId(int16_t bankId) const override;
	bool IsBankIdCanonical(int16_t bankId) const { return GetCanonicalBankId(bankId) == bankId; };
	
	FGraphicsView*	GetScreen() const override;
	const uint8_t*	GetScreenBuffer(int& width, int& height) const override;
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

	void RegisterDataRead(uint16_t pc, uint16_t dataAddr);
	void RegisterDataWrite(uint16_t pc, uint16_t dataAddr, uint8_t value);

	bool ExportAsmForCurrentGame();

	void SoftResetMachine();

	const std::unordered_map<std::string, FGamesList>& GetGamesLists() const { return	GamesLists; }
	const FPCEConfig* GetPCEGlobalConfig() { return (const FPCEConfig*)pGlobalConfig; }
	FGameDebugStats* GetGameDebugStats() const { return pGameDebugStats; }
	FGameDbEntry* GetGameDbEntry() const { return pGameDbEntry; }

	// Geargrafx helpers
	GeargrafxCore* GetCore() const { return pCore; }
	Memory* GetMemory() const { return pMemory; }
	Media* GetMedia() const { return pMedia; }
	int GetVPos() const { return *pVPos; }
	HuC6280_State* Get6280State() const { return p6280State; }
	HuC6270_State* Get6270State() const { return p6270State; }

	bool IsCDROM() const;
	const FCDROMAnalyser* GetCDROMAnalyser() const { return pCDROMAnalyser; }

	// Callbacks from geargrafx
	void OnInstructionStarted(uint16_t pc, uint8_t opcode);
	void OnInstructionFinished(uint16_t pc/*, uint8_t opcode*/);
	void OnVRAMWritten(uint16_t vramAddr, uint16_t value);
	void OnVRAMRead(uint16_t vramAddr, uint16_t value);
	void OnIRQ(uint16_t vector, uint16_t interruptedPc, uint16_t routineAddr);

	uint8_t* GetFrameBuffer() const { return pFrameBuffer; }
	int16_t* GetAudioBuffer() const { return pAudioBuf; }

	FSpriteViewer* GetSpriteViewer() const { return pSpriteViewer; }
	FGraphicsViewerBase* GetGraphicsViewer() const { return pGraphicsViewer; }
	FVRAMViewer* GetVRAMViewer() const { return pVRAMViewer; }
	FBatchGameLoadViewer* GetBatchGameLoadViewer() const { return pBatchGameLoadViewer;	}
	FVRAMAnalysisState* GetVRAMAnalysisState() const { return pVRAMState; }
	FRecentMemoryAccess* GetRecentMemAccess() const { return pRecentMemoryAccess; }

	void EnableGeargrafxCallbacks(bool bEnabled);

	int  CreateUserPalette(int dynamicPaletteIndex);

	// Get the PCE bank index (0-255) for a given bank id.
	uint8_t GetHwBankIndex(uint16_t bankId);

	const FBankSet& GetBankSet(int hwBankIndex) const;
	FBankSet* GetBankSetPtr(int hwBankIndex) const;
	FBankSet* GetBankSetFromBankId(int16_t bankId) const { return (bankId >= 0 && bankId < FCodeAnalysisState::BankCount) ? BankSetLookup[bankId] : nullptr; }
	bool IsUnusedBank(int16_t bankId) const;

	int GetGameBankCount() const;
	void MapMprBank(uint8_t mprIndex, uint8_t newBankIndex);

	// Maximum number of PC Engine hardware banks/pages.
	static constexpr int kNumHwBanks = 256;
	static constexpr int kNumRomBanks = 128;
	static constexpr int kNumMprSlots = 8;
	
	static constexpr int kFramebufferSize = 2048 * 512 * 4;
	
	// Lookup for which bank set is in each MPR slot
	int MprBankSet[kNumMprSlots] = { -1, -1, -1, -1, -1, -1, -1, -1 };

	FEmuDebugStats* pDebugStats = nullptr;

	int EmuFramesToRun = 1;

	static const char* kPCERomGameListName;
	static const char* kCDRomGameListName;

protected:

	void ResetProject();

	bool LoadMachineState(const char* path, int index = -1);
	bool SaveMachineState(const char* path, int index = -1);

	void SaveGameDbEntry();

	void CreateBanks();
	void CheckPhysicalMemoryRangeIsMapped();
	void CheckMemoryMap();
	void CheckDupeMprBankIds();
	void UpdateDebugStats();
	void ResetBanks();
	void MapMprBanks();
	int16_t GetBankIdForMprSlot(uint8_t hwBankIndex, uint8_t mprIndex);
	
	bool MapBankIdToMprSlot(uint8_t mprIndex, int16_t bankId);

	void BuildCanonicalBankIdLookup();
	void BuildBankSetLookup();

	void InitPalettes();
	void UpdatePalettes();
	void DetectDirtyBanks();

	void AddLabels();
	void AddInterruptVectorFunctionLabels(int16_t bankId);

protected:
	GeargrafxCore* pCore = nullptr;
	Media* pMedia = nullptr;
	Memory* pMemory = nullptr;
	uint8_t* pFrameBuffer = nullptr;
	int16_t* pAudioBuf = nullptr;
	int* pVPos = nullptr; // HuC6270 vertical position, cached for speed.
	HuC6280_State* p6280State = nullptr;
	HuC6270_State* p6270State = nullptr;
	FVRAMAnalysisState* pVRAMState = nullptr;
	FCDROMAnalyser* pCDROMAnalyser = nullptr;
	FPCECPUEmulator6502* pPCE6502CPU;
	FRecentMemoryAccess* pRecentMemoryAccess = nullptr;

	FPCEViewer* pPCEViewer = nullptr;
	FBatchGameLoadViewer* pBatchGameLoadViewer = nullptr;
	FSpriteViewer* pSpriteViewer = nullptr;
	FVRAMViewer* pVRAMViewer = nullptr;
	FCDROMViewer* pCDROMViewer = nullptr;

	// used for BANK_SWITCH_DEBUG
	bool bDoneInitialBankMapping = false;

	int16_t MprBankId[kNumMprSlots] = { -1, -1, -1, -1, -1, -1, -1, -1 };
	int16_t MprBankIdPrev[kNumMprSlots] = { -1, -1, -1, -1, -1, -1, -1, -1 };

	// The bank set storage. Indexed by hw bank index.
	// Not every entry will be used - it depends on how many banks the current game has. 
	FBankSet BankSets[kNumHwBanks];

	// BankSet redirection table. These hold ptrs into the BankSets.
	// Every entry should be filled. There may be duplicate entries.
	FBankSet* BankSetPtrs[kNumHwBanks] = { nullptr };

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

	uint8_t* pEditModeBackupState = nullptr;
	size_t EditModeBackupStateSize = 0;
};
