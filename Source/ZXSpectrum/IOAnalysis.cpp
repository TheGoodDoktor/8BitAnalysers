#include "IOAnalysis.h"

#include "CodeAnalyser/UI/CodeAnalyserUI.h"
#include "SpectrumEmu.h"

#include <chips/z80.h>
#include "imgui.h"

std::map< SpeccyIODevice, const char*> g_DeviceNames = 
{
	{SpeccyIODevice::Keyboard, "Keyboard"},
	{SpeccyIODevice::Ear, "Ear"},
	{SpeccyIODevice::Mic, "Mic"},
	{SpeccyIODevice::Beeper, "Beeper"},
	{SpeccyIODevice::BorderColour, "Border Colour"},
	{SpeccyIODevice::KempstonJoystick, "Kempston Joystick"},
	{SpeccyIODevice::FloatingBus, "Floating Bus"},	
	{SpeccyIODevice::MemoryBank, "Memory Bank Switch"},
	{SpeccyIODevice::SoundChip, "Sound Chip (AY)"},
	{SpeccyIODevice::Unknown, "Unknown"},
};

void	FIOAnalysis::Init(FSpectrumEmu* pEmu)
{
	pSpectrumEmu = pEmu;
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

void FIOAnalysis::IOHandler(uint16_t pc, uint64_t pins)
{
	 const FAddressRef PCaddrRef = pSpectrumEmu->CodeAnalysis.AddressRefFromPhysicalAddress(pc);

	// handle IO
	//todo generalise to specific devices
	if (pins & Z80_IORQ)
	{
		// WIP based on above
		if (pins & Z80_RD)
		{
			SpeccyIODevice readDevice = SpeccyIODevice::None;

			// Spectrum ULA (...............0)
			//	Bits 5 and 7 as read by INning from Port 0xfe are always one
			if ((pins & Z80_A0) == 0)
				readDevice = SpeccyIODevice::Keyboard;
			else if ((pins & (Z80_A7 | Z80_A6 | Z80_A5)) == 0) // Kempston Joystick (........000.....)
				readDevice = SpeccyIODevice::KempstonJoystick;
			else if (pins & 0xff)
				readDevice = SpeccyIODevice::FloatingBus;	// Joffa's Floating Bus
			// 128K specific
			else if(pSpectrumEmu->ZXEmuState.type == ZX_TYPE_128)
			{
				if ((pins & (Z80_A15 | Z80_A14 | Z80_A1)) == (Z80_A15 | Z80_A14))
					readDevice = SpeccyIODevice::SoundChip;
			}

			if (readDevice != SpeccyIODevice::None)
			{
				FIOAccess& ioDevice = IODeviceAcceses[(int)readDevice];
				ioDevice.Callers.RegisterAccess(PCaddrRef);
				ioDevice.ReadCount++;
				ioDevice.FrameReadCount++;
			}
		}
		else if (pins & Z80_WR)
		{
			// an IO write
			SpeccyIODevice writeDevice = SpeccyIODevice::None;
			const uint8_t data = Z80_GET_DATA(pins);

			if ((pins & Z80_A0) == 0)
			{
				// Spectrum ULA (...............0)
				
				// has border colour changed?
				if((data & 7) != (LastFE & 7))
					writeDevice = SpeccyIODevice::BorderColour;

				// has beeper changed
				if((data & (1 << 4)) != (LastFE & (1 << 4)))
					writeDevice = SpeccyIODevice::Beeper;


				// has mic output changed
				if ((data & (1 << 3)) != (LastFE & (1 << 3)))
					writeDevice = SpeccyIODevice::Mic;
				
				LastFE = data;
			}
			// 128K specific
			else if (pSpectrumEmu->ZXEmuState.type == ZX_TYPE_128)
			{
				if ((pins & (Z80_A15 | Z80_A1)) == 0)
					writeDevice = SpeccyIODevice::MemoryBank;
				else if ((pins & (Z80_A15 | Z80_A14 | Z80_A1)) == (Z80_A15 | Z80_A14))	// select AY-3-8912 register (11............0.)
					writeDevice = SpeccyIODevice::SoundChip;
				else if ((pins & (Z80_A15 | Z80_A14 | Z80_A1)) == Z80_A15)	// write to AY-3-8912 (10............0.) 
					writeDevice = SpeccyIODevice::SoundChip;
			}

			if (writeDevice != SpeccyIODevice::None)
			{
				FIOAccess& ioDevice = IODeviceAcceses[(int)writeDevice];
				ioDevice.Callers.RegisterAccess(PCaddrRef);
				ioDevice.WriteCount++;
				ioDevice.FrameReadCount++;
			}

		}
	}
}

void FIOAnalysis::DrawUI()
{
	FCodeAnalysisState& state = pSpectrumEmu->CodeAnalysis;
	FCodeAnalysisViewState& viewState = state.GetFocussedViewState();
	ImGuiWindowFlags window_flags = ImGuiWindowFlags_HorizontalScrollbar;
	ImGui::BeginChild("DrawIOAnalysisGUIChild1", ImVec2(ImGui::GetWindowContentRegionWidth() * 0.25f, 0), false, window_flags);
	FIOAccess *pSelectedIOAccess = nullptr;
	static SpeccyIODevice selectedDevice = SpeccyIODevice::None;

	for (int i = 0; i < (int)SpeccyIODevice::Count; i++)
	{
		FIOAccess &ioAccess = IODeviceAcceses[i];
		const SpeccyIODevice device = (SpeccyIODevice)i;

		const bool bSelected = (int)selectedDevice == i;

		if (ImGui::Selectable(g_DeviceNames[device], bSelected))
		{
			selectedDevice = device;
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

		ImGui::Text("Reads %d (frame %d)", ioAccess.ReadCount, ioAccess.FrameReadCount);
		ImGui::Text("Writes %d (frame %d)", ioAccess.WriteCount, ioAccess.FrameWriteCount);

		ImGui::Text("Callers");
		for (const auto &accessPC : ioAccess.Callers.GetReferences())
		{
			ImGui::PushID(accessPC.Val);
			ShowCodeAccessorActivity(state, accessPC);
			ImGui::Text("   ");
			ImGui::SameLine();
			DrawCodeAddress(state, viewState, accessPC);
			ImGui::PopID();
		}
	}

	// reset for frame
	for (int i = 0; i < (int)SpeccyIODevice::Count; i++)
	{
		IODeviceAcceses[i].FrameReadCount = 0;
		IODeviceAcceses[i].FrameWriteCount = 0;
	}

	ImGui::EndChild();

}

void FIOAnalysis::Reset()
{
  for (int i = 0; i < (int)SpeccyIODevice::Count; i++)
  {
	 IODeviceAcceses[i].Callers.Reset();
  }
}

