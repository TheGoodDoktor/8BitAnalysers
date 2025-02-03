#pragma once

#pragma once

#include <stddef.h>
#include <cstdint>
#include "imgui.h"

class FCodeAnalysisState;
class FBBCEmulator;
class FGraphicsView;

// class for drawing the BBC
class FBBCDisplay
{
public:
	void	Init(FCodeAnalysisState* pAnalysis, FBBCEmulator* pBBCEmu);
	void	Tick();
	void	DrawUI();

	//uint16_t	GetScreenBitmapAddress(int pixelX, int pixelY);
	//uint16_t	GetScreenCharAddress(int pixelX, int pixelY);
	//uint16_t	GetColourRAMAddress(int pixelX, int pixelY);

private:
	void	OverlayHighlightAddress(ImVec2 pos);


	FBBCEmulator*		pBBCEmu = nullptr;
	FCodeAnalysisState* pCodeAnalysis = nullptr;

	// screen char selection
	bool			bScreenCharSelected = false;
	uint16_t		SelectBitmapAddr = 0;
	uint16_t		SelectCharAddr = 0;
	uint16_t		SelectColourRamAddr = 0;
	float			SelectedCharX = 0;
	float			SelectedCharY = 0;

	FGraphicsView*		pScreenView = nullptr;	

	bool			bWindowFocused = false;
};