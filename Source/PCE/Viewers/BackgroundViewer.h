#pragma once

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
	void UpdateBackground();

private:
	uint8_t* BackgroundBuffer = nullptr;

	int BufferWidth = 32;
	int BufferHeight = 32;
	ImTextureID BackgroundTexture = 0;
};
