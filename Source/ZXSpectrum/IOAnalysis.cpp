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
	{SpeccyIODevice::BorderColour, "BorderColour"},
	{SpeccyIODevice::KempstonJoystick, "KempstonJoystick"},
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
#if 0
 * if (pins & Z80_RD) {
             an IO read
                FIXME: reading from port xxFF should return 'current VRAM data'
            
if ((pins & Z80_A0) == 0) {
	/* Spectrum ULA (...............0)
		Bits 5 and 7 as read by INning from Port 0xfe are always one
	*/
	uint8_t data = (1 << 7) | (1 << 5);
	/* MIC/EAR flags -> bit 6 */
	if (sys->last_fe_out & (1 << 3 | 1 << 4)) {
		data |= (1 << 6);
	}
	/* keyboard matrix bits are encoded in the upper 8 bit of the port address */
	uint16_t column_mask = (~(Z80_GET_ADDR(pins) >> 8)) & 0x00FF;
	const uint16_t kbd_lines = kbd_test_lines(&sys->kbd, column_mask);
	data |= (~kbd_lines) & 0x1F;
	Z80_SET_DATA(pins, data);
}
else if ((pins & (Z80_A7 | Z80_A6 | Z80_A5)) == 0) {
	/* Kempston Joystick (........000.....) */
	Z80_SET_DATA(pins, sys->kbd_joymask | sys->joy_joymask);
}
else if (sys->type == ZX_TYPE_128) {
	/* read from AY-3-8912 (11............0.) */
	if ((pins & (Z80_A15 | Z80_A14 | Z80_A1)) == (Z80_A15 | Z80_A14)) {
		pins = ay38910_iorq(&sys->ay, AY38910_BC1 | pins) & Z80_PIN_MASK;
	}
}
		}
		else if (pins & Z80_WR) {
		// an IO write
		const uint8_t data = Z80_GET_DATA(pins);
		if ((pins & Z80_A0) == 0) {
			/* Spectrum ULA (...............0)
				"every even IO port addresses the ULA but to avoid
				problems with other I/O devices, only FE should be used"
				FIXME:
					bit 3: MIC output (CAS SAVE, 0=On, 1=Off)
			*/
			sys->border_color = _zx_palette[data & 7] & 0xFFD7D7D7;
			sys->last_fe_out = data;
			beeper_set(&sys->beeper, 0 != (data & (1 << 4)));
		}
		else if (sys->type == ZX_TYPE_128) {
			/* Spectrum 128 memory control (0.............0.)
				http://8bit.yarek.pl/computer/zx.128/
			*/
			if ((pins & (Z80_A15 | Z80_A1)) == 0) {
				if (!sys->memory_paging_disabled) {
					sys->last_mem_config = data;
					/* bit 3 defines the video scanout memory bank (5 or 7) */
					sys->display_ram_bank = (data & (1 << 3)) ? 7 : 5;
					/* only last memory bank is mappable */
					mem_map_ram(&sys->mem, 0, 0xC000, 0x4000, sys->ram[data & 0x7]);

					/* ROM0 or ROM1 */
					if (data & (1 << 4)) {
						/* bit 4 set: ROM1 */
						mem_map_rom(&sys->mem, 0, 0x0000, 0x4000, sys->rom[1]);
					}
					else {
						/* bit 4 clear: ROM0 */
						mem_map_rom(&sys->mem, 0, 0x0000, 0x4000, sys->rom[0]);
					}
				}
				if (data & (1 << 5)) {
					/* bit 5 prevents further changes to memory pages
						until computer is reset, this is used when switching
						to the 48k ROM
					*/
					sys->memory_paging_disabled = true;
				}
			}
			else if ((pins & (Z80_A15 | Z80_A14 | Z80_A1)) == (Z80_A15 | Z80_A14)) {
				/* select AY-3-8912 register (11............0.) */
				ay38910_iorq(&sys->ay, AY38910_BDIR | AY38910_BC1 | pins);
			}
			else if ((pins & (Z80_A15 | Z80_A14 | Z80_A1)) == Z80_A15) {
				/* write to AY-3-8912 (10............0.) */
				ay38910_iorq(&sys->ay, AY38910_BDIR | pins);
			}
		}
		}
 */

#endif

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

