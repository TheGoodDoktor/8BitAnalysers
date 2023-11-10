#pragma once

#include <cstdint>

#include "imgui.h"
#include "Misc/InputEventHandler.h"

class FCPCEmu;
class FCodeAnalysisState;
struct FCodeAnalysisViewState;
struct FPalette;

#ifndef NDEBUG
//#define CPCVIEWER_EXTRA_DEBUG
#endif

class FCPCViewer
{
public:
	FCPCViewer() {}

	void	Init(FCPCEmu* pEmu);
	void	Draw();
	void	Tick(void);

	const uint32_t* GetFrameBuffer() const { return FrameBuffer; }
	const ImTextureID GetScreenTexture() const { return ScreenTexture; }

private:
	bool	OnHovered(const ImVec2& pos);
	float	DrawScreenCharacter(int xChar, int yChar, float x, float y, float pixelHeight) const;
	void	CalculateScreenProperties();
	int		GetScreenModeForPixelLine(int yPos) const;
	const FPalette& GetPaletteForPixelLine(int yPos) const;
	ImU32	GetFlashColour() const;

#ifdef CPCVIEWER_EXTRA_DEBUG
	void	DrawTestScreen();
#endif
private:

	FCPCEmu* pCPCEmu = nullptr;

	uint32_t* FrameBuffer = 0;	// pixel buffer to store emu output
	ImTextureID	ScreenTexture = 0;		// texture

	bool bWindowFocused = false;
	float TextureWidth = 0;
	float TextureHeight = 0;

	// The x position of where the screen starts. Effectively, this is the width of the left border.
	int ScreenEdgeL = 0;
	// The y position of where the screen starts. Effectively, this is the height of the top border.
	int ScreenTop = 0;
	// The width of the screen in pixels. Note: this is currently using mode 1 coordinates. When in mode 0, you will need to divide this by 2.
	int ScreenWidth = 0;
	// The height of the screen in pixels.
	int ScreenHeight = 0;
	// The height in pixels of a cpc character "square". CPC can have non-square characters - even 1 pixel high if you wish.
	int CharacterHeight = 0;
	// Number of horizontal characters that can be displayed. Note: in mode 0 you will need to divide this by 2. This is possibly a bit poo.
	int HorizCharCount = 0;

	int	FrameCounter = 0;

#ifdef CPCVIEWER_EXTRA_DEBUG
	bool bClickWritesToScreen = false;
#endif
};

