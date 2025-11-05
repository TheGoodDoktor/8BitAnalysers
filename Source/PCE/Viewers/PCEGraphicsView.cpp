#include "PCEGraphicsView.h"

#include <geargrafx_core.h>
#include "../PCEEmu.h"

void FPCEGraphicsView::Draw4bppSpriteImage(const uint8_t* pSrc, int xp, int yp, int widthPixels, int heightPixels, int paletteIndex)
{
	GeargrafxCore* pCore = pPCEEmu->GetCore();

	HuC6260* huc6260 = pCore->GetHuC6260();
	HuC6270* huc6270 = pCore->GetHuC6270_1();
	u16* vram = huc6270->GetVRAM();
	u16* sat = huc6270->GetSAT();
	u16* color_table = huc6260->GetColorTable();
}

#if 0
void FPCEGraphicsView::DrawMode1Image(const uint8_t* pSrc, int xp, int yp, int widthPixels, int heightPixels, int paletteIndex)
{
	if (const uint32_t* pPalette = GetPaletteFromPaletteNo(paletteIndex))
	{
		FGraphicsView::Draw2BppImageAt(pSrc, xp, yp, widthPixels, heightPixels, pPalette);
	}
}

void FPCEGraphicsView::DrawMode0Image(const uint8_t* pSrc, int xp, int yp, int widthPixels, int heightPixels, int paletteIndex)
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
#endif