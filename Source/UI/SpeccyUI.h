#pragma once

#include "Speccy/Speccy.h"

struct FSpeccyUI
{
	FSpeccy*		pSpeccy;
	ui_zx_t			UIZX;
	ImTextureID		GraphicsViewTexture;
};


FSpeccyUI* InitSpeccyUI(FSpeccy *pSpeccy);
void ShutdownSpeccyUI(FSpeccyUI*pSpeccyUI);
void UpdatePreTickSpeccyUI(FSpeccyUI*pSpeccyUI);
void UpdatePostTickSpeccyUI(FSpeccyUI*pSpeccyUI);

