#pragma once

#include <cstdint>

#include <CodeAnalyser/CodeAnalyserTypes.h>
#include <CodeAnalyser/UI/GraphicsViewer.h>
#include "imgui.h"
#include "huc6270_defines.h"

class FPCEGraphicsView;
class FPCEEmu;
struct FGame;

struct FScanlineSnapshot
{
	uint16_t bxr = 0;     // latched horizontal scroll offset
	int32_t  byr_eff = 0; // effective vertical scroll offset (bg_offset_y at render time)
	uint16_t mwr = 0;     // memory width register (determines BAT/screen size)
	uint16_t cr = 0;      // control register (bg/sprite enable bits)
};

class FPCEGraphicsViewer : public FGraphicsViewer
{
public:
	FPCEGraphicsViewer(FEmuBase *pEmu) : FGraphicsViewer(pEmu)
	{
		ScreenWidth = 320;
		ScreenHeight = 200;
	}

	void	DrawUI() override;
	bool	Init(void) override;

	const uint32_t* GetCurrentPalette() const override;

	void	OnScanlineDraw(int rasterLine, uint16_t bxr, int32_t byrEff, uint16_t mwr, uint16_t cr);
	void	OnFrameStart(const uint16_t* sat);

private:
	void	DrawTest(void);
	void	DrawScreenViewer(void) override;

private:
	FPCEEmu*	pPCEEmu = nullptr;

	uint8_t*	ScreenBuffer = nullptr;
	ImTextureID	ScreenTexture = nullptr;
	int			TextureWidth = 0;
	int			TextureHeight = 0;
	bool		bDrawBackground = true;
	bool		bDrawSprites = true;

	static constexpr int kMaxScanlines = 242;
	FScanlineSnapshot	ScanlineSnapshots[kMaxScanlines] = {};
	uint16_t			FrameSAT[HUC6270_SAT_SIZE] = {}; // SAT snapshot taken at start of each frame
	int					LastRenderedScanline = -1;        // last raster line captured this frame (-1 = no data)

#ifndef NDEBUG
	FPCEGraphicsView* pTestPCEGraphicsView = 0;
#endif
};
