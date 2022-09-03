#pragma once

class FSpectrumEmu;
struct FGraphicsView;
struct FGame;
struct FGameConfig;
struct FViewerConfig;

// Base class for game viewer data
struct FGameViewerData
{
	FSpectrumEmu*		pEmu = nullptr;
	FGraphicsView*		pSpriteGraphicsView = nullptr;
	FGraphicsView*		pScreenGraphicsView = nullptr;
};

void InitGameViewer(FGameViewerData *pGameViewer, FGameConfig *pGameConfig);

//FGameViewerData *InitMiscGameViewer(FSpeccyUI *pUI, FGameConfig *pGameConfig);
//void DrawMiscGameViewer(FSpeccyUI *pUI, FGame *pGame);

// View Config Management
bool AddViewerConfig(FViewerConfig* pConfig);
FViewerConfig*	GetViewConfigForGame(const char *pGameName);
