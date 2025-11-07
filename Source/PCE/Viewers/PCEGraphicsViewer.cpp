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

extern int gSegWidth;
extern int gSegHeight;

int gPosX = 16;
int gPosY = 16;
int gSegCountX = 2;
int gSegCountY = 2;

void FPCEGraphicsViewer::DrawScreenViewer()
{
	ImGui::SliderInt("seg width", &gSegWidth, 8, 64);
	ImGui::SliderInt("seg height", &gSegHeight, 8, 64);
	ImGui::SliderInt("seg count X", &gSegCountX, 8, 256);
	ImGui::SliderInt("seg count Y", &gSegCountY, 8, 256);
	int width = gSegWidth * gSegCountX;
	int height = gSegHeight * gSegCountY;
	ImGui::SliderInt("x", &gPosX, 0, 256 - width);
	ImGui::SliderInt("y", &gPosY, 0, 256 - height);
	ImGui::Text("width %d", width);
	ImGui::Text("height %d", height);
//#if 0
	//const uint32_t* pPalette = pCPCEmu->Screen.GetCurrentPalette().GetData();
	if (1)
	{
		const uint16_t spriteAddress = 0x585f;
		const uint8_t* ptr = pPCEEmu->GetMemPtr(spriteAddress);
		//pTestGraphicsView->Draw4BppWideImageAt(ptr, 32, 32, 14, 21, pPalette);
		pTestPCEGraphicsView->Clear(0xfffff00);
		pTestPCEGraphicsView->Draw4bppSpriteImage(ptr, gPosX, gPosY, width, height);
		pTestPCEGraphicsView->Draw();
	}
//#endif
}
