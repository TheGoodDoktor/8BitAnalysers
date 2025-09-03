#pragma once

#include "imgui.h"
#include "CodeAnalyser/UI/ViewerBase.h"

class FPCEEmu;

class FPCEViewer : public FViewerBase
{
public:
	FPCEViewer(FEmuBase* pEmu);

	virtual bool Init() override;
	virtual void Shutdown() override {}
	virtual void DrawUI() override;
	
	void Tick();

private:
	FPCEEmu* pPCEEmu = nullptr;

	ImTextureID	ScreenTexture = 0;		// texture

	bool bWindowFocused = false;
	int TextureWidth = 0;
	int TextureHeight = 0;
};
