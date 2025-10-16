#pragma once

#include "CodeAnalyser/UI/ViewerBase.h"

class FPCEEmu;

class FJoypadViewer : public FViewerBase
{
public:
	FJoypadViewer(FEmuBase* pEmu);

	virtual bool Init() override;
	virtual void Shutdown() override {}
	virtual void DrawUI() override;

	FPCEEmu* pPCEEmu = nullptr;

	bool bUpPressed = false;
	bool bDownPressed = false;
	bool bLeftPressed = false;
	bool bRightPressed = false;
	bool bSelectPressed = false;
	bool bRunPressed = false;
	bool bIPressed = false;
	bool bIIPressed = false;
};
