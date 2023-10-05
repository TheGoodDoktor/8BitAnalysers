#include "IOAnalyser.h"


#include "Util/Misc.h"

#include <imgui.h>

#include "CodeAnalyser.h"
#include "UI/CodeAnalyserUI.h"




void FIOAnalyser::Init(FCodeAnalysisState* ptrCodeAnalysis)
{
	pCodeAnalysis = ptrCodeAnalysis;
}

void FIOAnalyser::Shutdown()
{

}

void FIOAnalyser::RegisterIORead(FAddressRef pc, uint16_t IOAddress, uint8_t value)
{
	FIOAccessInfo& access = IOAccesses[IOAddress];
	access.IOAddress = IOAddress;
	access.ReadLocations.insert(pc);
}

void FIOAnalyser::RegisterIOWrite(FAddressRef pc, uint16_t IOAddress, uint8_t value)
{
	FIOAccessInfo& access = IOAccesses[IOAddress];
	access.IOAddress = IOAddress;
	access.WriteLocations.insert(pc);
}

void FIOAnalyser::FrameTick(void)
{

}

void FIOAnalyser::DrawUI(void)
{
	ImGuiWindowFlags windowFlags = ImGuiWindowFlags_HorizontalScrollbar;

	// List
	ImGui::BeginChild("IOAnalyserList", ImVec2(ImGui::GetWindowContentRegionWidth() * 0.25f, 0), false, windowFlags);

	for (const auto& read : IOAccesses)
	{
		const FIOAccessInfo& IOAccess = read.second;

		if (ImGui::Selectable(NumStr(IOAccess.IOAddress),SelectedIOAddress == IOAccess.IOAddress))
		{
			SelectedIOAddress = IOAccess.IOAddress;
		}
	}

	ImGui::EndChild();

	ImGui::SameLine();

	// Details
	ImGui::BeginChild("IOAnalyserDetails", ImVec2(0, 0), false, windowFlags);

	const auto ioDetailsIt = IOAccesses.find(SelectedIOAddress);

	if(ioDetailsIt != IOAccesses.end())
	{
		const FIOAccessInfo& IOAccess = ioDetailsIt->second;

		ImGui::Text("Reads:");
		for (const auto& read : IOAccess.ReadLocations)
		{
			DrawCodeAddress(*pCodeAnalysis,pCodeAnalysis->GetFocussedViewState(),read);
		}
		ImGui::Text("Writes:");
		for (const auto& write : IOAccess.WriteLocations)
		{
			DrawCodeAddress(*pCodeAnalysis, pCodeAnalysis->GetFocussedViewState(), write);
		}
	}
	ImGui::EndChild();

}