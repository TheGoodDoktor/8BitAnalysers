#pragma once

#include <cstdint>

class FCodeAnalysisState;

class FGraphicsView;

class FC64GraphicsViewer
{
public:
	void	Init(FCodeAnalysisState* pAnalysis,void* pC64Emu);
	void	Shutdown();
	void	DrawUI();

	void	DrawHiResSpriteAt(uint16_t addr, int xp, int yp);
	void	DrawMultiColourSpriteAt(uint16_t addr, int xp, int yp);
private:
	void*				C64Emu = nullptr;
	FCodeAnalysisState* CodeAnalysis = nullptr;

	FGraphicsView*		CharacterView = nullptr;
	FGraphicsView*		SpriteView = nullptr;

	int		VicBankNo = 0;

	bool			SpriteMultiColour = false;
	bool			CharacterMultiColour = false;
	uint32_t		SpriteCols[4];	// colours to draw sprites
	uint32_t		CharCols[4];	// colours to draw characters
};