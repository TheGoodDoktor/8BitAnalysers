#pragma once

#define UI_DBG_USE_Z80
#define UI_DASM_USE_Z80
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
#include "ui/ui_zx.h"

#include <map>
#include "Viewers/SpriteViewer.h"
#include "MemoryHandlers.h"
#include "Disassembler.h"
#include "FunctionHandlers.h"
#include "CodeAnalyser/CodeAnalyser.h"
#include "Viewers/ViewerBase.h"
#include "Viewers/GraphicsViewer.h"
#include "Viewers/SpectrumViewer.h"
#include "Viewers/FrameTraceViewer.h"
#include "SnapshotLoaders/GamesList.h"
#include "IOAnalysis.h"
#include "SnapshotLoaders/RZXLoader.h"

struct FGame;
struct FGameViewer;
struct FGameViewerData;
struct FGameConfig;
struct FViewerConfig;

enum class ESpectrumModel
{
	Spectrum48K,
	Spectrum128K
};



struct FSpectrumConfig
{
	ESpectrumModel	Model;
	int				NoStateBuffers = 0;
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
	}

	bool	Init(const FSpectrumConfig& config);
	void	Shutdown();
	void	StartGame(FGameConfig* pGameConfig);
	bool	StartGame(const char* pGameName);
	void	SaveCurrentGameData();
	void	DrawMainMenu(double timeMS);
	void	DrawCheatsUI();

	int		TrapFunction(uint16_t pc, int ticks, uint64_t pins);
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
	uint16_t	GetPC(void) override;
	bool		IsAddressBreakpointed(uint16_t addr) override;
	bool		ToggleExecBreakpointAtAddress(uint16_t addr) override;
	bool		ToggleDataBreakpointAtAddress(uint16_t addr, uint16_t dataSize) override;
	void		Break(void) override;
	void		Continue(void) override;
	void		StepOver(void) override;
	void		StepInto(void) override;
	void		GraphicsViewerSetAddress(uint16_t address) override;
	bool		ShouldExecThisFrame(void) const override;
	void		InsertROMLabels(FCodeAnalysisState& state) override;
	void		InsertSystemLabels(FCodeAnalysisState& state) override;
	//ICPUInterface End

	void AddMemoryHandler(const FMemoryAccessHandler& handler)
	{
		MemoryAccessHandlers.push_back(handler);
	}

	void GraphicsViewerGoToAddress(uint16_t address)
	{
		GraphicsViewer.Address = address;
	}

	// TODO: Make private

	// Emulator 
	zx_t			ZXEmuState;	// Chips Spectrum State
	int				CurrentLayer = 0;	// layer ??

	unsigned char*	FrameBuffer;	// pixel buffer to store emu output
	ImTextureID		Texture;		// texture 
	
	bool			ExecThisFrame = true; // Whether the emulator should execute this frame (controlled by UI)
	float			ExecSpeedScale = 1.0f;

	// Chips UI
	ui_zx_t			UIZX;

	FGame *			pActiveGame = nullptr;

	FGamesList		GamesList;
	FGamesList		RZXGamesList;

	//Viewers
	FSpectrumViewer			SpectrumViewer;
	FFrameTraceViewer		FrameTraceViewer;
	FGraphicsViewerState	GraphicsViewer;
	FCodeAnalysisState		CodeAnalysis;
	FIOAnalysis				IOAnalysis;

	// Code analysis pages - 48K spectrum for now...
	static const int kNoROMPages = 16;
	static const int kNoRAMPages = 48;
	FCodeAnalysisPage		ROMPages[kNoROMPages];
	FCodeAnalysisPage		RAMPages[kNoRAMPages];

	// Memory handling
	std::string				SelectedMemoryHandler;
	std::vector< FMemoryAccessHandler>	MemoryAccessHandlers;
	std::vector< FMemoryAccess>	FrameScreenPixWrites;
	std::vector< FMemoryAccess>	FrameScreenAttrWrites;

	FMemoryStats	MemStats;

	// Function info
	uint16_t							SelectedFunction;
	std::map<uint16_t, FFunctionInfo>	Functions;
	std::vector<uint16_t>				FunctionStack;
	FDasmState							FunctionDasm;

	uint16_t dasmCurr = 0;

	static const int kPCHistorySize = 32;
	uint16_t PCHistory[kPCHistorySize];
	int PCHistoryPos = 0;

	FRZXManager		RZXManager;

	bool bShowImGuiDemo = false;
private:
	z80_tick_t	OldTickCB = nullptr;
	void*		OldTickUserData = nullptr;

	std::vector<FViewerBase*>	Viewers;
};


void PlotImageAt(const uint8_t *pSrc, int xp, int yp, int w, int h, uint32_t *pDest, int destWidth, uint8_t colAttr = 0x7);
uint16_t GetScreenPixMemoryAddress(int x, int y);
uint16_t GetScreenAttrMemoryAddress(int x, int y);
void GetScreenAddressCoords(uint16_t addr, int& x, int& y);
