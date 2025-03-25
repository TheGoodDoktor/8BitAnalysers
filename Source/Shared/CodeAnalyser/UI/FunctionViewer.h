#pragma once

#include <Misc/EmuBase.h>

class FFunctionViewer : public FViewerBase
{
public:
	FFunctionViewer(FEmuBase* pEmu) : FViewerBase(pEmu) { Name = "Function Viewer"; }

	bool	Init() override;
	void	Shutdown() override;
	void	DrawUI() override;
};

