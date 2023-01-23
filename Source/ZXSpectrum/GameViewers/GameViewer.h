#pragma once

class FSpectrumEmu;
class FZXGraphicsView;
struct FGame;
struct FGameConfig;
struct FViewerConfig;

// Base class for game viewer data
struct FGameViewerData
{
	virtual ~FGameViewerData();

	FSpectrumEmu*		pEmu = nullptr;

	FZXGraphicsView*	pSpriteGraphicsView = nullptr;
	FZXGraphicsView*	pScreenGraphicsView = nullptr;
};

void InitGameViewer(FGameViewerData *pGameViewer, FGameConfig *pGameConfig);

// View Config Management
bool AddViewerConfig(FViewerConfig* pConfig);
FViewerConfig*	GetViewConfigForGame(const char *pGameName);
