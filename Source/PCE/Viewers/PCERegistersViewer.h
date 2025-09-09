#pragma once

#include "imgui.h"
#include "CodeAnalyser/UI/ViewerBase.h"

class FPCEEmu;

class FPCERegistersViewer : public FViewerBase
{
public:
	FPCERegistersViewer(FEmuBase* pEmu);

	virtual bool Init() override;
	virtual void Shutdown() override {}
	virtual void DrawUI() override;

private:
	FPCEEmu* pPCEEmu = nullptr;
};
