#include "IOAnalysis.h"

#include "Speccy/Speccy.h"
#include "CodeAnalyser/CodeAnalyserUI.h"

#include "imgui.h"

void InitIOAnalysis(FIOAnalysisState &state)
{
	// setup
	{
		FIOAccess &keyboard = state.IODeviceAcceses[(int)SpeccyIODevice::Keyboard];
		keyboard.Name = "keyboard";
	}
	{
		FIOAccess &beeper = state.IODeviceAcceses[(int)SpeccyIODevice::Beeper];
		beeper.Name = "Beeper";
	}
	{
		FIOAccess &unknown = state.IODeviceAcceses[(int)SpeccyIODevice::Unknown];
		unknown.Name = "Unknown";
	}
}

SpeccyIODevice GetIODeviceFromIOAddress(uint16_t ioAddr, bool bWrite)
{
	if (bWrite == false)
	{
		switch (ioAddr)
		{
		case 0xfefe:	// row 0 (shift - V)
		case 0xfdfe:	// row 1 (A - G)
		case 0xfbfe:	// row 2 (Q - T)
		case 0xf7fe:	// row 3 (1 - 5)
		case 0xeffe:	// row 4 (0 - 6)
		case 0xdffe:	// row 5 (p - Y)
		case 0xbffe:	// row 6 (enter - H)
		case 0x7ffe:	// row 7 (space - B)
			return SpeccyIODevice::Keyboard;
		}
	}
	else
	{
		if ((ioAddr & 0xff) == 0xfe)
		{
			// TODO: separate beep & border etc
			return SpeccyIODevice::Beeper;
		}
	}
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
		FIOAccess &ioDevice = state.IODeviceAcceses[(int)device];
		FIOAccess &ioAccessMapItem = state.IOAccessMap[ioAddr];
		ioAccessMapItem.Callers[pc]++;
		ioDevice.Callers[pc]++;

		if (pins & Z80_RD)
		{
			// log IO read
			ioAccessMapItem.ReadCount++;
			ioDevice.ReadCount++;
		}

		if (pins & Z80_WR)
		{
			// log IO write
			ioAccessMapItem.WriteCount++;
			ioDevice.WriteCount++;
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
		const bool bSelected = (int)selectedDevice == i;

		if (ImGui::Selectable(ioAccess.Name.c_str(), bSelected))
		{
			selectedDevice = (SpeccyIODevice)i;
		}

		if(bSelected)
			pSelectedIOAccess = &ioAccess;

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

