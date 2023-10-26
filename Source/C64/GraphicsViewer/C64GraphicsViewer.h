#pragma once

#include <cstdint>

class FCodeAnalysisState;
class FGraphicsView;
class FC64Emulator;

enum class EC64ScreenMode
{
	HiresText,
	MulticolourText,
	HiresBitmap,
	MulticolourBitmap,
	ECMText,

	Invalid
};

class FC64GraphicsViewer
{
public:
	void	Init(FC64Emulator* pC64Emu);
	void	Shutdown();
	void	DrawUI();

private:
	void	DrawSpritesViewer();
	void	DrawCharactersViewer();
	void	DrawCharacterScreen(bool bMulticolour, bool ECM);
	void	DrawBitmapScreen(bool bMulticolour);
	void	DrawScreenViewer();
	void	DrawHiResSpriteAt(uint16_t addr, int xp, int yp);
	void	DrawMultiColourSpriteAt(uint16_t addr, int xp, int yp);
private:
	FC64Emulator*		C64Emu = nullptr;
	FCodeAnalysisState* CodeAnalysis = nullptr;

	FGraphicsView*		CharacterView = nullptr;
	FGraphicsView*		SpriteView = nullptr;
	FGraphicsView*		ScreenView = nullptr;
	
	int					VicBankNo = 0;
	EC64ScreenMode		ScreenMode = EC64ScreenMode::HiresText;

	bool			SpriteMultiColour = false;
	bool			CharacterMultiColour = false;
	uint32_t		SpriteCols[4];	// colours to draw sprites
	uint32_t		CharCols[4];	// colours to draw characters
};