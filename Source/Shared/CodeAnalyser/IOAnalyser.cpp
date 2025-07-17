#include "IOAnalyser.h"


#include "Util/Misc.h"

#include <imgui.h>

#include "CodeAnalyser.h"
#include "UI/CodeAnalyserUI.h"

#if 0
#define CHIPS_UI_IMPL
#include <ui/ui_util.h>
#include <ui/ui_chip.h>
#include <ui/ui_ay38910.h>
#endif


// IO Analyser

void FIOAnalyser::Init(FCodeAnalysisState* ptrCodeAnalysis)
{
	pCodeAnalysis = ptrCodeAnalysis;
}

void FIOAnalyser::Shutdown()
{

}

void FIOAnalyser::RegisterIORead(FAddressRef pc, uint16_t IOAddress, uint8_t value)
{
	//FIOAccessInfo& access = IOAccesses[IOAddress];
	//access.IOAddress = IOAddress;
	//access.ReadLocations.insert(pc);
}

void FIOAnalyser::RegisterIOWrite(FAddressRef pc, uint16_t IOAddress, uint8_t value)
{
	//FIOAccessInfo& access = IOAccesses[IOAddress];
	//access.IOAddress = IOAddress;
	//access.WriteLocations.insert(pc);
}

void FIOAnalyser::FrameTick(void)
{
	for (FIODevice* pDevice : Devices)
		pDevice->OnFrameTick();
}

void FIOAnalyser::OnMachineFrameStart(void)
{
	for (FIODevice* pDevice : Devices)
		pDevice->OnMachineFrameStart();
}

void FIOAnalyser::OnMachineFrameEnd(void)
{
	for (FIODevice* pDevice : Devices)
		pDevice->OnMachineFrameEnd();
}


void FIOAnalyser::DrawUI(void)
{
	ImGuiWindowFlags windowFlags = ImGuiWindowFlags_HorizontalScrollbar;

	// List
	ImGui::BeginChild("IOAnalyserList", ImVec2(ImGui::GetContentRegionAvail().x * 0.25f, 0), false, windowFlags);

	for (int i=0; i< Devices.size();i++)
	{
		FIODevice* pDevice = Devices[i];
		if (ImGui::Selectable(pDevice->GetName(),SelectedDeviceIndex == i))
		{
			SelectedDeviceIndex = i;
		}
	}

	ImGui::EndChild();

	ImGui::SameLine();

	// Details
	ImGui::BeginChild("IOAnalyserDetails", ImVec2(0, 0), false, windowFlags);

	//const auto ioDetailsIt = IOAccesses.find(SelectedIOAddress);

	if(SelectedDeviceIndex != -1)
	{
		FIODevice* pDevice = Devices[SelectedDeviceIndex];
		pDevice->DrawDetailsUI();
		
	}
	ImGui::EndChild();

}

