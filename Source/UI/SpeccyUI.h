#pragma once

#include "Speccy/Speccy.h"
#include <map>

struct FSpeccyUI;

struct FGameViewer
{
	std::string	Name;
	bool		bOpen;
	void		(*pDrawFunction)(FSpeccyUI *pSpeccyUI, FGameViewer &viewer);
	void *		pUserData = nullptr;
};

struct FSpeccyUI
{
	FSpeccy*		pSpeccy;
	ui_zx_t			UIZX;

	unsigned char*	GraphicsViewPixelBuffer;
	ImTextureID		GraphicsViewTexture;

	std::map<std::string, FGameViewer>	GameViewers;
};




FSpeccyUI* InitSpeccyUI(FSpeccy *pSpeccy);
void ShutdownSpeccyUI(FSpeccyUI*pSpeccyUI);
void UpdatePreTickSpeccyUI(FSpeccyUI*pSpeccyUI);
void UpdatePostTickSpeccyUI(FSpeccyUI*pSpeccyUI);

FGameViewer &AddGameViewer(FSpeccyUI *pUI, const char *pName);


