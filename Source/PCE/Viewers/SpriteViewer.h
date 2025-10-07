#pragma once

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
};
