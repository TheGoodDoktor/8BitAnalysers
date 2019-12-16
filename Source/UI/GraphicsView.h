#pragma once

#include "imgui.h"
#include <cstdint>

struct FSpeccy;
struct FSpeccyUI;

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
void DrawGraphicsView(const FGraphicsView &graphicsView);

enum class GraphicsViewMode
{
	Charater,	// 8x8 bitmap graphics
	Screen		// Native frame buffer format
};

// Graphics Viewer
struct FGraphicsViewerState
{
	uint16_t		Address = 0;
	GraphicsViewMode	ViewMode = GraphicsViewMode::Charater;

	int				XSize = 1;
	int				YSize = 1;

	FGraphicsView*	pGraphicsView = nullptr;
	FSpeccy*		pSpeccy = nullptr;

};

bool InitGraphicsViewer(FGraphicsViewerState &state);
void DrawGraphicsViewer(FGraphicsViewerState &state);
