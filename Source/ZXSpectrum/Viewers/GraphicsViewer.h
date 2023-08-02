#pragma once

#include "imgui.h"
#include <cstdint>
#include "SpriteViewer.h"
#include <map>
#include <string>

#include <CodeAnalyser/CodeAnalyserTypes.h>

class FSpectrumEmu;
struct FGame;
class FZXGraphicsView;


enum class GraphicsViewMode : int
{
	Character,	// 8x8 bitmap graphics
	CharacterWinding,	// winding bitmap (0,0) (1,0) (1,1) (0,1)

	Count
};

// Graphics Viewer
// TODO: Make class
struct FGraphicsViewerState
{
	int32_t			Bank = -1;
	uint16_t		AddressOffset = 0;	// offset to view from the start of the region (bank or physical address space)
	uint32_t		MemorySize = 0x10000;	// size of area being viewed
	FAddressRef		ClickedAddress;
	GraphicsViewMode	ViewMode = GraphicsViewMode::Character;
	//bool			bColumnMode = true;
	int				HeatmapThreshold = 4;

	int				XSizePixels = 8;			// Image X Size in pixels
	int				YSizePixels = 8;			// Image Y Size in pixels
	int				ImageCount = 1;	// how many images?
	bool			YSizePixelsFineCtrl = false;

	std::string		NewConfigName;

	std::string				SelectedSpriteList;
	std::map<std::string, FUISpriteList>	SpriteLists;

	// housekeeping
	FZXGraphicsView* pGraphicsView = nullptr;
	FZXGraphicsView* pScreenView = nullptr;
	FSpectrumEmu*	pEmu = nullptr;		// can we phase this out?
	FGame*			pGame = nullptr;	// can we phase this out?

};

bool InitGraphicsViewer(FGraphicsViewerState &state);
void ShutdownGraphicsViewer(FGraphicsViewerState& state);
void DrawGraphicsViewer(FGraphicsViewerState &state);
