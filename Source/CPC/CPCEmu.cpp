#include <cstdint>

#define CHIPS_UI_IMPL

#include <imgui.h>
#include "CPCEmu.h"

#include "CPCConfig.h"
#include "GameConfig.h"
#include "Util/FileUtil.h"
#include "Util/GraphicsView.h"
#include "CodeAnalyser/UI/CodeAnalyserUI.h"
#include <CodeAnalyser/CodeAnalysisState.h>
#include "CodeAnalyser/CodeAnalysisJson.h"
#include "Debug/DebugLog.h"
#include "CpcChipsImpl.h"

#include "Exporters/AssemblerExport.h"
#include "CodeAnalyser/UI/CharacterMapViewer.h"
#include "App.h"
#include "Viewers/CRTCViewer.h"
#include "Viewers/OverviewViewer.h"

#include <sokol_audio.h>
#include "cpc-roms.h"

#include <ImGuiSupport/ImGuiTexture.h>

#define RUN_AHEAD_TO_GENERATE_SCREEN

// Disabled for now
//#define ENABLE_CPC_6128

const std::string kAppTitle = "CPC Analyser";
const char* kGlobalConfigFilename = "GlobalConfig.json";

void StoreRegisters_Z80(FCodeAnalysisState& state);

#if 0
// sam. this was taken from  _ui_cpc_memptr()
// do we need to replace it somehow?
// Memory access functions
uint8_t* MemGetPtr(cpc_t* cpc, int layer, uint16_t addr)
{
	if (layer == 1) // GA
	{
		uint8_t* ram = &cpc->ram[0][0];
		return ram + addr;
	}
	else if (layer == 2) // ROMS
	{
		if (addr < 0x4000)
		{
			return &cpc->rom_os[addr];
		}
		else if (addr >= 0xC000)
		{
			return &cpc->rom_basic[addr - 0xC000];
		}
		else
		{
			return 0;
		}
	}
	else if (layer == 3) // AMSDOS
	{
		if ((CPC_TYPE_6128 == cpc->type) && (addr >= 0xC000))
		{
			return &cpc->rom_amsdos[addr - 0xC000];
		}
		else
		{
			return 0;
		}
	}
	else
	{
		/* one of the 7 RAM layers */
		const int ram_config_index = (CPC_TYPE_6128 == cpc->type) ? (cpc->ga.ram_config & 7) : 0;
		const int ram_bank = layer - _UI_CPC_MEMLAYER_RAM0;
		bool ram_mapped = false;
		for (int i = 0; i < 4; i++)
		{
			if (ram_bank == _ui_cpc_ram_config[ram_config_index][i])
			{
				const uint16_t start = 0x4000 * i;
				const uint32_t end = start + 0x4000;
				ram_mapped = true;
				if ((addr >= start) && (addr < end))
				{
					return &cpc->ram[ram_bank][addr - start];
				}
			}
		}
		if (!ram_mapped && (CPC_TYPE_6128 != cpc->type))
		{
			/* if the RAM bank is not currently mapped to a CPU visible address,
					just use start address zero, this will throw off disassemblers
					though
			*/
			if (addr < 0x4000)
			{
				return &cpc->ram[ram_bank][addr];
			}
		}
	}
	/* fallthrough: address isn't mapped to physical RAM */
	return 0;
}

uint8_t MemReadFunc(int layer, uint16_t addr, void* user_data)
{
	cpc_t* cpc = (cpc_t*) user_data;
    //cpc_t* cpc = ui_cpc->cpc;
    if (layer == _UI_CPC_MEMLAYER_CPU) {
        /* CPU mapped RAM layer */
        return mem_rd(&cpc->mem, addr);
    }
    else {
        uint8_t* ptr = _ui_cpc_memptr(cpc, layer, addr);
        if (ptr) {
            return *ptr;
        }
        else {
            return 0xFF;
        }
    }
}

void MemWriteFunc(int layer, uint16_t addr, uint8_t data, void* user_data)
{
	cpc_t* cpc = (cpc_t*)user_data;
	if (layer == 0) 
	{
		mem_wr(&cpc->mem, addr, data);
	}
	else 
	{
		uint8_t* ptr = MemGetPtr(cpc, layer, addr);
		if (ptr) 
		{
			*ptr = data;
		}
	}
}
#endif

uint8_t	FCpcEmu::ReadByte(uint16_t address) const
{
	return mem_rd(const_cast<mem_t*>(&CpcEmuState.mem), address);
}

uint16_t FCpcEmu::ReadWord(uint16_t address) const 
{
	return ReadByte(address) | (ReadByte(address + 1) << 8);
}

const uint8_t* FCpcEmu::GetMemPtr(uint16_t address) const 
{
	const int bank = address >> 14;
	const int bankAddr = address & 0x3fff;

	return &CpcEmuState.ram[bank][bankAddr];

	// sam todo figure this out
#if SPECCY
	if (CpcEmuState.type == CPC_TYPE_464)
	{
		// this logic is from the Speccy so wont work on the cpc
	
		const int bank = address >> 14;
		const int bankAddr = address & 0x3fff;

		if (bank == 0)
			return &CpcEmuState.rom[0][bankAddr];
		else
			return &CpcEmuState.ram[bank - 1][bankAddr];
	}
	else
	{
		const uint8_t memConfig = CpcEmuState.last_mem_config; // last mem config doesn't exist on cpc

		if (address < 0x4000)
			return &ZXEmuState.rom[(memConfig & (1 << 4)) ? 1 : 0][address];
		else if (address < 0x8000)
			return &ZXEmuState.ram[5][address - 0x4000];
		else if (address < 0xC000)
			return &ZXEmuState.ram[2][address - 0x8000];
		else
			return &ZXEmuState.ram[memConfig & 7][address - 0xC000];
	}
#endif
}

void FCpcEmu::WriteByte(uint16_t address, uint8_t value)
{
	mem_wr(&CpcEmuState.mem, address, value);
}

FAddressRef FCpcEmu::GetPC(void) 
{
	return CodeAnalysis.Debugger.GetPC();
} 

uint16_t FCpcEmu::GetSP(void)
{
	return CpcEmuState.cpu.sp;
}

void* FCpcEmu::GetCPUEmulator(void) const
{
	return (void*)&CpcEmuState.cpu;
}

class FScreenPixMemDescGenerator : public FMemoryRegionDescGenerator
{
public:
	FScreenPixMemDescGenerator(FCpcEmu* pEmu)
		: pCpcEmu(pEmu)
	{
		UpdateScreenMemoryLocation();
	}

	const char* GenerateAddressString(uint16_t addr) override
	{
		// todo: deal with screen mode? display both scr mode's x coords?
		int xp = 0, yp = 0;
		if (pCpcEmu->Screen.GetScreenAddressCoords(addr, xp, yp))
			sprintf(DescStr, "Screen: %d,%d", xp, yp);
		else
			sprintf(DescStr, "Screen: ?,?");
		return DescStr;
	}

	void UpdateScreenMemoryLocation()
	{
		RegionMin = pCpcEmu->Screen.GetScreenAddrStart();
		RegionMax = pCpcEmu->Screen.GetScreenAddrEnd();
	}
private:
	FCpcEmu* pCpcEmu = 0;
	char DescStr[32] = { 0 };
};

void FCpcEmu::GraphicsViewerSetView(FAddressRef address)
{
	GraphicsViewer.GoToAddress(address);
}

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
	FCpcEmu* pEmu = (FCpcEmu*)user_data;
	if(pEmu->pGlobalConfig->bEnableAudio)
		saudio_push(samples, num_samples);
}

void	FCpcEmu::OnInstructionExecuted(int ticks, uint64_t pins)
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
uint64_t FCpcEmu::Z80Tick(int num, uint64_t pins)
{
	FCodeAnalysisState& state = CodeAnalysis;
	FDebugger& debugger = CodeAnalysis.Debugger;

	// we have to pass data to the tick through an internal state struct because the z80_t struct only gets updated after an emulation exec period
	z80_t* pCPU = (z80_t*)state.CPUInterface->GetCPUEmulator();
	const uint16_t pc = GetPC().Address;

	Screen.Tick();

	const am40010_crt_t& crt = CpcEmuState.ga.crt;
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

	const bool bNewOp = z80_opdone(&CpcEmuState.cpu);

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

// might need to move this to the chipsimpl code as we use chips functions/defines only available when CHIPS_IMPL is defined
//#if FIXME
	if (pins & Z80_IORQ)
	{
		// This is still needed because it deals with adding events to the event trace.
		IOAnalysis.IOHandler(pc, pins);


#if FIXME
		// This is a failed attempt to get bank switching working, including switching in rom banks. 
		// Some of this logic is based on what happens in _cpc_bankswitch().
		// note: some of this code logic is duplicated in IOAnalysis.cpp in HandleGateArray
		if (pins & (Z80_RD | Z80_WR))
		{
			if ((pins & (AM40010_A14 | AM40010_A15)) == AM40010_A14)
			{
				// extract 8-bit data bus from 64-bit pin mask
				//#define _AM40010_GET_DATA(p) ((uint8_t)(((p)&0xFF0000ULL)>>16))
				const uint8_t data = ((uint8_t)(((pins) & 0xFF0000ULL) >> 16));

				/* data bits 6 and 7 select the register type */
				switch (data & ((1 << 7) | (1 << 6)))
				{
					case (1 << 7):
					{
						am40010_registers_t& regs = CpcEmuState.ga.regs;
						if (regs.config & AM40010_CONFIG_LROMEN)
						{
							// disable low rom
							SetROMBankLo(ROM_NONE);
						}
						else
						{
							// enable low rom
							SetROMBankLo(ROM_OS);
						}

						if (regs.config & AM40010_CONFIG_HROMEN)
						{
							// disable high rom
							SetROMBankHi(ROM_NONE);
						}
						else
						{
							// enable high rom
							SetROMBankHi(CpcEmuState.ga.rom_select == 7 ? ROM_AMSDOS : ROM_BASIC);
						}
						// sam is this right?
						CodeAnalysis.SetAllBanksDirty();
					}
					break;

					/* RAM bank switching (6128 only) */
					case (1 << 6) | (1 << 7) :
					{
						int bankPresetIndex;
						if (AM40010_CPC_TYPE_6128 == CpcEmuState.type)
						{
							// sam todo. only set ram banks if dirty?
							bankPresetIndex = CpcEmuState.ga.ram_config & 7;
							SetRAMBanksPreset(bankPresetIndex);
						}
						break;
					}
				}
			}		
		}
#endif // FIXME
	}

	return pins;
}

static uint64_t Z80TickThunk(int num, uint64_t pins, void* user_data)
{
	FCpcEmu* pEmu = (FCpcEmu*)user_data;
	return pEmu->Z80Tick(num, pins);
}

// Set the low rom bank (0x0000 - 0x4000).
// This is the OS ROM.
void FCpcEmu::SetROMBankLo(int bankNo)
{
	const int16_t bankId = bankNo == ROM_NONE ? ROM_NONE : ROMBanks[bankNo];
	if (CurROMBankLo == bankId)
		return;

	//LOGDEBUG("%s OS ROM", bankNo == ROM_NONE ? "Disable" : "Enable");
 
// sam. currently disabled until separate banks for read and write are supported
#if 0
	// Unmap old bank
	CodeAnalysis.UnMapBank(CurROMBankLo, 0);
	if (bankNo != ROM_NONE)
		CodeAnalysis.MapBank(bankId, 0, EBankAccess::Read);
#endif
	CurROMBankLo = bankId;
}

// Set the high rom bank (0xc000 - 0xffff).
// This can either be the AMSDOS or the BASIC ROM
void FCpcEmu::SetROMBankHi(int bankNo)
{
	const int16_t bankId = bankNo == ROM_NONE ? ROM_NONE : ROMBanks[bankNo];
	if (CurROMBankHi == bankId)
		return;

	//LOGDEBUG("%s %s ROM", bankNo == ROM_NONE ? "Disable" : "Enable", bankNo == ROM_AMSDOS ? "AMSDOS" : "BASIC");

// sam. currently disabled until separate banks for read and write are supported
#if 0
	// Unmap old bank
	CodeAnalysis.UnMapBank(CurROMBankHi, 0);
	if (bankNo != ROM_NONE)
		CodeAnalysis.MapBank(bankId, 0, EBankAccess::Read);
#endif
	CurROMBankHi = bankId;
}

// Slot is physical 16K memory region (0-3) 
// Bank is a 16K CPC RAM bank (0-7)
void FCpcEmu::SetRAMBank(int slot, int bankNo)
{
	const int16_t bankId = RAMBanks[bankNo];
	if (CurRAMBank[slot] == bankId)
		return;

	// Unmap old bank
	const int startPage = slot * kNoBankPages;
	CodeAnalysis.UnMapBank(CurRAMBank[slot], startPage);
	CodeAnalysis.MapBank(bankId, startPage);

	CodeAnalysis.GetBank(RAMBanks[bankNo])->PrimaryMappedPage = slot * 16;

	CurRAMBank[slot] = bankId;
}

void FCpcEmu::SetRAMBanksPreset(int bankPresetIndex)
{
	//_cpc_ram_config isn't available because we don't have CHIPS_IMPL.
	// might have to move this whole function to cpchipsimpl.c.
	// or write a GetRamConfig() function in cpcchipsimpl.c
#if FIXME
	const int slot0BankIndex = _cpc_ram_config[bankPresetIndex][0];
	const int slot1BankIndex = _cpc_ram_config[bankPresetIndex][1];
	const int slot2BankIndex = _cpc_ram_config[bankPresetIndex][2];
	const int slot3BankIndex = _cpc_ram_config[bankPresetIndex][3];

	SetRAMBank(0, slot0BankIndex);
	SetRAMBank(1, slot1BankIndex);
	SetRAMBank(2, slot2BankIndex);
	SetRAMBank(3, slot3BankIndex);
#endif

	// sam todo: only set dirty banks that have changed?
	CodeAnalysis.SetAllBanksDirty();
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
	FCpcEmu* pEmu = (FCpcEmu*)user_data;
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
		const uint32_t col = GetCpcColour(event.Value & 0x1f);

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

bool FCpcEmu::Init(const FCpcConfig& config)
{
#ifndef NDEBUG
	LOGINFO("Init CPCEmu...");
#endif

	const std::string memStr = config.Model == ECpcModel::CPC_6128 ? " (CPC 6128)" : " (CPC 464)";
	SetWindowTitle((std::string(kAppTitle) + memStr).c_str());
	SetWindowIcon("CPCALogo.png");

	// Initialise the CPC emulator
	pGlobalConfig = new FCPCConfig();
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
	GameLoader.Init(this);
	GamesList.Init(&GameLoader);
	if (config.Model == ECpcModel::CPC_6128)
		GamesList.EnumerateGames(pGlobalConfig->SnapshotFolder128.c_str());
	else
		GamesList.EnumerateGames(pGlobalConfig->SnapshotFolder.c_str());
	
	// Turn caching on for the game loader. This means that if the same game is loaded more than once
	// the second time will involve no disk i/o.
	// We use this so we can quickly restore the game state after running ahead to generate the frame buffer in StartGame().
	GameLoader.SetCachingEnabled(true);

	Screen.Init(this);

	//cpc_type_t type = CPC_TYPE_464;
	//cpc_type_t type = CPC_TYPE_6128;
	cpc_type_t type = config.Model == ECpcModel::CPC_6128 ? CPC_TYPE_6128 : CPC_TYPE_464;
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

	cpc_init(&CpcEmuState, &desc);

	// Clear UI
	memset(&UICpc, 0, sizeof(ui_cpc_t));

	//ui_init(zxui_draw);
	{
		ui_cpc_desc_t desc = { 0 };
		desc.cpc = &CpcEmuState;
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

		ui_cpc_init(&UICpc, &desc);
	}

	// This is where we add the viewers we want
	Viewers.push_back(new FCrtcViewer(this));
	Viewers.push_back(new FOverviewViewer(this));

	// Initialise Viewers
	for (auto Viewer : Viewers)
	{
		if (Viewer->Init() == false)
		{
			// TODO: report error
		}
	}

	GraphicsViewer.Init(&CodeAnalysis, this);

	IOAnalysis.Init(this);
	CpcViewer.Init(this);
	CodeAnalysis.ViewState[0].Enabled = true;	// always have first view enabled

	// Setup memory description handlers
	pScreenMemDescGenerator = new FScreenPixMemDescGenerator(this);
	AddMemoryRegionDescGenerator(pScreenMemDescGenerator);

	LoadGameConfigs(this);

	GetCurrentPalette().SetColourCount(16);
	// Set up code analysis
	// initialise code analysis pages

	// Low ROM 0x0000 - 0x3fff
	ROMBanks[ROM_OS] = CodeAnalysis.CreateBank("ROM OS", 16, CpcEmuState.rom_os, true);
	CodeAnalysis.GetBank(ROMBanks[ROM_OS])->PrimaryMappedPage = 0;

	// High ROM AMSDOS 0xc000 - 0xffff
	ROMBanks[ROM_AMSDOS] = CodeAnalysis.CreateBank("ROM AMSDOS", 16, CpcEmuState.rom_amsdos, true);
	CodeAnalysis.GetBank(ROMBanks[ROM_AMSDOS])->PrimaryMappedPage = 48;

	// High ROM BASIC 0xc000 - 0xffff
	ROMBanks[ROM_BASIC] = CodeAnalysis.CreateBank("ROM BASIC", 16, CpcEmuState.rom_basic, true);
	CodeAnalysis.GetBank(ROMBanks[ROM_BASIC])->PrimaryMappedPage = 48;

	// create & register RAM banks
	for (int bankNo = 0; bankNo < kNoRAMBanks; bankNo++)
	{
		char bankName[32];
		sprintf(bankName, "RAM %d", bankNo);
		RAMBanks[bankNo] = CodeAnalysis.CreateBank(bankName, 16, CpcEmuState.ram[bankNo], false);
	}

	// Setup initial machine memory config
	if (config.Model == ECpcModel::CPC_464)
	{
		CodeAnalysis.GetBank(RAMBanks[0])->PrimaryMappedPage = 0;
		CodeAnalysis.GetBank(RAMBanks[1])->PrimaryMappedPage = 16;
		CodeAnalysis.GetBank(RAMBanks[2])->PrimaryMappedPage = 32;
		CodeAnalysis.GetBank(RAMBanks[3])->PrimaryMappedPage = 48;

		SetRAMBank(0, 0);	// 0x0000 - 0x3fff
		SetRAMBank(1, 1);	// 0x4000 - 0x7fff
		SetRAMBank(2, 2);	// 0x8000 - 0xBfff
		SetRAMBank(3, 3);	// 0xc000 - 0xffff
	}
	else
	{
		CodeAnalysis.GetBank(RAMBanks[0])->PrimaryMappedPage = 0;
		CodeAnalysis.GetBank(RAMBanks[1])->PrimaryMappedPage = 16;
		CodeAnalysis.GetBank(RAMBanks[2])->PrimaryMappedPage = 32;
		CodeAnalysis.GetBank(RAMBanks[3])->PrimaryMappedPage = 48;

		SetRAMBank(0, 0);	// 0x0000 - 0x3fff
		SetRAMBank(1, 1);	// 0x4000 - 0x7fff
		SetRAMBank(2, 2);	// 0x8000 - 0xBfff
		SetRAMBank(3, 3);	// 0xc000 - 0xffff
	}

	// load the command line game if none specified then load the last game
	bool bLoadedGame = false;

	if (config.SpecificGame.empty() == false)
	{
		bLoadedGame = StartGame(config.SpecificGame.c_str());
	}
	else if (pGlobalConfig->LastGame.empty() == false)
	{
		bLoadedGame = StartGame(pGlobalConfig->LastGame.c_str());
	}
	// Start ROM if no game has been loaded
	if (bLoadedGame == false)
	{
#if SPECCY
		std::string romJsonFName = kRomInfo48JsonFile;

		if (config.Model == ESpectrumModel::Spectrum128K)
			romJsonFName = kRomInfo128JsonFile;
#endif
		CodeAnalysis.Init(this);

#if SPECCY
		if (FileExists(romJsonFName.c_str()))
			ImportAnalysisJson(CodeAnalysis, romJsonFName.c_str());
#endif
	}

	FDebugger& debugger = CodeAnalysis.Debugger;
	debugger.RegisterEventType((int)EEventType::None, "None", 0);
	debugger.RegisterEventType((int)EEventType::ScreenPixWrite,				"Screen RAM Write",	0xff0000ff, nullptr, EventShowPixValue);
	debugger.RegisterEventType((int)EEventType::PaletteSelect,				"Palette Select",	0xffffffff, IOPortEventShowAddress, PaletteEventShowValue);
	debugger.RegisterEventType((int)EEventType::PaletteColour,				"Palette Colour",	0xff00ffff, IOPortEventShowAddress, PaletteEventShowValue);
	debugger.RegisterEventType((int)EEventType::BorderColour,				"Border Colour",	0xff00ff00, IOPortEventShowAddress, PaletteEventShowValue);
	debugger.RegisterEventType((int)EEventType::ScreenModeChange,			"Screen Mode",		0xff0080ff, IOPortEventShowAddress, ScreenModeShowValue);
	debugger.RegisterEventType((int)EEventType::CrtcRegisterSelect,			"CRTC Reg. Select",	0xffff00ff, CRTCWriteEventShowAddress, CRTCWriteEventShowValue);
	debugger.RegisterEventType((int)EEventType::CrtcRegisterRead,			"CRTC Reg. Read",	0xffff0000, CRTCWriteEventShowAddress, CRTCWriteEventShowValue);
	debugger.RegisterEventType((int)EEventType::CrtcRegisterWrite,			"CRTC Reg. Write",	0xffffff00, CRTCWriteEventShowAddress, CRTCWriteEventShowValue);
	debugger.RegisterEventType((int)EEventType::KeyboardRead,				"Keyboard Read",	0xff808080, IOPortEventShowAddress, IOPortEventShowValue);
	debugger.RegisterEventType((int)EEventType::ScreenMemoryAddressChange,	"Set Scr. Addr.",	0xffff69b4, nullptr, ScreenAddrChangeEventShowValue);

	CodeAnalysis.MemoryAnalyser.SetScreenMemoryArea(Screen.GetScreenAddrStart(), Screen.GetScreenAddrEnd());

#if SPECCY
	// Setup IO analyser
	if (config.Model == ESpectrumModel::Spectrum128K)
	{
		AYSoundChip.SetEmulator(&ZXEmuState.ay);
		CodeAnalysis.IOAnalyser.AddDevice(&AYSoundChip);
	}
#endif

	bInitialised = true;

#ifndef NDEBUG
	LOGINFO("Init CPCEmu...Done");
#endif
	return true;
}

void FCpcEmu::Shutdown()
{
	SaveCurrentGameData();	// save on close

	// Save Global Config - move to function?

	if (pActiveGame != nullptr)
		pGlobalConfig->LastGame = pActiveGame->pConfig->Name;

	pGlobalConfig->NumberDisplayMode = GetNumberDisplayMode();
	pGlobalConfig->bShowOpcodeValues = CodeAnalysis.pGlobalConfig->bShowOpcodeValues;
	pGlobalConfig->BranchLinesDisplayMode = CodeAnalysis.pGlobalConfig->BranchLinesDisplayMode;

	pGlobalConfig->Save(kGlobalConfigFilename);

	GraphicsViewer.Shutdown();
}

void FCpcEmu::StartGame(FGameConfig* pGameConfig, bool bLoadGameData /* =  true*/)
{
#ifndef NDEBUG
	LOGINFO("Start game '%s'", pGameConfig->Name.c_str());
#endif

	// reset systems
	MemoryAccessHandlers.clear();	// remove old memory handlers
	ResetMemoryStats(MemStats);
	FrameTraceViewer.Reset();
	GraphicsViewer.Reset();
	Screen.Reset();

	const std::string memStr = CpcEmuState.type == CPC_TYPE_6128 ? " (CPC 6128)" : " (CPC 464)";
	const std::string windowTitle = kAppTitle + " - " + pGameConfig->Name + memStr;
	SetWindowTitle(windowTitle.c_str());

	// start up game
#if SPECCY
	if (pActiveGame != nullptr)
		delete pActiveGame->pViewerData;
#endif
	delete pActiveGame;

	FGame* pNewGame = new FGame;
	pNewGame->pConfig = pGameConfig;
	pGameConfig->Cpc6128Game = CpcEmuState.type == CPC_TYPE_6128;
#if SPECCY
	pNewGame->pViewerConfig = pGameConfig->pViewerConfig;
	assert(pGameConfig->pViewerConfig != nullptr);
#endif
	pActiveGame = pNewGame;

#if SPECCY
	pNewGame->pViewerData = pNewGame->pViewerConfig->pInitFunction(this, pGameConfig);
	GenerateSpriteListsFromConfig(GraphicsViewer, pGameConfig);
#endif
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
		const std::string analysisJsonFName = root + "AnalysisJson/" + pGameConfig->Name + ".json";
		const std::string analysisStateFName = root + "AnalysisState/" + pGameConfig->Name + ".astate";
		const std::string saveStateFName = root + "SaveStates/" + pGameConfig->Name + ".state";

		ImportAnalysisJson(CodeAnalysis, analysisJsonFName.c_str());
		ImportAnalysisState(CodeAnalysis, analysisStateFName.c_str());

#if SPECCY
		GraphicsViewer.LoadGraphicsSets(graphicsSetsJsonFName.c_str());

		LoadGameState(this, saveStateFName.c_str());

		if (FileExists(romJsonFName.c_str()))
			ImportAnalysisJson(CodeAnalysis, romJsonFName.c_str());

		// where do we want pokes to live?
		LoadPOKFile(*pGameConfig, std::string(pGlobalConfig->PokesFolder + pGameConfig->Name + ".pok").c_str());
#endif
	}

	ReAnalyseCode(CodeAnalysis);
	GenerateGlobalInfo(CodeAnalysis);
#if SPECCY
	FormatSpectrumMemory(CodeAnalysis);
#endif
	CodeAnalysis.SetAddressRangeDirty();

#ifdef RUN_AHEAD_TO_GENERATE_SCREEN
	// Run the cpc for long enough to generate a frame buffer, otherwise the user will be staring at a black screen.
	// sam todo: run for exactly 1 video frame. The current technique is crude and can render >1 frame, including partial frames and produce 
	// a glitch when continuing execution.
	// todo mute audio so we don't hear a frame of audio
	CodeAnalysis.Debugger.Continue();
	cpc_exec(&CpcEmuState, 48000);

	ImGui_UpdateTextureRGBA(CpcViewer.GetScreenTexture(), CpcViewer.GetFrameBuffer());

	// Load the game again (from memory - it should be cached) to restore the cpc state.
	const std::string snapFolder = pGlobalConfig->SnapshotFolder;
	const std::string gameFile = snapFolder + pGameConfig->SnapshotFile;
	GamesList.LoadGame(gameFile.c_str());
#endif

	// Start in break mode so the memory will be in it's initial state. 
	// Otherwise, if we export an asm file once the game is running the memory will be in an arbitrary state.
	CodeAnalysis.Debugger.SetPC(CodeAnalysis.AddressRefFromPhysicalAddress(CpcEmuState.cpu.pc - 1));
	CodeAnalysis.Debugger.Break();

	CodeAnalysis.Debugger.RegisterNewStackPointer(CpcEmuState.cpu.sp, FAddressRef());

	// some extra initialisation for creating new analysis from snnapshot
	if (bLoadGameData == false)
	{
		FAddressRef initialPC = CodeAnalysis.AddressRefFromPhysicalAddress(CpcEmuState.cpu.pc);
		SetItemCode(CodeAnalysis, initialPC);
		CodeAnalysis.Debugger.SetPC(initialPC);
	}

#if SPECCY
	FGlobalConfig& globalConfig = GetGlobalConfig();
	GraphicsViewer.SetImagesRoot((globalConfig.WorkspaceRoot + "GraphicsSets/" + pGameConfig->Name + "/").c_str());
#endif
}

bool FCpcEmu::StartGame(const char* pGameName)
{
	for (const auto& pGameConfig : GetGameConfigs())
	{
		if (pGameConfig->Name == pGameName)
		{
			const std::string snapFolder = CpcEmuState.type == CPC_TYPE_6128 ? pGlobalConfig->SnapshotFolder128 : pGlobalConfig->SnapshotFolder;
			const std::string gameFile = snapFolder + pGameConfig->SnapshotFile;
			
			if (GamesList.LoadGame(gameFile.c_str()))
			{
				StartGame(pGameConfig);
				return true;
			}
		}
	}

	return false;
}

// save config & data
void FCpcEmu::SaveCurrentGameData()
{
	if (pActiveGame != nullptr)
	{
		FGameConfig* pGameConfig = pActiveGame->pConfig;
		if (pGameConfig->Name.empty())
		{

		}
		else
		{
			const std::string root = pGlobalConfig->WorkspaceRoot;
			const std::string configFName = root + "Configs/" + pGameConfig->Name + ".json";
			const std::string dataFName = root + "GameData/" + pGameConfig->Name + ".bin";
			const std::string analysisJsonFName = root + "AnalysisJson/" + pGameConfig->Name + ".json";
			const std::string analysisStateFName = root + "AnalysisState/" + pGameConfig->Name + ".astate";
			const std::string saveStateFName = root + "SaveStates/" + pGameConfig->Name + ".state";
			EnsureDirectoryExists(std::string(root + "Configs").c_str());
			EnsureDirectoryExists(std::string(root + "GameData").c_str());
			EnsureDirectoryExists(std::string(root + "AnalysisJson").c_str());
			EnsureDirectoryExists(std::string(root + "AnalysisState").c_str());
			EnsureDirectoryExists(std::string(root + "SaveStates").c_str());

			// set config values
			for (int i = 0; i < FCodeAnalysisState::kNoViewStates; i++)
			{
				const FCodeAnalysisViewState& viewState = CodeAnalysis.ViewState[i];
				FCodeAnalysisViewConfig& viewConfig = pGameConfig->ViewConfigs[i];

				viewConfig.bEnabled = viewState.Enabled;
				viewConfig.ViewAddress = viewState.GetCursorItem().IsValid() ? viewState.GetCursorItem().AddressRef : FAddressRef();
			}

			SaveGameConfigToFile(*pGameConfig, configFName.c_str());
#if SPECCY			
			// The Future
			SaveGameState(this, saveStateFName.c_str());
#endif // #if SPECCY
			ExportAnalysisJson(CodeAnalysis, analysisJsonFName.c_str());
			ExportAnalysisState(CodeAnalysis, analysisStateFName.c_str());
			//ExportGameJson(this, analysisJsonFName.c_str());
#if SPECCY			
			GraphicsViewer.SaveGraphicsSets(graphicsSetsJsonFName.c_str());
#endif
		}
	}

	// TODO: get this working?
#if	SAVE_ROM_JSON
	const std::string romJsonFName = root + kRomInfoJsonFile;
	ExportROMJson(CodeAnalysis, romJsonFName.c_str());
#endif
}

bool FCpcEmu::NewGameFromSnapshot(int snapshotIndex)
{
	if (GamesList.LoadGame(snapshotIndex))
	{
		const FGameSnapshot& game = GamesList.GetGame(snapshotIndex);

		// Remove any existing config 
		RemoveGameConfig(game.DisplayName.c_str());

		FGameConfig* pNewConfig = CreateNewGameConfigFromSnapshot(game);

		if (pNewConfig != nullptr)
		{
			StartGame(pNewConfig, /* bLoadGameData */ false);
			AddGameConfig(pNewConfig);
			SaveCurrentGameData();

			return true;
		}
	}
	return false;
}

void FCpcEmu::DrawFileMenu()
{
	if (ImGui::BeginMenu("File"))
	{
		if (ImGui::BeginMenu("New Game from Snapshot File"))
		{
			const int numGames = GamesList.GetNoGames();
			if (!numGames)
			{
				const std::string snapFolder = CpcEmuState.type == CPC_TYPE_6128 ? pGlobalConfig->SnapshotFolder128 : pGlobalConfig->SnapshotFolder;
				ImGui::Text("No snapshots found in snapshot directory:\n\n'%s'.\n\nSnapshot directory is set in GlobalConfig.json", snapFolder.c_str());
			}
			else
			{
				for (int gameNo = 0; gameNo < GamesList.GetNoGames(); gameNo++)
				{
					const FGameSnapshot& game = GamesList.GetGame(gameNo);

					if (ImGui::MenuItem(game.DisplayName.c_str()))
					{
						bool bGameExists = false;

						// does the game config exist?
						for (const auto& pGameConfig : GetGameConfigs())
						{
							if (pGameConfig->SnapshotFile == game.DisplayName)
								bGameExists = true;
						}
						if (bGameExists)
						{
							bReplaceGamePopup = true;
							ReplaceGameSnapshotIndex = gameNo;
						}
						else
						{
							NewGameFromSnapshot(gameNo);
						}
					}
				}
			}
			ImGui::EndMenu();
		}

		if (ImGui::BeginMenu("Open Game"))
		{
			if (GetGameConfigs().empty())
			{
				ImGui::Text("No games found.\n\nFirst, create a game via the 'New Game from Snapshot File' menu.");
			}
			else
			{
				for (const auto& pGameConfig : GetGameConfigs())
				{
					if (ImGui::MenuItem(pGameConfig->Name.c_str()))
					{
						const std::string snapFolder = CpcEmuState.type == CPC_TYPE_6128 ? pGlobalConfig->SnapshotFolder128 : pGlobalConfig->SnapshotFolder;
						const std::string gameFile = snapFolder + pGameConfig->SnapshotFile;

						if (GamesList.LoadGame(gameFile.c_str()))
						{
							StartGame(pGameConfig);
						}
					}
				}
			}

			ImGui::EndMenu();
		}


		if (ImGui::MenuItem("Save Game Data"))
		{
			SaveCurrentGameData();
		}
		/*if (ImGui::MenuItem("Export Binary File"))
		{
			/*if (pActiveGame != nullptr)
			{
				const std::string dir = pGlobalConfig->WorkspaceRoot + "OutputBin/";
				EnsureDirectoryExists(dir.c_str());
				std::string outBinFname = dir + pActiveGame->pConfig->Name + ".bin";
				uint8_t *pSpecMem = new uint8_t[65536];
				for (int i = 0; i < 65536; i++)
					pSpecMem[i] = ReadByte(i);
				SaveBinaryFile(outBinFname.c_str(), pSpecMem, 65536);
				delete [] pSpecMem;
			}
		}*/

		if (ImGui::MenuItem("Export ASM File"))
		{
			// ImGui popup windows can't be activated from within a Menu so we set a flag to act on outside of the menu code.
			bExportAsm = true;
		}
		ImGui::EndMenu();
	}
}

void FCpcEmu::DrawSystemMenu()
{
	if (ImGui::BeginMenu("System"))
	{
		/*if (pActiveGame && ImGui::MenuItem("Reload Snapshot"))
		{
			const std::string snapFolder = pGlobalConfig->SnapshotFolder;
			const std::string gameFile = snapFolder + pActiveGame->pConfig->SnapshotFile;
			GamesList.LoadGame(gameFile.c_str());
		}*/
		if (ImGui::MenuItem("Reset"))
		{
			cpc_reset(&CpcEmuState);
			ui_dbg_reset(&UICpc.dbg);
			//IOAnalysis.Reset();
		}

		if (ImGui::BeginMenu("Joystick"))
		{
			/*if (ImGui::MenuItem("None", 0, (pZXUI->zx->joystick_type == ZX_JOYSTICKTYPE_NONE)))
			{
				pZXUI->zx->joystick_type = ZX_JOYSTICKTYPE_NONE;
			}
			if (ImGui::MenuItem("Kempston", 0, (pZXUI->zx->joystick_type == ZX_JOYSTICKTYPE_KEMPSTON)))
			{
				pZXUI->zx->joystick_type = ZX_JOYSTICKTYPE_KEMPSTON;
			}
			if (ImGui::MenuItem("Sinclair #1", 0, (pZXUI->zx->joystick_type == ZX_JOYSTICKTYPE_SINCLAIR_1)))
			{
				pZXUI->zx->joystick_type = ZX_JOYSTICKTYPE_SINCLAIR_1;
			}
			if (ImGui::MenuItem("Sinclair #2", 0, (pZXUI->zx->joystick_type == ZX_JOYSTICKTYPE_SINCLAIR_2)))
			{
				pZXUI->zx->joystick_type = ZX_JOYSTICKTYPE_SINCLAIR_2;
			}*/
			ImGui::EndMenu();
		}
		ImGui::EndMenu();
	}
}

void FCpcEmu::DrawHardwareMenu()
{
	if (ImGui::BeginMenu("Hardware"))
	{
		ImGui::MenuItem("Memory Map", 0, &UICpc.memmap.open);
		ImGui::MenuItem("Keyboard Matrix", 0, &UICpc.kbd.open);
		ImGui::MenuItem("Audio Output", 0, &UICpc.audio.open);
		ImGui::MenuItem("Z80 CPU", 0, &UICpc.cpu.open);
		ImGui::MenuItem("AM40010 (Gate Array)", 0, &UICpc.ga.open);
		ImGui::MenuItem("AY-3-8912 (PSG)", 0, &UICpc.psg.open);
		ImGui::EndMenu();
	}
}

void FCpcEmu::DrawOptionsMenu()
{
	if (ImGui::BeginMenu("Options"))
	{
		if (ImGui::BeginMenu("Number Mode"))
		{
			bool bClearCode = false;
			if (ImGui::MenuItem("Decimal", 0, GetNumberDisplayMode() == ENumberDisplayMode::Decimal))
			{
				SetNumberDisplayMode(ENumberDisplayMode::Decimal);
				CodeAnalysis.SetAllBanksDirty();
				bClearCode = true;
			}
			if (ImGui::MenuItem("Hex - FEh", 0, GetNumberDisplayMode() == ENumberDisplayMode::HexAitch))
			{
				SetNumberDisplayMode(ENumberDisplayMode::HexAitch);
				CodeAnalysis.SetAllBanksDirty();
				bClearCode = true;
			}
			if (ImGui::MenuItem("Hex - $FE", 0, GetNumberDisplayMode() == ENumberDisplayMode::HexDollar))
			{
				SetNumberDisplayMode(ENumberDisplayMode::HexDollar);
				CodeAnalysis.SetAllBanksDirty();
				bClearCode = true;
			}

			// clear code text so it can be written again
			if (bClearCode)
			{
				for (int i = 0; i < 1 << 16; i++)
				{
					FCodeInfo* pCodeInfo = CodeAnalysis.GetCodeInfoForAddress(i);
					if (pCodeInfo && pCodeInfo->Text.empty() == false)
						pCodeInfo->Text.clear();

				}
			}

			ImGui::EndMenu();
		}

		ImGui::MenuItem("Enable Audio", 0, &pGlobalConfig->bEnableAudio);
		ImGui::MenuItem("Edit Mode", 0, &CodeAnalysis.bAllowEditing);
#if SPECCY
		ImGui::MenuItem("Scan Line Indicator", 0, &pGlobalConfig->bShowScanLineIndicator);
		if(pActiveGame!=nullptr)
			ImGui::MenuItem("Save Snapshot with game", 0, &pActiveGame->pConfig->WriteSnapshot);
#endif
		ImGui::MenuItem("Show Opcode Values", 0, &CodeAnalysis.pGlobalConfig->bShowOpcodeValues);

		if (ImGui::BeginMenu("Display Branch Lines"))
		{
			if (ImGui::MenuItem("Off", 0, CodeAnalysis.pGlobalConfig->BranchLinesDisplayMode == 0))
			{
				CodeAnalysis.pGlobalConfig->BranchLinesDisplayMode = 0;
			}
			if (ImGui::MenuItem("Minimal", 0, CodeAnalysis.pGlobalConfig->BranchLinesDisplayMode == 1))
			{
				CodeAnalysis.pGlobalConfig->BranchLinesDisplayMode = 1;
			}
			if (ImGui::MenuItem("Full", 0, CodeAnalysis.pGlobalConfig->BranchLinesDisplayMode == 2))
			{
				CodeAnalysis.pGlobalConfig->BranchLinesDisplayMode = 2;
			}

			ImGui::EndMenu();
		}
#ifndef NDEBUG
		ImGui::MenuItem("Show Config", 0, &CodeAnalysis.Config.bShowConfigWindow);
		ImGui::MenuItem("ImGui Demo", 0, &bShowImGuiDemo);
		ImGui::MenuItem("ImPlot Demo", 0, &bShowImPlotDemo);
#endif // NDEBUG
		ImGui::EndMenu();
	}
}

void FCpcEmu::DrawToolsMenu()
{
	// Note: this is a WIP menu, it'll be added in when it works properly!
#ifndef NDEBUG
	if (ImGui::BeginMenu("Tools"))
	{
		ImGui::EndMenu();
	}
#endif
}

void FCpcEmu::DrawWindowsMenu()
{
	if (ImGui::BeginMenu("Windows"))
	{
		//ImGui::MenuItem("DebugLog", 0, &bShowDebugLog);
		if (ImGui::BeginMenu("Code Analysis"))
		{
			for (int codeAnalysisNo = 0; codeAnalysisNo < FCodeAnalysisState::kNoViewStates; codeAnalysisNo++)
			{
				char menuName[32];
				sprintf(menuName, "Code Analysis %d", codeAnalysisNo + 1);
				ImGui::MenuItem(menuName, 0, &CodeAnalysis.ViewState[codeAnalysisNo].Enabled);
			}

			ImGui::EndMenu();
		}

		for (auto Viewer : Viewers)
		{
			ImGui::MenuItem(Viewer->GetName(), 0, &Viewer->bOpen);

		}
		ImGui::EndMenu();
	}
}

void FCpcEmu::DrawDebugMenu()
{
	/*if (ImGui::BeginMenu("Debug"))
	{
		ImGui::MenuItem("Memory Heatmap", 0, &pCPCUI->dbg.ui.show_heatmap);
		if (ImGui::BeginMenu("Memory Editor"))
		{
			ImGui::MenuItem("Window #1", 0, &pCPCUI->memedit[0].open);
			ImGui::MenuItem("Window #2", 0, &pCPCUI->memedit[1].open);
			ImGui::MenuItem("Window #3", 0, &pCPCUI->memedit[2].open);
			ImGui::MenuItem("Window #4", 0, &pCPCUI->memedit[3].open);
			ImGui::EndMenu();
		}

		ImGui::EndMenu();
	}*/
}

void FCpcEmu::DrawMenus()
{
	DrawFileMenu();
	DrawSystemMenu();
	DrawHardwareMenu();
	DrawOptionsMenu();
	DrawToolsMenu();
	DrawWindowsMenu();
	DrawDebugMenu();
}

void FCpcEmu::DrawMainMenu(double timeMS)
{
	bExportAsm = false;
	bReplaceGamePopup = false;

	if (ImGui::BeginMainMenuBar())
	{
		DrawMenus();

		// draw emu timings
		ImGui::SameLine(ImGui::GetWindowWidth() - 120);
		if (UICpc.dbg.dbg.stopped)
			ImGui::Text("emu: stopped");
		else
			ImGui::Text("emu: %.2fms", timeMS);

		ImGui::EndMainMenuBar();
	}

	// Draw any modal popups that have been requested from clicking on menu items.
	// This is a workaround for an open bug.
	// https://github.com/ocornut/imgui/issues/331
	DrawExportAsmModalPopup();
	DrawReplaceGameModalPopup();
}

void FCpcEmu::DrawExportAsmModalPopup()
{
	if (bExportAsm)
	{
		ImGui::OpenPopup("Export ASM File");
	}
	if (ImGui::BeginPopupModal("Export ASM File", NULL, ImGuiWindowFlags_AlwaysAutoResize))
	{
		static ImU16 addrStart = 0;//kScreenAttrMemEnd + 1;
		static ImU16 addrEnd = 0xffff;

		ImGui::Text("Address range to export");
		bool bHex = GetNumberDisplayMode() != ENumberDisplayMode::Decimal;
		const char* formatStr = bHex ? "%x" : "%u";
		ImGuiInputTextFlags flags = bHex ? ImGuiInputTextFlags_CharsHexadecimal : ImGuiInputTextFlags_CharsDecimal;

		ImGui::InputScalar("Start", ImGuiDataType_U16, &addrStart, NULL, NULL, formatStr, flags);
		ImGui::SameLine();
		ImGui::InputScalar("End", ImGuiDataType_U16, &addrEnd, NULL, NULL, formatStr, flags);

		if (ImGui::Button("Export", ImVec2(120, 0)))
		{
			if (addrEnd > addrStart)
			{
				if (pActiveGame != nullptr)
				{
					const std::string dir = pGlobalConfig->WorkspaceRoot + "OutputASM/";
					EnsureDirectoryExists(dir.c_str());

					char addrRangeStr[16];
					if (bHex)
						snprintf(addrRangeStr, 16, "_%x_%x", addrStart, addrEnd);
					else
						snprintf(addrRangeStr, 16, "_%u_%u", addrStart, addrEnd);

					std::string outBinFname = dir + pActiveGame->pConfig->Name + addrRangeStr + ".asm";

					ExportAssembler(CodeAnalysis, outBinFname.c_str(), addrStart, addrEnd);
				}
				ImGui::CloseCurrentPopup();
			}
		}
		ImGui::SetItemDefaultFocus();
		ImGui::SameLine();
		if (ImGui::Button("Cancel", ImVec2(120, 0)))
		{
			ImGui::CloseCurrentPopup();
		}
		ImGui::EndPopup();
	}
}

void FCpcEmu::DrawReplaceGameModalPopup()
{
	if (bReplaceGamePopup)
	{
		ImGui::OpenPopup("Overwrite Game?");
	}
	if (ImGui::BeginPopupModal("Overwrite Game?", NULL, ImGuiWindowFlags_AlwaysAutoResize))
	{
		ImGui::Text("Do you want to overwrite existing game data?\nAny reverse engineering progress will be lost!\n\n");
		ImGui::Separator();

		if (ImGui::Button("Overwrite", ImVec2(120, 0)))
		{
			if (GamesList.LoadGame(ReplaceGameSnapshotIndex))
			{
				const FGameSnapshot& game = GamesList.GetGame(ReplaceGameSnapshotIndex);

				for (const auto& pGameConfig : GetGameConfigs())
				{
					if (pGameConfig->SnapshotFile == game.DisplayName)
					{
						NewGameFromSnapshot(ReplaceGameSnapshotIndex);
						break;
					}
				}
			}
			ImGui::CloseCurrentPopup();
		}
		ImGui::SetItemDefaultFocus();
		ImGui::SameLine();
		if (ImGui::Button("Cancel", ImVec2(120, 0)))
		{
			ImGui::CloseCurrentPopup();
		}
		ImGui::EndPopup();
	}
}

void FCpcEmu::Tick()
{
	FDebugger& debugger = CodeAnalysis.Debugger;

	CpcViewer.Tick();

	if (debugger.IsStopped() == false)
	{
		const float frameTime = std::min(1000000.0f / ImGui::GetIO().Framerate, 32000.0f) * ExecSpeedScale;
		const uint32_t microSeconds = std::max(static_cast<uint32_t>(frameTime), uint32_t(1));

		CodeAnalysis.OnFrameStart();
		
		StoreRegisters_Z80(CodeAnalysis);

		cpc_exec(&CpcEmuState, microSeconds);
		
		FrameTraceViewer.CaptureFrame();

		CodeAnalysis.OnFrameEnd();
	}
	
	UpdateCharacterSets(CodeAnalysis);

	UpdatePalette();

	DrawDockingView();
}

// todo: delete?
void FCpcEmu::DrawMemoryTools()
{
	if (ImGui::Begin("Memory Tools") == false)
	{
		ImGui::End();
		return;
	}
	if (ImGui::BeginTabBar("MemoryToolsTabBar"))
	{
		if (ImGui::BeginTabItem("IO Analysis"))
		{
			//IOAnalysis.DrawUI();
			ImGui::EndTabItem();
		}

		ImGui::EndTabBar();
	}

	ImGui::End();
}

void FCpcEmu::DrawUI()
{
	ui_cpc_t* pCPCUI = &UICpc;
	const double timeMS = 1000.0f / ImGui::GetIO().Framerate;
	
	// Draw the main menu
	DrawMainMenu(timeMS);

	if (pCPCUI->memmap.open)
	{
		// sam todo work out why SpectrumEmu.cpp has it's own version of UpdateMemmap()
		// why doesn't it call _ui_zx_update_memmap()?
		//UpdateMemmap(pCPCUI);
	}

	if (pCPCUI->memmap.open) 
	{
		_ui_cpc_update_memmap(pCPCUI);
	}

	// call the Chips UI functions
	ui_audio_draw(&pCPCUI->audio, pCPCUI->cpc->audio.sample_pos);
	ui_z80_draw(&pCPCUI->cpu);
	ui_ay38910_draw(&pCPCUI->psg);
	ui_kbd_draw(&pCPCUI->kbd);
	ui_memmap_draw(&pCPCUI->memmap);
	ui_am40010_draw(&pCPCUI->ga);
	
	// Draw registered viewers
	for (auto Viewer : Viewers)
	{
		if (Viewer->bOpen)
		{
			if (ImGui::Begin(Viewer->GetName(), &Viewer->bOpen))
				Viewer->DrawUI();
			ImGui::End();
		}
	}

	if (ImGui::Begin("Debugger"))
	{
		CodeAnalysis.Debugger.DrawUI();
	}
	ImGui::End();

	if (ImGui::Begin("Memory Analyser"))
	{
		CodeAnalysis.MemoryAnalyser.DrawUI();
	}
	ImGui::End();

	if (ImGui::Begin("CPC View"))
	{
		CpcViewer.Draw();
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

	GraphicsViewer.Draw();
	//DrawMemoryTools();

	// Code analysis views
	for (int codeAnalysisNo = 0; codeAnalysisNo < FCodeAnalysisState::kNoViewStates; codeAnalysisNo++)
	{
		char name[32];
		sprintf(name, "Code Analysis %d", codeAnalysisNo + 1);
		if (CodeAnalysis.ViewState[codeAnalysisNo].Enabled)
		{
			if (ImGui::Begin(name, &CodeAnalysis.ViewState[codeAnalysisNo].Enabled))
			{
				DrawCodeAnalysisData(CodeAnalysis, codeAnalysisNo);
			}
			ImGui::End();
		}
	}

	ImGui::SetNextWindowSize(ImVec2(500, 400), ImGuiCond_FirstUseEver);
	if (ImGui::Begin("Character Maps"))
	{
		DrawCharacterMapViewer(CodeAnalysis, CodeAnalysis.GetFocussedViewState());
	}
	ImGui::End();
}

bool FCpcEmu::DrawDockingView()
{
	//SCOPE_PROFILE_CPU("UI", "DrawUI", ProfCols::UI);

	static bool opt_fullscreen_persistant = true;
	bool opt_fullscreen = opt_fullscreen_persistant;
	//static ImGuiDockNodeFlags dockspace_flags = ImGuiDockNodeFlags_PassthruCentralNode;
	bool bOpen = false;
	ImGuiDockNodeFlags dockFlags = 0;

	// We are using the ImGuiWindowFlags_NoDocking flag to make the parent window not dockable into,
	// because it would be confusing to have two docking targets within each others.
	ImGuiWindowFlags window_flags = ImGuiWindowFlags_MenuBar | ImGuiWindowFlags_NoDocking;
	if (opt_fullscreen)
	{
		ImGuiViewport* viewport = ImGui::GetMainViewport();
		ImGui::SetNextWindowPos(viewport->Pos);
		ImGui::SetNextWindowSize(viewport->Size);
		ImGui::SetNextWindowViewport(viewport->ID);
		ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 0.0f);
		ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 0.0f);
		window_flags |= ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove;
		window_flags |= ImGuiWindowFlags_NoBringToFrontOnFocus | ImGuiWindowFlags_NoNavFocus;
	}

	// When using ImGuiDockNodeFlags_PassthruCentralNode, DockSpace() will render our background and handle the pass-thru hole, so we ask Begin() to not render a background.
	if (dockFlags & ImGuiDockNodeFlags_PassthruCentralNode)
		window_flags |= ImGuiWindowFlags_NoBackground;

	// Important: note that we proceed even if Begin() returns false (aka window is collapsed).
	// This is because we want to keep our DockSpace() active. If a DockSpace() is inactive, 
	// all active windows docked into it will lose their parent and become undocked.
	// We cannot preserve the docking relationship between an active window and an inactive docking, otherwise 
	// any change of dockspace/settings would lead to windows being stuck in limbo and never being visible.
	bool bQuit = false;
	ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0.0f, 0.0f));
	if (ImGui::Begin("DockSpace Demo", &bOpen, window_flags))
	{
		ImGui::PopStyleVar();

		if (opt_fullscreen)
			ImGui::PopStyleVar(2);

		// DockSpace
		ImGuiIO& io = ImGui::GetIO();
		if (io.ConfigFlags & ImGuiConfigFlags_DockingEnable)
		{
			const ImGuiID dockspaceId = ImGui::GetID("MyDockSpace");
			ImGui::DockSpace(dockspaceId, ImVec2(0.0f, 0.0f), dockFlags);
		}

		//bQuit = MainMenu();
		//DrawDebugWindows(uiState);
		DrawUI();
		ImGui::End();
	}
	else
	{
		ImGui::PopStyleVar();
		bQuit = true;
	}

	return bQuit;
}

void FCpcConfig::ParseCommandline(int argc, char** argv)
{
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
#ifdef ENABLE_CPC_6128
			Model = ECpcModel::CPC_6128;
#endif
		}
		else if (*argIt == std::string("-game"))
		{
			if (++argIt == argList.end())
			{
				LOGERROR("-game : No game specified");
				break;
			}
			SpecificGame = *++argIt;
		}

		++argIt;
	}
}

void FCpcEmu::UpdatePalette()
{
	FPalette& palette = GetCurrentPalette();
	for (int i = 0; i < palette.GetColourCount(); i++)
	{
		palette.SetColour(i, CpcEmuState.ga.hw_colors[CpcEmuState.ga.regs.ink[i]]);
	}
}