#pragma once

#include "Speccy/Speccy.h"
#include <map>
#include "Viewers/SpriteViewer.h"
#include "MemoryHandlers.h"
#include "Disassembler.h"
#include "FunctionHandlers.h"
#include "CodeAnalyser/CodeAnalyser.h"
#include "Viewers/GraphicsViewer.h"
#include "Viewers/SpectrumViewer.h"
#include "Viewers/FrameTraceViewer.h"
#include "IOAnalysis.h"

struct FGame;
struct FGameViewer;
struct FGameViewerData;
struct FGameConfig;
struct FViewerConfig;

struct FGame
{
	FGameConfig *		pConfig	= nullptr;
	FViewerConfig *		pViewerConfig = nullptr;
	FGameViewerData *	pViewerData = nullptr;
};

/*struct FGameViewer
{
	std::string	Name;
	bool		bOpen;
	void		(*pDrawFunction)(FSpeccyUI *pSpeccyUI, FGameViewer &viewer);
	void *		pUserData = nullptr;
};*/


class FSpectrumEmu : public ICPUInterface
{
public:
	FSpectrumEmu()
	{
		CPUType = ECPUType::Z80;
	}

	bool	Init(FSpeccy* pSpeccy);
	void	Shutdown();
	void	StartGame(FGameConfig* pGameConfig);
	bool	StartGame(const char* pGameName);
	void	SaveCurrentGameData();
	void	DrawMainMenu(double timeMS);
	void	DrawCheatsUI();
	void	UpdatePreTick();
	void	UpdatePostTick();
	void	DrawMemoryTools();
	void	DrawUI();
	bool	DrawDockingView();


	// disable copy & assign because this class is big!
	FSpectrumEmu(const FSpectrumEmu&) = delete;
	FSpectrumEmu& operator= (const FSpectrumEmu&) = delete;

	//ICPUInterface Begin
	uint8_t		ReadByte(uint16_t address) const override;
	uint16_t	ReadWord(uint16_t address) const override;
	uint16_t	GetPC(void) override;
	void		Break(void) override;
	void		Continue(void) override;
	void		GraphicsViewerSetAddress(uint16_t address) override;
	bool		ExecThisFrame(void) override;
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

	FSpeccy*		pSpeccy = nullptr;
	ui_zx_t			UIZX;

	FGame *				pActiveGame = nullptr;

	FSpectrumViewer			SpectrumViewer;
	FFrameTraceViewer		FrameTraceViewer;
	FGraphicsViewerState	GraphicsViewer;
	FCodeAnalysisState		CodeAnalysis;
	FIOAnalysisState		IOAnalysis;

	// Code analysis pages - 48K spectrum for now...
	static const int kNoROMPages = 16;
	static const int kNoRAMPages = 48;
	FCodeAnalysisPage		ROMPages[kNoROMPages];
	FCodeAnalysisPage		RAMPages[kNoRAMPages];

	// Memory handling
	std::string				SelectedMemoryHandler;
	std::vector< FMemoryAccessHandler>	MemoryAccessHandlers;

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

	bool bShowImGuiDemo = false;
};



//FSpeccyUI* InitSpeccyUI(FSpeccy *pSpeccy);
//SpeccyUI* GetSpeccyEmulator();
//bool StartGame(FSpectrumEmu* pUI, const char *pGameName);
//void SaveCurrentGameData(FSpeccyUI *pUI);
//void ShutdownSpeccyUI(FSpeccyUI*pSpeccyUI);
//void UpdatePreTickSpeccyUI(FSpeccyUI*pSpeccyUI);
//void UpdatePostTickSpeccyUI(FSpeccyUI*pSpeccyUI);

//void AddMemoryHandler(FSpeccyUI *pUI, const FMemoryAccessHandler &handler);

//FGameViewer &AddGameViewer(FSpeccyUI *pUI, const char *pName);
void PlotImageAt(const uint8_t *pSrc, int xp, int yp, int w, int h, uint32_t *pDest, int destWidth, uint8_t colAttr = 0x7);

//void PlotCharacterBlockAt(const FSpeccy *pSpeccy, uint16_t addr, int xp, int yp, int w, int h, uint32_t *pDest, int destWidth);
//void PlotCharacterAt(const uint8_t *pSrc, int xp, int yp, uint32_t *pDest, int destWidth);


