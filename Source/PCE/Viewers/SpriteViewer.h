#pragma once

#include "imgui.h"

#include "CodeAnalyser/UI/ViewerBase.h"

class FPCEEmu;

class FSpriteViewer : public FViewerBase
{
public:
	FSpriteViewer(FEmuBase* pEmu);

	virtual bool Init() override;
	virtual void Shutdown() override {}
	virtual void DrawUI() override;

	FPCEEmu* pPCEEmu = nullptr;
public:
	int HighlightSprite = -1;

private:
	void ResetScreenTexture();
	void UpdateSpriteBuffers();

private:
	static const int kNumSprites = 64;
	ImTextureID	SpriteTextures[kNumSprites] = { 0 };
	int SpriteWidths[kNumSprites];
	int SpriteHeights[kNumSprites];
	uint8_t* SpriteBuffers[kNumSprites];

	ImTextureID	ScreenTexture = 0;
	int TextureWidth = 0;
	int TextureHeight = 0;

	bool bShowMagnifier = true;
	int BackgroundColour = 0;
};
