#pragma once

#include <cstdint>

// Graphics View Code - TODO: Move
struct FGraphicsView
{
	int				Width = 0;
	int				Height = 0;
	uint32_t*		PixelBuffer = nullptr;
	void*		Texture = nullptr;
};

FGraphicsView* CreateGraphicsView(int width, int height);
void FreeGraphicsView(FGraphicsView* pView);
void ClearGraphicsView(FGraphicsView& graphicsView, const uint32_t col);
void DrawGraphicsView(const FGraphicsView& graphicsView, float xSize, float ySize, bool bScale = false, bool bMagnifier = true);
void DrawGraphicsView(const FGraphicsView& graphicsView, bool bMagnifier = true);
