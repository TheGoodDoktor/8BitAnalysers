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
	pTestPCEGraphicsView = new FPCEGraphicsView(pPCEEmu, 128, 128);
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

void FPCEGraphicsViewer::DrawScreenViewer()
{
	ImGui::Text("todo");
//#if 0
	//const uint32_t* pPalette = pCPCEmu->Screen.GetCurrentPalette().GetData();
	if (1)
	{
		const uint16_t spriteAddress = 0x585f;
		const uint8_t* ptr = pPCEEmu->GetMemPtr(spriteAddress);
		//pTestGraphicsView->Draw4BppWideImageAt(ptr, 32, 32, 14, 21, pPalette);
		pTestPCEGraphicsView->Draw4bppSpriteImage(ptr, 16, 16, 32, 32);
		pTestPCEGraphicsView->Draw();
	}
//#endif
}
