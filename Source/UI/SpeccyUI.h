#pragma once

#include "Speccy/Speccy.h"
#include <map>
#include "SpriteViewer.h"

struct FSpeccyUI;
struct FGameViewer;

struct FGameConfig
{
	std::string		Name;
	std::string		Z80file;
	void(*pDrawFunction)(FSpeccyUI *pSpeccyUI, FGameConfig* pGameConfig);
	std::map<std::string, FSpriteDefConfig> SpriteConfigs;
	void *			pUserData;
};

/*struct FGameViewer
{
	std::string	Name;
	bool		bOpen;
	void		(*pDrawFunction)(FSpeccyUI *pSpeccyUI, FGameViewer &viewer);
	void *		pUserData = nullptr;
};*/

struct FSpeccyUI
{
	FSpeccy*		pSpeccy;
	ui_zx_t			UIZX;

	unsigned char*	GraphicsViewPixelBuffer;
	ImTextureID		GraphicsViewTexture;

	//std::map<std::string, FGameViewer>	GameViewers;

	std::vector<FGameConfig *>	GameConfigs;
	FGameConfig *				pActiveGame = nullptr;

	std::string				SelectedSpriteList;
	std::map<std::string, FUISpriteList>	SpriteLists;
	
};




FSpeccyUI* InitSpeccyUI(FSpeccy *pSpeccy);
void ShutdownSpeccyUI(FSpeccyUI*pSpeccyUI);
void UpdatePreTickSpeccyUI(FSpeccyUI*pSpeccyUI);
void UpdatePostTickSpeccyUI(FSpeccyUI*pSpeccyUI);

//FGameViewer &AddGameViewer(FSpeccyUI *pUI, const char *pName);
void PlotImageAt(const uint8_t *pSrc, int xp, int yp, int w, int h, uint32_t *pDest, int destWidth, uint8_t colAttr = 0x7);

//void PlotCharacterBlockAt(const FSpeccy *pSpeccy, uint16_t addr, int xp, int yp, int w, int h, uint32_t *pDest, int destWidth);
//void PlotCharacterAt(const uint8_t *pSrc, int xp, int yp, uint32_t *pDest, int destWidth);


