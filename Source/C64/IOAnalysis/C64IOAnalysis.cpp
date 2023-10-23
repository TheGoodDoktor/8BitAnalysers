#include "C64IOAnalysis.h"
#include "CodeAnalyser/CodeAnalyser.h"
#include <imgui.h>

void	FC64IOAnalysis::Init(FCodeAnalysisState* pAnalysis)
{
	VICAnalysis.Init(pAnalysis);
	SIDAnalysis.Init(pAnalysis);
	CIA1Analysis.Init(pAnalysis);
	CIA2Analysis.Init(pAnalysis);

	// TODO: set initial VIC register values
	for (int i = 0; i < 64; i++)
	{
		
	}
}

void	FC64IOAnalysis::Reset()
{
	VICAnalysis.Reset();
	SIDAnalysis.Reset();
	CIA1Analysis.Reset();
	CIA2Analysis.Reset();
}


void	FC64IOAnalysis::RegisterIORead(uint16_t addr, FAddressRef pc)
{

}

void	FC64IOAnalysis::RegisterIOWrite(uint16_t addr, uint8_t val, FAddressRef pc)
{
	// VIC D000 - D3FFF
	if (addr >= 0xd000 && addr < 0xd400)
		VICAnalysis.OnRegisterWrite(addr & 0x3f, val, pc);
	// SID D400 - D7FFF
	if (addr >= 0xd400 && addr < 0xd800)
		SIDAnalysis.OnRegisterWrite(addr & 0x1f, val, pc);
	// CIA 1
	if (addr >= 0xdc00 && addr < 0xdd00)
		CIA1Analysis.OnRegisterWrite(addr & 0xf, val, pc);
	// CIA 2
	if (addr >= 0xdd00 && addr < 0xde00)
		CIA2Analysis.OnRegisterWrite(addr & 0xf, val, pc);
}



void	FC64IOAnalysis::DrawIOAnalysisUI(void)
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