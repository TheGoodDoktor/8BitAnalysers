#pragma once

#include <stddef.h>
#include <cstdint>

class FCodeAnalysisState;

// claas for drawing the C64
class FC64Display
{
public:
	void	Init(FCodeAnalysisState* pAnalysis, void* pC64Emu);
	void	DrawUI();
	size_t          GetPixelBufferSize() const { return FramePixelBufferSize; }
	unsigned char*	GetPixelBuffer() { return FramePixelBuffer; }

	uint16_t	GetScreenBitmapAddress(int pixelX, int pixelY);
	uint16_t	GetScreenCharAddress(int pixelX, int pixelY);
	uint16_t	GetColourRAMAddress(int pixelX, int pixelY);

private:
	void*				C64Emu = nullptr;
	FCodeAnalysisState* CodeAnalysis = nullptr;

	// screen char selection
	bool			bScreenCharSelected = false;
	uint16_t		SelectBitmapAddr = 0;
	uint16_t		SelectCharAddr = 0;
	uint16_t		SelectColourRamAddr = 0;
	int				SelectedCharX = 0;
	int				SelectedCharY = 0;

	size_t          FramePixelBufferSize = 0;
	unsigned char*	FramePixelBuffer = nullptr;
	void*			FrameBufferTexture = nullptr;
	void*			DebugFrameBufferTexture = nullptr;
};