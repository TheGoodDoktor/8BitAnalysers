#pragma once

#include <cstdint>
#include <vector>

#include "imgui.h"
#include "Misc/InputEventHandler.h"
#include "CodeAnalyser/CodeAnalyserTypes.h"

class FSpectrumEmu;
class FCodeAnalysisState;
struct FCodeAnalysisViewState;

class FSpectrumViewer
{
public:
	FSpectrumViewer() {}

	void	Init(FSpectrumEmu* pEmu);
	void	Draw();
	void	Tick(void);

	const uint32_t* GetFrameBuffer() const { return FrameBuffer; }

private:
	// private methods
	void	DrawCoordinatePositions( const ImVec2& imguiPos, int xp, int yp);
	void	DrawSelectedCharUI(const ImVec2& pos);
	bool	OnHovered(const ImVec2& pos, FCodeAnalysisState& codeAnalysis, FCodeAnalysisViewState& viewState);
	ImU32	GetFlashColour() const;

private:
	FSpectrumEmu* pSpectrumEmu = nullptr;

	uint32_t*		FrameBuffer;	// pixel buffer to store emu output
	ImTextureID		ScreenTexture;		// texture 

	// screen inspector
	bool		bScreenCharSelected = false;
	FAddressRef	SelectPixAddr;
	FAddressRef	SelectAttrAddr;
	int			SelectedCharX = 0;
	int			SelectedCharY = 0;
	int			FoundCharIndex = 0;
	std::vector<FAddressRef>	FoundCharAddresses;
	uint8_t		CharData[8] = {0};
	bool		bCharSearchWrap = true;
	bool		bWindowFocused = false;

	bool		bBreakOnCharPixelWrite = false;
	FAddressRef	CharacterPixelBPAddress;
	bool		bBreakOnCharAttrWrite = false;
	FAddressRef	CharacterAttrBPAddress;

	FAddressRef	XCoordAddress;	// for debugging
	FAddressRef	YCoordAddress;
	bool		bInvertXCoord = false;
	bool		bInvertYCoord = false;
	bool		bShowCoordinates = false;

	//int			FrameCounter = 0;
	//int			Scale = 0;
};