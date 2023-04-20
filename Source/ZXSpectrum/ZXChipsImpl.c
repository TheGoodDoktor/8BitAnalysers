#define CHIPS_IMPL
#include "ZXChipsImpl.h"

#define CHIPS_UTIL_IMPL
#include "util/z80dasm.h"
#include "util/m6502dasm.h"

// decode whole screen
void DecodeScreen(zx_t* pZX)
{
	const int oldScanlineVal = pZX->scanline_y;
	pZX->scanline_y = 0;
	for (int i = 0; i < pZX->frame_scan_lines; i++)
	{
		_zx_decode_scanline(pZX);
	}
	pZX->scanline_y = oldScanlineVal;
}

