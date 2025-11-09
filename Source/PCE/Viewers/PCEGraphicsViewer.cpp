#include "PCEGraphicsViewer.h"


#include <Util/GraphicsView.h>
#include <CodeAnalyser/CodeAnalyser.h>
#include "CodeAnalyser/UI/CodeAnalyserUI.h"
#include <ImGuiSupport/ImGuiScaling.h>
#include "PCEGraphicsView.h"

#include "../PCEEmu.h"

bool FPCEGraphicsViewer::Init()
{
	pPCEEmu = (FPCEEmu*)pEmulator;
	FGraphicsViewer::Init();

//#if 0
	// test view - REMOVE
	pTestPCEGraphicsView = new FPCEGraphicsView(pPCEEmu, 256, 256);
	pTestPCEGraphicsView->Clear(0xfffff00);
//#endif

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
	}
	ImGui::EndTabBar();
}

int gPosX = 16;
int gPosY = 16;
int gXSize = 2;
int gYSize = 2;
int gOffset = 0;
int gPalette = 0;
extern bool gPCEGraphicsViewMonoSprites;

void FPCEGraphicsViewer::DrawScreenViewer()
{
	ImGui::SliderInt("x size (in 16x16 blocks)", &gXSize, 1, 256);
	ImGui::SliderInt("y size in (16x16 blocks)", &gYSize, 1, 256);
	ImGui::SliderInt("x", &gPosX, 0, 256 - 16);
	ImGui::SliderInt("y", &gPosY, 0, 256 - 16);
	ImGui::SliderInt("memory offset", &gOffset, 0, 0x2000);
	ImGui::InputInt("palette", &gPalette);
	ImGui::Checkbox("monochrome", &gPCEGraphicsViewMonoSprites);

//#if 0
	if (1)
	{
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
	}
//#endif
}
