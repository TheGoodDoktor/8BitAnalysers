#pragma once

#include <Misc/EmuBase.h>

struct FFunctionInfo;

class FFunctionViewer : public FViewerBase
{
public:
	FFunctionViewer(FEmuBase* pEmu) : FViewerBase(pEmu) { Name = "Function Viewer"; }

	bool	Init() override;
	void	Shutdown() override;
	void	DrawUI() override;

	void	DrawFunctionList();
	void	DrawFunctionDetails(FFunctionInfo* pFunctionInfo);
private:
	FAddressRef		SelectedFunction;
	bool			bOnlyShowVisitedFunctions = false;
};

