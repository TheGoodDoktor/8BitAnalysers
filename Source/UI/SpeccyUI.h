#pragma once

#include "Speccy/Speccy.h"
#include <map>
#include "SpriteViewer.h"
#include "MemoryHandlers.h"
#include "Disassembler.h"
#include "FunctionHandlers.h"

struct FSpeccyUI;
struct FGame;
struct FGameViewer;
struct FGameViewerData;
struct FGameConfig;
struct FCodeAnalysisState;

struct FGame
{
	FGameConfig *		pConfig;
	FGameViewerData *	pViewerData;
};

/*struct FGameViewer
{
	std::string	Name;
	bool		bOpen;
	void		(*pDrawFunction)(FSpeccyUI *pSpeccyUI, FGameViewer &viewer);
	void *		pUserData = nullptr;
};*/

enum class LabelType
{
	Data,
	Function,
	Code,
};

struct FLabelInfo
{
	std::string				Name;
	LabelType				Type;
	std::map<uint16_t,int>	References;
};


struct FSpeccyUI
{
	FSpeccyUI(){}
	
	// disable copy & assign because this class is big!
	FSpeccyUI(const FSpeccyUI&) = delete;
	FSpeccyUI& operator= (const FSpeccyUI&) = delete;
	
	FSpeccy*		pSpeccy = nullptr;
	ui_zx_t			UIZX;

	FGraphicsView*	pGraphicsViewerView = nullptr;

	std::vector<FGameConfig *>	GameConfigs;
	FGame *				pActiveGame = nullptr;

	std::string				SelectedSpriteList;
	std::map<std::string, FUISpriteList>	SpriteLists;

	// labels
	FLabelInfo*				Labels[0x10000];

	// Memory handling
	std::string				SelectedMemoryHandler;
	std::vector< FMemoryAccessHandler>	MemoryAccessHandlers;

	FMemoryStats	MemStats;

	// Function info
	uint16_t							SelectedFunction;
	std::map<uint16_t, FFunctionInfo>	Functions;
	std::vector<uint16_t>				FunctionStack;
	FDasmState							FunctionDasm;

	FCodeAnalysisState *				pCodeAnalysis = nullptr;

	uint16_t dasmCurr = 0;

	static const int kPCHistorySize = 32;
	uint16_t PCHistory[kPCHistorySize];
	int PCHistoryPos = 0;
};




FSpeccyUI* InitSpeccyUI(FSpeccy *pSpeccy);
void ShutdownSpeccyUI(FSpeccyUI*pSpeccyUI);
void UpdatePreTickSpeccyUI(FSpeccyUI*pSpeccyUI);
void UpdatePostTickSpeccyUI(FSpeccyUI*pSpeccyUI);

void AddMemoryHandler(FSpeccyUI *pUI, const FMemoryAccessHandler &handler);

//FGameViewer &AddGameViewer(FSpeccyUI *pUI, const char *pName);
void PlotImageAt(const uint8_t *pSrc, int xp, int yp, int w, int h, uint32_t *pDest, int destWidth, uint8_t colAttr = 0x7);

//void PlotCharacterBlockAt(const FSpeccy *pSpeccy, uint16_t addr, int xp, int yp, int w, int h, uint32_t *pDest, int destWidth);
//void PlotCharacterAt(const uint8_t *pSrc, int xp, int yp, uint32_t *pDest, int destWidth);


