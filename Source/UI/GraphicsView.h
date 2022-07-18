#pragma once

#include "imgui.h"
#include <cstdint>
#include "SpriteViewer.h"
#include <map>
#include <string>

struct FSpeccy;
struct FSpeccyUI;
struct FGame;

// Graphics View Code - TODO: Move
struct FGraphicsView
{
	int				Width = 0;
	int				Height = 0;
	uint32_t*		PixelBuffer = nullptr;
	ImTextureID		Texture = nullptr;
};

FGraphicsView *CreateGraphicsView(int width, int height);
void ClearGraphicsView(FGraphicsView &graphicsView, const uint32_t col);
void DrawGraphicsView(const FGraphicsView &graphicsView, const ImVec2 &size, bool bScale = false, bool bMagnifier = true);
void DrawGraphicsView(const FGraphicsView &graphicsView, bool bMagnifier = true);

enum class GraphicsViewMode
{
	Character,	// 8x8 bitmap graphics
	CharacterWinding,	// winding bitmap (0,0) (1,0) (1,1) (0,1)
	Screen,		// Native frame buffer format

	Count
};

// Graphics Viewer
struct FGraphicsViewerState
{
	uint16_t		Address = 0;
	uint16_t		ClickedAddress = 0;
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
	FGraphicsView*	pGraphicsView = nullptr;
	FSpeccy*		pSpeccy = nullptr;
	FSpeccyUI*		pUI = nullptr;
	FGame*			pGame = nullptr;

};

bool InitGraphicsViewer(FGraphicsViewerState &state);
void DrawGraphicsViewer(FGraphicsViewerState &state);

void GraphicsViewerGoToAddress(uint16_t address);
