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

enum class ItemType
{
	Label,
	Code,
	Data,
};

struct FItem
{
	ItemType		Type;
	std::string		Comment;
	uint16_t		Address;
	uint16_t		ByteSize;
	int				FrameLastAccessed = -1;
};

struct FLabelInfo : FItem
{
	FLabelInfo() { Type = ItemType::Label; }

	std::string				Name;
	LabelType				LabelType;
	std::map<uint16_t,int>	References;	
};

struct FCodeInfo : FItem
{
	FCodeInfo() :FItem() { Type = ItemType::Code; }

	std::string		Text;				// Disassembly text
	uint16_t		JumpAddress = 0;	// optional jump address
};

enum class DataType
{
	Byte,
	Word,
	Text,		// ascii text
	Graphics,	// pixel data
	Blob,		// opaque data blob
};

struct FDataInfo : FItem
{
	FDataInfo() :FItem() { Type = ItemType::Data; }
	
	DataType	DataType = DataType::Byte;

	std::map<uint16_t, int>	References;	// address and counts of data access instructions
};

enum class Key
{
	SetItemData,
	SetItemText,
	SetItemCode,

	AddLabel,
	Rename,

	Count
};


struct FSpeccyUI
{
	FSpeccyUI(){}
	
	// disable copy & assign because this class is big!
	FSpeccyUI(const FSpeccyUI&) = delete;
	FSpeccyUI& operator= (const FSpeccyUI&) = delete;
	
	FSpeccy*		pSpeccy = nullptr;
	ui_zx_t			UIZX;

	int				KeyConfig[(int)Key::Count];

	FGraphicsView*	pGraphicsViewerView = nullptr;

	std::vector<FGameConfig *>	GameConfigs;
	FGame *				pActiveGame = nullptr;

	std::string				SelectedSpriteList;
	std::map<std::string, FUISpriteList>	SpriteLists;

	// code analysis information
	static const int kAddressSize = 1 << 16;
	FLabelInfo*				Labels[kAddressSize];
	FCodeInfo*				CodeInfo[kAddressSize];
	FDataInfo*				DataInfo[kAddressSize];
	bool					bCodeAnalysisDataDirty = false;

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

	int	CurrentFrameNo = 0;

	bool bShowImGuiDemo = false;
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


