#include "IOAnalysis.h"

#include "Speccy/Speccy.h"
#include "CodeAnalyser/CodeAnalyserUI.h"

#include "imgui.h"

void InitIOAnalysis(FIOAnalysisState &state)
{
	// setup
	{
		FIOAccess &keybaord = state.IODeviceAcceses[(int)SpeccyIODevice::Keyboard];
	}
}

SpeccyIODevice GetIODeviceFromIOAddress(uint16_t ioAddr, bool bWrite)
{
	return SpeccyIODevice::Unknown;
}

void IOAnalysisHanler(FIOAnalysisState &state,uint16_t pc, uint64_t pins)
{
	// handle IO
	//todo generalise to specific devices
	if (pins & Z80_IORQ)
	{
		uint16_t ioAddr = Z80_GET_ADDR(pins);
		const SpeccyIODevice device = GetIODeviceFromIOAddress(ioAddr, !!(pins & Z80_WR));
		FIOAccess &ioAccess = state.IOAccessMap[ioAddr];
		ioAccess.Callers[pc]++;

		if (pins & Z80_RD)
		{
			// log IO read
			ioAccess.ReadCount++;
		}

		if (pins & Z80_WR)
		{
			// log IO write
			ioAccess.WriteCount++;
		}
	}
}

void DrawIOAnalysis(FIOAnalysisState &state)
{

	ImGuiWindowFlags window_flags = ImGuiWindowFlags_HorizontalScrollbar;
	ImGui::BeginChild("DrawIOAnalysisGUIChild1", ImVec2(ImGui::GetWindowContentRegionWidth() * 0.25f, 0), false, window_flags);
	FIOAccess *pSelectedIOAccess = nullptr;
	static SpeccyIODevice selectedDevice = SpeccyIODevice::None;

	for (int i = 0; i < (int)SpeccyIODevice::Count; i++)
	{
		FIOAccess &ioAccess = state.IODeviceAcceses[i];

		//ImGui::Text("%04Xh - %d reads, %d writes", ioAccess.Address, ioAccess.ReadCount, ioAccess.WriteCount);

		if (ImGui::Selectable(ioAccess.Name.c_str(), (int)selectedDevice == i))
		{
			selectedDevice = (SpeccyIODevice)i;
			pSelectedIOAccess = &ioAccess;
		}

	}
	ImGui::EndChild();

	ImGui::SameLine();

	// Handler details
	ImGui::BeginChild("DrawIOAnalysisGUIChild2", ImVec2(0, 0), false, window_flags);
	if (pSelectedIOAccess != nullptr)
	{
		FIOAccess &ioAccess = *pSelectedIOAccess;

		ImGui::Text("Reads %d", ioAccess.ReadCount);
		ImGui::Text("Writes %d", ioAccess.WriteCount);

		ImGui::Text("Callers");
		for (const auto &accessPC : ioAccess.Callers)
		{
			ImGui::PushID(accessPC.first);
			DrawCodeAddress(*state.pCodeAnalysis, accessPC.first);
			ImGui::SameLine();
			ImGui::Text(" - %d accesses", accessPC.second);
			ImGui::PopID();
		}
	}

	ImGui::EndChild();

}

