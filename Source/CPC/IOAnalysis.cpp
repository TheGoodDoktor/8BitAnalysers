#include "IOAnalysis.h"

#include "CodeAnalyser/UI/CodeAnalyserUI.h"
#include "CPCEmu.h"

#include <chips/z80.h>
#include "chips/am40010.h"

#include "imgui.h"

std::map< CpcIODevice, const char*> g_DeviceNames = 
{
	{CpcIODevice::Keyboard, "Keyboard"},
	{CpcIODevice::Joystick, "Joystick"},
	{CpcIODevice::CRTC, "CRTC"},
	{CpcIODevice::PaletteRegisterSelect, "Palette Select"},
	{CpcIODevice::PaletteWrite, "Palette Write"},
	{CpcIODevice::BorderColour, "BorderColour"},
#if SPECCY
	{SpeccyIODevice::MemoryBank, "Memory Bank Switch"},
	{SpeccyIODevice::SoundChip, "Sound Chip (AY)"},
#endif
	{CpcIODevice::Unknown, "Unknown"},
};

void FIOAnalysis::Init(FCpcEmu* pEmu)
{
	pCpcEmu = pEmu;
}

// todo get rid of the code duplication here
void FIOAnalysis::HandlePPI(uint64_t pins, CpcIODevice& readDevice, CpcIODevice& writeDevice)
{
  if (pins & I8255_RD)
  {
	 //if ((pins & (I8255_A0 | I8255_A1)) == 0) // think we were missing some keyboard reads because of this check
	 {
		i8255_t& ppi = pCpcEmu->CpcEmuState.ppi;
		if (!((ppi.control & I8255_CTRL_A) == I8255_CTRL_A_OUTPUT))
		{
		  uint64_t ay_pins = 0;
		  uint8_t ay_ctrl = ppi.pc.outp;
		  if (ay_ctrl & (1 << 7)) ay_pins |= AY38910_BDIR;
		  if (ay_ctrl & (1 << 6)) ay_pins |= AY38910_BC1;

		  if (ay_pins & (AY38910_BDIR | AY38910_BC1))
		  {
			 if (!(ay_pins & AY38910_BDIR))
			 {
				const ay38910_t* ay = &pCpcEmu->CpcEmuState.psg;
				if (ay->addr < AY38910_NUM_REGISTERS)
				{
				  if (ay->addr == AY38910_REG_IO_PORT_A)
				  {
					 if ((ay->enable & (1 << 6)) == 0)
					 {
						if (pCpcEmu->CpcEmuState.kbd.active_columns & (1 << 9))
						{
							readDevice = CpcIODevice::Joystick;
						}
						else
						{
							readDevice = CpcIODevice::Keyboard;

							RegisterEvent((uint8_t)EEventType::KeyboardRead, Z80_GET_ADDR(pins), AY38910_GET_DATA(pins));
						}
					 }
				  }
				}
			 }
		  }
		}
	 }
  }
  else if (pins & I8255_WR)
  {
	 if ((pins & (I8255_A0 | I8255_A1)) == I8255_A1)
	 {
		const uint8_t ay_ctrl = pCpcEmu->CpcEmuState.ppi.pc.outp & ((1 << 7) | (1 << 6));
		if (ay_ctrl)
		{
		  uint64_t ay_pins = 0;
		  if (ay_ctrl & (1 << 7)) { ay_pins |= AY38910_BDIR; }
		  if (ay_ctrl & (1 << 6)) { ay_pins |= AY38910_BC1; }
		  if (ay_pins & (AY38910_BDIR | AY38910_BC1))
		  {
			 if (!(ay_pins & AY38910_BDIR))
			 {
				const ay38910_t* ay = &pCpcEmu->CpcEmuState.psg;
				if (ay->addr < AY38910_NUM_REGISTERS)
				{
					if (ay->addr == AY38910_REG_IO_PORT_A)
					{
						if ((ay->enable & (1 << 6)) == 0)
						{
							writeDevice = CpcIODevice::Keyboard;

							// todo register event here?
						}
					}
				}
			 }
		  }
		}
	 }
  }
}

// Sam. I had to duplicate this table as it's declared as static in the chips code.
/* readable / writable per chip type and register (1: writable, 2 : readable, 3 : read / write) */
static uint8_t _mc6845_rw[MC6845_NUM_TYPES][0x20] = {
	 { 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 3, 3, 3, 3, 2, 2, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 },
	 { 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 3, 3, 2, 2, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 2 },
	 { 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 3, 3, 2, 2, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }
};

void FIOAnalysis::HandleCRTC(uint64_t pins, CpcIODevice& readDevice, CpcIODevice& writeDevice)
{
  if (pins & MC6845_CS) // chip select
  {
	 mc6845_t* c = &pCpcEmu->CpcEmuState.crtc;

	 // note: some of this logic for checking pins looks inverted but it's not.
	 // if the bit is _not_ set then the statement it's true.

	 /* register select (active: data register, inactive: address register) */
	 if (pins & MC6845_RS) 
	 {
		/* read/write register value */
		
		// get currently selected register
		int r = c->sel & 0x1F;

		if (pins & MC6845_RW)
		{
		  /* read register value (only if register is readable) */
		  uint8_t val = 0;
		  if (_mc6845_rw[c->type][r] & (1 << 1))
		  {
			 //readDevice = CpcIODevice::CRTC;
		  }
		  readDevice = CpcIODevice::CRTC;
		}
		else 
		{
		  /* write register value (only if register is writable) */
		  if (_mc6845_rw[c->type][r] & (1 << 0))
		  {
			 writeDevice = CpcIODevice::CRTC;

			 // think data should potentially be this, to limit number of bits per register?
			 // MC6845_GET_DATA(pins) & _mc6845_mask[r]
			 RegisterEvent((uint8_t)EEventType::CrtcRegisterWrite, r, Z80_GET_DATA(pins));
			 
			 if (r == 12 || r == 13)
			 {
				 // Add a bespoke event for changing the screen memory address, so we can see which address it got changed to
				 // when displayed in the event list.
				 RegisterEvent((uint8_t)EEventType::ScreenMemoryAddressChange, pCpcEmu->Screen.GetScreenAddrStart(), r);
			 }
		  }
		}
	 }
	 else 
	 {
		/* register address selected */
		/* read/write (active: write, inactive: read) */
		if (pins & MC6845_RW)
		{
		  // ???
		  // potentially crtc read 
		  readDevice = CpcIODevice::CRTC;
		}
		else
		{
		  /* write to address register */
		  writeDevice = CpcIODevice::CRTC;
		
		  RegisterEvent((uint8_t)EEventType::CrtcRegisterSelect , MC6845_GET_ADDR(pins), Z80_GET_DATA(pins));
		}
	 }
  }
}

#define AM40010_GET_DATA(p) ((uint8_t)(((p)&0xFF0000ULL)>>16))

void FIOAnalysis::HandleGateArray(uint64_t pins, CpcIODevice& readDevice, CpcIODevice& writeDevice)
{
	am40010_t& ga = pCpcEmu->CpcEmuState.ga;

	if ((pins & (AM40010_A14 | AM40010_A15)) == AM40010_A14) 
	{
		const uint8_t data = AM40010_GET_DATA(pins);
		
		// data bits 6 and 7 select the register type
		switch (data & ((1 << 7) | (1 << 6))) 
		{
			/* select color pen:
				bit 4 set means 'select border pen', otherwise
				one of the 16 ink pens
			*/
		case 0:
			writeDevice = CpcIODevice::PaletteRegisterSelect;

			RegisterEvent((uint8_t)EEventType::PaletteSelect, Z80_GET_ADDR(pins), data);
			break;

		case (1 << 6):
		{
			if (ga.regs.inksel & (1 << 4))
			{
				writeDevice = CpcIODevice::BorderColour;

				RegisterEvent((uint8_t)EEventType::BorderColour, Z80_GET_ADDR(pins), data);
			}
			else
			{
				writeDevice = CpcIODevice::PaletteWrite;

				RegisterEvent((uint8_t)EEventType::PaletteColour, Z80_GET_ADDR(pins), data);
			}
			break;
		}
			/* update the config register:
				- bits 0 and 1 select the video mode (updated at next HSYNC):
					00: 160x200 @ 16 colors
					01: 320x200 @ 4 colors
					10: 620x200 @ 2 colors
					11: 160x200 @ 2 colors (undocumented, currently not supported)
				- bit 2: LROMEN (lower ROM enable)
				- bit 3: HROMEN (upper ROM enable)
				- bit 4: IRQ_RESET (not a flip-flop, only a one-shot signal)
			*/
		case (1 << 7):
		{
			const int curScrMode = ga.regs.config & AM40010_CONFIG_MODE;
			if (curScrMode != CurScreenMode)
			{
				RegisterEvent((uint8_t)EEventType::ScreenModeChange, Z80_GET_ADDR(pins), curScrMode);
			}
			CurScreenMode = curScrMode;
		}
		break;

		// RAM bank switching (6128 only)
		case (1 << 6) | (1 << 7) :
			/*if (AM40010_CPC_TYPE_6128 == ga->cpc_type)
			{
				uint8_t ram_dirty = (ga->ram_config ^ data) & 7;
				ga->ram_config = data & 7;
				if (0 != ram_dirty) 
				{
					ga->bankswitch_cb(ga->ram_config, ga->regs.config, ga->rom_select, ga->user_data);
				}
			}*/
		break;

		}
	}

	// upper ROM bank select
	/*if ((pins & (AM40010_A13 | AM40010_WR)) == AM40010_WR)
	{
		const uint8_t data = _AM40010_GET_DATA(pins);
		bool rom_select_dirty = ga.rom_select != data;
		ga.rom_select = data;
		if (rom_select_dirty) 
		{
			ga.bankswitch_cb(ga.ram_config, ga.regs.config, ga.rom_select, ga.user_data);
		}
	}*/
}

void FIOAnalysis::RegisterEvent(uint8_t type, uint16_t address, uint8_t value)
{
	const uint16_t pc = pCpcEmu->CodeAnalysis.Debugger.GetPC().Address;
	const FAddressRef pcAddrRef = pCpcEmu->CodeAnalysis.AddressRefFromPhysicalAddress(pc);
	const uint16_t scanlinePos = pCpcEmu->CpcEmuState.ga.crt.v_pos;
	pCpcEmu->CodeAnalysis.Debugger.RegisterEvent(type, pcAddrRef, address, value, scanlinePos);
}

void FIOAnalysis::IOHandler(uint16_t pc, uint64_t pins)
{
	 const FAddressRef PCaddrRef = pCpcEmu->CodeAnalysis.AddressRefFromPhysicalAddress(pc);

	 CpcIODevice readDevice = CpcIODevice::None;
	 CpcIODevice writeDevice = CpcIODevice::None;

	 if ((pins & Z80_IORQ) && (pins & (Z80_RD | Z80_WR))) 
	 {
		 if ((pins & Z80_A11) == 0)
		 {
			// PPI chip in/out
			 uint64_t ppi_pins = (pins & Z80_PIN_MASK) | I8255_CS;
			 if (pins & Z80_A9) { ppi_pins |= I8255_A1; }
			 if (pins & Z80_A8) { ppi_pins |= I8255_A0; }
			 if (pins & Z80_RD) { ppi_pins |= I8255_RD; }
			 if (pins & Z80_WR) { ppi_pins |= I8255_WR; }

			 if (ppi_pins & I8255_CS)
			 {
				HandlePPI(ppi_pins, readDevice, writeDevice);
			 }
		 }

		 if ((pins & Z80_A14) == 0) 
		 {
			// CRTC (6845) in/out 
			uint64_t crtc_pins = (pins & Z80_PIN_MASK) | MC6845_CS;
			if (pins & Z80_A9) { crtc_pins |= MC6845_RW; }
			if (pins & Z80_A8) { crtc_pins |= MC6845_RS; }
			HandleCRTC(crtc_pins, readDevice, writeDevice);
		 }
	 }

	 HandleGateArray(pins, readDevice, writeDevice);

	 if (writeDevice != CpcIODevice::None)
	 {
		 FIOAccess& ioDevice = IODeviceAcceses[(int)writeDevice];
		 ioDevice.Writers.RegisterAccess(PCaddrRef);
		 ioDevice.WriteCount++;
		 ioDevice.FrameReadCount++;
	 }

	 if (readDevice != CpcIODevice::None)
	 {
		FIOAccess& ioDevice = IODeviceAcceses[(int)readDevice];
		ioDevice.Readers.RegisterAccess(PCaddrRef);
		ioDevice.ReadCount++;
		ioDevice.FrameReadCount++;
	 }
}

void FIOAnalysis::DrawUI()
{
	FCodeAnalysisState& state = pCpcEmu->CodeAnalysis;
	FCodeAnalysisViewState& viewState = state.GetFocussedViewState();
	ImGuiWindowFlags window_flags = ImGuiWindowFlags_HorizontalScrollbar;
	
	if (ImGui::Button("Reset"))
	{
	  Reset();
	}
	ImGui::Separator();

	ImGui::BeginChild("DrawIOAnalysisGUIChild1", ImVec2(ImGui::GetWindowContentRegionWidth() * 0.25f, 0), false, window_flags);
	FIOAccess *pSelectedIOAccess = nullptr;
	static CpcIODevice selectedDevice = CpcIODevice::None;

	for (int i = 0; i < (int)CpcIODevice::Count; i++)
	{
		FIOAccess &ioAccess = IODeviceAcceses[i];
		const CpcIODevice device = (CpcIODevice)i;

		const bool bSelected = (int)selectedDevice == i;

		if (ioAccess.FrameReadCount || ioAccess.FrameWriteCount)
		{
		  ImGui::PushStyleColor(ImGuiCol_Text, IM_COL32(255, 255, 0, 255));
		}

		if (ImGui::Selectable(g_DeviceNames[device], bSelected))
		{
			selectedDevice = device;
		}

		if (ioAccess.FrameReadCount || ioAccess.FrameWriteCount)
		{
		  ImGui::PopStyleColor();
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

		ImGui::Text("Readers");
		if (ioAccess.Readers.IsEmpty())
		{
		  ImGui::Text("   None");
		}
		else
		{
		  for (const auto& accessPC : ioAccess.Readers.GetReferences())
		  {
			 ImGui::PushID(accessPC.Val);
			 ShowCodeAccessorActivity(state, accessPC);
			 ImGui::Text("   ");
			 ImGui::SameLine();
			 DrawCodeAddress(state, viewState, accessPC);
			 ImGui::PopID();
		  }
		}

		ImGui::Text("Writers");
		if (ioAccess.Writers.IsEmpty())
		{
		  ImGui::Text("   None");
		}
		else
		{
		  for (const auto& accessPC : ioAccess.Writers.GetReferences())
		  {
			 ImGui::PushID(accessPC.Val);
			 ShowCodeAccessorActivity(state, accessPC);
			 ImGui::Text("   ");
			 ImGui::SameLine();
			 DrawCodeAddress(state, viewState, accessPC);
			 ImGui::PopID();
		  }
		}
	}

	if (!pCpcEmu->CodeAnalysis.Debugger.IsStopped())
	{
	  // reset for frame
	  for (int i = 0; i < (int)CpcIODevice::Count; i++)
	  {
		 IODeviceAcceses[i].FrameReadCount = 0;
		 IODeviceAcceses[i].FrameWriteCount = 0;
	  }
	}

	ImGui::EndChild();

}

void FIOAnalysis::Reset()
{
  for (int i = 0; i < (int)CpcIODevice::Count; i++)
  {
	 IODeviceAcceses->ReadCount = 0;
	 IODeviceAcceses->WriteCount = 0;
	 IODeviceAcceses[i].Readers.Reset();
	 IODeviceAcceses[i].Writers.Reset();
  }

  CurScreenMode = -1;
}
