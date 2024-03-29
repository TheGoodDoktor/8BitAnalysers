#pragma once

#include "Misc/EmuBase.h"

class FCPCEmu;

class FCrtcViewer : public FViewerBase
{
public:
			FCrtcViewer(FEmuBase* pEmu) :FViewerBase(pEmu) { Name = "CRTC"; }
	bool	Init(void) override;
	void	Shutdown(void) override {}
	void	DrawUI() override;

private:
};