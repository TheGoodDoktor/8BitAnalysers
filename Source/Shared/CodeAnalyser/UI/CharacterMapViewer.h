#pragma once

#include "../../Util/GraphicsView.h"
#include "Misc/EmuBase.h"

class FCharacterMapGrid;
struct FCodeAnalysisViewState;

void DrawMaskInfoComboBox(EMaskInfo* pValue);
void DrawColourInfoComboBox(EColourInfo* pValue);
void DrawCharacterSetComboBox(FCodeAnalysisState& state, FAddressRef& addr);

struct FCharacterMapViewerUIState
{
	FAddressRef				SelectedCharMapAddr;
	FCharMapCreateParams	Params;

	FAddressRef				SelectedCharAddress;
	int						SelectedCharX = -1;
	int						SelectedCharY = -1;

	int						OffsetX = 0;
	int						OffsetY = 0;

	float					Scale = 1.0f;
};

class FCharacterMapViewer : public FViewerBase
{
public:
	FCharacterMapViewer(FEmuBase* pEmu) : FViewerBase(pEmu) { Name = "Character Maps"; }

	bool	Init(void) override;
	void	Shutdown() override;
	void	DrawUI(void) override;

	void	GoToAddress(FAddressRef addr);

	void	SetGridSize(int x,int y);

	void	FixupAddressRefs();

private:
	void	DrawCharacterMapViewer(void);
	void	DrawCharacterSetViewer(void);
	void	DrawCharacterMaps(void);
	void	DrawCharacterMap(void);

	// Viewer setup
	FCharacterMapGrid*	ViewerGrid = nullptr;

	FAddressRef SelectedCharSetAddr;
	FCharSetCreateParams CharSetParams;
	FCharacterMapViewerUIState UIState;
};