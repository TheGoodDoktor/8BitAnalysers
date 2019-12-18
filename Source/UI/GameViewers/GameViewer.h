#pragma once

struct FSpeccyUI;
struct FGraphicsView;
struct FGame;
struct FGameConfig;

// Base class for game viewer data
struct FGameViewerData
{
	FSpeccyUI*			pUI = nullptr;
	FGraphicsView*		pSpriteGraphicsView = nullptr;
	FGraphicsView*		pScreenGraphicsView = nullptr;
};

void InitGameViewer(FGameViewerData *pGameViewer, FGameConfig *pGameConfig);

FGameViewerData *InitMiscGameViewer(FSpeccyUI *pUI, FGameConfig *pGameConfig);
void DrawMiscGameViewer(FSpeccyUI *pUI, FGame *pGame);