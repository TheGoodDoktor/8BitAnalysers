#pragma once

#include "CodeAnalyser/UI/ViewerBase.h"
#include "imgui_club/imgui_memory_editor.h"

#include <string>
#include <vector>

class FPCEEmu;

class FMemoryViewer : public FViewerBase
{
public:
	FMemoryViewer(FEmuBase* pEmu);

	virtual bool Init() override;
	virtual void Shutdown() override {}
	virtual void DrawUI() override;

private:
	void BuildMemoryList();

	struct FMemViewRegion
	{
		std::string Name;
		void*       Data    = nullptr;
		size_t      Size    = 0;
		size_t      BaseAddr = 0;
	};

	FPCEEmu*                    pPCEEmu        = nullptr;
	MemoryEditor                Editor;
	std::vector<FMemViewRegion> Regions;
	int                         SelectedRegion = 0;
};
