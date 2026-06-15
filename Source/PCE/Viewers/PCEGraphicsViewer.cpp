#include "PCEGraphicsViewer.h"

#include <algorithm>
#include <Util/GraphicsView.h>
#include <CodeAnalyser/CodeAnalyser.h>
#include "CodeAnalyser/UI/CodeAnalyserUI.h"
#include <ImGuiSupport/ImGuiScaling.h>
#include <ImGuiSupport/ImGuiTexture.h>
#include "PCEGraphicsView.h"

#include "../PCEEmu.h"
#include <geargrafx_core.h>


const uint32_t* FPCEGraphicsViewer::GetCurrentPalette() const
{
	// Return first BG palette as a sensible fallback
	return GetPaletteFromPaletteNo(0);
}

bool FPCEGraphicsViewer::Init()
{
	pPCEEmu = (FPCEEmu*)pEmulator;
	FGraphicsViewer::Init();
	BitmapFormat = EBitmapFormat::Sprite4Bpp_PCE;

#ifndef NDEBUG
	// test view - REMOVE
	pTestPCEGraphicsView = new FPCEGraphicsView(pPCEEmu, 256, 256);
	pTestPCEGraphicsView->Clear(0xfffff00);
#endif
	return true;
}

void FPCEGraphicsViewer::DrawUI(void)
{
	if (ImGui::BeginTabBar("GraphicsViewTabBar"))
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

#ifndef NDEBUG
		if (ImGui::BeginTabItem("Test"))
		{
			DrawTest();
			ImGui::EndTabItem();
		}
#endif
	}
	ImGui::EndTabBar();
}

void FPCEGraphicsViewer::DrawScreenViewer()
{
	GeargrafxCore* pCore = pPCEEmu->GetCore();
	FCodeAnalysisState& codeAnalysis = GetCodeAnalysis();

	// Recreate texture if screen resolution changed
	GG_Runtime_Info info;
	pCore->GetRuntimeInfo(info);
	if (info.screen_width != TextureWidth || info.screen_height != TextureHeight)
	{
		TextureWidth  = info.screen_width;
		TextureHeight = info.screen_height;
		delete[] ScreenBuffer;
		ScreenBuffer = new uint8_t[TextureWidth * TextureHeight * 4]();
		if (ScreenTexture)
			ImGui_FreeTexture(ScreenTexture);
		ScreenTexture = ImGui_CreateTextureRGBA(ScreenBuffer, TextureWidth, TextureHeight);
	}

	if (TextureWidth == 0 || TextureHeight == 0)
		return;

	if (codeAnalysis.Debugger.IsStopped())
		pCore->RenderPartialFrame(ScreenBuffer);
	else
		memset(ScreenBuffer, 0, TextureWidth * TextureHeight * 4);

	ImGui_UpdateTextureRGBA(ScreenTexture, ScreenBuffer);

	FGlobalConfig* pConfig = codeAnalysis.pGlobalConfig;
	ImGui::InputInt("Scale", &pConfig->GfxScreenScale, 1, 1);
	pConfig->GfxScreenScale = MAX(1, pConfig->GfxScreenScale);
	const float scale = (float)pConfig->GfxScreenScale;

	ImGui::Image(ScreenTexture, ImVec2(TextureWidth * scale, TextureHeight * scale));
}

#ifndef NDEBUG
int gPosX = 16;
int gPosY = 16;
int gXSize = 2;
int gYSize = 2;
int gOffset = 0;
int gPalette = 0;
#endif

void FPCEGraphicsViewer::DrawTest()
{
#ifndef NDEBUG
	ImGui::SliderInt("x size (in 16x16 blocks)", &gXSize, 1, 256);
	ImGui::SliderInt("y size in (16x16 blocks)", &gYSize, 1, 256);
	ImGui::SliderInt("x", &gPosX, 0, 256 - 16);
	ImGui::SliderInt("y", &gPosY, 0, 256 - 16);
	ImGui::SliderInt("memory offset", &gOffset, 0, 0x2000);
	ImGui::InputInt("palette", &gPalette);

	// rabio lepus rom 17 offset 0x1200
	FCodeAnalysisBank* pBank = pPCEEmu->GetCodeAnalysis().GetBank(100);
	if (pBank)
	{
		const uint8_t* ptr = pBank->Memory + 0x1200; // rabbit.32x32 bank 100
		//const uint8_t* ptr = pBank->Memory + 0x1100; // carrot.16x32 bank 100
		//const uint8_t* ptr = pBank->Memory + 0x1600; // bowtie. 16x16 bank 100
		//const uint8_t* ptr = pBank->Memory + 0x1980; // circle. 16x16 bank 100
		//const uint8_t* ptr = pBank->Memory + gOffset; // bowtie. 16x16 bank 100
		pTestPCEGraphicsView->Clear(0xfffff00);
		pTestPCEGraphicsView->Draw4bppSpriteImage(ptr, gPosX, gPosY, gXSize, gYSize, gPalette);
		pTestPCEGraphicsView->Draw();
	}
#endif // !NDEBUG
}