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

struct FSpeccyUI;
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




// TODO: Mae this a speccy emulator class
struct FSpeccyUI
{
	FSpeccyUI(){}
	
	// disable copy & assign because this class is big!
	FSpeccyUI(const FSpeccyUI&) = delete;
	FSpeccyUI& operator= (const FSpeccyUI&) = delete;
	
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

	// screen inspector
	bool		bScreenCharSelected = false;
	uint16_t	SelectPixAddr = 0;
	uint16_t	SelectAttrAddr = 0;
	int			SelectedCharX = 0;
	int			SelectedCharY = 0;
	bool		CharDataFound = false;
	uint16_t	FoundCharDataAddress = 0;

	uint16_t dasmCurr = 0;

	static const int kPCHistorySize = 32;
	uint16_t PCHistory[kPCHistorySize];
	int PCHistoryPos = 0;

	bool bShowImGuiDemo = false;

	// trace
	//int					ShowFrame = 0;
	//int					CurrentTraceFrame = 0;
	//static const int	kNoFramesInTrace = 300;
	//FSpeccyFrameTrace	FrameTrace[kNoFramesInTrace];
};




FSpeccyUI* InitSpeccyUI(FSpeccy *pSpeccy);
FSpeccyUI* GetSpeccyUI();
bool StartGame(FSpeccyUI* pUI, const char *pGameName);
void SaveCurrentGameData(FSpeccyUI *pUI);
void ShutdownSpeccyUI(FSpeccyUI*pSpeccyUI);
void UpdatePreTickSpeccyUI(FSpeccyUI*pSpeccyUI);
void UpdatePostTickSpeccyUI(FSpeccyUI*pSpeccyUI);

void AddMemoryHandler(FSpeccyUI *pUI, const FMemoryAccessHandler &handler);

//FGameViewer &AddGameViewer(FSpeccyUI *pUI, const char *pName);
void PlotImageAt(const uint8_t *pSrc, int xp, int yp, int w, int h, uint32_t *pDest, int destWidth, uint8_t colAttr = 0x7);

//void PlotCharacterBlockAt(const FSpeccy *pSpeccy, uint16_t addr, int xp, int yp, int w, int h, uint32_t *pDest, int destWidth);
//void PlotCharacterAt(const uint8_t *pSrc, int xp, int yp, uint32_t *pDest, int destWidth);


