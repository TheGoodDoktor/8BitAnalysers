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

#include <map>
#include <string>
#include "Viewers/SpriteViewer.h"
#include "MemoryHandlers.h"
//#include "Disassembler.h"
//#include "FunctionHandlers.h"
#include "CodeAnalyser/CodeAnalyser.h"
#include "Viewers/ViewerBase.h"
#include "Viewers/ZXGraphicsViewer.h"
#include "Viewers/SpectrumViewer.h"
#include "Viewers/FrameTraceViewer.h"
#include "SnapshotLoaders/GamesList.h"
#include "IOAnalysis.h"
#include "SnapshotLoaders/RZXLoader.h"
#include "Util/Misc.h"

struct FGame;
struct FGameViewer;
struct FGameViewerData;
struct FGameConfig;
struct FViewerConfig;
struct FSkoolFileInfo;

enum class ESpectrumModel
{
	Spectrum48K,
	Spectrum128K
};



struct FSpectrumConfig
{
	void ParseCommandline(int argc, char** argv);
	ESpectrumModel	Model = ESpectrumModel::Spectrum48K;
	std::string		SpecificGame;
	std::string		SkoolkitImport;
};

struct FGame
{
	FGameConfig *		pConfig	= nullptr;
	FViewerConfig *		pViewerConfig = nullptr;
	FGameViewerData *	pViewerData = nullptr;
};


class FSpectrumEmu : public ICPUInterface
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

	bool	Init(const FSpectrumConfig& config);
	void	Shutdown();

	bool	IsInitialised() const { return bInitialised; }

	void	StartGame(FGameConfig* pGameConfig, bool bLoadGameData = true);
	bool	StartGame(const char* pGameName);
	void	SaveCurrentGameData();
	bool	NewGameFromSnapshot(int snapshotIndex);

	void	DrawMainMenu(double timeMS);
	void	DrawExportAsmModalPopup();
	void	DrawReplaceGameModalPopup();
	void	DrawCheatsUI();
	bool	ImportSkoolFile(const char* pFilename, const char* pOutSkoolInfoName = nullptr, FSkoolFileInfo* pSkoolInfo=nullptr);
	bool	ExportSkoolFile(bool bHexadecimal, const char* pName = nullptr);
	void	DoSkoolKitTest(const char* pGameName, const char* pInSkoolFileName, bool bHexadecimal, const char* pOutSkoolName = nullptr);
	void	AppFocusCallback(int focused);

	void	OnInstructionExecuted(int ticks, uint64_t pins);
	uint64_t Z80Tick(int num, uint64_t pins);

	void	Tick();
	void	DrawMemoryTools();
	void	DrawUI();
	bool	DrawDockingView();

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
	void		GraphicsViewerSetView(FAddressRef address) override;
	void*		GetCPUEmulator(void) const override;
	//ICPUInterface End

	void		FormatSpectrumMemory(FCodeAnalysisState& state);

	void SetROMBank(int bankNo);
	void SetRAMBank(int slot, int bankNo);

	void AddMemoryHandler(const FMemoryAccessHandler& handler)
	{
		MemoryAccessHandlers.push_back(handler);
	}
#if 0
	void GraphicsViewerGoToAddress(FAddressRef address)
	{
		// TODO: this code should be in the graphics viewer
		const FCodeAnalysisBank* pBank = CodeAnalysis.GetBank(address.BankId);
		
		if (pBank == nullptr || pBank->IsMapped())	// default to physical memory view
		{
			GraphicsViewer.AddressOffset = address.Address;
			GraphicsViewer.bShowPhysicalMemory = true;
		}
		else
		{
			GraphicsViewer.AddressOffset = address.Address - pBank->GetMappedAddress();
			GraphicsViewer.Bank = address.BankId;
			GraphicsViewer.bShowPhysicalMemory = false;
		}
	}

	void GraphicsViewerSetCharWidth(uint16_t width)
	{
		GraphicsViewer.XSizePixels = width * 8;
	}
#endif
	// TODO: Make private
//private:
	// Emulator 
	zx_t			ZXEmuState;	// Chips Spectrum State
	uint8_t*		MappedInMemory = nullptr;

	float			ExecSpeedScale = 1.0f;

	// Chips UI
	ui_zx_t			UIZX;

	FGame *			pActiveGame = nullptr;

	FGamesList		GamesList;
	FGamesList		RZXGamesList;

	//Viewers
	FSpectrumViewer			SpectrumViewer;
	FFrameTraceViewer		FrameTraceViewer;
	FZXGraphicsViewer		GraphicsViewer;
	FCodeAnalysisState		CodeAnalysis;
	FIOAnalysis				IOAnalysis;

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

	bool		bShowImGuiDemo = false;
	bool		bShowImPlotDemo = false;
private:
	std::vector<FViewerBase*>	Viewers;

	bool	bReplaceGamePopup = false;
	bool	bExportAsm = false;

	int		ReplaceGameSnapshotIndex = 0;

	bool	bShowDebugLog = false;
	bool	bInitialised = false;
};


uint16_t GetScreenPixMemoryAddress(int x, int y);
uint16_t GetScreenAttrMemoryAddress(int x, int y);
bool GetScreenAddressCoords(uint16_t addr, int& x, int& y);
bool GetAttribAddressCoords(uint16_t addr, int& x, int& y);