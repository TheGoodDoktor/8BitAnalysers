#include "BBCViewer.h"

#include "Util/GraphicsView.h"
#include "../BBCEmu/BBCEmu.h"
#include "../BBCEmulator.h"
#include "Mode7Screen.h"
#include "BitmapScreen.h"

void FBBCDisplay::Init(FCodeAnalysisState* pAnalysis, FBBCEmulator* pEmu)
{
	pCodeAnalysis = pAnalysis;
	pBBCEmu = pEmu;

	pScreenView = new FGraphicsView(320, 256);
}

void FBBCDisplay::Tick()
{
	const bbc_t& bbc = pBBCEmu->GetBBC();
	const mc6845_t& crtc = bbc.crtc;

	const int widthChars = crtc.h_displayed;
	const int heightChars = crtc.v_displayed;
	const int characterHeight = bbc.video_ula.teletext ? 20 : crtc.max_scanline_addr + 1;
	const int characterWidth = bbc.video_ula.teletext ? 16 : 8;

	const int screenWidth = widthChars * characterWidth;
	const int screenHeight = heightChars * characterHeight;

	if (screenWidth != pScreenView->GetWidth() || screenHeight != pScreenView->GetHeight())
	{
		delete pScreenView;
		pScreenView = new FGraphicsView(screenWidth, screenHeight);
	}

	if (bbc.video_ula.teletext)
		DrawMode7ScreenToGraphicsView(pBBCEmu, pScreenView);
	else
		UpdateScreenPixelImage(pBBCEmu, pScreenView);

	for (int key = ImGuiKey_NamedKey_BEGIN; key < ImGuiKey_COUNT; key++)
	{
		if (ImGui::IsKeyPressed((ImGuiKey)key, false))
		{
			const int bbcKey = BBCKeyFromImGuiKey((ImGuiKey)key);
			if (bbcKey != 0 && bWindowFocused)
				bbc_key_down(&pBBCEmu->GetBBC(), bbcKey);
		}
		else if (ImGui::IsKeyReleased((ImGuiKey)key))
		{
			const int bbcKey = BBCKeyFromImGuiKey((ImGuiKey)key);
			if (bbcKey != 0)
				bbc_key_up(&pBBCEmu->GetBBC(), bbcKey);
		}
	}
}

void FBBCDisplay::DrawUI()
{
	pScreenView->Draw(320, 256);

	bWindowFocused = ImGui::IsWindowFocused();
}

void FBBCDisplay::OverlayHighlightAddress(ImVec2 pos)
{
}
