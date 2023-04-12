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


enum class GraphicsViewMode
{
	Character,	// 8x8 bitmap graphics
	CharacterWinding,	// winding bitmap (0,0) (1,0) (1,1) (0,1)
	Screen,		// Native frame buffer format

	Count
};

// Graphics Viewer
// TODO: Make class
struct FGraphicsViewerState
{
	int32_t			Bank = -1;
	uint16_t		AddressOffset = 0;	// ofset to view from the start of the region (bank or physical address space)
	uint32_t		MemorySize = 0x10000;	// size of area being viewed
	FAddressRef		ClickedAddress;
	GraphicsViewMode	ViewMode = GraphicsViewMode::Character;
	//bool			bColumnMode = true;
	int				HeatmapThreshold = 4;

	int				XSize = 1;			// Image X Size in characters
	int				YSize = 8;			// Image Y Size in pixels
	int				ImageCount = 10;	// how many images?

	std::string		NewConfigName;

	std::string				SelectedSpriteList;
	std::map<std::string, FUISpriteList>	SpriteLists;

	// housekeeping
	FZXGraphicsView*	pGraphicsView = nullptr;
	FSpectrumEmu*	pEmu = nullptr;
	FGame*			pGame = nullptr;

};

bool InitGraphicsViewer(FGraphicsViewerState &state);
void DrawGraphicsViewer(FGraphicsViewerState &state);
