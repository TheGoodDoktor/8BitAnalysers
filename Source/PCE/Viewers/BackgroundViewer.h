#pragma once

#include <cstdint>
#include <vector>
#include "imgui.h"
#include "CodeAnalyser/UI/ViewerBase.h"

class FPCEEmu;

class FBackgroundViewer : public FViewerBase
{
public:
	FBackgroundViewer(FEmuBase* pEmu);

	virtual bool Init() override;
	virtual void Shutdown() override {}
	virtual void DrawUI() override;

	FPCEEmu* pPCEEmu = nullptr;
private:
	bool UpdateBackground();

private:
	uint8_t*              BackgroundBuffer  = nullptr;
	int                   BufferWidth       = 0;
	int                   BufferHeight      = 0;
	ImTextureID           BackgroundTexture = 0;

	std::vector<uint16_t> ShadowBAT;
	int                   LastRenderedFrame = -1;
	uint16_t              ShadowColorTable[512] = {};
};
