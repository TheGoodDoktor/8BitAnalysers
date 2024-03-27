#pragma once

#define UI_DBG_USE_Z80
#define UI_DASM_USE_Z80
#include "chips/chips_common.h"
#include "chips/z80.h"
#include "chips/m6502.h"
#include "chips/beeper.h"
#include "chips/ay38910.h"
#include "util/z80dasm.h"
#include "util/m6502dasm.h"
#include "chips/mem.h"
#include "chips/kbd.h"
#include "chips/clk.h"
#include "systems/zx.h"
#include "chips/mem.h"
/*
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
#include "ui/ui_snapshot.h"
#include "ui/ui_zx.h"
*/
//#include <map>
#include <string>
#include "Viewers/SpriteViewer.h"
#include "MemoryHandlers.h"
//#include "Disassembler.h"
//#include "FunctionHandlers.h"
#include "CodeAnalyser/CodeAnalyser.h"
#include "CodeAnalyser/IOAnalyser.h"
#include "Viewers/ViewerBase.h"
#include "Viewers/ZXGraphicsViewer.h"
#include "Viewers/SpectrumViewer.h"
#include "Viewers/FrameTraceViewer.h"
#include "Misc/GamesList.h"
#include "IOAnalysis.h"
#include "SnapshotLoaders/RZXLoader.h"
#include "Util/Misc.h"
#include "SpectrumDevices.h"
#include "Misc/EmuBase.h"

struct FGame;
struct FGameViewer;
struct FGameViewerData;
struct FProjectConfig;
struct FViewerConfig;
struct FZXSpectrumConfig;
struct FZXSpectrumGameConfig;

enum class ESpectrumModel
{
	Spectrum48K,
	Spectrum128K
};

struct FSpectrumLaunchConfig : public FEmulatorLaunchConfig
{
	void ParseCommandline(int argc, char** argv) override;
	ESpectrumModel	Model = ESpectrumModel::Spectrum48K;
	std::string		SkoolkitImport;
};

struct FGame
{
	FProjectConfig *		pConfig	= nullptr;
	FViewerConfig *		pViewerConfig = nullptr;
	FGameViewerData *	pViewerData = nullptr;
};

struct FSnapshot
{
	bool		bValid = false;
	ImTextureID	Thumbnail = 0;
	zx_t		State;
};


class FSpectrumEmu : public FEmuBase
{
public:
	FSpectrumEmu()
	{
		CPUType = ECPUType::Z80;
		for (int i = 0; i < kNoROMBanks; i++)
			ROMBanks[i] = -1;
		for (int i = 0; i < kNoRAMBanks; i++)
			RAMBanks[i] = -1;
	}

	bool	Init(const FEmulatorLaunchConfig& config) override;
    bool    InitForModel(ESpectrumModel model);
	void	Shutdown() override;
	void	Tick() override;
	void	Reset() override;
    void    OnEnterEditMode(void) override;
    void    OnExitEditMode(void) override;

	bool	LoadLua() override;
    
	bool	LoadEmulatorFile(const FEmulatorFile* pSnapshot) override;

	bool	NewProjectFromEmulatorFile(const FEmulatorFile& snapshot) override;
	bool	LoadProject(FProjectConfig* pGameConfig, bool bLoadGame) override;
	bool	SaveProject() override;

	bool	IsInitialised() const { return bInitialised; }

	//void	DrawMainMenu(double timeMS);
	
	void	FileMenuAdditions(void) override;
	void	SystemMenuAdditions(void)  override;
	void	OptionsMenuAdditions(void) override;
	void	WindowsMenuAdditions(void)  override;


	//void	DrawExportAsmModalPopup();
	//void	DrawReplaceGameModalPopup();
	void	DrawCheatsUI();
	void	AppFocusCallback(int focused) override;

	void	OnInstructionExecuted(int ticks, uint64_t pins);
	uint64_t Z80Tick(int num, uint64_t pins);

	void	DrawMemoryTools();
	void	DrawEmulatorUI() override;
	//bool	DrawDockingView();

	// disable copy & assign because this class is big!
	FSpectrumEmu(const FSpectrumEmu&) = delete;
	FSpectrumEmu& operator= (const FSpectrumEmu&) = delete;

	//ICPUInterface Begin
	uint8_t		ReadByte(uint16_t address) const override;
	uint16_t	ReadWord(uint16_t address) const override;
	const uint8_t*	GetMemPtr(uint16_t address) const override;
	void		WriteByte(uint16_t address, uint8_t value) override;

	FAddressRef	GetPC(void) override;
	uint16_t	GetSP(void) override;
	void*		GetCPUEmulator(void) const override;
	//ICPUInterface End

	void		FormatSpectrumMemory(FCodeAnalysisState& state);

    ESpectrumModel  GetCurrentSpectrumModel() const { return ZXEmuState.type == ZX_TYPE_128 ? ESpectrumModel::Spectrum128K : ESpectrumModel::Spectrum48K;}
	void SetROMBank(int bankNo);
	void SetRAMBank(int slot, int bankNo);

	void AddMemoryHandler(const FMemoryAccessHandler& handler)
	{
		MemoryAccessHandlers.push_back(handler);
	}

	const FZXSpectrumConfig* GetZXSpectrumGlobalConfig() { return (const FZXSpectrumConfig*)pGlobalConfig; }

	// machine snapshots
	// put in EmuBase? - when we're happy
	int			GetNoMachineSnapshots() const { return kNoSnapshots;}
	bool		SaveMachineSnapshot(int snapshotNo);
	bool		LoadMachineSnapshot(int snapshotNo);
	ImTextureID	GetMachineSnapshotThumbnail(int snapshotNo) const;
	// TODO: Make private
//private:
	// Emulator 
	zx_t			ZXEmuState;		// Chips Spectrum State
    zx_t            BackupState;	// Backup state for edit mode

	// snapshots
	static const int kNoSnapshots = 5;
	FSnapshot		Snapshots[kNoSnapshots];

	uint8_t*		MappedInMemory = nullptr;

	float			ExecSpeedScale = 1.0f;

	// Chips UI
	//ui_zx_t			UIZX;

	FGame *			pActiveGame = nullptr;

	//FGamesList		GamesList;
	//FGamesList		RZXGamesList;

	//Viewers
	FSpectrumViewer			SpectrumViewer;
	FFrameTraceViewer		FrameTraceViewer;
	//FZXGraphicsViewer		GraphicsViewer;
	//FCodeAnalysisState		CodeAnalysis;

	// IO Devices
	FSpectrumKeyboard	Keyboard;
	FSpectrumBeeper		Beeper;
	FAYAudioDevice			AYSoundChip;
	FSpectrum128MemoryCtrl	MemoryControl;
    
    FScreenPixMemDescGenerator  PixMemDescGenerator;
    FScreenAttrMemDescGenerator AttrMemDescGenerator;

	// Code analysis pages - to cover 48K & 128K Spectrums
	static const int	kNoBankPages = 16;	// no of pages per physical address slot (16k)
	static const int	kNoRAMPages = 128;
	static const int	kNoROMBanks = 2;
	static const int	kNoRAMBanks = 8;

	int16_t				ROMBanks[kNoROMBanks];
	int16_t				RAMBanks[kNoRAMBanks];
	int16_t				CurROMBank = -1;
	int16_t				CurRAMBank[4] = { -1,-1,-1,-1 };

	// Memory handling
	std::string							SelectedMemoryHandler;
	std::vector< FMemoryAccessHandler>	MemoryAccessHandlers;

	FMemoryStats	MemStats;

	// interrupt handling info
	bool			bHasInterruptHandler = false;
	uint16_t		InterruptHandlerAddress = 0;
	
	uint16_t		PreviousPC = 0;		// store previous pc
	int				InstructionsTicks = 0;

	FRZXManager		RZXManager;
	int				RZXFetchesRemaining = 0;

private:
	//std::vector<FViewerBase*>	Viewers;

	//bool	bReplaceGamePopup = false;
	//bool	bExportAsm = false;

	//int		ReplaceGameSnapshotIndex = 0;

	//bool	bShowDebugLog = false;
	bool	bInitialised = false;
};


uint16_t GetScreenPixMemoryAddress(int x, int y);
uint16_t GetScreenAttrMemoryAddress(int x, int y);
bool GetScreenAddressCoords(uint16_t addr, int& x, int& y);
bool GetAttribAddressCoords(uint16_t addr, int& x, int& y);
