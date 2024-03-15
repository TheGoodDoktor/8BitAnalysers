#include <cstdint>

#define CHIPS_UI_IMPL
#define SAVE_NEW_DIRS 1

#include <imgui.h>
#include "CPCEmu.h"

#include "CPCConfig.h"
#include "CPCGameConfig.h"
#include "Util/FileUtil.h"
#include "Util/GraphicsView.h"
#include "CodeAnalyser/UI/CodeAnalyserUI.h"
#include <CodeAnalyser/CodeAnalysisState.h>
#include <CodeAnalyser/AssemblerExport.h>
#include "CodeAnalyser/CodeAnalysisJson.h"
#include "CPCGameConfig.h"
#include "Debug/DebugLog.h"
#include "CPCChipsImpl.h"

#include "CodeAnalyser/UI/CharacterMapViewer.h"
#include "App.h"
#include "Viewers/CRTCViewer.h"
#include "CodeAnalyser/UI/OverviewViewer.h"

#include <sokol_audio.h>
#include "cpc-roms.h"

#include <ImGuiSupport/ImGuiTexture.h>

#include "LuaScripting/LuaSys.h"
#include "CPCLuaAPI.h"

//#define RUN_AHEAD_TO_GENERATE_SCREEN

const std::string kAppTitle = "CPC Analyser";
const char* kGlobalConfigFilename = "GlobalConfig.json";

void StoreRegisters_Z80(FCodeAnalysisState& state);

uint8_t	FCPCEmu::ReadByte(uint16_t address) const
{
	return mem_rd(const_cast<mem_t*>(&CPCEmuState.mem), address);
}

uint16_t FCPCEmu::ReadWord(uint16_t address) const 
{
	return ReadByte(address) | (ReadByte(address + 1) << 8);
}

const uint8_t* FCPCEmu::GetMemPtr(uint16_t address) const 
{
	const int bank = address >> 14;
	const int bankAddr = address & 0x3fff;

	return &CPCEmuState.ram[bank][bankAddr];
}

void FCPCEmu::WriteByte(uint16_t address, uint8_t value)
{
	mem_wr(&CPCEmuState.mem, address, value);
}

FAddressRef FCPCEmu::GetPC(void) 
{
	return CodeAnalysis.Debugger.GetPC();
} 

uint16_t FCPCEmu::GetSP(void)
{
	return CPCEmuState.cpu.sp;
}

void* FCPCEmu::GetCPUEmulator(void) const
{
	return (void*)&CPCEmuState.cpu;
}

class FScreenPixMemDescGenerator : public FMemoryRegionDescGenerator
{
public:
	FScreenPixMemDescGenerator(FCPCEmu* pEmu)
		: pCPCEmu(pEmu)
	{
		UpdateScreenMemoryLocation();
	}

	const char* GenerateAddressString(FAddressRef addr) override
	{
		if (const FCodeAnalysisBank* pBank = pCPCEmu->GetCodeAnalysis().GetBank(addr.BankId))
		{
			// ROM can't be screen memory
			if (pBank->bReadOnly)
				return nullptr;

			// todo: deal with screen mode? display both scr mode's x coords?
			int xp = 0, yp = 0;
			if (pCPCEmu->Screen.GetScreenAddressCoords(addr.Address, xp, yp))
				sprintf(DescStr, "Screen: %d,%d", xp, yp);
			else
				sprintf(DescStr, "Screen: ?,?");
			return DescStr;
		}
		return nullptr;
	}

	void UpdateScreenMemoryLocation()
	{
		RegionMin = pCPCEmu->Screen.GetScreenAddrStart();
		RegionMax = pCPCEmu->Screen.GetScreenAddrEnd();
	}
private:
	FCPCEmu* pCPCEmu = 0;
	char DescStr[32] = { 0 };
};

/* reboot callback */
static void boot_cb(cpc_t* sys, cpc_type_t type)
{
	cpc_desc_t desc = {}; // TODO
	cpc_init(sys, &desc);
}

void* gfx_create_texture(int w, int h)
{
	return ImGui_CreateTextureRGBA(nullptr, w, h);
}

void gfx_update_texture(void* h, void* data, int data_byte_size)
{
	ImGui_UpdateTextureRGBA(h, (unsigned char *)data);
}

void gfx_destroy_texture(void* h)
{
	
}

/* audio-streaming callback */
static void PushAudio(const float* samples, int num_samples, void* user_data)
{
	FCPCEmu* pEmu = (FCPCEmu*)user_data;
	if(pEmu->GetGlobalConfig()->bEnableAudio)
		saudio_push(samples, num_samples);
}

void	FCPCEmu::OnInstructionExecuted(int ticks, uint64_t pins)
{
	FCodeAnalysisState& state = CodeAnalysis;
	const uint16_t addr = Z80_GET_ADDR(pins);
	const bool bMemAccess = !!((pins & Z80_CTRL_PIN_MASK) & Z80_MREQ);
	const bool bWrite = (pins & Z80_CTRL_PIN_MASK) == (Z80_MREQ | Z80_WR);
	const uint16_t pc = pins & 0xffff;	// set PC to pc of instruction just executed

	RegisterCodeExecuted(state, pc, PreviousPC);
	MemoryHandlerTrapFunction(pc, ticks, pins, this);

#if ENABLE_CAPTURES
	FLabelInfo* pLabel = state.GetLabelForAddress(pc);
	if (pLabel != nullptr)
	{
		if (pLabel->LabelType == ELabelType::Function)
			trapId = kCaptureTrapId;
	}
#endif

	PreviousPC = pc;
}

// Note - you can't read the cpu vars during tick
// They are only written back at end of exec function
uint64_t FCPCEmu::Z80Tick(int num, uint64_t pins)
{
	FCodeAnalysisState& state = CodeAnalysis;
	FDebugger& debugger = CodeAnalysis.Debugger;

	// we have to pass data to the tick through an internal state struct because the z80_t struct only gets updated after an emulation exec period
	z80_t* pCPU = (z80_t*)state.CPUInterface->GetCPUEmulator();
	const uint16_t pc = GetPC().Address;

	Screen.Tick();

	const am40010_crt_t& crt = CPCEmuState.ga.crt;
	const uint16_t scanlinePos = crt.v_pos;
	static uint16_t lastScanlinePos = 0;

	if (lastScanlinePos != scanlinePos)
	{
		if (scanlinePos == 0)
		{
			debugger.OnMachineFrameStart();
		}
		if (scanlinePos == 311)
		{
			debugger.OnMachineFrameEnd();
		}
	}
	lastScanlinePos = scanlinePos;

	/* memory and IO requests */
	if (pins & Z80_MREQ)
	{
		const uint16_t addr = Z80_GET_ADDR(pins);
		const uint8_t value = Z80_GET_DATA(pins);
		if (pins & Z80_RD) 
		{
			if (false)
			{
				// todo interrupt handler?
			}
			else
			{
				if (state.bRegisterDataAccesses)
					RegisterDataRead(state, pc, addr);
			}
		}
		else if (pins & Z80_WR) 
		{
			if (state.bRegisterDataAccesses)
				RegisterDataWrite(state, pc, addr, value);
			const FAddressRef addrRef = state.AddressRefFromPhysicalAddress(addr);
			const FAddressRef pcAddrRef = state.AddressRefFromPhysicalAddress(pc);
			state.SetLastWriterForAddress(addr, pcAddrRef);

			// Log screen pixel writes
			if (addr >= Screen.GetScreenAddrStart() && addr <= Screen.GetScreenAddrEnd())
			{
				debugger.RegisterEvent((uint8_t)EEventType::ScreenPixWrite, pcAddrRef, addr, value, scanlinePos);
			}
		}
	}

	// Memory gets remapped here

	InstructionsTicks++;

	const bool bNewOp = z80_opdone(&CPCEmuState.cpu);

	if (bNewOp)
	{
		OnInstructionExecuted(InstructionsTicks, pins);
		InstructionsTicks = 0;
	}

	// sam. need to do this somewhere better. like when the screen ram address gets set 
	CodeAnalysis.MemoryAnalyser.SetScreenMemoryArea(Screen.GetScreenAddrStart(), Screen.GetScreenAddrEnd());
	pScreenMemDescGenerator->UpdateScreenMemoryLocation();

	//debugger.CPUTick(pins);
	CodeAnalysis.OnCPUTick(pins);

	const am40010_t& ga = CPCEmuState.ga;
	if (pins & Z80_IORQ)
	{
		// This is still needed because it deals with adding events to the event trace.
		IOAnalysis.IOHandler(pc, pins);

		// note: some of this code logic is duplicated in IOAnalysis.cpp in HandleGateArray
		if (pins & (Z80_RD | Z80_WR))
		{
			if ((pins & Z80_A13) == 0)
			{
				// ROM select. This will get called when an OUT $dfXX instruction happens
				int selectedRomSlot = Z80_GET_DATA(pins);

				if (bExternalROMSupport)
				{
					// Try to select the requested rom slot.
					// Rom slot will change if we could not select the slot. 
					selectedRomSlot = SelectUpperROM(selectedRomSlot);
				}

				if (selectedRomSlot != -1)
				{
					bool bDirty = selectedRomSlot != CurUpperROMSlot;
					if (bDirty)
					{
						UpdateBankMappings();
						
						if (bExternalROMSupport)
						{
							// Call the modified Chips bank switch code, in order to switch to the newly selected upper ROM.
							// Note: the bank switch CB will have already been called by the Chips code but we need to call it
							// again after selecting the upper ROM.
							// The alternative would have been to call SelectUpperROM() in the bank switch CB but that would mean
							// calling it more often than we need to.
						
							ChipsBankSwitchCB(ga.ram_config, ga.regs.config, ga.rom_select, ga.user_data);
						}
					}
					CurUpperROMSlot = selectedRomSlot;
				}

				const FAddressRef pcAddrRef = state.AddressRefFromPhysicalAddress(pc);
				debugger.RegisterEvent((uint8_t)EEventType::UpperROMSelect, pcAddrRef, Z80_GET_ADDR(pins), Z80_GET_DATA(pins), scanlinePos);
			}

			if ((pins & (AM40010_A14 | AM40010_A15)) == AM40010_A14)
			{
				const FAddressRef pcAddrRef = state.AddressRefFromPhysicalAddress(pc);
				const uint16_t addr = Z80_GET_ADDR(pins);
				const uint8_t data = Z80_GET_DATA(pins);

				/* data bits 6 and 7 select the register type */
				switch (data & ((1 << 7) | (1 << 6)))
				{
					case (1 << 7):
					{
						// ROM enable/disable.
						// This occurs when an OUT $7fXX instruction happens.
						const uint8_t ROMEnableDirty = (LastGateArrayConfig ^ ga.regs.config) & (AM40010_CONFIG_LROMEN | AM40010_CONFIG_HROMEN);
						LastGateArrayConfig = ga.regs.config;
						if (ROMEnableDirty != 0)
						{
							UpdateBankMappings();
							debugger.RegisterEvent((uint8_t)EEventType::ROMBankSwitch, pcAddrRef, addr, data, scanlinePos);
						}
					}
					break;

					/* RAM bank switching (6128 only) */
					case (1 << 6) | (1 << 7) :
					{
						if (CPCEmuState.type == CPC_TYPE_6128)
						{
							const uint8_t RAMConfigDirty = (LastGateArrayRAMConfig ^ CPCEmuState.ga.ram_config) & 7;
							LastGateArrayRAMConfig = CPCEmuState.ga.ram_config;
							if (RAMConfigDirty)
							{
								UpdateBankMappings();
								debugger.RegisterEvent((uint8_t)EEventType::RAMBankSwitch, pcAddrRef, addr, data, scanlinePos);
							}
						}
						break;
					}
				}
			}		
		}
	}

	return pins;
}

static uint64_t Z80TickThunk(int num, uint64_t pins, void* user_data)
{
	FCPCEmu* pEmu = (FCPCEmu*)user_data;
	return pEmu->Z80Tick(num, pins);
}

bool FCPCEmu::CanSelectUpperROM(uint8_t romSlot)
{
	// Check the currently selected rom bank contains memory. As the roms are specified in the global config file 
	// the user may have removed the rom file entry or the rom file may have failed to load.
	if (const FCodeAnalysisBank* pBank = GetCodeAnalysis().GetBank(UpperROMSlot[romSlot]))
	{
		if (pBank->Memory)
		{
			return true;
		}
		else
		{
			std::string error = "Currently selected upper ROM bank '" + pBank->Name + "' contains no memory.";

			if (const char* pROMName = GetCPCGlobalConfig()->GetUpperROMSlotName(romSlot))
			{
				if (pROMName[0] != 0)
					SetLastError("%s. ROM file '%s' may not have loaded", error.c_str(), pROMName);
				else
					SetLastError("%s. ROM file does not exist in upper rom slot %d", error.c_str(), romSlot);
			}

			return false;
		}
	}
	
	return false;
}

bool FCPCEmu::InitBankMappings()
{
	am40010_t& ga = CPCEmuState.ga;
	if (bExternalROMSupport)
	{
		if (!CanSelectUpperROM(ga.rom_select))
		{
			return false;
		}

		CurUpperROMSlot = ga.rom_select;	
	
		SelectUpperROM(CurUpperROMSlot);
	}

	UpdateBankMappings();

	if (bExternalROMSupport)
	{
		ChipsBankSwitchCB(ga.ram_config, ga.regs.config, ga.rom_select, ga.user_data);
	}
	return true;
}

// This is a copy of the _cpc_ram_config array from the Chips code 
static const int gCPCRAMConfig[8][4] =
{
	 { 0, 1, 2, 3 }, // 0
	 { 0, 1, 2, 7 }, // 1
	 { 4, 5, 6, 7 }, // 2
	 { 0, 3, 2, 7 }, // 3
	 { 0, 4, 2, 3 }, // 4
	 { 0, 5, 2, 3 }, // 5
	 { 0, 6, 2, 3 }, // 6
	 { 0, 7, 2, 3 }  // 7
};

void FCPCEmu::UpdateBankMappings()
{
	const uint8_t romEnable = CPCEmuState.ga.regs.config;
	uint8_t ramPreset = 0;
	int16_t upperRomBank = ROMBanks[EROMBank::BASIC];

	if (CPCEmuState.type == CPC_TYPE_6128)
	{
		ramPreset = CPCEmuState.ga.ram_config & 7;
		if (CPCEmuState.ga.rom_select == 7)
			upperRomBank = ROMBanks[EROMBank::AMSDOS];
	}

	const int bankIndex0 = gCPCRAMConfig[ramPreset][0];
	const int bankIndex1 = gCPCRAMConfig[ramPreset][1];
	const int bankIndex2 = gCPCRAMConfig[ramPreset][2];
	const int bankIndex3 = gCPCRAMConfig[ramPreset][3];

	if (romEnable & AM40010_CONFIG_LROMEN)
	{
		// The RAM is now read/write 
		SetRAMBank(0, bankIndex0, EBankAccess::Read);	// 0x0000 - 0x3fff
	}
	else
	{
		// ROM now shares the same address space as RAM.
		// Reads go to ROM and writes go to RAM. RAM behind ROM.
		CodeAnalysis.MapBank(ROMBanks[EROMBank::OS], 0, EBankAccess::Read);
	}

	SetRAMBank(1, bankIndex1, EBankAccess::ReadWrite);	// 0x4000 - 0x7fff
	SetRAMBank(2, bankIndex2, EBankAccess::ReadWrite);	// 0x8000 - 0xbfff

	if (romEnable & AM40010_CONFIG_HROMEN)
	{
		// The RAM is now read/write 
		SetRAMBank(3, bankIndex3, EBankAccess::Read);	// 0xc000 - 0xffff
	}
	else
	{
		if (bExternalROMSupport)
		{
			CodeAnalysis.MapBank(UpperROMSlot[CurUpperROMSlot], 48, EBankAccess::Read);
		}
		else
		{
			CodeAnalysis.MapBank(upperRomBank, 48, EBankAccess::Read);
		}
	}

	// sam. Do I need to do this here?
	CodeAnalysis.SetAllBanksDirty();
}

// Slot is physical 16K memory region (0-3) 
// Bank is a 16K CPC RAM bank (0-7)
void FCPCEmu::SetRAMBank(int slot, int bankNo, EBankAccess access)
{
	const int16_t bankId = RAMBanks[bankNo];
	
	const int startPage = slot * kNoBankPages;
	CodeAnalysis.MapBank(bankId, startPage, access);
	
	if (CPCEmuState.type == CPC_TYPE_6128)
		CodeAnalysis.SetBankPrimaryPage(bankId, slot * 16);

	CurRAMBank[slot] = bankId;
}

// callback function to save snapshot to a numbered slot
void UISnapshotSaveCB(size_t slot_index)
{
}

// callback function to load snapshot from numbered slot
bool UISnapshotLoadCB(size_t slot_index)
{
	return true;
}

void DebugCB(void* user_data, uint64_t pins)
{
	FCPCEmu* pEmu = (FCPCEmu*)user_data;
	pEmu->Z80Tick(0, pins);
}

/// keyboard / port LUT
static std::map<uint8_t, std::vector<std::string>> g_KeyPortLUT =
{
	{0x40, {"FDOT",		"ENTER",	"F3",		"F6",		"F9",		"CURDOWN",	"CURRIGHT",	"CURUP"}},
	{0x41, {"F0",		"F2",		"F1",		"F5",		"F8",		"F7",		"COPY",		"CURLEFT"}},
	{0x42, {"CONTROL",	"\\",		"SHIFT",	"F4",		"]",		"RETURN",	"[",		"CLR"}},
	{0x43, {".",		"/",		":",		";",		"P",		"@",		"-",		"^"}},
	{0x44, {",",		"M",		"K",		"L",		"I",		"O",		"9",		"0"}},
	{0x45, {"SPACE",	"N",		"J",		"H",		"Y",		"U",		"7",		"8"}},
	{0x46, {"V",		"B",		"F",		"G(J2F)",	"T(J2R)",	"R(J2L)",	"5(J2D)",	"6(J2U)"}},
	{0x47, {"X",		"C",		"D",		"S",		"W",		"E",		"3",		"4"}},
	{0x48, {"Z",		"CAPSLOCK",	"A",		"TAB",		"Q",		"ESC",		"2",		"1"}},
	{0x49, {"DEL",		"J1F3",		"J1F2",		"J11F",		"J1R",		"J1L",		"J1D",		"J1U"}},
};


// Event viewer address/value visualisers - move somewhere?
void IOPortEventShowAddress(FCodeAnalysisState& state, const FEvent& event)
{
	if (event.Type == (int)EEventType::KeyboardRead)
	{
		const auto& portRow = g_KeyPortLUT.find(event.Address&0xff);
		if (portRow != g_KeyPortLUT.end())
		{
			ImGui::Text("Row:");
			for (const auto& key : portRow->second)
			{
				ImGui::SameLine();
				ImGui::Text("%s", key.c_str());
			}
		}
		else
			ImGui::Text("Port: %s", NumStr(event.Address));
	}
	else
	{
		ImGui::Text("IO Port: %s", NumStr(event.Address));
	}
}

void UpperROMSelectShowValue(FCodeAnalysisState& state, const FEvent& event)
{
	ImDrawList* dl = ImGui::GetWindowDrawList();
	ImVec2 pos = ImGui::GetCursorScreenPos();
	const float rectSize = ImGui::GetTextLineHeight();
	const float textOffset = (rectSize / 2.0f) - (ImGui::CalcTextSize("0").x / 2.0f);

	char tmp[16];
	for (int i = 0; i < 4; i++)
	{
		dl->AddRect(ImVec2(pos.x, pos.y), ImVec2(pos.x + rectSize, pos.y + rectSize), 0xffffffff);
		if (i == 3)
		{
			snprintf(tmp, 16, "%x", event.Value);
			dl->AddText(ImVec2(pos.x + textOffset, pos.y), 0xffffffff, tmp);
		}
		pos.x += rectSize;
	}

	pos.x += rectSize / 2.f;
	snprintf(tmp, 16, "(%s)", NumStr(event.Value));
	dl->AddText(ImVec2(pos.x, pos.y), 0xffffffff, tmp);
}

void RAMBankSwitchShowValue(FCodeAnalysisState& state, const FEvent& event)
{
	ImDrawList* dl = ImGui::GetWindowDrawList();
	ImVec2 pos = ImGui::GetCursorScreenPos();
	const float rectSize = ImGui::GetTextLineHeight();
	const float textOffset = (rectSize / 2.0f) - (ImGui::CalcTextSize("0").x / 2.0f);

	const int ramPreset = event.Value & 0x7;
	int bankIndex[4];
	bankIndex[0] = gCPCRAMConfig[ramPreset][0];
	bankIndex[1] = gCPCRAMConfig[ramPreset][1];
	bankIndex[2] = gCPCRAMConfig[ramPreset][2];
	bankIndex[3] = gCPCRAMConfig[ramPreset][3];

	char tmp[16];
	for (int i = 0; i < 4; i++)
	{
		dl->AddRect(ImVec2(pos.x, pos.y), ImVec2(pos.x + rectSize, pos.y + rectSize), 0xffffffff);
		snprintf(tmp, 16, "%d", bankIndex[i]);
		dl->AddText(ImVec2(pos.x + textOffset, pos.y), 0xffffffff, tmp);
		pos.x += rectSize;
	}

	pos.x += rectSize / 2.f;
	snprintf(tmp, 16, "(%s)", NumStr(event.Value));
	dl->AddText(ImVec2(pos.x, pos.y), 0xffffffff, tmp);
}

void ROMBankSwitchShowValue(FCodeAnalysisState& state, const FEvent& event)
{
	ImDrawList* dl = ImGui::GetWindowDrawList();
	ImVec2 pos = ImGui::GetCursorScreenPos();
	const float rectSize = ImGui::GetTextLineHeight();
	const uint8_t romEnable = event.Value & 0x1F;

	// Some text to hover over (because the rects are not hoverable).
	ImGui::Text("        ");

	for (int i = 0; i < 4; i++)
	{
		if ((i == 0 && !(romEnable & AM40010_CONFIG_LROMEN)) || (i == 3 && !(romEnable & AM40010_CONFIG_HROMEN)))
		{
			dl->AddRectFilled(ImVec2(pos.x, pos.y), ImVec2(pos.x + rectSize, pos.y + rectSize), 0xffffffff);
		}

		dl->AddRect(ImVec2(pos.x, pos.y), ImVec2(pos.x + rectSize, pos.y + rectSize), 0xffffffff);
		pos.x += rectSize;
	}
	
	if (ImGui::IsItemHovered())
	{
		ImGui::BeginTooltip();

		ImGui::Text("Lower ROM: %s.", romEnable & AM40010_CONFIG_LROMEN ? "OFF" : "ON");
		ImGui::SameLine();
		ImGui::Text("Upper ROM: %s.", romEnable & AM40010_CONFIG_HROMEN ? "OFF" : "ON");

		ImGui::EndTooltip();
	}
}

void IOPortEventShowValue(FCodeAnalysisState& state, const FEvent& event)
{
	if (event.Type == (int)EEventType::KeyboardRead)
	{
		if (event.Value == 0xff)	// no key down
		{
			ImGui::Text("No Keys");
		}
		else
		{
			const auto& portRow = g_KeyPortLUT.find(event.Address&0xff);
			if (portRow != g_KeyPortLUT.end())
			{
				for (int i = 7; i >= 0; i--)
				{
					if ((event.Value & (1 << i)) == 0)
					{
						const char* pString = portRow->second[7 - i].c_str();
						ImGui::Text("%s", pString);
						ImGui::SameLine();
					}
				}
			}
		}
	}
	else
	{
		ImGui::Text("%s", NumStr(event.Value));
	}
}

const char* g_CRTCRegNames[18] =
{
"Horiz.Total",						
"Horiz. Displayed",
"Horiz. Sync Pos.",
"Sync Widths",
"Vert. Total",
"Vert. Total Adjust",
"Vert. Displayed",
"Vert. Sync Pos.",
"Interlace and Skew",
"Max Scan lines",
"Cursor Start",
"Cursor End",
"Disp. Start Addr. H",
"Disp. Start Addr. L",
"Cursor H",
"Cursor L",
"Light Pen H",
"Light Pen L",
};

void CRTCWriteEventShowAddress(FCodeAnalysisState& state, const FEvent& event)
{
	const int regIndex = event.Address & 0x1f;
	ImGui::Text("Port %s: R%d %s", NumStr(event.Address), regIndex, regIndex < 18 ? g_CRTCRegNames[regIndex] : "");
}

void CRTCWriteEventShowValue(FCodeAnalysisState& state, const FEvent& event)
{
	if (GetNumberDisplayMode() != ENumberDisplayMode::Decimal)
		ImGui::Text("%d (%s)", event.Value, NumStr(event.Value));
	else
		ImGui::Text("%d", event.Value);
}

void ScreenAddrChangeEventShowValue(FCodeAnalysisState& state, const FEvent& event)
{
	if (event.Value == 12)
		ImGui::Text("R12 (H)");
	else if (event.Value == 13)
		ImGui::Text("R13 (L)");
	else
		ImGui::Text("Unknown");
}

void PaletteEventShowValue(FCodeAnalysisState& state, const FEvent& event)
{
	if (event.Type == (uint8_t)EEventType::PaletteColour)
	{
		ImDrawList* dl = ImGui::GetWindowDrawList();
		ImVec2 pos = ImGui::GetCursorScreenPos();
		const float line_height = ImGui::GetTextLineHeight();
		const float rectSize = line_height;

		const uint8_t colAttr = event.Value;

		const bool bBright = !!(colAttr & (1 << 6));
		const uint32_t col = GetCPCColour(event.Value & 0x1f);

		const ImVec2 rectMin(pos.x, pos.y);
		const ImVec2 rectMax(pos.x + rectSize, pos.y + rectSize);
		dl->AddRectFilled(rectMin, rectMax, col);
		ImGui::Text("   %s", NumStr(event.Value));
	}
	else
	{
		ImGui::Text("%s", NumStr(event.Value));
	}
}

void ScreenModeShowValue(FCodeAnalysisState& state, const FEvent& event)
{
	ImGui::Text("Mode %d", event.Value);
}

bool FCPCEmu::Init(const FEmulatorLaunchConfig& launchConfig)
{
	FEmuBase::Init(launchConfig);

	FCPCLaunchConfig& cpcLaunchConfig = (FCPCLaunchConfig&)launchConfig;
#ifndef NDEBUG
	LOGINFO("Init CPCEmu...");
#endif

	const std::string memStr = cpcLaunchConfig.Model == ECPCModel::CPC_6128 ? " (CPC 6128)" : " (CPC 464)";
	SetWindowTitle((std::string(kAppTitle) + memStr).c_str());
	SetWindowIcon("CPCALogo.png");

	// Initialise the CPC emulator
	pGlobalConfig = new FCPCConfig();
	pGlobalConfig->Init();
	pGlobalConfig->Load(kGlobalConfigFilename);
	CodeAnalysis.SetGlobalConfig(pGlobalConfig);
	SetNumberDisplayMode(pGlobalConfig->NumberDisplayMode);

	// temp hack to stop it crashing with a null pointer exception.
	static const uint32_t ColourLUT[8] = { 0 };
	CodeAnalysis.Config.CharacterColourLUT = ColourLUT;

	// set supported bitmap format
	CodeAnalysis.Config.bSupportedBitmapTypes[(int)EBitmapFormat::Bitmap_1Bpp] = true;
	CodeAnalysis.Config.bSupportedBitmapTypes[(int)EBitmapFormat::ColMap2Bpp_CPC] = true;
	CodeAnalysis.Config.bSupportedBitmapTypes[(int)EBitmapFormat::ColMap4Bpp_CPC] = true;
	for (int i = 0; i < FCodeAnalysisState::kNoViewStates; i++)
	{
		CodeAnalysis.ViewState[i].CurBitmapFormat = EBitmapFormat::ColMap2Bpp_CPC;
	}

	// A class that deals with loading games.
	const FCPCConfig* pCPCConfig = GetCPCGlobalConfig();
	GameLoader.Init(this);
	GamesList.SetLoader(&GameLoader);
	if (cpcLaunchConfig.Model == ECPCModel::CPC_6128)
		GamesList.EnumerateGames(pCPCConfig->SnapshotFolder128.c_str());
	else
		GamesList.EnumerateGames(pGlobalConfig->SnapshotFolder.c_str());
	
#ifdef RUN_AHEAD_TO_GENERATE_SCREEN
	// Turn caching on for the game loader. This means that if the same game is loaded more than once
	// the second time will involve no disk i/o.
	// We use this so we can quickly restore the game state after running ahead to generate the frame buffer in StartGame().
	GameLoader.SetCachingEnabled(true);
#endif

	Screen.Init(this);

	cpc_type_t type = cpcLaunchConfig.Model == ECPCModel::CPC_6128 ? CPC_TYPE_6128 : CPC_TYPE_464;
	cpc_joystick_type_t joy_type = CPC_JOYSTICK_NONE;

	cpc_desc_t desc;
	memset(&desc, 0, sizeof(cpc_desc_t));
	desc.type = type;
	desc.joystick_type = joy_type;
	
	// audio
	desc.audio.callback.func = PushAudio;	// our audio callback
	desc.audio.callback.user_data = this;
	desc.audio.sample_rate = saudio_sample_rate();
	
	// roms
	desc.roms.cpc464.os.ptr = dump_cpc464_os_bin;
	desc.roms.cpc464.os.size= sizeof(dump_cpc464_os_bin);
	desc.roms.cpc464.basic.ptr = dump_cpc464_basic_bin;
	desc.roms.cpc464.basic.size = sizeof(dump_cpc464_basic_bin);
	desc.roms.cpc6128.os.ptr = dump_cpc6128_os_bin;
	desc.roms.cpc6128.os.size = sizeof(dump_cpc6128_os_bin);
	desc.roms.cpc6128.basic.ptr = dump_cpc6128_basic_bin;
	desc.roms.cpc6128.basic.size = sizeof(dump_cpc6128_basic_bin);
	desc.roms.cpc6128.amsdos.ptr = dump_cpc6128_amsdos_bin;
	desc.roms.cpc6128.amsdos.size = sizeof(dump_cpc6128_amsdos_bin);
	
	// setup debug hook
	desc.debug.callback.func = DebugCB;
	desc.debug.callback.user_data = this;
	desc.debug.stopped = CodeAnalysis.Debugger.GetDebuggerStoppedPtr();

	cpc_init(&CPCEmuState, &desc);

	if (type == CPC_TYPE_6128)
	{
		// disable upper rom support on 6128 for now as it's broken
		bExternalROMSupport = false;
	}
	if (bExternalROMSupport)
	{
		CPCEmuState.ga.bankswitch_cb = ChipsBankSwitchCB;
	}
	SetExternalROMSupportEnabled(bExternalROMSupport);

	// Clear UI
	
	memset(&UICPC, 0, sizeof(ui_cpc_t));

	{
		ui_cpc_desc_t desc = { 0 };
		desc.cpc = &CPCEmuState;
		desc.boot_cb = boot_cb;

		desc.dbg_texture.create_cb = gfx_create_texture;
		desc.dbg_texture.update_cb = gfx_update_texture;
		desc.dbg_texture.destroy_cb = gfx_destroy_texture;

		desc.dbg_keys.stop.keycode = ImGui::GetKeyIndex(ImGuiKey_Space);
		desc.dbg_keys.stop.name = "F5";
		desc.dbg_keys.cont.keycode = ImGui::GetKeyIndex(ImGuiKey_F5);
		desc.dbg_keys.cont.name = "F5";
		desc.dbg_keys.step_over.keycode = ImGui::GetKeyIndex(ImGuiKey_F6);
		desc.dbg_keys.step_over.name = "F6";
		desc.dbg_keys.step_into.keycode = ImGui::GetKeyIndex(ImGuiKey_F7);
		desc.dbg_keys.step_into.name = "F7";
		desc.dbg_keys.toggle_breakpoint.keycode = ImGui::GetKeyIndex(ImGuiKey_F9);
		desc.dbg_keys.toggle_breakpoint.name = "F9";

		desc.snapshot.load_cb = UISnapshotLoadCB;
		desc.snapshot.save_cb = UISnapshotSaveCB;

		ui_cpc_init(&UICPC, &desc);
	}


	// This is where we add the viewers we want
	AddViewer(new FCrtcViewer(this));
	AddViewer(new FOverviewViewer(this));
	AddViewer(new FCharacterMapViewer(this));
	pGraphicsViewer = new FCPCGraphicsViewer(this);
	AddViewer(pGraphicsViewer);

	IOAnalysis.Init(this);
	CPCViewer.Init(this);
	CodeAnalysis.Config.bShowBanks = true;
	CodeAnalysis.ViewState[0].Enabled = true;	// always have first view enabled

	// Setup memory description handlers
	pScreenMemDescGenerator = new FScreenPixMemDescGenerator(this);
	AddMemoryRegionDescGenerator(pScreenMemDescGenerator);

	LoadCPCGameConfigs(this);

	// Set up code analysis
	// initialise code analysis pages
	
	InitExternalROMs(pCPCConfig, CPCEmuState.type == CPC_TYPE_6128);
	
	// Low ROM 0x0000 - 0x3fff
	ROMBanks[EROMBank::OS] = CodeAnalysis.CreateBank("ROM OS", 16, CPCEmuState.rom_os, true, 0x0000);
	//CodeAnalysis.GetBank(ROMBanks[EROMBank::OS])->PrimaryMappedPage = 0;

	// High ROM AMSDOS 0xc000 - 0xffff
	ROMBanks[EROMBank::AMSDOS] = CodeAnalysis.CreateBank("ROM AMSDOS", 16, CPCEmuState.rom_amsdos, true, 0xC000);
	//CodeAnalysis.GetBank(ROMBanks[EROMBank::AMSDOS])->PrimaryMappedPage = 48;

	// High ROM BASIC 0xc000 - 0xffff
	ROMBanks[EROMBank::BASIC] = CodeAnalysis.CreateBank("ROM BASIC", 16, CPCEmuState.rom_basic, true, 0xC000);
	//CodeAnalysis.GetBank(ROMBanks[EROMBank::BASIC])->PrimaryMappedPage = 48;

	// create & register RAM banks
	for (int bankNo = 0; bankNo < kNoRAMBanks; bankNo++)
	{
		char bankName[32];
		sprintf(bankName, "RAM %d", bankNo);
		RAMBanks[bankNo] = CodeAnalysis.CreateBank(bankName, 16, CPCEmuState.ram[bankNo], false, 0x0000);
	}

	// Setup initial machine memory config
	if (cpcLaunchConfig.Model == ECPCModel::CPC_464)
	{
		CodeAnalysis.SetBankPrimaryPage(RAMBanks[0], 0);
		CodeAnalysis.SetBankPrimaryPage(RAMBanks[1], 16);
		CodeAnalysis.SetBankPrimaryPage(RAMBanks[2], 32);
		CodeAnalysis.SetBankPrimaryPage(RAMBanks[3], 48);

		SetRAMBank(0, 0, EBankAccess::ReadWrite);	// 0x0000 - 0x3fff
		SetRAMBank(1, 1, EBankAccess::ReadWrite);	// 0x4000 - 0x7fff
		SetRAMBank(2, 2, EBankAccess::ReadWrite);	// 0x8000 - 0xBfff
		SetRAMBank(3, 3, EBankAccess::ReadWrite);	// 0xc000 - 0xffff

		// We don't want these banks to show up in the Code Analysis view, so set primary mapped page to -1.
		CodeAnalysis.SetBankPrimaryPage(RAMBanks[4], -1);
		CodeAnalysis.SetBankPrimaryPage(RAMBanks[5], -1);
		CodeAnalysis.SetBankPrimaryPage(RAMBanks[6], -1);
		CodeAnalysis.SetBankPrimaryPage(RAMBanks[7], -1);
	}
	else
	{
		CodeAnalysis.SetBankPrimaryPage(RAMBanks[0], 0);
		CodeAnalysis.SetBankPrimaryPage(RAMBanks[1], 16);
		CodeAnalysis.SetBankPrimaryPage(RAMBanks[2], 32);
		CodeAnalysis.SetBankPrimaryPage(RAMBanks[3], 48);

		SetRAMBank(0, 0, EBankAccess::ReadWrite);	// 0x0000 - 0x3fff
		SetRAMBank(1, 1, EBankAccess::ReadWrite);	// 0x4000 - 0x7fff
		SetRAMBank(2, 2, EBankAccess::ReadWrite);	// 0x8000 - 0xBfff
		SetRAMBank(3, 3, EBankAccess::ReadWrite);	// 0xc000 - 0xffff
	}

	if (bExternalROMSupport)
	{
		// Setup upper ROM slots
		UpperROMSlot[0] = ROMBanks[EROMBank::BASIC];

		if (CPCEmuState.type == CPC_TYPE_6128)
			UpperROMSlot[7] = ROMBanks[EROMBank::AMSDOS];

		char romName[16];
		for (int i = 1; i < kNumUpperROMSlots; i++)
		{
			const uint8_t* pROMData = GetUpperROMSlot(i);
			sprintf(romName, "Upper ROM %d", i);
			UpperROMSlot[i] = CodeAnalysis.CreateBank(romName, 16, (uint8_t*)pROMData, true, 0xC000);
			if (!pROMData)
			{
				// If we don't have a rom loaded for this slot then set the primary mapped page to -1.
				// This prevents the bank being displayed in the Code Analysis view.
				CodeAnalysis.SetBankPrimaryPage(UpperROMSlot[i], -1);
			}
		}
	}

	FDebugger& debugger = CodeAnalysis.Debugger;
	debugger.RegisterEventType((int)EEventType::ScreenPixWrite, "Screen RAM Write", 0xff0000ff, nullptr, EventShowPixValue);
	debugger.RegisterEventType((int)EEventType::PaletteSelect, "Palette Select", 0xffffffff, IOPortEventShowAddress, PaletteEventShowValue);
	debugger.RegisterEventType((int)EEventType::PaletteColour, "Palette Colour", 0xff00ffff, IOPortEventShowAddress, PaletteEventShowValue);
	debugger.RegisterEventType((int)EEventType::BorderColour, "Border Colour", 0xff00ff00, IOPortEventShowAddress, PaletteEventShowValue);
	debugger.RegisterEventType((int)EEventType::ScreenModeChange, "Screen Mode", 0xffccccff, IOPortEventShowAddress, ScreenModeShowValue);
	debugger.RegisterEventType((int)EEventType::CrtcRegisterSelect, "CRTC Reg. Select", 0xffff00ff, CRTCWriteEventShowAddress, CRTCWriteEventShowValue);
	debugger.RegisterEventType((int)EEventType::CrtcRegisterRead, "CRTC Reg. Read", 0xffff0000, CRTCWriteEventShowAddress, CRTCWriteEventShowValue);
	debugger.RegisterEventType((int)EEventType::CrtcRegisterWrite, "CRTC Reg. Write", 0xffffff00, CRTCWriteEventShowAddress, CRTCWriteEventShowValue);
	debugger.RegisterEventType((int)EEventType::KeyboardRead, "Keyboard Read", 0xff808080, IOPortEventShowAddress, IOPortEventShowValue);
	debugger.RegisterEventType((int)EEventType::ScreenMemoryAddressChange, "Set Scr. Addr.", 0xffff69b4, nullptr, ScreenAddrChangeEventShowValue);
	debugger.RegisterEventType((int)EEventType::RAMBankSwitch, "RAM Banks Switch", 0xff006699, IOPortEventShowAddress, RAMBankSwitchShowValue);
	debugger.RegisterEventType((int)EEventType::ROMBankSwitch, "ROM Bank Switch", 0xff3357ff, IOPortEventShowAddress, ROMBankSwitchShowValue);
	debugger.RegisterEventType((int)EEventType::UpperROMSelect, "Upper ROM Select", 0xff3f0c90, IOPortEventShowAddress, UpperROMSelectShowValue);

	// load the command line game if none specified then load the last game
	bool bLoadedGame = false;

	if (launchConfig.SpecificGame.empty() == false)
	{
		bLoadedGame = StartGameFromName(launchConfig.SpecificGame.c_str(), true);
	}
	else if (pGlobalConfig->LastGame.empty() == false)
	{
		bLoadedGame = StartGameFromName(pGlobalConfig->LastGame.c_str(), true);
	}
	
	// Start ROM if no game has been loaded
	if (bLoadedGame == false)
	{
		CodeAnalysis.Init(this);
	}

	CodeAnalysis.MemoryAnalyser.SetScreenMemoryArea(Screen.GetScreenAddrStart(), Screen.GetScreenAddrEnd());

	bInitialised = true;

#ifndef NDEBUG
	LOGINFO("Init CPCEmu...Done");
#endif
	return true;
}

void FCPCEmu::Shutdown()
{
	FEmuBase::Shutdown();

	SaveCurrentGameData();	// save on close

	// Save Global Config - move to function?

	// todo: dont save active game if the game didn't load successfully
	if (pActiveGame != nullptr)
		pGlobalConfig->LastGame = pActiveGame->pConfig->Name;

	pGlobalConfig->NumberDisplayMode = GetNumberDisplayMode();
	pGlobalConfig->bShowOpcodeValues = CodeAnalysis.pGlobalConfig->bShowOpcodeValues;
	pGlobalConfig->BranchLinesDisplayMode = CodeAnalysis.pGlobalConfig->BranchLinesDisplayMode;

	pGlobalConfig->Save(kGlobalConfigFilename);

	//GraphicsViewer.Shutdown();
}

bool FCPCEmu::StartGame(FGameConfig* pGameConfig, bool bLoadGameData)
{
	FCPCGameConfig* pCPCGameConfig = (FCPCGameConfig*)pGameConfig;

#ifndef NDEBUG
	LOGINFO("Start game '%s'", pGameConfig->Name.c_str());
#endif

	// reset systems
	MemoryAccessHandlers.clear();	// remove old memory handlers
	ResetMemoryStats(MemStats);
	// sam todo
	//FrameTraceViewer.Reset();
	pGraphicsViewer->Reset();
	Screen.Reset();

	// Clear the cpc frame buffer with a single colour. Otherwise we may see the framebuffer from the previous game.
	const chips_display_info_t dispInfo = cpc_display_info(&CPCEmuState);
	memset(dispInfo.frame.buffer.ptr, 0, dispInfo.frame.buffer.size);
	
	const std::string memStr = CPCEmuState.type == CPC_TYPE_6128 ? " (CPC 6128)" : " (CPC 464)";
	const std::string windowTitle = kAppTitle + " - " + pGameConfig->Name + memStr;
	SetWindowTitle(windowTitle.c_str());

	// start up game
	delete pActiveGame;

	FGame* pNewGame = new FGame;
	pNewGame->pConfig = pGameConfig;
	pCPCGameConfig->bCPC6128Game = CPCEmuState.type == CPC_TYPE_6128;
	pActiveGame = pNewGame;

	//GenerateSpriteListsFromConfig(GraphicsViewer, pGameConfig);

	// Initialise code analysis
	CodeAnalysis.Init(this);
	
	IOAnalysis.Reset();

	// Set options from config
	for (int i = 0; i < FCodeAnalysisState::kNoViewStates; i++)
	{
		CodeAnalysis.ViewState[i].Enabled = pGameConfig->ViewConfigs[i].bEnabled;
		CodeAnalysis.ViewState[i].GoToAddress(pGameConfig->ViewConfigs[i].ViewAddress);
	}

	if (bLoadGameData)
	{
		const std::string root = pGlobalConfig->WorkspaceRoot;

		std::string analysisJsonFName = root + "AnalysisJson/" + pGameConfig->Name + ".json";
		std::string graphicsSetsJsonFName = root + "GraphicsSets/" + pGameConfig->Name + ".json";
		std::string analysisStateFName = root + "AnalysisState/" + pGameConfig->Name + ".astate";
		std::string saveStateFName = root + "SaveStates/" + pGameConfig->Name + ".state";

		// check for new location & adjust paths accordingly
		const std::string gameRoot = pGlobalConfig->WorkspaceRoot + pGameConfig->Name + "/";
		if (FileExists((gameRoot + "Config.json").c_str()))
		{
			analysisJsonFName = gameRoot + "Analysis.json";
			graphicsSetsJsonFName = gameRoot + "GraphicsSets.json";
			analysisStateFName = gameRoot + "AnalysisState.bin";
			saveStateFName = gameRoot + "SaveState.bin";
		}

		if (LoadGameState(saveStateFName.c_str()) == false)
		{
			GamesList.LoadGame(pGameConfig->Name.c_str());
		}
		
		if (!InitBankMappings())
		{
			return false;
		}

		ImportAnalysisJson(CodeAnalysis, analysisJsonFName.c_str());
		ImportAnalysisState(CodeAnalysis, analysisStateFName.c_str());

		pGraphicsViewer->LoadGraphicsSets(graphicsSetsJsonFName.c_str());
	}
	else
	{
		if (!GamesList.LoadGame(pGameConfig->Name.c_str()))
			return false;

		InitBankMappings();
	}

	ReAnalyseCode(CodeAnalysis);
	GenerateGlobalInfo(CodeAnalysis);
	CodeAnalysis.SetAddressRangeDirty();

#ifdef RUN_AHEAD_TO_GENERATE_SCREEN
	// Run the cpc for long enough to generate a frame buffer, otherwise the user will be staring at a black screen.
	// sam todo: run for exactly 1 video frame. The current technique is crude and can render >1 frame, including partial frames and produce 
	// a glitch when continuing execution.
	// todo mute audio so we don't hear a frame of audio
	CodeAnalysis.Debugger.Continue();
	cpc_exec(&CPCEmuState, 48000);

	ImGui_UpdateTextureRGBA(CPCViewer.GetScreenTexture(), CPCViewer.GetFrameBuffer());

	// Load the game again (from memory - it should be cached) to restore the cpc state.
	//const std::string snapFolder = pGlobalConfig->SnapshotFolder;
	//const std::string gameFile = snapFolder + pGameConfig->SnapshotFile;
	GamesList.LoadGame(pGameConfig->Name.c_str());
#endif
	
	// Start in break mode so the memory will be in it's initial state. 
	// Otherwise, if we export an asm file once the game is running the memory will be in an arbitrary state.
	CodeAnalysis.Debugger.SetPC(CodeAnalysis.AddressRefFromPhysicalAddress(CPCEmuState.cpu.pc - 1));
	CodeAnalysis.Debugger.Break();

	CodeAnalysis.Debugger.RegisterNewStackPointer(CPCEmuState.cpu.sp, FAddressRef());

	// some extra initialisation for creating new analysis from snnapshot
	if (bLoadGameData == false)
	{
		FAddressRef initialPC = CodeAnalysis.AddressRefFromPhysicalAddress(CPCEmuState.cpu.pc);
		SetItemCode(CodeAnalysis, initialPC);
		CodeAnalysis.Debugger.SetPC(initialPC);
	}
	
	pGraphicsViewer->SetImagesRoot((pGlobalConfig->WorkspaceRoot + "/" + pGameConfig->Name + "/GraphicsSets/").c_str());

	pCurrentGameConfig = pGameConfig;

	LoadLua();
	return true;
}

bool FCPCEmu::LoadLua()
{
	// Setup Lua - reinitialised for each game
	const std::string gameRoot = pGlobalConfig->WorkspaceRoot + pCurrentGameConfig->Name + "/";
	if (LuaSys::Init(this))
	{
		RegisterCPCLuaAPI(LuaSys::GetGlobalState());

		for (const auto& gameScript : pCurrentGameConfig->LuaSourceFiles)
		{
			std::string luaScriptFName = gameRoot + gameScript;
			LuaSys::LoadFile(luaScriptFName.c_str(), true);
		}
		return true;
	}

	return false;
}

const uint32_t kMachineStateMagic = 0xBeefCafe;
static cpc_t g_SaveSlot;

bool FCPCEmu::SaveGameState(const char* fname)
{
	// save game snapshot
	FILE* fp = fopen(fname, "wb");
	if (fp != nullptr)
	{
		// write magic
		fwrite(&kMachineStateMagic, sizeof(kMachineStateMagic), 1, fp);

		const uint32_t versionNo = cpc_save_snapshot(&CPCEmuState, &g_SaveSlot);
		fwrite(&versionNo, sizeof(versionNo), 1, fp);
		fwrite(&g_SaveSlot, sizeof(cpc_t), 1, fp);

		fclose(fp);
		return true;
	}

	return false;
}

bool FCPCEmu::LoadGameState(const char* fname)
{
	FILE* fp = fopen(fname, "rb");
	if (fp == NULL)
		return false;

	uint32_t magicVal;
	fread(&magicVal, sizeof(magicVal), 1, fp);
	if (magicVal != kMachineStateMagic)
		return false;

	uint32_t snapshotVersion = 0;
	fread(&snapshotVersion, sizeof(snapshotVersion), 1, fp);
	fread(&g_SaveSlot, sizeof(cpc_t), 1, fp);	// load into save slot

	const bool bSuccess = cpc_load_snapshot(&CPCEmuState, 1, &g_SaveSlot);

	if (CPCEmuState.type == CPC_TYPE_6128)
	{
		UpdateBankMappings();
	}

	fclose(fp);
	return bSuccess;
}

// save config & data
bool FCPCEmu::SaveCurrentGameData(void)
{
	if (pActiveGame != nullptr)
	{
		FGameConfig* pGameConfig = pActiveGame->pConfig;
		if (pGameConfig == nullptr || pGameConfig->Name.empty())
			return false;

#if SAVE_NEW_DIRS
		const std::string root = pGlobalConfig->WorkspaceRoot + pGameConfig->Name + "/";
		const std::string configFName = root + "Config.json";
		const std::string analysisJsonFName = root + "Analysis.json";
		const std::string graphicsSetsJsonFName = root + "GraphicsSets.json";
		const std::string analysisStateFName = root + "AnalysisState.bin";
		const std::string saveStateFName = root + "SaveState.bin";
		EnsureDirectoryExists(root.c_str());
#else

		const std::string root = pGlobalConfig->WorkspaceRoot;
		const std::string configFName = root + "Configs/" + pGameConfig->Name + ".json";
		//const std::string dataFName = root + "GameData/" + pGameConfig->Name + ".bin";
		const std::string analysisJsonFName = root + "AnalysisJson/" + pGameConfig->Name + ".json";
		const std::string graphicsSetsJsonFName = root + "GraphicsSets/" + pGameConfig->Name + ".json";
		const std::string analysisStateFName = root + "AnalysisState/" + pGameConfig->Name + ".astate";
		const std::string saveStateFName = root + "SaveStates/" + pGameConfig->Name + ".state";
		EnsureDirectoryExists(std::string(root + "Configs").c_str());
		EnsureDirectoryExists(std::string(root + "GameData").c_str());
		EnsureDirectoryExists(std::string(root + "AnalysisJson").c_str());
		EnsureDirectoryExists(std::string(root + "GraphicsSets").c_str());
		EnsureDirectoryExists(std::string(root + "AnalysisState").c_str());
		EnsureDirectoryExists(std::string(root + "SaveStates").c_str());
#endif

		// set config values
		for (int i = 0; i < FCodeAnalysisState::kNoViewStates; i++)
		{
			const FCodeAnalysisViewState& viewState = CodeAnalysis.ViewState[i];
			FCodeAnalysisViewConfig& viewConfig = pGameConfig->ViewConfigs[i];

			viewConfig.bEnabled = viewState.Enabled;
			viewConfig.ViewAddress = viewState.GetCursorItem().IsValid() ? viewState.GetCursorItem().AddressRef : FAddressRef();
		}

		SaveGameConfigToFile(*pGameConfig, configFName.c_str());
		SaveGameState(saveStateFName.c_str());
		ExportAnalysisJson(CodeAnalysis, analysisJsonFName.c_str());
		ExportAnalysisState(CodeAnalysis, analysisStateFName.c_str());
		//ExportGameJson(this, analysisJsonFName.c_str());
		pGraphicsViewer->SaveGraphicsSets(graphicsSetsJsonFName.c_str());
	}

	return true;
}

bool FCPCEmu::NewGameFromSnapshot(const FGameSnapshot& snaphot)
{
	// Remove any existing config 
	RemoveGameConfig(snaphot.DisplayName.c_str());

	FCPCGameConfig* pNewConfig = CreateNewCPCGameConfigFromSnapshot(snaphot);

	if (pNewConfig != nullptr)
	{
		if (!StartGame(pNewConfig, /* bLoadGameData */ false))
			return false;

		AddGameConfig(pNewConfig);
		SaveCurrentGameData();

		return true;
	}

	return false;
}

void FCPCEmu::Reset()
{
	FEmuBase::Reset();
	cpc_reset(&CPCEmuState);
	// Resetting rom_select to 0 because Chips doesn't do it.
	CPCEmuState.ga.rom_select = 0;
	ui_dbg_reset(&UICPC.dbg);

	FCPCGameConfig* pBasicConfig = (FCPCGameConfig*)GetGameConfigForName("AmstradBasic");

	if (pBasicConfig == nullptr)
		pBasicConfig = CreateNewAmstradBasicConfig();

	InitBankMappings();
	
	StartGame(pBasicConfig, false);	// reset code analysis

	CodeAnalysis.Debugger.Continue();
}

void FCPCEmu::Tick()
{
	FEmuBase::Tick();

	FDebugger& debugger = CodeAnalysis.Debugger;

	CPCViewer.Tick();

	if (debugger.IsStopped() == false)
	{
		const float frameTime = std::min(1000000.0f / ImGui::GetIO().Framerate, 32000.0f) * ExecSpeedScale;
		const uint32_t microSeconds = std::max(static_cast<uint32_t>(frameTime), uint32_t(1));

		CodeAnalysis.OnFrameStart();
		
		StoreRegisters_Z80(CodeAnalysis);

		cpc_exec(&CPCEmuState, microSeconds);
		
		// sam todo
		//FrameTraceViewer.CaptureFrame();

		CodeAnalysis.OnFrameEnd();
	}
	
	UpdateCharacterSets(CodeAnalysis);

	UpdatePalette();

	DrawDockingView();
}

// These functions are used to add to the bottom of the menus
void	FCPCEmu::FileMenuAdditions(void)
{
	if (ImGui::MenuItem("Export Binary File"))
	{
		if (pActiveGame != nullptr)
		{
			const std::string dir = pGlobalConfig->WorkspaceRoot + "OutputBin/";
			EnsureDirectoryExists(dir.c_str());
			std::string outBinFname = dir + pActiveGame->pConfig->Name + ".bin";
			// todo fixup for 6128
			uint8_t* pCPCMem = new uint8_t[65536];
			memset(pCPCMem, 0, 65536);

			uint16_t addr = 0;
			for (int b = 0; b < kNoRAMBanks; b++)
			{
				if (const FCodeAnalysisBank* pBank = GetCodeAnalysis().GetBank(RAMBanks[b]))
				{
					if (pBank->PrimaryMappedPage != -1)
					{
						for (int i = 0; i < pBank->GetSizeBytes(); i++)
						{
							pCPCMem[addr] = pBank->Memory[i];
							addr++;
						}
					}
				}
			}
			
			SaveBinaryFile(outBinFname.c_str(), pCPCMem, 65536);
			delete[] pCPCMem;
		}
	}
}

void	FCPCEmu::SystemMenuAdditions(void)
{
	ImGui::MenuItem("Memory Map", 0, &UICPC.memmap.open);
	//ImGui::MenuItem("Keyboard Matrix", 0, &UICPC.kbd.open);
	//ImGui::MenuItem("Audio Output", 0, &UICPC.audio.open);
	//ImGui::MenuItem("Z80 CPU", 0, &UICPC.cpu.open);
	//ImGui::MenuItem("AM40010 (Gate Array)", 0, &UICPC.ga.open);
	//ImGui::MenuItem("AY-3-8912 (PSG)", 0, &UICPC.psg.open);

	if (ImGui::BeginMenu("Joystick"))
	{
		if (ImGui::MenuItem("Disabled", 0, (CPCEmuState.joystick_type == CPC_JOYSTICK_NONE)))
		{
			CPCEmuState.joystick_type = CPC_JOYSTICK_NONE;
		}
		if (ImGui::MenuItem("Enabled (Cursor Keys and Space)", 0, (CPCEmuState.joystick_type == CPC_JOYSTICK_DIGITAL)))
		{
			CPCEmuState.joystick_type = CPC_JOYSTICK_DIGITAL;
		}
		ImGui::EndMenu();
	}
}

void	FCPCEmu::OptionsMenuAdditions(void)
{
}

void	FCPCEmu::WindowsMenuAdditions(void)
{
}


void FCPCEmu::DrawEmulatorUI()
{
	ui_cpc_t* pCPCUI = &UICPC;
	const double timeMS = 1000.0f / ImGui::GetIO().Framerate;
	
	// Draw the main menu
	//DrawMainMenu(timeMS);
	
	if (pCPCUI->memmap.open) 
	{
		_ui_cpc_update_memmap(pCPCUI);
	}
	
	/*
	// call the Chips UI functions
	ui_audio_draw(&pCPCUI->audio, pCPCUI->cpc->audio.sample_pos);
	ui_z80_draw(&pCPCUI->cpu);
	ui_ay38910_draw(&pCPCUI->psg);
	ui_kbd_draw(&pCPCUI->kbd);
	ui_am40010_draw(&pCPCUI->ga);
	*/
	
	ui_memmap_draw(&pCPCUI->memmap);
	
	if (ImGui::Begin("CPC View", nullptr, ImGuiWindowFlags_NoNavInputs))
	{
		CPCViewer.Draw();
	}
	ImGui::End();

	// sam. get this working.
	/*
	if (ImGui::Begin("Frame Trace"))
	{
		FrameTraceViewer.Draw();
	}
	ImGui::End();*/

#ifndef NDEBUG
	// config
	if (CodeAnalysis.Config.bShowConfigWindow)
	{
		if (ImGui::Begin("Configuration", &CodeAnalysis.Config.bShowConfigWindow))
			DrawCodeAnalysisConfigWindow(CodeAnalysis);
		ImGui::End();
	}
#endif

	//GraphicsViewer.Draw();
	//DrawMemoryTools();
}


void FCPCLaunchConfig::ParseCommandline(int argc, char** argv)
{
	FEmulatorLaunchConfig::ParseCommandline(argc, argv);	// call base class

	std::vector<std::string> argList;
	for (int arg = 0; arg < argc; arg++)
	{
		argList.emplace_back(argv[arg]);
	}

	auto argIt = argList.begin();
	argIt++;	// skip exe name
	while (argIt != argList.end())
	{
		if (*argIt == std::string("-128"))
		{
			Model = ECPCModel::CPC_6128;
		}

		++argIt;
	}
}

void FCPCEmu::UpdatePalette()
{
	FPalette& palette = Screen.GetCurrentPalette();
	for (int i = 0; i < palette.GetColourCount(); i++)
	{
		palette.SetColour(i, CPCEmuState.ga.hw_colors[CPCEmuState.ga.regs.ink[i]]);
	}
}