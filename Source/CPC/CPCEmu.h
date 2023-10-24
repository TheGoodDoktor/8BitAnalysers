#pragma once

#include "SnapshotLoaders/GameLoader.h"

#define UI_DBG_USE_Z80
#define UI_DASM_USE_Z80

#pragma once

#include "chips/chips_common.h"
#include "chips/z80.h"
#include "chips/ay38910.h"
#include "util/z80dasm.h"
#include "chips/mem.h"
#include "chips/kbd.h"
#include "chips/clk.h"

#include "chips/i8255.h"
#include "chips/mc6845.h"
#include "chips/am40010.h"
#include "chips/upd765.h"
#include "chips/fdd.h"
#include "chips/fdd_cpc.h"
#include "systems/cpc.h"
#include "chips/mem.h"

#include "ui/ui_util.h"
#include "ui/ui_fdd.h"
#include "ui/ui_chip.h"
#include "ui/ui_z80.h"
#include "ui/ui_ay38910.h"
#include "ui/ui_audio.h"
#include "ui/ui_kbd.h"
#include "ui/ui_dasm.h"
#include "ui/ui_dbg.h"
#include "ui/ui_memedit.h"
#include "ui/ui_memmap.h"
#include "ui/ui_mc6845.h"
#include "ui/ui_i8255.h"
#include "ui/ui_upd765.h"
#include "ui/ui_am40010.h"
#include "ui/ui_snapshot.h"
#include "ui/ui_cpc.h"

#include <map>
#include <string>

#include "CPCScreen.h"
#include "CodeAnalyser/CodeAnalyser.h"
//#include "CodeAnalyser/IOAnalyser.h"
#include "Viewers/CPCViewer.h"
#include "Viewers/FrameTraceViewer.h"
#include "Viewers/CPCGraphicsViewer.h"
#include "MemoryHandlers.h"
#include "IOAnalysis.h"

struct FGameViewerData;
struct FGameConfig;
struct FViewerConfig;
class FViewerBase;
class FScreenPixMemDescGenerator;
struct FCPCConfig;

enum class ECpcModel
{
	CPC_464,
	CPC_6128,
	CPC_KCKompact,
};

enum EROMBank
{
	ROM_OS = 0,
	ROM_AMSDOS,
	ROM_BASIC,

	ROM_NONE,
};

struct FCpcConfig
{
	void ParseCommandline(int argc, char** argv);
	ECpcModel Model = ECpcModel::CPC_464;
	std::string	SpecificGame;
};

struct FGame
{
	FGameConfig* pConfig = nullptr;
	FViewerConfig* pViewerConfig = nullptr;
	FGameViewerData* pViewerData = nullptr;
};

class FCpcEmu : public ICPUInterface
{
public:
	FCpcEmu()
	{
		CPUType = ECPUType::Z80;
	}

	bool	Init(const FCpcConfig& config);
	void	Shutdown();
	void	StartGame(FGameConfig* pGameConfig, bool bLoadGameData = true);
	bool	StartGame(const char* pGameName);
	void	SaveCurrentGameData();

	void	OnInstructionExecuted(int ticks, uint64_t pins);
	uint64_t Z80Tick(int num, uint64_t pins);

	void	Tick();
	void	DrawMemoryTools();
	void	DrawUI();
	bool	DrawDockingView();

	void	DrawFileMenu();
	void	DrawSystemMenu();
	void	DrawHardwareMenu();
	void	DrawOptionsMenu();
	void	DrawToolsMenu();
	void	DrawWindowsMenu();
	void	DrawDebugMenu();
	void	DrawMenus();
	void	DrawMainMenu(double timeMS);
	void	DrawExportAsmModalPopup();
	void	DrawReplaceGameModalPopup();

	// disable copy & assign because this class is big!
	FCpcEmu(const FCpcEmu&) = delete;
	FCpcEmu& operator= (const FCpcEmu&) = delete;

	//ICPUInterface Begin
	uint8_t	ReadByte(uint16_t address) const override;
	uint16_t	ReadWord(uint16_t address) const override;
	const uint8_t* GetMemPtr(uint16_t address) const override;
	void		WriteByte(uint16_t address, uint8_t value) override;
	FAddressRef	GetPC(void) override;
	uint16_t	GetSP(void) override;
	void		GraphicsViewerSetView(FAddressRef address) override;
	void*		GetCPUEmulator(void) const override;
	//ICPUInterface End
	
	//void SetROMBank(int bankNo);
	void SetROMBankLo(int bankNo);
	void SetROMBankHi(int bankNo);
	void SetRAMBank(int slot, int bankNo);
	void SetRAMBanksPreset(int bankPresetIndex);

	void UpdatePalette();

	bool NewGameFromSnapshot(int snapshotIndex);

	// Emulator 
	cpc_t			CpcEmuState;		// Chips CPC State
	uint8_t*		MappedInMemory = nullptr;
	FCPCConfig*		pGlobalConfig = nullptr;

	float			ExecSpeedScale = 1.0f;

	ui_cpc_t		UICpc;

	FGame*			pActiveGame = nullptr;

	// Viewers
	FCpcViewer				CpcViewer;
	FFrameTraceViewer		FrameTraceViewer;
	FCPCGraphicsViewer		GraphicsViewer;
	FCodeAnalysisState		CodeAnalysis;
	
	// todo: refactor this to move all event related code out of it
	FIOAnalysis				IOAnalysis;

	// todo: hook this up to the IO Analyser
	// IO Devices
	//FAYAudioDevice		AYSoundChip;

	static const int	kNoBankPages = 16;	// no of pages per physical address slot (16k)
	static const int	kNoRAMPages = 128;
	static const int	kNoROMBanks = 3;
	static const int	kNoRAMBanks = 8;

	int16_t				ROMBanks[kNoROMBanks];
	int16_t				RAMBanks[kNoRAMBanks];
	//FCodeAnalysisPage	ROMPages[kNoROMPages];
	//FCodeAnalysisPage	RAMPages[kNoRAMPages];
	
	int16_t				CurROMBankLo = -1;
	int16_t				CurROMBankHi = -1;
	int16_t				CurRAMBank[4] = { -1,-1,-1,-1 };

	// Memory handling
	std::string				SelectedMemoryHandler;
	std::vector< FMemoryAccessHandler>	MemoryAccessHandlers;
	//std::vector< FMemoryAccess>	FrameScreenPixWrites;

	FMemoryStats	MemStats;

	uint16_t		PreviousPC = 0;		// store previous pc
	int			InstructionsTicks = 0;

	bool	bShowImGuiDemo = false;
	bool	bShowImPlotDemo = false;

	const FGamesList& GetGamesList() const { return GamesList;  }

	FCPCScreen Screen;

private:
	FGamesList		GamesList;
	FCpcGameLoader	GameLoader;

	std::vector<FViewerBase*>	Viewers;

	FScreenPixMemDescGenerator* pScreenMemDescGenerator = 0;

	bool bReplaceGamePopup = false;
	int ReplaceGameSnapshotIndex = 0;

	bool bExportAsm = false;

	bool	bInitialised = false;
};
