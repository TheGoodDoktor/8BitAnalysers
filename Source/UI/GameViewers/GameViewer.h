#pragma once

struct FSpeccyUI;
struct FGraphicsView;
struct FGameConfig;

// Base class for game viewer data
struct FGameViewerData
{
	FSpeccyUI*			pUI = nullptr;
	FGraphicsView*		pSpriteGraphicsView = nullptr;
	FGraphicsView*		pScreenGraphicsView = nullptr;
};

void InitGameViewer(FGameViewerData *pGameViewer, FGameConfig *pGameConfig);
