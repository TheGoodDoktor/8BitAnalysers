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

	ImGui::Text("Application average %.3f ms/frame (%.1f FPS)", 1000.0f / ImGui::GetIO().Framerate, ImGui::GetIO().Framerate);

	bWindowFocused = ImGui::IsWindowHovered() || ImGui::IsWindowFocused();
}

void FPCEViewer::Tick()
{
	if (ImGui::IsKeyPressed(ImGuiKey_UpArrow, false))
	{
		if (bWindowFocused)
			pPCEEmu->GetCore()->KeyPressed(GG_CONTROLLER_1, GG_KEY_UP);
	}
	else if (ImGui::IsKeyReleased(ImGuiKey_UpArrow))
	{
		pPCEEmu->GetCore()->KeyReleased(GG_CONTROLLER_1, GG_KEY_UP);
	}

	if (ImGui::IsKeyPressed(ImGuiKey_DownArrow, false))
	{
		if (bWindowFocused)
			pPCEEmu->GetCore()->KeyPressed(GG_CONTROLLER_1, GG_KEY_DOWN);
	}
	else if (ImGui::IsKeyReleased(ImGuiKey_DownArrow))
	{
		pPCEEmu->GetCore()->KeyReleased(GG_CONTROLLER_1, GG_KEY_DOWN);
	}

	if (ImGui::IsKeyPressed(ImGuiKey_LeftArrow, false))
	{
		if (bWindowFocused)
			pPCEEmu->GetCore()->KeyPressed(GG_CONTROLLER_1, GG_KEY_LEFT);
	}
	else if (ImGui::IsKeyReleased(ImGuiKey_LeftArrow))
	{
		pPCEEmu->GetCore()->KeyReleased(GG_CONTROLLER_1, GG_KEY_LEFT);
	}

	if (ImGui::IsKeyPressed(ImGuiKey_RightArrow, false))
	{
		if (bWindowFocused)
			pPCEEmu->GetCore()->KeyPressed(GG_CONTROLLER_1, GG_KEY_RIGHT);
	}
	else if (ImGui::IsKeyReleased(ImGuiKey_RightArrow))
	{
		pPCEEmu->GetCore()->KeyReleased(GG_CONTROLLER_1, GG_KEY_RIGHT);
	}

	if (ImGui::IsKeyPressed(ImGuiKey_S, false))
	{
		if (bWindowFocused)
			pPCEEmu->GetCore()->KeyPressed(GG_CONTROLLER_1, GG_KEY_RUN);
	}
	else if (ImGui::IsKeyReleased(ImGuiKey_S))
	{
		pPCEEmu->GetCore()->KeyReleased(GG_CONTROLLER_1, GG_KEY_RUN);
	}

	if (ImGui::IsKeyPressed(ImGuiKey_A, false))
	{
		if (bWindowFocused)
			pPCEEmu->GetCore()->KeyPressed(GG_CONTROLLER_1, GG_KEY_SELECT);
	}
	else if (ImGui::IsKeyReleased(ImGuiKey_A))
	{
		pPCEEmu->GetCore()->KeyReleased(GG_CONTROLLER_1, GG_KEY_SELECT);
	}

	if (ImGui::IsKeyPressed(ImGuiKey_X, false))
	{
		if (bWindowFocused)
			pPCEEmu->GetCore()->KeyPressed(GG_CONTROLLER_1, GG_KEY_I);
	}
	else if (ImGui::IsKeyReleased(ImGuiKey_X))
	{
		pPCEEmu->GetCore()->KeyReleased(GG_CONTROLLER_1, GG_KEY_I);
	}

	if (ImGui::IsKeyPressed(ImGuiKey_Z, false))
	{
		if (bWindowFocused)
			pPCEEmu->GetCore()->KeyPressed(GG_CONTROLLER_1, GG_KEY_II);
	}
	else if (ImGui::IsKeyReleased(ImGuiKey_Z))
	{
		pPCEEmu->GetCore()->KeyReleased(GG_CONTROLLER_1, GG_KEY_II);
	}
}
