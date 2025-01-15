#include "BBCIOAnalysis.h"
#include "CodeAnalyser/CodeAnalyser.h"
#include <imgui.h>

void	FBBCIOAnalysis::Init(FBBCEmulator* pEmulator)
{
	FredAnalysis.Init(pEmulator);
}

void	FBBCIOAnalysis::Reset()
{
	FredAnalysis.Reset();
}


void	FBBCIOAnalysis::RegisterIORead(uint16_t addr, FAddressRef pc)
{
	const uint8_t page = addr >> 8;
	const uint8_t reg = addr & 0xff;

	if (page == kFredPage)
	{
		FredAnalysis.OnRegisterRead(reg, pc);
	}
	else if (page == kJimPage)
	{
		JimAnalysis.OnRegisterRead(reg, pc);
	}
	else if (page == kSheilaPage)
	{
		SheilaAnalysis.OnRegisterRead(reg, pc);
	}
}

void	FBBCIOAnalysis::RegisterIOWrite(uint16_t addr, uint8_t val, FAddressRef pc)
{
	const uint8_t page = addr >> 8;
	const uint8_t reg = addr & 0xff;

	if (page == kFredPage)
	{
		FredAnalysis.OnRegisterWrite(reg, val, pc);
	}
	else if (page == kJimPage)
	{
		JimAnalysis.OnRegisterWrite(reg, val, pc);
	}
	else if (page == kSheilaPage)
	{
		SheilaAnalysis.OnRegisterWrite(reg, val, pc);
	}
}



void	FBBCIOAnalysis::DrawIOAnalysisUI(void)
{
#if 0
	if (ImGui::BeginTabBar("IO Tab Bar"))
	{
		if (ImGui::BeginTabItem("VIC"))
		{
			//VICAnalysis.DrawUI();
			ImGui::EndTabItem();
		}

		if (ImGui::BeginTabItem("SID"))
		{
			//SIDAnalysis.DrawUI();
			ImGui::EndTabItem();
		}

		if (ImGui::BeginTabItem("CIA1"))
		{
			//CIA1Analysis.DrawUI();
			ImGui::EndTabItem();
		}

		if (ImGui::BeginTabItem("CIA2"))
		{
			//CIA2Analysis.DrawUI();
			ImGui::EndTabItem();
		}

		ImGui::EndTabBar();
	}
#endif
}