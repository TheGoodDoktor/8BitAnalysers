#pragma once

#include <cstdint>

#include <CodeAnalyser/CodeAnalyserTypes.h>
#include <CodeAnalyser/UI/GraphicsViewer.h>
#include "imgui.h"

class FPCEEmu;
struct FGame;
class FPCEGraphicsView;

class FPCEGraphicsViewer : public FGraphicsViewer
{
public:
	FPCEGraphicsViewer(FEmuBase *pEmu) : FGraphicsViewer(pEmu)
	{
		ScreenWidth = 320;
		ScreenHeight = 200;
	}

	void	DrawUI() override;
	void	DrawScreenViewer(void) override;
	bool	Init(void) override;

private:
	FPCEEmu*	pPCEEmu = nullptr;

//#if 0
	FPCEGraphicsView* pTestPCEGraphicsView = 0;
	//FCPCGraphicsView* pTestCPCGraphicsView = 0;
//#endif
};
