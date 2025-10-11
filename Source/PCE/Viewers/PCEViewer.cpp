#include "PCEViewer.h"

#include <imgui.h>
#include "../PCEEmu.h"
#include <geargrafx_core.h>

#include "SpriteViewer.h"
#include <CodeAnalyser/UI/UIColours.h>

#include <ImGuiSupport/ImGuiTexture.h>
#include <ImGuiSupport/ImGuiDrawing.h>

FPCEViewer::FPCEViewer(FEmuBase* pEmu)
: FViewerBase(pEmu) 
{ 
	Name = "PCE Viewer";
	pPCEEmu = static_cast<FPCEEmu*>(pEmu);
}

bool FPCEViewer::Init()
{
	ResetScreenTexture();

	return true;
}


void FPCEViewer::DrawUI()
{		
	ResetScreenTexture();

	const FGlobalConfig* pConfig = pPCEEmu->GetGlobalConfig();
	const float scale = (float)pConfig->ImageScale;
	FCodeAnalysisState& state = pPCEEmu->GetCodeAnalysis();

	ImGui_UpdateTextureRGBA(ScreenTexture, pPCEEmu->GetFrameBuffer());
	ImGui::Text("Screen Size = %d x %d", TextureWidth, TextureHeight);

	const ImVec2 pos = ImGui::GetCursorScreenPos();

	ImGui::Image(ScreenTexture, ImVec2((float)TextureWidth * scale, (float)TextureHeight * scale));

	// Draw an indicator to show which scanline is being drawn
	// I couldn't get this working properly.
	/*if (pConfig->bShowScanLineIndicator && state.Debugger.IsStopped())
	{
		ImDrawList* dl = ImGui::GetWindowDrawList();

		const int scanlineY = MIN(MAX(pPCEEmu->GetCore()->GetHuC6270_1()->m_raster_line, 0), TextureHeight);
		
		dl->AddLine(ImVec2(pos.x + (4 * scale), pos.y + (scanlineY * scale)), ImVec2(pos.x + (TextureWidth - 8) * scale, pos.y + (scanlineY * scale)), 0x50ffffff);
		DrawArrow(dl, ImVec2(pos.x - 2, pos.y + (scanlineY * scale) - 6), false);
		DrawArrow(dl, ImVec2(pos.x + (TextureWidth - 11) * scale, pos.y + (scanlineY * scale) - 6), true);
	}*/

	DrawHighlightedSprite(pos, scale);

	ImGui::Text("Application average %.3f ms/frame (%.1f FPS)", 1000.0f / ImGui::GetIO().Framerate, ImGui::GetIO().Framerate);

	bWindowFocused = ImGui::IsWindowHovered() || ImGui::IsWindowFocused();
}

void FPCEViewer::DrawHighlightedSprite(ImVec2 cursorPos, float scale)
{
	const int spriteIndex = pPCEEmu->GetSpriteViewer()->HighlightSprite;
	if (spriteIndex != -1)
	{
		GeargrafxCore* pCore = pPCEEmu->GetCore();
		HuC6270* pHuc6270 = pCore->GetHuC6270_1();
		u16* pSat = pHuc6270->GetSAT();
		const float screen_scale = scale;

		const u16 sprite_flags = pSat[(spriteIndex * 4) + 3] & 0xB98F;
		const int width = k_huc6270_sprite_width[(sprite_flags >> 8) & 0x01];
		const int height = k_huc6270_sprite_height[(sprite_flags >> 12) & 0x03];

		const int sprite_y = (pSat[spriteIndex * 4] & 0x03FF) + 3;
		const int sprite_x = pSat[(spriteIndex * 4) + 1] & 0x03FF;
		const u16 pattern = (pSat[(spriteIndex * 4) + 2] >> 1) & 0x03FF;

		const bool h_flip = (sprite_flags & 0x0800) != 0;
		const bool v_flip = (sprite_flags & 0x8000) != 0;

		const int palette = sprite_flags & 0x0F;
		const bool priority = (sprite_flags & 0x0080) != 0;

		const float real_x = (float)(sprite_x - 32);
		const float real_y = (float)(sprite_y - 64);

		float rectx_min = cursorPos.x + (real_x * screen_scale);
		float rectx_max = cursorPos.x + ((real_x + width) * screen_scale);
		float recty_min = cursorPos.y + (real_y * screen_scale);
		float recty_max = cursorPos.y + ((real_y + height) * screen_scale);

		rectx_min = fminf(fmaxf(rectx_min, cursorPos.x), cursorPos.x + (TextureWidth * screen_scale));
		rectx_max = fminf(fmaxf(rectx_max, cursorPos.x), cursorPos.x + (TextureWidth * screen_scale));
		recty_min = fminf(fmaxf(recty_min, cursorPos.y), cursorPos.y + (TextureHeight * screen_scale));
		recty_max = fminf(fmaxf(recty_max, cursorPos.y), cursorPos.y + (TextureHeight * screen_scale));

		ImDrawList* draw_list = ImGui::GetWindowDrawList();
		draw_list->AddRect(ImVec2(rectx_min, recty_min), ImVec2(rectx_max, recty_max), Colours::GetFlashColour(), 0.f, 0, 2.f);
	}
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

void FPCEViewer::ResetScreenTexture()
{
	GG_Runtime_Info info;
	pPCEEmu->GetCore()->GetRuntimeInfo(info);

	if (ScreenTexture && info.screen_height == TextureHeight && info.screen_width == TextureWidth)
		return;

	TextureWidth = info.screen_width;
	TextureHeight = info.screen_height;

	if (ScreenTexture != nullptr)
		ImGui_FreeTexture(ScreenTexture);
	
	ScreenTexture = ImGui_CreateTextureRGBA(pPCEEmu->GetFrameBuffer(), TextureWidth, TextureHeight);
}