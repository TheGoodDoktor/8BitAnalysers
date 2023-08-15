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
	void	DrawCoordinatePositions(FCodeAnalysisState& codeAnalysis, const ImVec2& pos);
	bool	OnHovered(const ImVec2& pos, FCodeAnalysisState& codeAnalysis, FCodeAnalysisViewState& viewState);

private:
	FSpectrumEmu* pSpectrumEmu = nullptr;

	uint32_t*		FrameBuffer;	// pixel buffer to store emu output
	ImTextureID		ScreenTexture;		// texture 

	// screen inspector
	bool		bScreenCharSelected = false;
	uint16_t	SelectPixAddr = 0;
	uint16_t	SelectAttrAddr = 0;
	int			SelectedCharX = 0;
	int			SelectedCharY = 0;
	//bool		CharDataFound = false;
	//uint16_t	FoundCharDataAddress = 0;
	int			FoundCharIndex = 0;
	std::vector<FAddressRef>	FoundCharAddresses;
	uint8_t		CharData[8] = {0};
	bool		bCharSearchWrap = true;
	bool		bWindowFocused = false;

	FAddressRef	XCoordAddress;	// for debugging
	FAddressRef	YCoordAddress;
	bool		bInvertXCoord = false;
	bool		bInvertYCoord = false;
	bool		bShowCoordinates = false;
};