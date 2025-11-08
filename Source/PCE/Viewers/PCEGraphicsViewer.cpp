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

extern int gSegHeight;
extern int gStride;

int gPosX = 16;
int gPosY = 16;
int gSegCountX = 2;
int gSegCountY = 2;

void FPCEGraphicsViewer::DrawScreenViewer()
{
	ImGui::SliderInt("seg height", &gSegHeight, 8, 64);
	ImGui::SliderInt("seg count X", &gSegCountX, 8, 256);
	ImGui::SliderInt("seg count Y", &gSegCountY, 8, 256);
	int height = gSegHeight * gSegCountY;
	ImGui::SliderInt("x", &gPosX, 0, 256 - 16);
	ImGui::SliderInt("y", &gPosY, 0, 256 - height);
	ImGui::InputInt("stride", &gStride);
	ImGui::Text("height %d", height);
//#if 0
	//const uint32_t* pPalette = pCPCEmu->Screen.GetCurrentPalette().GetData();
	if (1)
	{
		const uint16_t spriteAddress = 0x585f;
		//const uint8_t* ptr = pPCEEmu->GetMemPtr(spriteAddress);
		// rabio lepus rom 17 offset 0x1200
		FCodeAnalysisBank* pBank = pPCEEmu->GetCodeAnalysis().GetBank(100);	
		if (pBank)
		{
			//const uint8_t* ptr = pBank->Memory + 0x1200; // rabbit.32x32 bank 100
			//const uint8_t* ptr = pBank->Memory + 0x1600; // bowtie. 16x16 bank 100
			const uint8_t* ptr = pBank->Memory + 0x1980; // circle. 16x16 bank 100
			pTestPCEGraphicsView->Clear(0xfffff00);
			pTestPCEGraphicsView->Draw4bppSpriteImage(ptr, gPosX, gPosY, 16, height);
			pTestPCEGraphicsView->Draw();
		}
	}
//#endif
}
