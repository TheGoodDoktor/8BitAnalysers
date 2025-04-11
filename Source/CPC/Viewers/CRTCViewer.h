#pragma once

#include "CodeAnalyser/UI/ViewerBase.h"

class FCPCEmu;
class FEmuBase;

class FCrtcViewer : public FViewerBase
{
public:
			FCrtcViewer(FEmuBase* pEmu) :FViewerBase(pEmu) { Name = "CRTC"; }
	bool	Init(void) override;
	void	Shutdown(void) override {}
	void	DrawUI() override;

private:
};