#pragma once

#include "CodeAnalyser/UI/ViewerBase.h"

class FPCEEmu;

class FGameDbViewer : public FViewerBase
{
public:
	FGameDbViewer(FEmuBase* pEmu);

	virtual bool Init() override;
	virtual void Shutdown() override {}
	virtual void DrawUI() override;

protected:
	void DrawGameDbTable();
	void DrawOverview();
	void DrawBanksTab();

protected:

	FPCEEmu* pPCEEmu = nullptr;
	int m_BanksTabSelectedGame = -1;
};
