#include "PCEViewer.h"

#include <imgui.h>
#include "../PCEEmu.h"
#include <geargrafx_core.h>

#include <ImGuiSupport/ImGuiTexture.h>

FPCEViewer::FPCEViewer(FEmuBase* pEmu)
: FViewerBase(pEmu) 
{ 
	Name = "PCE Viewer";
	pPCEEmu = static_cast<FPCEEmu*>(pEmu);
}

bool FPCEViewer::Init()
{
	GG_Runtime_Info info;
	pPCEEmu->GetCore()->GetRuntimeInfo(info);

	TextureWidth = info.screen_width;
	TextureHeight = info.screen_height;

	ScreenTexture = ImGui_CreateTextureRGBA(pPCEEmu->GetFrameBuffer(), TextureWidth, TextureHeight);

	return true;
}

void FPCEViewer::DrawUI()
{		
	ImGui_UpdateTextureRGBA(ScreenTexture, pPCEEmu->GetFrameBuffer());
    ImGui::Text("Screen Size = %d x %d", TextureWidth, TextureHeight);

	ImGui::Image(ScreenTexture, ImVec2((float)TextureWidth/* * scale*/, (float)TextureHeight/* * scale*/));

	//ImGui::Text("Application average %.3f ms/frame (%.1f FPS)", 1000.0f / ImGui::GetIO().Framerate, ImGui::GetIO().Framerate);

	bWindowFocused = ImGui::IsWindowHovered() || ImGui::IsWindowFocused();
}
