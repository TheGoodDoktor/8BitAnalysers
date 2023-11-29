#pragma once

#include "../../Util/GraphicsView.h"
#include "Misc/EmuBase.h"

class FCharacterMapGrid;
struct FCodeAnalysisViewState;

void DrawMaskInfoComboBox(EMaskInfo* pValue);
void DrawColourInfoComboBox(EColourInfo* pValue);
void DrawCharacterSetComboBox(FCodeAnalysisState& state, FAddressRef& addr);

class FCharacterMapViewer : public FViewerBase
{
public:
	FCharacterMapViewer(FEmuBase* pEmu) : FViewerBase(pEmu) { Name = "Character Maps"; }

	bool	Init(void) override;
	void	Shutdown() override;
	void	DrawUI(void) override;

	void	GoToAddress(FAddressRef addr);

	void	SetGridSize(int x,int y);
private:
	void	DrawCharacterMapViewer(void);

	// Viewer setup
	FCharacterMapGrid*	ViewerGrid = nullptr;
};