#pragma once

#include "imgui.h"

// Graphics View Code - TODO: Move
struct FGraphicsView
{
	int				Width = 0;
	int				Height = 0;
	unsigned char*	PixelBuffer = nullptr;
	ImTextureID		Texture = nullptr;
};

FGraphicsView *CreateGraphicsView(int width, int height);
void DrawGraphicsView(const FGraphicsView &graphicsView, const ImVec2 &size);
void DrawGraphicsView(const FGraphicsView &graphicsView);

