#pragma once

#include "ViewerBase.h"
#include "Misc/EmuBase.h"
#include "../CodeAnalyserTypes.h"

class FCodeAnalysisState;
class FEmuBase;

// Separate viewer for Globals - put in its own file
class FGlobalsViewer : public FViewerBase
{
public:
	FGlobalsViewer(FEmuBase* pEmu) : FViewerBase(pEmu) { Name = "Globals"; }
	bool	Init(void) override;
	void	Shutdown(void) override;
	void	DrawUI() override;

	void	FixupAddressRefs() override;

	void	ToggleRebuild()
	{
		bRebuildFilteredGlobalDataItems = true;
		bRebuildFilteredGlobalFunctions = true;
	}
	void	Reset()
	{
		ShowROMLabels = false;
		FilterText.clear();
		DataTypeFilter = EDataTypeFilter::All;
		//GlobalDataItemsFilter.Reset();
		//GlobalFunctionsFilter.Reset();
		bRebuildFilteredGlobalDataItems = true;
		bRebuildFilteredGlobalFunctions = true;
		DataSortMode = EDataSortMode::Location;
		FunctionSortMode = EFunctionSortMode::Location;
	}
private:
	void	DrawGlobals();


	// for global Filters
	bool						ShowROMLabels = false;
	std::string					FilterText;
	EDataTypeFilter						DataTypeFilter = EDataTypeFilter::All;
	FLabelListFilter			GlobalDataItemsFilter;
	std::vector<FCodeAnalysisItem>	FilteredGlobalDataItems;
	bool										bRebuildFilteredGlobalDataItems = true;
	EDataSortMode				DataSortMode = EDataSortMode::Location;
	FLabelListFilter				GlobalFunctionsFilter;
	std::vector<FCodeAnalysisItem>	FilteredGlobalFunctions;
	bool										bRebuildFilteredGlobalFunctions = true;
	EFunctionSortMode				FunctionSortMode = EFunctionSortMode::Location;
};

