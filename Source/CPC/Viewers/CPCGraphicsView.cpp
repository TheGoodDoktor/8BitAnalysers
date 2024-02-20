#include "CPCGraphicsView.h"

void FCPCGraphicsView::DrawMode1Image(const uint8_t* pSrc, int xp, int yp, int widthPixels, int heightPixels, int paletteIndex)
{
	if (const uint32_t* pPalette = GetPaletteFromPaletteNo(paletteIndex))
	{
		FGraphicsView::Draw2BppImageAt(pSrc, xp, yp, widthPixels, heightPixels, pPalette);
	}
}

void FCPCGraphicsView::DrawMode0Image(const uint8_t* pSrc, int xp, int yp, int widthPixels, int heightPixels, int paletteIndex)
{
	if (const FPaletteEntry* pPaletteEntry = GetPaletteEntry(paletteIndex))
	{
		if (pPaletteEntry->NoColours < 16)
			return;

		const uint32_t* pPalette = GetPaletteFromPaletteNo(paletteIndex);
		if (pPalette)
		{
			FGraphicsView::Draw4BppWideImageAt(pSrc, xp, yp, widthPixels, heightPixels, pPalette);
		}
	}
}