#pragma once

#include <cstdint>

// this is a surface on which to draw game graphics
class FGraphicsView
{
public:
	FGraphicsView(int width, int height);
	~FGraphicsView();

	void Clear(const uint32_t col = 0xff000000);
	void Draw(float xSize, float ySize, bool bScale = false, bool bMagnifier = true);
	void Draw(bool bMagnifier = true);

	void DrawCharLine(uint8_t charLine, int xp, int yp, uint32_t inkCol, uint32_t paperCol);

	// Draw image from a bitmap
	// Size is given in (8x8) chars
	void DrawBitImage(const uint8_t* pSrc, int xp, int yp, int widthChars, int heightChars, uint32_t inkCol, uint32_t paperCol);

	// Draw image from a bitmap
	// Size is given in (8x8) chars
	// image is arranged chat by char
	void DrawBitImageChars(const uint8_t* pSrc, int xp, int yp, int widthChars, int heightChars, uint32_t inkCol, uint32_t paperCol);

	uint32_t* GetPixelBuffer() { return PixelBuffer; }
	const uint32_t* GetPixelBuffer() const { return PixelBuffer; }
private:
	int				Width = 0;
	int				Height = 0;
	uint32_t*		PixelBuffer = nullptr;
	void*			Texture = nullptr;
};

