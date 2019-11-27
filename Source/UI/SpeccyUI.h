#pragma once

#include "Speccy/Speccy.h"
#include <map>
#include "SpriteViewer.h"

struct FSpeccyUI;
struct FGame;
struct FGameViewer;
struct FGameViewerData;

struct FGameConfig
{
	std::string		Name;
	std::string		Z80file;
	
	FGameViewerData *(*pInitFunction)(FSpeccyUI *pUI, FGameConfig *pGameConfig);
	void(*pDrawFunction)(FSpeccyUI *pSpeccyUI, FGame* pGame);

	std::map<std::string, FSpriteDefConfig> SpriteConfigs;
};

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

enum class MemoryAccessType
{
	Read,
	Write,
	Execute
};

struct FMemoryAccessHandler
{
	// configuration
	std::string			Name;
	bool				bEnabled = true;
	bool				bBreak = false;
	MemoryAccessType	Type;
	uint16_t			MemStart;
	uint16_t			MemEnd;
	
	void(*pHandlerFunction)(FMemoryAccessHandler &handler, FGame* pGame, uint16_t pc, uint64_t pins) = nullptr;

	// stats
	int						TotalCount = 0;
	std::map<uint16_t, int>	CallerCounts;
	std::map<uint16_t, int>	AddressCounts;
};

enum class MemoryUse
{
	Code,
	Data,
	Unknown	// unknown/unused - never read from or written to
};

struct FMemoryBlock
{
	MemoryUse	Use;
	uint16_t	StartAddress;
	uint16_t	EndAddress;
	bool		ContainsSelfModifyingCode = false;
};

struct FMemoryStats
{
	// counters for each memory address
	int		ExecCount[0xffff];
	int		ReadCount[0xffff];
	int		WriteCount[0xffff];

	std::vector< FMemoryBlock>	MemoryBlockInfo;

	std::vector<uint16_t>	CodeAndDataList;
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
	//unsigned char*	GraphicsViewPixelBuffer = nullptr;
	//ImTextureID		GraphicsViewTexture;

	//std::map<std::string, FGameViewer>	GameViewers;

	std::vector<FGameConfig *>	GameConfigs;
	FGame *				pActiveGame = nullptr;

	std::string				SelectedSpriteList;
	std::map<std::string, FUISpriteList>	SpriteLists;

	// Memory handling
	std::string				SelectedMemoryHandler;
	std::vector< FMemoryAccessHandler>	MemoryAccessHandlers;

	FMemoryStats	MemStats;

	uint16_t dasmCurr = 0;
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


