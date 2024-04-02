#define CHIPS_UI_IMPL
#include <imgui.h>

#include "SpectrumEmu.h"
#include "ZXChipsImpl.h"
#include <cstdint>

#include "ZXSpectrumConfig.h"
#include "GameData.h"
#include <ImGuiSupport/ImGuiTexture.h>
#include "GameViewers/GameViewer.h"
#include "GameViewers/StarquakeViewer.h"
#include "GameViewers/MiscGameViewers.h"
#include "Viewers/SpectrumViewer.h"
#include "Viewers/ZXGraphicsView.h"
#include "CodeAnalyser/UI/OverviewViewer.h"
#include "Util/FileUtil.h"

//#include "ui/ui_dbg.h"
#include "MemoryHandlers.h"
#include "CodeAnalyser/CodeAnalyser.h"
#include "CodeAnalyser/UI/CodeAnalyserUI.h"

#include "zx-roms.h"
#include <algorithm>
#include <sokol_audio.h>
#include "SkoolkitSupport.h"
#include "Debug/DebugLog.h"
#include "Debug/ImGuiLog.h"
#include <cassert>
#include <Util/Misc.h>

#include "SpectrumConstants.h"

#include "CodeAnalyser/UI/CharacterMapViewer.h"
#include "App.h"
#include <CodeAnalyser/CodeAnalysisState.h>
#include "CodeAnalyser/CodeAnalysisJson.h"
#include "ZXSpectrumGameConfig.h"

#include "LuaScripting/LuaSys.h"
#include "SpectrumLuaAPI.h"
#include "SnapshotLoaders/Z80Loader.h"
#include "SnapshotLoaders/SNALoader.h"
#include "SnapshotLoaders/TAPLoader.h"
#include "SnapshotLoaders/TZXLoader.h"

#define ENABLE_RZX 1
#define SAVE_ROM_JSON 0

#define SAVE_NEW_DIRS 1

#define ENABLE_CAPTURES 0
const int kCaptureTrapId = 0xffff;

const char* kGlobalConfigFilename = "GlobalConfig.json";
const char* kRomInfo48JsonFile = "RomInfo.json";
const char* kRomInfo128JsonFile = "RomInfo128.json";
const std::string kAppTitle = "Spectrum Analyser";

uint8_t		FSpectrumEmu::ReadByte(uint16_t address) const
{
	return mem_rd(const_cast<mem_t*>(&ZXEmuState.mem), address);
}

uint16_t	FSpectrumEmu::ReadWord(uint16_t address) const 
{
	return ReadByte(address) | (ReadByte(address + 1) << 8);
}

const uint8_t* FSpectrumEmu::GetMemPtr(uint16_t address) const 
{
#if 0
	const uint8_t* ptr = nullptr;
	

	if (ZXEmuState.type == ZX_TYPE_48K)
	{
		const int bank = address >> 14;
		const int bankAddr = address & 0x3fff;

		if (bank == 0)
			ptr = &ZXEmuState.rom[0][bankAddr];
		else
			ptr = &ZXEmuState.ram[bank - 1][bankAddr];
	}
	else
	{
		const uint8_t memConfig = ZXEmuState.last_mem_config;

		if (address < 0x4000)
			ptr = &ZXEmuState.rom[(memConfig & (1 << 4)) ? 1 : 0][address];
		else if (address < 0x8000)
			ptr = &ZXEmuState.ram[5][address - 0x4000];
		else if (address < 0xC000)
			ptr = &ZXEmuState.ram[2][address - 0x8000];
		else
			ptr = &ZXEmuState.ram[memConfig & 7][address - 0xC000];
	}

	// so why aren't we using this? - switch to this if we don't assert
	assert(ptr == mem_readptr(const_cast<mem_t*>(&ZXEmuState.mem), address));

	return ptr;
#endif
	return mem_readptr(const_cast<mem_t*>(&ZXEmuState.mem), address);
}


void FSpectrumEmu::WriteByte(uint16_t address, uint8_t value)
{
	mem_wr(&ZXEmuState.mem, address, value);
}


FAddressRef	FSpectrumEmu::GetPC(void) 
{
	return CodeAnalysis.Debugger.GetPC();
} 

uint16_t	FSpectrumEmu::GetSP(void)
{
	return ZXEmuState.cpu.sp;
}

void* FSpectrumEmu::GetCPUEmulator(void) const
{
	return (void *)&ZXEmuState.cpu;
}

void FSpectrumEmu::FormatSpectrumMemory(FCodeAnalysisState& state) 
{
	// Format screen pixel memory if it hasn't already been
	if (state.GetLabelForPhysicalAddress(kScreenPixMemStart) == nullptr)
	{
		AddLabel(state, 0x4000, "ScreenPixels", ELabelType::Data);

		FDataInfo* pScreenPixData = state.GetReadDataInfoForAddress(kScreenPixMemStart);
		pScreenPixData->DataType = EDataType::ScreenPixels;
		//pScreenPixData->Address = kScreenPixMemStart;
		pScreenPixData->ByteSize = kScreenPixMemSize;
	}

	// Format attribute memory if it hasn't already
	if (state.GetLabelForPhysicalAddress(kScreenAttrMemStart) == nullptr)
	{
		AddLabel(state, 0x5800, "ScreenAttributes", ELabelType::Data);

		FDataFormattingOptions format;
		format.StartAddress = state.AddressRefFromPhysicalAddress(kScreenAttrMemStart);
		format.DataType = EDataType::ColAttr;
		format.ItemSize = 32;
		format.NoItems = 24;
		FormatData(state, format);
	}
}


//FSpectrumCPUInterface	SpeccyCPUIF;

/* reboot callback */
static void boot_cb(zx_t* sys, zx_type_t type)
{
	zx_desc_t desc = {}; // TODO
	zx_init(sys, &desc);
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
	FSpectrumEmu* pEmu = (FSpectrumEmu*)user_data;
	if(pEmu->GetGlobalConfig()->bEnableAudio)
		saudio_push(samples, num_samples);
}

void	FSpectrumEmu::OnInstructionExecuted(int ticks, uint64_t pins)
{
	FCodeAnalysisState &state = CodeAnalysis;
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
		if(pLabel->LabelType == ELabelType::Function)
			trapId = kCaptureTrapId;
	}
#endif

	PreviousPC = pc;
}

/*
int UIEvalBreakpoint(ui_dbg_t* dbg_win, uint16_t pc, int ticks, uint64_t pins, void* user_data)
{
	return 0;
}*/

uint64_t FSpectrumEmu::Z80Tick(int num, uint64_t pins)
{
	FCodeAnalysisState &state = CodeAnalysis;
	FDebugger& debugger = CodeAnalysis.Debugger;
	z80_t& cpu = ZXEmuState.cpu;
	const uint16_t pc = GetPC().Address;
	static uint64_t lastTickPins = 0;
	const uint64_t risingPins = pins & (pins ^ lastTickPins);
	lastTickPins = pins;
	static uint16_t lastScanlinePos = 0;
	const uint16_t scanlinePos = (uint16_t)ZXEmuState.scanline_y;

	// trigger frame events on scanline pos
	if(scanlinePos != lastScanlinePos)
	{
		if (scanlinePos == 0)	// first scanline
			CodeAnalysis.OnMachineFrameStart();
		if (scanlinePos == ZXEmuState.frame_scan_lines)	// last scanline
			CodeAnalysis.OnMachineFrameEnd();
	}
	lastScanlinePos = scanlinePos;

	/* memory and IO requests */
	if (pins & Z80_MREQ) 
	{
		/* a memory request machine cycle
			FIXME: 'contended memory' accesses should inject wait states
		*/
		const uint16_t addr = Z80_GET_ADDR(pins);
		const uint8_t value = Z80_GET_DATA(pins);
		if (pins & Z80_RD)
		{
			if (risingPins & Z80_INT)	// check if in interrupt - could this be done in the shared code analysis?
			{
				// TODO: read is to fetch interrupt handler address
				//LOGINFO("Interrupt Handler at: %x", value);
				const uint8_t im = cpu.im;

				if (im == 2)
				{
					const uint8_t i = cpu.i;	// I register has high byte of interrupt vector
					const uint16_t interruptVector = (i << 8) | value;
					const uint16_t interruptHandler = state.CPUInterface->ReadWord(interruptVector);
					bHasInterruptHandler = true;
					InterruptHandlerAddress = interruptHandler;
				}

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
			
			if (addr >= kScreenPixMemStart && addr <= kScreenPixMemEnd)
			{
				debugger.RegisterEvent((uint8_t)EEventType::ScreenPixWrite, pcAddrRef, addr, value, scanlinePos);
			}
			else if (addr >= kScreenAttrMemStart && addr < kScreenAttrMemEnd)
			{
				debugger.RegisterEvent((uint8_t)EEventType::ScreenAttrWrite, pcAddrRef, addr, value, scanlinePos);
			}
		}
	}

	// Handle IO operations
	if (pins & Z80_IORQ)
	{
		const FAddressRef pcAddrRef = state.AddressRefFromPhysicalAddress(pc);
		const uint8_t data = Z80_GET_DATA(pins);
		const uint16_t addr = Z80_GET_ADDR(pins);

		//IOAnalysis.IOHandler(pc, pins);

		if (pins & Z80_RD)
		{
			if ((pins & Z80_A0) == 0)
			{
				debugger.RegisterEvent((uint8_t)EEventType::KeyboardRead, pcAddrRef, addr , data, scanlinePos);
				Keyboard.RegisterKeyboardRead(pcAddrRef,addr,data);
			}
			else if ((pins & (Z80_A7 | Z80_A6 | Z80_A5)) == 0) // Kempston Joystick (........000.....)
			{
				debugger.RegisterEvent((uint8_t)EEventType::KempstonJoystickRead, pcAddrRef, addr, data, scanlinePos);
			}
			else if (pins & 0xff)
			{
				debugger.RegisterEvent((uint8_t)EEventType::FloatingBusRead, pcAddrRef, addr, data, scanlinePos);
			}
			// 128K specific
			else if (ZXEmuState.type == ZX_TYPE_128)
			{
				if ((pins & (Z80_A15 | Z80_A14 | Z80_A1)) == (Z80_A15 | Z80_A14))
					debugger.RegisterEvent((uint8_t)EEventType::SoundChipRead, pcAddrRef, addr, data, scanlinePos);
			}
		}
		else if (pins & Z80_WR)
		{
			// handle bank switching on speccy 128
			if ((pins & Z80_A0) == 0)
			{
				static uint8_t LastFE = 0;
				// Spectrum ULA (...............0)

				// has border colour changed?
				if ((data & 7) != (LastFE & 7))
					debugger.RegisterEvent((uint8_t)EEventType::SetBorderColour, pcAddrRef, Z80_GET_ADDR(pins), data, scanlinePos);

				// has beeper changed
				if ((data & (1 << 4)) != (LastFE & (1 << 4)))
				{
					debugger.RegisterEvent((uint8_t)EEventType::OutputBeeper, pcAddrRef, Z80_GET_ADDR(pins), data, scanlinePos);
					Beeper.RegisterBeeperWrite(pcAddrRef,data);
				}

				// has mic output changed
				if ((data & (1 << 3)) != (LastFE & (1 << 3)))
					debugger.RegisterEvent((uint8_t)EEventType::OutputMic, pcAddrRef, Z80_GET_ADDR(pins), data, scanlinePos);

				LastFE = data;
			}
			else if (ZXEmuState.type == ZX_TYPE_128)
			{
				if ((pins & (Z80_A15 | Z80_A1)) == 0)
				{
					if (!ZXEmuState.memory_paging_disabled)
					{
						debugger.RegisterEvent((uint8_t)EEventType::SwitchMemoryBanks, pcAddrRef, Z80_GET_ADDR(pins), data, scanlinePos);

						const int ramBank = data & 0x7;
						const int romBank = (data & (1 << 4)) ? 1 : 0;
						const int displayRamBank = (data & (1 << 3)) ? 7 : 5;

						SetROMBank(romBank);
						SetRAMBank(3, ramBank);

						MemoryControl.RegisterMemoryConfigWrite(pcAddrRef, data);
					}
				}
				else if ((pins & (Z80_A15 | Z80_A14 | Z80_A1)) == (Z80_A15 | Z80_A14))	// select AY-3-8912 register (11............0.)
				{
					debugger.RegisterEvent((uint8_t)EEventType::SoundChipRegisterSelect, pcAddrRef, addr, data, scanlinePos);
					AYSoundChip.SelectAYRegister(pcAddrRef, data);
				}
				else if ((pins & (Z80_A15 | Z80_A14 | Z80_A1)) == Z80_A15)	// write to AY-3-8912 (10............0.) 
				{
					debugger.RegisterEvent((uint8_t)EEventType::SoundChipRegisterWrite, pcAddrRef, addr, data, scanlinePos);
					AYSoundChip.WriteAYRegister(pcAddrRef, data);
				}
			}
		}
	}

	InstructionsTicks++;

	const bool bNewOp = z80_opdone(&ZXEmuState.cpu);

	if (bNewOp)
	{
		OnInstructionExecuted(InstructionsTicks, pins);
		InstructionsTicks = 0;
	}

	CodeAnalysis.OnCPUTick(pins);
	//debugger.CPUTick(pins);
	return pins;
}

static uint64_t Z80TickThunk(int num, uint64_t pins, void* user_data)
{
	FSpectrumEmu* pEmu = (FSpectrumEmu*)user_data;
	return pEmu->Z80Tick(num, pins);
}

// Bank is ROM bank 0 or 1
// this is always slot 0
void FSpectrumEmu::SetROMBank(int bankNo)
{
	const int16_t bankId = ROMBanks[bankNo];
	if (CurROMBank == bankId)
		return;
	// Unmap old bank
	//CodeAnalysis.UnMapBank(CurROMBank, 0);
	CodeAnalysis.MapBank(bankId, 0);
	CurROMBank = bankId;
}

// Slot is physical 16K memory region (0-3) 
// Bank is a 16K Spectrum RAM bank (0-7)
void FSpectrumEmu::SetRAMBank(int slot, int bankNo)
{
	const int16_t bankId = RAMBanks[bankNo];
	if (CurRAMBank[slot] == bankId)
		return;

	// Unmap old bank
	const int startPage = slot * kNoBankPages;
	//CodeAnalysis.UnMapBank(CurRAMBank[slot], startPage);
	CodeAnalysis.MapBank(bankId, startPage, EBankAccess::ReadWrite);

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
	FSpectrumEmu* pEmu = (FSpectrumEmu*)user_data;
	pEmu->Z80Tick(0, pins);
}

// keyboard/port LUT
static std::map<uint16_t, std::vector<std::string>> g_KeyPortLUT =
{
	{0xfefe, {"Shift","Z","X","C","V"}},
	{0xfdfe, {"A","S","D","F","G"}},
	{0xfbfe, {"Q","W","E","R","T"}},
	{0xf7fe, {"1","2","3","4","5"}},
	{0xeffe, {"0","9","8","7","6"}},
	{0xdffe, {"P","O","I","U","Y"}},
	{0xbffe, {"Enter","L","K","J","H"}},
	{0x7ffe, {"Space","Sym","M","N","B"}},
};

// keyboard/port LUT
static std::vector<std::pair<uint16_t, const char*>> g_IOReadPortNames =
{
	{0xfefe, "Keys: Shift,Z,X,C,V"},
	{0xfdfe, "Keys: A,S,D,F,G"},
	{0xfbfe, "Keys: Q,W,E,R,T"},
	{0xf7fe, "Keys: 1,2,3,4,5"},
	{0xeffe, "Keys: 0,9,8,7,6"},
	{0xdffe, "Keys: P,O,I,U,Y"},
	{0xbffe, "Keys: Enter,L,K,J,H"},
	{0x7ffe, "Keys: Space,Sym,M,N,B"},
};

static std::vector<std::pair<uint16_t, const char*>> g_IOWritePortNames =
{
};




// Event viewer address/value visualisers - move somewhere?
void IOPortEventShowAddress(FCodeAnalysisState& state, const FEvent& event)
{
	if (event.Type == (int)EEventType::KeyboardRead)
	{
		const auto& portRow = g_KeyPortLUT.find(event.Address);
		if (portRow != g_KeyPortLUT.end())
		{
			ImGui::Text("Row:");
			for (const auto& key : portRow->second)
			{
				ImGui::SameLine();
				ImGui::Text("%s", key.c_str());
			}
		}
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
		if ((event.Value & 0x1f) == 0x1f)	// no key down
		{
			ImGui::Text("No Keys");
		}
		else
		{
			const auto& portRow = g_KeyPortLUT.find(event.Address);
			if (portRow != g_KeyPortLUT.end())
			{
				for (int i = 0; i < 5; i++)
				{
					if ((event.Value & (1 << i)) == 0)
					{
						ImGui::SameLine();
						ImGui::Text("%s", portRow->second[i].c_str());
					}
				}
			}
		}
	}
	else if (event.Type == (int)EEventType::SwitchMemoryBanks)
	{
		ImGui::Text("RAM:%d ROM:%d", event.Value & 0x7, (event.Value >> 4) & 1);
	}
	else if (event.Type == (int)EEventType::SoundChipRegisterSelect)
	{
		ImGui::Text("%s", g_AYRegNames[event.Value & 15]);
	}
	else
	{
		ImGui::Text("%s", NumStr(event.Value));
	}
}

// This shgould be callable after initialisation
bool FSpectrumEmu::InitForModel(ESpectrumModel model)
{
    // setup emu
    zx_type_t type = model == ESpectrumModel::Spectrum128K ? ZX_TYPE_128 : ZX_TYPE_48K;
    zx_joystick_type_t joy_type = ZX_JOYSTICKTYPE_NONE;

    zx_desc_t desc;
    memset(&desc, 0, sizeof(zx_desc_t));
    desc.type = type;
    desc.joystick_type = joy_type;
 
    // audio
    desc.audio.callback.func = PushAudio;    // our audio callback
    desc.audio.callback.user_data = this;
    desc.audio.sample_rate = saudio_sample_rate();
    
    // roms
    desc.roms.zx48k.ptr = dump_amstrad_zx48k_bin;
    desc.roms.zx48k.size = sizeof(dump_amstrad_zx48k_bin);
    desc.roms.zx128_0.ptr = dump_amstrad_zx128k_0_bin;
    desc.roms.zx128_0.size = sizeof(dump_amstrad_zx128k_0_bin);
    desc.roms.zx128_1.ptr = dump_amstrad_zx128k_1_bin;
    desc.roms.zx128_1.size = sizeof(dump_amstrad_zx128k_1_bin);

    // setup debug hook
    desc.debug.callback.func = DebugCB;
    desc.debug.callback.user_data = this;
    desc.debug.stopped = CodeAnalysis.Debugger.GetDebuggerStoppedPtr();

    zx_init(&ZXEmuState, &desc);
    
    // Clear UI
   /* memset(&UIZX, 0, sizeof(ui_zx_t));

    {
        ui_zx_desc_t desc = { 0 };
        desc.zx = &ZXEmuState;
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
        ui_zx_init(&UIZX, &desc);
    }
    */
    for (int bankNo = 0; bankNo < kNoRAMBanks; bankNo++)
        CodeAnalysis.GetBank(RAMBanks[bankNo])->PrimaryMappedPage = 48;
    
    // Setup initial machine memory config
    if (model == ESpectrumModel::Spectrum48K)
    {
		CodeAnalysis.SetBankPrimaryPage(RAMBanks[0], 16);
		CodeAnalysis.SetBankPrimaryPage(RAMBanks[1], 32);
		CodeAnalysis.SetBankPrimaryPage(RAMBanks[2], 48);
        //CodeAnalysis.GetBank(RAMBanks[0])->PrimaryMappedPage = 16;
        //CodeAnalysis.GetBank(RAMBanks[1])->PrimaryMappedPage = 32;
        //CodeAnalysis.GetBank(RAMBanks[2])->PrimaryMappedPage = 48;

        SetROMBank(0);
        SetRAMBank(1, 0);    // 0x4000 - 0x7fff
        SetRAMBank(2, 1);    // 0x8000 - 0xBfff
        SetRAMBank(3, 2);    // 0xc000 - 0xffff

        // Setup memory description handlers
        PixMemDescGenerator.SetRegionBankId(RAMBanks[0]);
        AttrMemDescGenerator.SetRegionBankId(RAMBanks[0]);
    }
    else
    {
		CodeAnalysis.SetBankPrimaryPage(RAMBanks[5], 16);
		CodeAnalysis.SetBankPrimaryPage(RAMBanks[2], 32);
		CodeAnalysis.SetBankPrimaryPage(RAMBanks[1], 48);
        //CodeAnalysis.GetBank(RAMBanks[5])->PrimaryMappedPage = 16;
        //CodeAnalysis.GetBank(RAMBanks[2])->PrimaryMappedPage = 32;
        //CodeAnalysis.GetBank(RAMBanks[0])->PrimaryMappedPage = 48;

        SetROMBank(0);
        SetRAMBank(1, 5);    // 0x4000 - 0x7fff
        SetRAMBank(2, 2);    // 0x8000 - 0xBfff
        SetRAMBank(3, 0);    // 0xc000 - 0xffff

        // Setup memory description handlers
        PixMemDescGenerator.SetRegionBankId(RAMBanks[5]);
        AttrMemDescGenerator.SetRegionBankId(RAMBanks[5]);
    }

    CodeAnalysis.Config.bShowBanks = model == ESpectrumModel::Spectrum128K;
    
    return true;
}

bool FSpectrumEmu::Init(const FEmulatorLaunchConfig& config)
{
	FEmuBase::Init(config);
	
	const FSpectrumLaunchConfig& spectrumLaunchConfig = (const FSpectrumLaunchConfig&)config;
    
	SetWindowTitle(kAppTitle.c_str());
	SetWindowIcon(GetBundlePath("SALogo.png"));

	// Initialise Emulator
	pGlobalConfig = new FZXSpectrumConfig();
    pGlobalConfig->Init();
	pGlobalConfig->Load(kGlobalConfigFilename);
	CodeAnalysis.SetGlobalConfig(pGlobalConfig);
	//FGlobalConfig& globalConfig = GetGlobalConfig();
	SetHexNumberDisplayMode(pGlobalConfig->NumberDisplayMode);
	SetNumberDisplayMode(pGlobalConfig->NumberDisplayMode);
	CodeAnalysis.Config.CharacterColourLUT = FZXGraphicsView::GetColourLUT();
	
	// set supported bitmap format
	CodeAnalysis.Config.bSupportedBitmapTypes[(int)EBitmapFormat::Bitmap_1Bpp] = true;
	for (int i = 0; i < FCodeAnalysisState::kNoViewStates; i++)
	{
		CodeAnalysis.ViewState[i].CurBitmapFormat = EBitmapFormat::Bitmap_1Bpp;
	}

	
	const FZXSpectrumConfig* pSpectrumConfig = GetZXSpectrumGlobalConfig();
	//GameLoader.Init(this);
	//GamesList.SetLoader(&GameLoader);
	//GamesList.EnumerateGames(pSpectrumConfig->SnapshotFolder.c_str());
	AddGamesList("Snapshot File", GetZXSpectrumGlobalConfig()->SnapshotFolder.c_str());

	//RZXManager.Init(this);
	//RZXGamesList.SetLoader(&GameLoader);
#if ENABLE_RZX
	AddGamesList("RZX File", GetZXSpectrumGlobalConfig()->RZXFolder.c_str());
#endif
    
	// This is where we add the viewers we want
	AddViewer(new FOverviewViewer(this));
	pCharacterMapViewer = new FCharacterMapViewer(this);
	AddViewer(pCharacterMapViewer);
	pCharacterMapViewer->SetGridSize(32,24);
	pGraphicsViewer = new FZXGraphicsViewer(this);
	AddViewer(pGraphicsViewer);

	SpectrumViewer.Init(this);
	FrameTraceViewer.Init(this);

	CodeAnalysis.ViewState[0].Enabled = true;	// always have first view enabled

	// register Viewers
	RegisterStarquakeViewer(this);
	RegisterGames(this);

	LoadZXSpectrumGameConfigs(this);

	// create & register ROM banks
	for (int bankNo = 0; bankNo < kNoROMBanks; bankNo++)
	{
		char bankName[32];
		snprintf(bankName,32, "ROM %d", bankNo);
		ROMBanks[bankNo] = CodeAnalysis.CreateBank(bankName, 16,ZXEmuState.rom[bankNo], true, 0x0000);
		//CodeAnalysis.GetBank(ROMBanks[bankNo])->PrimaryMappedPage = 0;
	}

	// create & register RAM banks
	for (int bankNo = 0; bankNo < kNoRAMBanks; bankNo++)
	{
		char bankName[32];
		snprintf(bankName, 32, "RAM %d", bankNo);
		RAMBanks[bankNo] = CodeAnalysis.CreateBank(bankName, 16, ZXEmuState.ram[bankNo], false, 0xC000);
		//CodeAnalysis.GetBank(RAMBanks[bankNo])->PrimaryMappedPage = 48;
	}
    
    if(InitForModel(spectrumLaunchConfig.Model) == false)
        return false;

	// load the command line game if none specified then load the last game
	bool bLoadedGame = false;

	if (config.SpecificGame.empty() == false)
	{
		bLoadedGame = StartGameFromName(config.SpecificGame.c_str(), true);
	}
	else if (pGlobalConfig->LastGame.empty() == false)
	{
		bLoadedGame = StartGameFromName(pGlobalConfig->LastGame.c_str(), true);
	}
	
	// Start ROM if no game has been loaded
	if(bLoadedGame == false)
	{
		std::string romJsonFName = kRomInfo48JsonFile;

		if (spectrumLaunchConfig.Model == ESpectrumModel::Spectrum128K)
			romJsonFName = kRomInfo128JsonFile;

		CodeAnalysis.Init(this);

		if (FileExists(GetBundlePath(romJsonFName.c_str())))
			ImportAnalysisJson(CodeAnalysis, GetBundlePath(romJsonFName.c_str()));
	}
	else
	{
		if (spectrumLaunchConfig.SkoolkitImport.empty() == false && config.SpecificGame.empty() == false)
			ImportSkoolFile(this, spectrumLaunchConfig.SkoolkitImport.c_str(), nullptr, nullptr);
	}

	// Setup Debugger
	FDebugger& debugger = CodeAnalysis.Debugger;
	//debugger.RegisterEventType((int)EEventType::None, "None", 0);
	debugger.RegisterEventType((int)EEventType::ScreenPixWrite, "Screen Pixel Write", 0xff0000ff, nullptr, EventShowPixValue);
	debugger.RegisterEventType((int)EEventType::ScreenAttrWrite, "Screen Attr Write", 0xff007fff, nullptr, EventShowAttrValue);
	debugger.RegisterEventType((int)EEventType::KeyboardRead, "Keyboard Read", 0xff00ff1f, IOPortEventShowAddress, IOPortEventShowValue);
	debugger.RegisterEventType((int)EEventType::KempstonJoystickRead, "Kempston Read", 0xff007f1f, IOPortEventShowAddress, IOPortEventShowValue);
	debugger.RegisterEventType((int)EEventType::FloatingBusRead, "Floating Bus Read", 0xff407f00, IOPortEventShowAddress, IOPortEventShowValue);
	debugger.RegisterEventType((int)EEventType::SoundChipRead, "AY Chip Read", 0xff007f40, IOPortEventShowAddress, IOPortEventShowValue);
	debugger.RegisterEventType((int)EEventType::SoundChipRegisterSelect, "AY Register Select", 0xff007f7f, IOPortEventShowAddress, IOPortEventShowValue);
	debugger.RegisterEventType((int)EEventType::SoundChipRegisterWrite, "AY Register Write", 0xff00ffff, IOPortEventShowAddress, IOPortEventShowValue);
	debugger.RegisterEventType((int)EEventType::SwitchMemoryBanks, "Switch Memory Banks", 0xffff00ff, IOPortEventShowAddress, IOPortEventShowValue);
	debugger.RegisterEventType((int)EEventType::SetBorderColour, "Set Border Colour", 0xff003f1f, IOPortEventShowAddress, IOPortEventShowValue);
	debugger.RegisterEventType((int)EEventType::OutputBeeper, "Output Beeper", 0xff0000ff, IOPortEventShowAddress, IOPortEventShowValue);
	debugger.RegisterEventType((int)EEventType::OutputMic, "Output Mic", 0xff0000ff, IOPortEventShowAddress, IOPortEventShowValue);

	// Setup Memory Analyser
	CodeAnalysis.MemoryAnalyser.AddROMArea(kROMStart, kROMEnd);
	CodeAnalysis.MemoryAnalyser.SetScreenMemoryArea(kScreenPixMemStart, kScreenAttrMemEnd);

	// Setup IO analyser
	Keyboard.Init(&ZXEmuState.kbd);
	CodeAnalysis.IOAnalyser.AddDevice(&Keyboard);
	Beeper.Init(&ZXEmuState.beeper);
	CodeAnalysis.IOAnalyser.AddDevice(&Beeper);
	//if (spectrumLaunchConfig.Model == ESpectrumModel::Spectrum128K)
	{
		AYSoundChip.Init(&ZXEmuState.ay);
		CodeAnalysis.IOAnalyser.AddDevice(&AYSoundChip);
		CodeAnalysis.IOAnalyser.AddDevice(&MemoryControl);
	}
    
    AddMemoryRegionDescGenerator(&PixMemDescGenerator);
    AddMemoryRegionDescGenerator(&AttrMemDescGenerator);

	// default assembler settings
	AssemblerExportStartAddress = kScreenAttrMemEnd + 1;
	AssemblerExportEndAddress = 0xffff;
    
    bInitialised = true;
	return true;
}

void FSpectrumEmu::Shutdown()
{
	FEmuBase::Shutdown();
	
	if (RZXManager.GetReplayMode() == EReplayMode::Off)
		SaveProject();	// save on close

	// Save Global Config - move to function?
	if (pActiveGame != nullptr)
		pGlobalConfig->LastGame = pActiveGame->pConfig->Name;

	pGlobalConfig->NumberDisplayMode = GetNumberDisplayMode();
	pGlobalConfig->bShowOpcodeValues = CodeAnalysis.pGlobalConfig->bShowOpcodeValues;
	pGlobalConfig->BranchLinesDisplayMode = CodeAnalysis.pGlobalConfig->BranchLinesDisplayMode;

	pGlobalConfig->Save(kGlobalConfigFilename);

	//GraphicsViewer.Shutdown();
}

bool FSpectrumEmu::LoadProject(FProjectConfig* pGameConfig, bool bLoadGameData /* =  true*/)
{
	assert(pGameConfig != nullptr);
	FZXSpectrumGameConfig *pSpectrumGameConfig = (FZXSpectrumGameConfig*)pGameConfig;
	
	// reset systems
	MemoryAccessHandlers.clear();	// remove old memory handlers
	ResetMemoryStats(MemStats);
	FrameTraceViewer.Reset();
	pGraphicsViewer->Reset();

	const std::string windowTitle = kAppTitle + " - " + pGameConfig->Name;
	SetWindowTitle(windowTitle.c_str());
	
	// start up game
	if(pActiveGame!=nullptr)
		delete pActiveGame->pViewerData;
	delete pActiveGame;
	pActiveGame = nullptr;
	
    // This stuff is a bit legacy
	FGame* pNewGame = new FGame;
	//pSpectrumGameConfig->Spectrum128KGame = ZXEmuState.type == ZX_TYPE_128;
	pNewGame->pConfig = pGameConfig;
	pNewGame->pViewerConfig = pSpectrumGameConfig->pViewerConfig;
	assert(pSpectrumGameConfig->pViewerConfig != nullptr);
	pActiveGame = pNewGame;
	pNewGame->pViewerData = pNewGame->pViewerConfig->pInitFunction(this, pSpectrumGameConfig);
	GenerateSpriteListsFromConfig(*(FZXGraphicsViewer*)pGraphicsViewer, pSpectrumGameConfig);

    InitForModel(pSpectrumGameConfig->Spectrum128KGame ? ESpectrumModel::Spectrum128K : ESpectrumModel::Spectrum48K);
	// Initialise code analysis
	CodeAnalysis.Init(this);

	// Set options from config
	for (int i = 0; i < FCodeAnalysisState::kNoViewStates; i++)
	{
		CodeAnalysis.ViewState[i].Enabled = pGameConfig->ViewConfigs[i].bEnabled;
		CodeAnalysis.ViewState[i].GoToAddress(pGameConfig->ViewConfigs[i].ViewAddress);
	}

	bool bLoadSnapshot = pGameConfig->EmulatorFile.FileName.empty() == false;

	// Are we loading a previously saved game
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
        
        if(pSpectrumGameConfig->GetSpectrumModel() != GetCurrentSpectrumModel())
        {
            InitForModel(pSpectrumGameConfig->GetSpectrumModel());
        }
       
		if (LoadGameState(this, saveStateFName.c_str()))
		{
			// if the game state loaded then we don't need the snapshot
			bLoadSnapshot = false;
		}

		if (FileExists(analysisJsonFName.c_str()))
		{
			ImportAnalysisJson(CodeAnalysis, analysisJsonFName.c_str());
			ImportAnalysisState(CodeAnalysis, analysisStateFName.c_str());
		}

		pGraphicsViewer->LoadGraphicsSets(graphicsSetsJsonFName.c_str());

		// where do we want pokes to live?
		if (pSpectrumGameConfig != nullptr)
			LoadPOKFile(*pSpectrumGameConfig, std::string(GetZXSpectrumGlobalConfig()->PokesFolder + pGameConfig->Name + ".pok").c_str());
	}

	// we always want to load the ROM info even if we aren't loading a previous analysis
	const std::string romJsonFName = (ZXEmuState.type == ZX_TYPE_128) ? kRomInfo128JsonFile : kRomInfo48JsonFile;

	if (FileExists(GetBundlePath(romJsonFName.c_str())))
		ImportAnalysisJson(CodeAnalysis, GetBundlePath(romJsonFName.c_str()));
	
	if (bLoadSnapshot)
	{
		// if the game state didn't load then reload the snapshot
		/*/const FGameSnapshot* snapshot = &CurrentGameSnapshot;//GamesList.GetGame(RemoveFileExtension(pGameConfig->SnapshotFile.c_str()).c_str());
		if (snapshot == nullptr)
		{
			SetLastError("Could not find '%s%s'",pGlobalConfig->SnapshotFolder.c_str(), pGameConfig->SnapshotFile.c_str());
			return false;
		}*/
		if (!LoadEmulatorFile(&pGameConfig->EmulatorFile))
		{
			return false;
		}
	}

	ReAnalyseCode(CodeAnalysis);
	GenerateGlobalInfo(CodeAnalysis);
	FormatSpectrumMemory(CodeAnalysis);
	CodeAnalysis.SetAddressRangeDirty();

	// Start in break mode so the memory will be in its initial state. 
	// Otherwise, if we export a skool/asm file once the game is running the memory could be in an arbitrary state.
	// 
	// decode whole screen
	ZXDecodeScreen(&ZXEmuState);
	CodeAnalysis.Debugger.Break();

	CodeAnalysis.Debugger.RegisterNewStackPointer(ZXEmuState.cpu.sp, FAddressRef());

	// some extra initialisation for creating new analysis from snnapshot
	if(bLoadGameData == false)
	{
		FAddressRef initialPC = CodeAnalysis.AddressRefFromPhysicalAddress(ZXEmuState.cpu.pc);
		SetItemCode(CodeAnalysis, initialPC);
		CodeAnalysis.Debugger.SetPC(initialPC);
	}

	pGraphicsViewer->SetImagesRoot((pGlobalConfig->WorkspaceRoot + "/" + pGameConfig->Name + "/GraphicsSets/").c_str());

	pCurrentProjectConfig = pGameConfig;

	LoadLua();
	return true;
}

bool FSpectrumEmu::LoadLua()
{
	// Setup Lua - reinitialised for each game
	const std::string gameRoot = pGlobalConfig->WorkspaceRoot + pCurrentProjectConfig->Name + "/";
	if (LuaSys::Init(this))
	{
		RegisterSpectrumLuaAPI(LuaSys::GetGlobalState());
		
		//LuaSys::LoadFile(GetBundlePath("Lua/ZXBase.lua"), pGlobalConfig->bEditLuaBaseFiles);

		for(const auto& gameScript : pCurrentProjectConfig->LuaSourceFiles)
		{
			std::string luaScriptFName = gameRoot + gameScript;
			LuaSys::LoadFile(luaScriptFName.c_str(), true);
		}
		return true;
	}

	return false;
}


// save config & data
bool FSpectrumEmu::SaveProject()
{
	if (pActiveGame != nullptr)
	{
		FProjectConfig *pGameConfig = pActiveGame->pConfig;
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

		AddGameConfig(pGameConfig);
		SaveGameConfigToFile(*pGameConfig, configFName.c_str());
		//SaveGameData(this, dataFName.c_str());		// The Past

		// The Future
		SaveGameState(this, saveStateFName.c_str());
		ExportAnalysisJson(CodeAnalysis, analysisJsonFName.c_str());
		ExportAnalysisState(CodeAnalysis, analysisStateFName.c_str());
		pGraphicsViewer->SaveGraphicsSets(graphicsSetsJsonFName.c_str());
	}

	// TODO: this could use
#if	SAVE_ROM_JSON
	const std::string romJsonFName = root + kRomInfoJsonFile;
	ExportAnalysisJson(CodeAnalysis, romJsonFName.c_str(), true);	// export ROMS only
#endif

	return true;
}

bool FSpectrumEmu::LoadEmulatorFile(const FEmulatorFile* pSnapshot)
{
	auto findIt = GamesLists.find(pSnapshot->ListName);
	if(findIt == GamesLists.end())
		return false;

	const std::string fileName = findIt->second.GetRootDir() + pSnapshot->FileName;
	const char* pFileName = fileName.c_str();

	switch (pSnapshot->Type)
	{
	case EEmuFileType::Z80:
		return LoadZ80File(this, pFileName);
	case EEmuFileType::SNA:
		return LoadSNAFile(this, pFileName);
	case EEmuFileType::TAP:
		return LoadTAPFile(this, pFileName);
	case EEmuFileType::TZX:
		return LoadTZXFile(this, pFileName);
	default:
		return false;
	}
}

bool FSpectrumEmu::NewProjectFromEmulatorFile(const FEmulatorFile& snapshot)
{
	// Remove any existing config 
	RemoveGameConfig(snapshot.DisplayName.c_str());

	FZXSpectrumGameConfig* pNewConfig = CreateNewZXGameConfigFromSnapshot(snapshot);

	if (pNewConfig != nullptr)
	{
        if (!LoadProject(pNewConfig, /* bLoadGameData */ false))
            return false;
        pNewConfig->Spectrum128KGame = GetCurrentSpectrumModel() == ESpectrumModel::Spectrum128K;
		pNewConfig->EmulatorFile = snapshot;

        AddGameConfig(pNewConfig);
		SaveProject();

		return true;
	}
	return false;
}


void FSpectrumEmu::FileMenuAdditions(void)	
{
	const FZXSpectrumConfig* pZXGlobalConfig = GetZXSpectrumGlobalConfig();
#if 0
	if (ImGui::BeginMenu("New Game from RZX File"))
	{
		if (RZXGamesList.GetNoGames() == 0)
		{
			ImGui::Text("No RZX files found in RZX directory:\n\n'%s'.\n\nRZX directory is set in GlobalConfig.json", pZXGlobalConfig->RZXFolder.c_str());
		}
		else
		{
			for (int gameNo = 0; gameNo < RZXGamesList.GetNoGames(); gameNo++)
			{
				const FGameSnapshot& game = RZXGamesList.GetGame(gameNo);

				if (ImGui::MenuItem(game.DisplayName.c_str()))
				{
					if (RZXManager.Load(game.FileName.c_str()))
					{
						FZXSpectrumGameConfig* pNewConfig = CreateNewZXGameConfigFromSnapshot(game);
						if (pNewConfig != nullptr)
							StartGame(pNewConfig, true);
					}
				}
			}
		}
		ImGui::EndMenu();
	}
#endif

	if (ImGui::MenuItem("Export Binary File"))
	{
		if (pActiveGame != nullptr)
		{
			const std::string dir = pGlobalConfig->WorkspaceRoot + "OutputBin/";
			EnsureDirectoryExists(dir.c_str());
			std::string outBinFname = dir + pActiveGame->pConfig->Name + ".bin";
			uint8_t* pSpecMem = new uint8_t[65536];
			for (int i = 0; i < 65536; i++)
				pSpecMem[i] = ReadByte(i);
			SaveBinaryFile(outBinFname.c_str(), pSpecMem, 65536);
			delete[] pSpecMem;
		}
	}

	if (ImGui::BeginMenu("Export Skool File"))
	{
		if (ImGui::MenuItem("Export as Hexadecimal"))
		{
			ExportSkoolFile(this, true /* bHexadecimal*/);
		}
		if (ImGui::MenuItem("Export as Decimal"))
		{
			ExportSkoolFile(this, false /* bHexadecimal*/);
		}
#ifndef NDEBUG
		if (ImGui::BeginMenu("DEBUG"))
		{
			if (ImGui::MenuItem("Export ROM"))
			{
				ExportSkoolFile(this, true /* bHexadecimal */, "rom");
			}
			ImGui::EndMenu();
		}
#endif // NDEBUG
		ImGui::EndMenu();
	}

	if (ImGui::MenuItem("Export Region Info File"))
	{
	}
}

void FSpectrumEmu::SystemMenuAdditions(void)
{
	if (ImGui::BeginMenu("Joystick"))
	{
		if (ImGui::MenuItem("None", 0, ZXEmuState.joystick_type == ZX_JOYSTICKTYPE_NONE))
		{
			ZXEmuState.joystick_type = ZX_JOYSTICKTYPE_NONE;
		}
		if (ImGui::MenuItem("Kempston", 0, ZXEmuState.joystick_type == ZX_JOYSTICKTYPE_KEMPSTON))
		{
			ZXEmuState.joystick_type = ZX_JOYSTICKTYPE_KEMPSTON;
		}
		if (ImGui::MenuItem("Sinclair #1", 0, ZXEmuState.joystick_type == ZX_JOYSTICKTYPE_SINCLAIR_1))
		{
			ZXEmuState.joystick_type = ZX_JOYSTICKTYPE_SINCLAIR_1;
		}
		if (ImGui::MenuItem("Sinclair #2", 0, ZXEmuState.joystick_type == ZX_JOYSTICKTYPE_SINCLAIR_2))
		{
			ZXEmuState.joystick_type = ZX_JOYSTICKTYPE_SINCLAIR_2;
		}
		ImGui::EndMenu();
	}
}

void FSpectrumEmu::OptionsMenuAdditions(void)
{
}

void FSpectrumEmu::WindowsMenuAdditions(void)
{
}

#if 0
void FSpectrumEmu::DrawMainMenu(double timeMS)
{
	ui_zx_t* pZXUI = &UIZX;
	assert(pZXUI && pZXUI->zx && pZXUI->boot_cb);

	bExportAsm = false;
	bReplaceGamePopup = false;

	const FZXSpectrumConfig* pZXGlobalConfig = GetZXSpectrumGlobalConfig();


	if (ImGui::BeginMainMenuBar())
	{
		if (ImGui::BeginMenu("File"))
		{
			if (ImGui::BeginMenu("New Game from Snapshot File"))
			{
				const int numGames = GamesList.GetNoGames();
				if (!numGames)
				{
					const std::string snapFolder = ZXEmuState.type == ZX_TYPE_128 ? pZXGlobalConfig->SnapshotFolder128 : pZXGlobalConfig->SnapshotFolder;
					ImGui::Text("No snapshots found in snapshot directory:\n\n'%s'.\n\nSnapshot directory is set in GlobalConfig.json", snapFolder.c_str());
				}
				else
				{
					for (int gameNo = 0; gameNo < numGames; gameNo++)
					{
						const FGameSnapshot& game = GamesList.GetGame(gameNo);

						if (ImGui::MenuItem(game.DisplayName.c_str()))
						{
							bool bGameExists = false;

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
								NewGameFromSnapshot(game);
							}
						}
					}
				}
				ImGui::EndMenu();
			}

#if ENABLE_RZX
			if (ImGui::BeginMenu("New Game from RZX File"))
			{
				if (RZXGamesList.GetNoGames() == 0)
				{
					ImGui::Text("No RZX files found in RZX directory:\n\n'%s'.\n\nRZX directory is set in GlobalConfig.json", pZXGlobalConfig->RZXFolder.c_str());
				}
				else
				{
					for (int gameNo = 0; gameNo < RZXGamesList.GetNoGames(); gameNo++)
					{
						const FGameSnapshot& game = RZXGamesList.GetGame(gameNo);

						if (ImGui::MenuItem(game.DisplayName.c_str()))
						{
							if (RZXManager.Load(game.FileName.c_str()))
							{
								FZXSpectrumGameConfig* pNewConfig = CreateNewZXGameConfigFromSnapshot(game);
								if (pNewConfig != nullptr)
									StartGame(pNewConfig, true);
							}
						}
					}
				}
				ImGui::EndMenu();
			}
#endif
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
							const std::string snapFolder = ZXEmuState.type == ZX_TYPE_128 ? pZXGlobalConfig->SnapshotFolder128 : pZXGlobalConfig->SnapshotFolder;
							const std::string gameFile = snapFolder + pGameConfig->SnapshotFile;

							if (GamesList.LoadGame(gameFile.c_str()))
							{
								StartGame((FZXSpectrumGameConfig*)pGameConfig, true);
							}
						}
					}
				}

				ImGui::EndMenu();
			}

			/*if (ImGui::MenuItem("Open POK File..."))
			{
				std::string pokFile;
				OpenFileDialog(pokFile, ".\\POKFiles", "POK\0*.pok\0");
			}*/
			
			if (RZXManager.GetReplayMode() == EReplayMode::Off)
			{
				if (ImGui::MenuItem("Save Game Data"))
					SaveCurrentGameData();
			}

			if (ImGui::MenuItem("Export Binary File"))
			{
				if (pActiveGame != nullptr)
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
			}

			if (ImGui::MenuItem("Export ASM File"))
			{
				// ImGui popup windows can't be activated from within a Menu so we set a flag to act on outside of the menu code.
				bExportAsm = true;
			}
			
			if (ImGui::BeginMenu("Export Skool File"))
			{
				if (ImGui::MenuItem("Export as Hexadecimal"))
				{
					ExportSkoolFile(true /* bHexadecimal*/);
				}
				if (ImGui::MenuItem("Export as Decimal"))
				{
					ExportSkoolFile(false /* bHexadecimal*/);
				}
#ifndef NDEBUG
				if (ImGui::BeginMenu("DEBUG"))
				{
					if (ImGui::MenuItem("Export ROM"))
					{
						ExportSkoolFile(true /* bHexadecimal */, "rom");
					}
					ImGui::EndMenu();
				}
#endif // NDEBUG
				ImGui::EndMenu();
			}

			if (ImGui::MenuItem("Export Region Info File"))
			{
			}
			ImGui::EndMenu();
		}
		
		if (ImGui::BeginMenu("System")) 
		{
			
			if (pActiveGame && ImGui::MenuItem("Reload Snapshot"))
			{
				//const std::string snapFolder = ZXEmuState.type == ZX_TYPE_128 ? pZXGlobalConfig->SnapshotFolder128 : pZXGlobalConfig->SnapshotFolder;
				//const std::string gameFile = snapFolder + pActiveGame->pConfig->SnapshotFile;
				GamesList.LoadGame(pActiveGame->pConfig->Name.c_str());
			}

			if (ImGui::MenuItem("Reset")) 
			{
				zx_reset(pZXUI->zx);
				ui_dbg_reset(&pZXUI->dbg);
			}
			/*if (ImGui::MenuItem("ZX Spectrum 48K", 0, (pZXUI->zx->type == ZX_TYPE_48K)))
			{
				pZXUI->boot_cb(pZXUI->zx, ZX_TYPE_48K);
				ui_dbg_reboot(&pZXUI->dbg);
			}
			if (ImGui::MenuItem("ZX Spectrum 128", 0, (pZXUI->zx->type == ZX_TYPE_128)))
			{
				pZXUI->boot_cb(pZXUI->zx, ZX_TYPE_128);
				ui_dbg_reboot(&pZXUI->dbg);
			}*/
			if (ImGui::BeginMenu("Joystick")) 
			{
				if (ImGui::MenuItem("None", 0, (pZXUI->zx->joystick_type == ZX_JOYSTICKTYPE_NONE)))
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
				}
				ImGui::EndMenu();
			}
			ImGui::EndMenu();
		}
		if (ImGui::BeginMenu("Hardware")) 
		{
			ImGui::MenuItem("Memory Map", 0, &pZXUI->memmap.open);
			ImGui::MenuItem("Keyboard Matrix", 0, &pZXUI->kbd.open);
			ImGui::MenuItem("Audio Output", 0, &pZXUI->audio.open);
			ImGui::MenuItem("Z80 CPU", 0, &pZXUI->cpu.open);
			if (pZXUI->zx->type == ZX_TYPE_128)
			{
				ImGui::MenuItem("AY-3-8912", 0, &pZXUI->ay.open);
			}
			else 
			{
				pZXUI->ay.open = false;
			}
			ImGui::EndMenu();
		}
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
			ImGui::MenuItem("Scan Line Indicator", 0, &pGlobalConfig->bShowScanLineIndicator);
			ImGui::MenuItem("Enable Audio", 0, &pGlobalConfig->bEnableAudio);
			ImGui::MenuItem("Edit Mode", 0, &CodeAnalysis.bAllowEditing);
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

			//if(pActiveGame!=nullptr)
			//	ImGui::MenuItem("Save Snapshot with game", 0, &pActiveGame->pConfig->WriteSnapshot);

#ifndef NDEBUG
			ImGui::MenuItem("Show Config", 0, &CodeAnalysis.Config.bShowConfigWindow);
			ImGui::MenuItem("ImGui Demo", 0, &bShowImGuiDemo);
			ImGui::MenuItem("ImPlot Demo", 0, &bShowImPlotDemo);
#endif // NDEBUG
			ImGui::EndMenu();
		}
		// Note: this is a WIP menu, it'll be added in when it works properly!
#ifndef NDEBUG
		if (ImGui::BeginMenu("Tools"))
		{
			
			ImGui::EndMenu();
		}
#endif
		if (ImGui::BeginMenu("Windows"))
		{
			ImGui::MenuItem("DebugLog", 0, &bShowDebugLog);
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
#if 0
		if (ImGui::BeginMenu("Debug")) 
		{
			//ImGui::MenuItem("CPU Debugger", 0, &pZXUI->dbg.ui.open);
			//ImGui::MenuItem("Breakpoints", 0, &pZXUI->dbg.ui.show_breakpoints);
			//ImGui::MenuItem("Memory Heatmap", 0, &pZXUI->dbg.ui.show_heatmap);
			/*if (ImGui::BeginMenu("Memory Editor"))
			{
				ImGui::MenuItem("Window #1", 0, &pZXUI->memedit[0].open);
				ImGui::MenuItem("Window #2", 0, &pZXUI->memedit[1].open);
				ImGui::MenuItem("Window #3", 0, &pZXUI->memedit[2].open);
				ImGui::MenuItem("Window #4", 0, &pZXUI->memedit[3].open);
				ImGui::EndMenu();
			}*/
			/*if (ImGui::BeginMenu("Disassembler")) 
			{
				ImGui::MenuItem("Window #1", 0, &pZXUI->dasm[0].open);
				ImGui::MenuItem("Window #2", 0, &pZXUI->dasm[1].open);
				ImGui::MenuItem("Window #3", 0, &pZXUI->dasm[2].open);
				ImGui::MenuItem("Window #4", 0, &pZXUI->dasm[3].open);
				ImGui::EndMenu();
			}*/
			ImGui::EndMenu();
		}
#endif
		/*if (ImGui::BeginMenu("ImGui"))
		{
			ImGui::MenuItem("Show Demo", 0, &pUI->bShowImGuiDemo);
			ImGui::EndMenu();
		}*/
		

		/*if (ImGui::BeginMenu("Game Viewers"))
		{
			for (auto &viewerIt : pUI->GameViewers)
			{
				FGameViewer &viewer = viewerIt.second;
				ImGui::MenuItem(viewerIt.first.c_str(), 0, &viewer.bOpen);
			}
			ImGui::EndMenu();
		}*/
		
		//ui_util_options_menu(timeMS, pZXUI->dbg.dbg.stopped);

		// draw emu timings
		ImGui::SameLine(ImGui::GetWindowWidth() - 120);
		if (pZXUI->dbg.dbg.stopped) 
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

void FSpectrumEmu::DrawExportAsmModalPopup()
{
	if (bExportAsm)
	{
		ImGui::OpenPopup("Export ASM File");
	}
	if (ImGui::BeginPopupModal("Export ASM File", NULL, ImGuiWindowFlags_AlwaysAutoResize))
	{
		static ImU16 addrStart = kScreenAttrMemEnd + 1;
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

void FSpectrumEmu::DrawReplaceGameModalPopup()
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
						NewGameFromSnapshot(game);
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
#endif

#if 0
static void UpdateMemmap(ui_zx_t* ui)
{
	assert(ui && ui->zx);
	ui_memmap_reset(&ui->memmap);
	if (ZX_TYPE_48K == ui->zx->type) 
	{
		ui_memmap_layer(&ui->memmap, "System");
		ui_memmap_region(&ui->memmap, "ROM", 0x0000, 0x4000, true);
		ui_memmap_region(&ui->memmap, "RAM", 0x4000, 0xC000, true);
	}
	else 
	{
		const uint8_t m = ui->zx->last_mem_config;
		ui_memmap_layer(&ui->memmap, "Layer 0");
		ui_memmap_region(&ui->memmap, "ZX128 ROM", 0x0000, 0x4000, !(m & (1 << 4)));
		ui_memmap_region(&ui->memmap, "RAM 5", 0x4000, 0x4000, true);
		ui_memmap_region(&ui->memmap, "RAM 2", 0x8000, 0x4000, true);
		ui_memmap_region(&ui->memmap, "RAM 0", 0xC000, 0x4000, 0 == (m & 7));
		ui_memmap_layer(&ui->memmap, "Layer 1");
		ui_memmap_region(&ui->memmap, "ZX48K ROM", 0x0000, 0x4000, 0 != (m & (1 << 4)));
		ui_memmap_region(&ui->memmap, "RAM 1", 0xC000, 0x4000, 1 == (m & 7));
		ui_memmap_layer(&ui->memmap, "Layer 2");
		ui_memmap_region(&ui->memmap, "RAM 2", 0xC000, 0x4000, 2 == (m & 7));
		ui_memmap_layer(&ui->memmap, "Layer 3");
		ui_memmap_region(&ui->memmap, "RAM 3", 0xC000, 0x4000, 3 == (m & 7));
		ui_memmap_layer(&ui->memmap, "Layer 4");
		ui_memmap_region(&ui->memmap, "RAM 4", 0xC000, 0x4000, 4 == (m & 7));
		ui_memmap_layer(&ui->memmap, "Layer 5");
		ui_memmap_region(&ui->memmap, "RAM 5", 0xC000, 0x4000, 5 == (m & 7));
		ui_memmap_layer(&ui->memmap, "Layer 6");
		ui_memmap_region(&ui->memmap, "RAM 6", 0xC000, 0x4000, 6 == (m & 7));
		ui_memmap_layer(&ui->memmap, "Layer 7");
		ui_memmap_region(&ui->memmap, "RAM 7", 0xC000, 0x4000, 7 == (m & 7));
	}
}
#endif
void StoreRegisters_Z80(FCodeAnalysisState& state);

bool GetIOInputFunc(uint16_t port, uint8_t* pInVal, void* pUserData)
{
	FSpectrumEmu* pEmu = (FSpectrumEmu*)pUserData;
	return pEmu->RZXManager.GetInput(port, *pInVal);
}


void FSpectrumEmu::Tick()
{
	FEmuBase::Tick();

	FDebugger& debugger = CodeAnalysis.Debugger;

	SpectrumViewer.Tick();

	if (debugger.IsStopped() == false)
	{
		const float frameTime = std::min(1000000.0f / ImGui::GetIO().Framerate, 32000.0f) * ExecSpeedScale;
		//const float frameTime = min(1000000.0f / 50, 32000.0f) * ExecSpeedScale;
		const uint32_t microSeconds = std::max(static_cast<uint32_t>(frameTime), uint32_t(1));

		CodeAnalysis.OnFrameStart();
		StoreRegisters_Z80(CodeAnalysis);
#if ENABLE_CAPTURES
		const uint32_t ticks_to_run = clk_ticks_to_run(&ZXEmuState.clk, microSeconds);
		uint32_t ticks_executed = 0;
		while (UIZX.dbg.dbg.z80->trap_id != kCaptureTrapId && ticks_executed < ticks_to_run)
		{
			ticks_executed += z80_exec(&ZXEmuState.cpu, ticks_to_run - ticks_executed);

			if (UIZX.dbg.dbg.z80->trap_id == kCaptureTrapId)
			{
				const uint16_t PC = GetPC();
				FMachineState* pMachineState = CodeAnalysis.GetMachineState(PC);
				if (pMachineState == nullptr)
				{
					pMachineState = AllocateMachineState(CodeAnalysis);
					CodeAnalysis.SetMachineStateForAddress(PC, pMachineState);
				}

				CaptureMachineState(pMachineState, this);
				UIZX.dbg.dbg.z80->trap_id = 0;
				_ui_dbg_continue(&UIZX.dbg);
			}
		}
		clk_ticks_executed(&ZXEmuState.clk, ticks_executed);
		kbd_update(&ZXEmuState.kbd);
#else
		if (RZXManager.GetReplayMode() == EReplayMode::Playback)
		{
			if (RZXFetchesRemaining <= 0)
				RZXFetchesRemaining += RZXManager.Update();
			const uint32_t fetchesProcessed = ZXExeEmu_UseFetchCount(&ZXEmuState, RZXFetchesRemaining, GetIOInputFunc, this);
			RZXFetchesRemaining -= fetchesProcessed;
		}
		else
		{
			ZXExeEmu(&ZXEmuState, microSeconds);
		}
#endif
		/*if (RZXManager.GetReplayMode() == EReplayMode::Playback)
		{
			assert(ZXEmuState.valid);
			uint32_t icount = RZXManager.Update();

			uint32_t ticks_to_run = clk_ticks_to_run(&ZXEmuState.clk, microSeconds);
			uint32_t ticks_executed = z80_exec(&ZXEmuState.cpu, ticks_to_run);
			clk_ticks_executed(&ZXEmuState.clk, ticks_executed);
			kbd_update(&ZXEmuState.kbd);
		}
		else
		{
			uint32_t frameTicks = ZXEmuState.frame_scan_lines* ZXEmuState.scanline_period;
			//zx_exec(&ZXEmuState, microSeconds);

			//uint32_t ticks_to_run = clk_ticks_to_run(&ZXEmuState.clk, microSeconds);
			//frameTicks = ticks_to_run;
			ZXEmuState.clk.ticks_to_run = frameTicks;
			const uint32_t ticksExecuted = z80_exec(&ZXEmuState.cpu, frameTicks);
			clk_ticks_executed(&ZXEmuState.clk, ticksExecuted);
			kbd_update(&ZXEmuState.kbd);
		}*/
		FrameTraceViewer.CaptureFrame();
		//FrameScreenPixWrites.clear();
		//FrameScreenAttrWrites.clear();
		CodeAnalysis.OnFrameEnd();
	}

	//UpdateCharacterSets(CodeAnalysis);

	// Draw UI
	DrawDockingView();
}

void FSpectrumEmu::Reset()
{
	// Reset speccy
	zx_reset(&ZXEmuState);
	//ui_dbg_reset(&pZXUI->dbg);

	FZXSpectrumGameConfig* pBasicConfig = (FZXSpectrumGameConfig * )GetGameConfigForName("ZXBasic");
	
	if(pBasicConfig == nullptr)
		pBasicConfig = CreateNewZXBasicConfig();

	LoadProject(pBasicConfig,false);	// reset code analysis
}

void    FSpectrumEmu::OnEnterEditMode(void)
{
    zx_save_snapshot(&ZXEmuState,&BackupState);
}

void    FSpectrumEmu::OnExitEditMode(void)
{
    zx_load_snapshot(&ZXEmuState, ZX_SNAPSHOT_VERSION, &BackupState);
}


void FSpectrumEmu::DrawMemoryTools()
{
	if (ImGui::Begin("Memory Tools") == false)
	{
		ImGui::End();
		return;
	}
	if (ImGui::BeginTabBar("MemoryToolsTabBar"))
	{

		if (ImGui::BeginTabItem("Memory Handlers"))
		{
			DrawMemoryHandlers(this);
			ImGui::EndTabItem();
		}

		if (ImGui::BeginTabItem("Memory Analysis"))
		{
			DrawMemoryAnalysis(this);
			ImGui::EndTabItem();
		}
		if (ImGui::BeginTabItem("Memory Diff"))
		{
			DrawMemoryDiffUI(this);
			ImGui::EndTabItem();
		}
		if (ImGui::BeginTabItem("IO Analysis"))
		{
			//IOAnalysis.DrawUI();
			ImGui::EndTabItem();
		}
		
		/*if (ImGui::BeginTabItem("Functions"))
		{
			DrawFunctionInfo(pUI);
			ImGui::EndTabItem();
		}*/

		ImGui::EndTabBar();
	}

	ImGui::End();
}



void FSpectrumEmu::DrawEmulatorUI()
{
	//ui_zx_t* pZXUI = &UIZX;
	const double timeMS = 1000.0f / ImGui::GetIO().Framerate;
	
	//DrawMainMenu(timeMS);
	/*
	if (pZXUI->memmap.open)
	{
		UpdateMemmap(pZXUI);
	}

	// call the Chips UI functions
	ui_audio_draw(&pZXUI->audio, pZXUI->zx->audio.sample_pos);
	ui_z80_draw(&pZXUI->cpu);
	ui_ay38910_draw(&pZXUI->ay);
	ui_kbd_draw(&pZXUI->kbd);
	ui_memmap_draw(&pZXUI->memmap);
	*/
	// Draw registered viewers
	/*for (auto Viewer : Viewers)
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

	if (ImGui::Begin("IO Analyser"))
	{
		CodeAnalysis.IOAnalyser.DrawUI();
	}
	ImGui::End();
	*/

	// show spectrum window
	if (ImGui::Begin("Spectrum View"))
	{
		SpectrumViewer.Draw();
	}
	ImGui::End();

	if (ImGui::Begin("Frame Trace"))
	{
		FrameTraceViewer.Draw();
	}
	ImGui::End();

	if (RZXManager.GetReplayMode() == EReplayMode::Playback)
	{
		if (ImGui::Begin("RZX Info"))
		{
			RZXManager.DrawUI();
		}
		ImGui::End();
	}

	// cheats 
	if (ImGui::Begin("Pokes"))
	{
		DrawCheatsUI();
	}
	ImGui::End();

	// game viewer
	if (ImGui::Begin("Game Viewer"))
	{
		if (pActiveGame != nullptr)
		{
			ImGui::Text("%s",pActiveGame->pConfig->Name.c_str());
			pActiveGame->pViewerConfig->pDrawFunction(this, pActiveGame);
		}
		
	}
	ImGui::End();

#ifndef NDEBUG
	// config
	if (CodeAnalysis.Config.bShowConfigWindow)
	{
		if(ImGui::Begin("Configuration", &CodeAnalysis.Config.bShowConfigWindow))
			DrawCodeAnalysisConfigWindow(CodeAnalysis);
		ImGui::End();
	}
#endif

	//GraphicsViewer.Draw();
	//DrawMemoryTools();

#if 0
	// COde analysis views
	for (int codeAnalysisNo = 0; codeAnalysisNo < FCodeAnalysisState::kNoViewStates; codeAnalysisNo++)
	{
		char name[32];
		sprintf(name, "Code Analysis %d", codeAnalysisNo + 1);
		if(CodeAnalysis.ViewState[codeAnalysisNo].Enabled)
		{
			if (ImGui::Begin(name,&CodeAnalysis.ViewState[codeAnalysisNo].Enabled))
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
#endif

	//if (bShowDebugLog)
	//	g_ImGuiLog.Draw("Debug Log", &bShowDebugLog);
}

// Cheats
// TODO: move this out to another file/class
void FSpectrumEmu::DrawCheatsUI()
{
	if (pActiveGame == nullptr)
		return;
	
	FZXSpectrumGameConfig& config = *(FZXSpectrumGameConfig*)pActiveGame->pConfig;

	if (config.Cheats.size() == 0)
	{
		ImGui::Text("No pokes loaded");
		return;
	}

	static int bAdvancedMode = 0;
    ImGui::RadioButton("Standard", &bAdvancedMode, 0);
	ImGui::SameLine();
    ImGui::RadioButton("Advanced", &bAdvancedMode, 1);
	ImGui::Separator();
    

	for (FCheat &cheat : config.Cheats)
	{
		ImGui::PushID(cheat.Description.c_str());
		bool bToggleCheat = false;
		bool bWasEnabled = cheat.bEnabled;
		int userDefinedCount = 0;
		
		if (cheat.bHasUserDefinedEntries)
		{
			ImGui::Text("%s",cheat.Description.c_str());
		}
		else
		{
			if (ImGui::Checkbox(cheat.Description.c_str(), &cheat.bEnabled))
			{
				bToggleCheat = true;
			}
		}
		
		for (auto &entry : cheat.Entries)
		{
			// Display memory locations in advanced mode
			if (bAdvancedMode)
			{
				ImGui::Text("%s:%s", NumStr(entry.Address), entry.bUserDefined ? "" : NumStr((uint8_t)entry.Value)); 
			}

			if (entry.bUserDefined)
			{
				char tempStr[16] = {0};
				snprintf(tempStr,16, "##Value %d", ++userDefinedCount);
				
				// Display the value of the memory location in the input field.
				// If the user has modified the value then display that instead.
				uint8_t value = entry.bUserDefinedValueDirty ? entry.Value : CodeAnalysis.ReadByte(entry.Address);
				
				if (bAdvancedMode)
					ImGui::SameLine();

				ImGui::SetNextItemWidth(45);
				ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(0, 0));

				if (ImGui::InputScalar(tempStr, ImGuiDataType_U8, &value, NULL, NULL, "%d", 0))
				{
					entry.Value = value;
					entry.bUserDefinedValueDirty = true;
				}
				ImGui::PopStyleVar();
			}

			if (bAdvancedMode)
			{
				DrawAddressLabel(CodeAnalysis, CodeAnalysis.GetFocussedViewState(), entry.Address);
			}
		}

		// Show the Apply and Revert buttons if we have any user defined values.
		if (cheat.bHasUserDefinedEntries)
		{
			if (ImGui::Button("Apply"))
			{
				cheat.bEnabled = true;
				bToggleCheat = true;
			}

			ImGui::SameLine();
			bool bDisabledRevert = !cheat.bEnabled; 
			if (bDisabledRevert)
				ImGui::BeginDisabled();
			if (ImGui::Button("Revert"))
			{
				cheat.bEnabled = false;
				bToggleCheat = true;
			}
			if (bDisabledRevert)
				ImGui::EndDisabled();
		}

		if (bToggleCheat)
		{
			for (auto &entry : cheat.Entries)
			{
				if (cheat.bEnabled)	// cheat activated
				{
					// store old value
					if (!bWasEnabled)
						entry.OldValue = ReadByte( entry.Address);
					WriteByte( entry.Address, static_cast<uint8_t>(entry.Value));
					entry.bUserDefinedValueDirty = false;
				}
				else
				{
					WriteByte( entry.Address, entry.OldValue);
				}

				// if code has been modified then clear the code text so it gets regenerated
				FCodeInfo* pCodeInfo = CodeAnalysis.GetCodeInfoForPhysicalAddress(entry.Address);
				if (pCodeInfo)
					pCodeInfo->Text.clear();
			
				CodeAnalysis.SetCodeAnalysisDirty(entry.Address);
			}

			LOGINFO("Poke %s: '%s' [%d byte(s)]", cheat.bEnabled ? "applied" : "reverted", cheat.Description.c_str(), cheat.Entries.size());

		}
		ImGui::Separator();
		ImGui::PopID();
	}
}

void FSpectrumEmu::AppFocusCallback(int focused)
{
	if (focused)
	{
		for(auto& listIt : GamesLists)
		{ 
			listIt.second.EnumerateGames();
		}
	}
}

bool FSpectrumEmu::SaveMachineSnapshot(int snapshotNo)
{
	if (snapshotNo > 0 && snapshotNo < kNoSnapshots)
	{
		FSnapshot& snapshot = Snapshots[snapshotNo];
		zx_save_snapshot(&ZXEmuState, &snapshot.State);

		if (snapshot.bValid == false)
		{
			// setup texture
			chips_display_info_t dispInfo = zx_display_info(&ZXEmuState);
			snapshot.Thumbnail = ImGui_CreateTextureRGBA(SpectrumViewer.GetFrameBuffer(), dispInfo.frame.dim.width, dispInfo.frame.dim.height);
			snapshot.bValid = true;
		}
		else
		{
			// update texture
			ImGui_UpdateTextureRGBA(snapshot.Thumbnail, SpectrumViewer.GetFrameBuffer());
		}
		
		return true;
	}

	return false;
}

bool FSpectrumEmu::LoadMachineSnapshot(int snapshotNo)
{
	if (snapshotNo > 0 && snapshotNo < kNoSnapshots)
	{
		FSnapshot& snapshot = Snapshots[snapshotNo];
		if(snapshot.bValid == false)
			return false;
		zx_load_snapshot(&ZXEmuState, ZX_SNAPSHOT_VERSION, &snapshot.State);
		return true;
	}

	return false;
}

ImTextureID	FSpectrumEmu::GetMachineSnapshotThumbnail(int snapshotNo) const
{
	if (snapshotNo > 0 && snapshotNo < kNoSnapshots)
	{
		const FSnapshot& snapshot = Snapshots[snapshotNo];
		if (snapshot.bValid == true)
			return snapshot.Thumbnail;
	}

	return 0;
}


void FSpectrumLaunchConfig::ParseCommandline(int argc, char** argv)
{
	FEmulatorLaunchConfig::ParseCommandline(argc,argv);	// call base class

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
			Model = ESpectrumModel::Spectrum128K;
		}
		else if (*argIt == std::string("-game"))
		{
			if (++argIt == argList.end())
			{
				LOGERROR("-game : No game specified");
				break;
			}
			SpecificGame = *argIt;
		}
		else if (*argIt == std::string("-skoolfile"))
		{
			if (SpecificGame.empty())
			{
				LOGERROR("-skoolfile : A game must be specified with the -game argument.");
				break;
			}

			if (++argIt == argList.end())
			{
				LOGERROR("-skoolfile : No skoolkit file specified");
				break;
			}
			SkoolkitImport = *argIt;
		}

		++argIt;
	}
}
// Util functions - move
uint16_t GetScreenPixMemoryAddress(int x, int y)
{
	if (x < 0 || x>255 || y < 0 || y> 191)
		return 0;

	const int char_x = x / 8;
	const uint16_t addr = 0x4000 | ((y & 7) << 8) | (((y >> 3) & 7) << 5) | (((y >> 6) & 3) << 11) | (char_x & 31);

	return addr;
}

uint16_t GetScreenAttrMemoryAddress(int x, int y)
{
	if (x < 0 || x>255 || y < 0 || y> 191)
		return 0;

	const int char_x = x / 8;
	const int char_y = y / 8;

	return 0x5800 + (char_y * 32) + char_x;
}

bool GetScreenAddressCoords(uint16_t addr, int& x, int &y)
{
	if (addr < 0x4000 || addr >= 0x5800)
		return false;

	const int y02 = (addr >> 8) & 7;	// bits 0-2
	const int y35 = (addr >> 5) & 7;	// bits 3-5
	const int y67 = (addr >> 11) & 3;	// bits 6 & 7
	x = (addr & 31) * 8;
	y = y02 | (y35 << 3) | (y67 << 6);
	return true;
}

bool GetAttribAddressCoords(uint16_t addr, int& x, int& y)
{
	if (addr < 0x5800 || addr >= 0x5B00)
		return false;
	int offset = addr - 0x5800;

	x = (offset & 31) << 3;
	y = (offset >> 5) << 3;

	return true;
}
