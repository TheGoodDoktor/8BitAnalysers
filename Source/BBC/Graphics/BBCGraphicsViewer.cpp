#include "BBCGraphicsViewer.h"
#include "../BBCEmulator.h"

#include <imgui.h>

FBBCGraphicsViewer::FBBCGraphicsViewer(FBBCEmulator* pEmu)
	: FGraphicsViewer(pEmu)
{
}

bool FBBCGraphicsViewer::Init()
{
	FGraphicsViewer::Init();

	return true;
}

void FBBCGraphicsViewer::Shutdown()
{
	FGraphicsViewer::Shutdown();
}

void FBBCGraphicsViewer::DrawUI()
{
	FGraphicsViewer::DrawUI();
	/*
	if (ImGui::BeginTabBar("Graphics Tab Bar"))
	{
		if (ImGui::BeginTabItem("GFX"))
		{
			DrawCharacterGraphicsViewer();
			ImGui::EndTabItem();
		}

		if (ImGui::BeginTabItem("Screen"))
		{
			DrawScreenViewer();
			ImGui::EndTabItem();
		}

		ImGui::EndTabBar();
	}*/
}

void FBBCGraphicsViewer::DrawScreenViewer()
{
	ImGui::Text("TODO: Screen Viewer");
}

