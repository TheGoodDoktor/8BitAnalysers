#include "CPCGraphicsView.h"

const uint32_t FCpcGraphicsView::ColourLUT[32] =
{
    0xff6B7D6E,         // #40 white
    0xff6D7D6E,         // #41 white
    0xff6BF300,         // #42 sea green
    0xff6DF3F3,         // #43 pastel yellow
    0xff6B0200,         // #44 blue
    0xff6802F0,         // #45 purple
    0xff687800,         // #46 cyan
    0xff6B7DF3,         // #47 pink
    0xff6802F3,         // #48 purple
    0xff6BF3F3,         // #49 pastel yellow
    0xff0DF3F3,         // #4A bright yellow
    0xffF9F3FF,         // #4B bright white
    0xff0605F3,         // #4C bright red
    0xffF402F3,         // #4D bright magenta
    0xff0D7DF3,         // #4E orange
    0xffF980FA,         // #4F pastel magenta
    0xff680200,         // #50 blue
    0xff6BF302,         // #51 sea green
    0xff01F002,         // #52 bright green
    0xffF2F30F,         // #53 bright cyan
    0xff010200,         // #54 black
    0xffF4020C,         // #55 bright blue
    0xff017802,         // #56 green
    0xffF47B0C,         // #57 sky blue
    0xff680269,         // #58 magenta
    0xff6BF371,         // #59 pastel green
    0xff04F571,         // #5A lime
    0xffF4F371,         // #5B pastel cyan
    0xff01026C,         // #5C red
    0xffF2026C,         // #5D mauve
    0xff017B6E,         // #5E yellow
    0xffF67B6E,         // #5F pastel blue
};


uint32_t FCpcGraphicsView::GetColFromAttr(uint8_t colBits, bool bBright)
{
	const uint32_t outCol = ColourLUT[colBits & 7];
	if (bBright == false)
		return outCol & 0xFFD7D7D7;
	else
		return outCol;
}

void FCpcGraphicsView::DrawCharLine(uint8_t charLine, int xp, int yp, uint8_t colAttr)
{
	const bool bBright = !!(colAttr & (1 << 6));
	const uint32_t inkCol = GetColFromAttr(colAttr & 7, bBright);
	const uint32_t paperCol = GetColFromAttr(colAttr >> 3, bBright);
	FGraphicsView::DrawCharLine(charLine, xp, yp, inkCol, paperCol);
}

void FCpcGraphicsView::DrawBitImage(const uint8_t* pSrc, int xp, int yp, int widthChars, int heightChars, uint8_t colAttr)
{
	const bool bBright = !!(colAttr & (1 << 6));
	const uint32_t inkCol = GetColFromAttr(colAttr & 7, bBright);
	const uint32_t paperCol = GetColFromAttr(colAttr >> 3, bBright);
	FGraphicsView::DrawBitImage(pSrc, xp, yp, widthChars, heightChars, inkCol, paperCol);
}

void FCpcGraphicsView::DrawBitImageChars(const uint8_t* pSrc, int xp, int yp, int widthChars, int heightChars, uint8_t colAttr)
{
	const bool bBright = !!(colAttr & (1 << 6));
	const uint32_t inkCol = GetColFromAttr(colAttr & 7, bBright);
	const uint32_t paperCol = GetColFromAttr(colAttr >> 3, bBright);
	FGraphicsView::DrawBitImageChars(pSrc, xp, yp, widthChars, heightChars, inkCol, paperCol);
}
