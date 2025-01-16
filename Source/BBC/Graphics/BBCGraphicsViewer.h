#pragma once

#include "CodeAnalyser/UI/GraphicsViewer.h"

class FBBCEmulator;

class FBBCGraphicsViewer : public FGraphicsViewer
{
	friend class FC64ScreenAccessGrid;
public:
	FBBCGraphicsViewer(FBBCEmulator* pEmu);
	bool	Init() override;
	void	Shutdown() override;
	void	DrawUI() override;

	void	DrawScreenViewer() override;

private:
	FGraphicsView* ScreenView = nullptr;

};