#pragma once

#include "ViewerBase.h"
//#include "Misc/EmuBase.h"
#include "../CodeAnalyserTypes.h"

class FCodeAnalysisState;
class FEmuBase;

struct FLabelListFilter
{
	std::string		FilterText;
	uint16_t		MinAddress = 0x0000;
	uint16_t		MaxAddress = 0xffff;
	bool			bNoMachineRoms = true;
	EDataTypeFilter DataType = EDataTypeFilter::All;
};

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
		bShowROMLabels = false;
		FilterText.clear();
		DataTypeFilter = EDataTypeFilter::All;
		//GlobalDataItemsFilter.Reset();
		//GlobalFunctionsFilter.Reset();
		bRebuildFilteredGlobalDataItems = true;
		bRebuildFilteredGlobalFunctions = true;
		DataSortMode = EDataSortMode::Location;
		FunctionSortMode = EFunctionSortMode::Location;
	}

	void ShowROMLabels(bool bShow)
	{
		bShowROMLabels = bShow;
		GlobalFunctionsFilter.bNoMachineRoms = !bShowROMLabels;
		GlobalDataItemsFilter.bNoMachineRoms = !bShowROMLabels;
		bRebuildFilteredGlobalFunctions = true;
		bRebuildFilteredGlobalDataItems = true;
	}
private:
	void	DrawGlobals();


	// for global Filters
	bool						bShowROMLabels = false;
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

