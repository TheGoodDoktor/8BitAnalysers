#pragma once

#include "../../Util/GraphicsView.h"
#include "Misc/EmuBase.h"
#include "MemoryAccessGrid.h"

struct FCodeAnalysisViewState;

void DrawMaskInfoComboBox(EMaskInfo* pValue);
void DrawColourInfoComboBox(EColourInfo* pValue);
void DrawCharacterSetComboBox(FCodeAnalysisState& state, FAddressRef& addr);

//void DrawCharacterMapViewer(FCodeAnalysisState& state, FCodeAnalysisViewState& viewState);

class FCharacterMapViewer : public FViewerBase
{
public:
	FCharacterMapViewer(FEmuBase* pEmu) : FViewerBase(pEmu) { Name = "Character Maps"; }

	bool	Init(void) override { return true; }
	void	Shutdown() override {}
	void	DrawUI(void) override;

	void	GoToAddress(FAddressRef addr);
private:
	void	DrawCharacterMapViewer(void);

	// Viewer setup
	FAddressRef	ViewerAddress;
	int			NumCharsX = 0;
	int			NumCharsY = 0;
	//FMemoryAccessGrid	ViewerGrid;
};