#pragma once

#include <cstdint>

#include <CodeAnalyser/CodeAnalyserTypes.h>
#include <CodeAnalyser/UI/GraphicsViewer.h>
#include "imgui.h"

class FPCEGraphicsView;
class FPCEEmu;
struct FGame;

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

#ifndef NDEBUG
	FPCEGraphicsView* pTestPCEGraphicsView = 0;
#endif
};
