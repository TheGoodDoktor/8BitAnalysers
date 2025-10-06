#pragma once

#include "CodeAnalyser/UI/ViewerBase.h"

class FPCEEmu;

class FPaletteViewer : public FViewerBase
{
public:
	FPaletteViewer(FEmuBase* pEmu);

	virtual bool Init() override;
	virtual void Shutdown() override {}
	virtual void DrawUI() override;

	FPCEEmu* pPCEEmu = nullptr;
};
