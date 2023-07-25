#define CHIPS_UI_IMPL
#include <imgui.h>

#include "SpectrumEmu.h"
#include "ZXChipsImpl.h"
#include <cstdint>

#include "GlobalConfig.h"
#include "GameData.h"
#include <ImGuiSupport/ImGuiTexture.h>
#include "GameViewers/GameViewer.h"
#include "GameViewers/StarquakeViewer.h"
#include "GameViewers/MiscGameViewers.h"
#include "Viewers/SpectrumViewer.h"
#include "Viewers/GraphicsViewer.h"
#include "Viewers/ZXGraphicsView.h"
#include "Viewers/BreakpointViewer.h"
#include "Viewers/OverviewViewer.h"
#include "Util/FileUtil.h"

#include "ui/ui_dbg.h"
#include "MemoryHandlers.h"
#include "CodeAnalyser/CodeAnalyser.h"
#include "CodeAnalyser/UI/CodeAnalyserUI.h"

#include "zx-roms.h"
#include <algorithm>
#include <sokol_audio.h>
#include "Exporters/SkoolkitExporter.h"
#include "Importers/SkoolkitImporter.h"
#include "Debug/DebugLog.h"
#include "Debug/ImGuiLog.h"
#include <cassert>
#include <Util/Misc.h>

#include "SpectrumConstants.h"

#include "Exporters/SkoolFileInfo.h"
#include "Exporters/AssemblerExport.h"
#include "CodeAnalyser/UI/CharacterMapViewer.h"
#include "GameConfig.h"
#include "App.h"
#include <CodeAnalyser/CodeAnalysisState.h>
#include "CodeAnalyser/CodeAnalysisJson.h"

#define ENABLE_RZX 1
#define SAVE_ROM_JSON 0

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
	if (ZXEmuState.type == ZX_TYPE_48K)
	{
		const int bank = address >> 14;
		const int bankAddr = address & 0x3fff;

		if (bank == 0)
			return &ZXEmuState.rom[0][bankAddr];
		else
			return &ZXEmuState.ram[bank - 1][bankAddr];
	}
	else
	{
		const uint8_t memConfig = ZXEmuState.last_mem_config;

		if (address < 0x4000)
			return &ZXEmuState.rom[(memConfig & (1 << 4)) ? 1 : 0][address];
		else if (address < 0x8000)
			return &ZXEmuState.ram[5][address - 0x4000];
		else if (address < 0xC000)
			return &ZXEmuState.ram[2][address - 0x8000];
		else
			return &ZXEmuState.ram[memConfig & 7][address - 0xC000];
	}
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


void FSpectrumEmu::GraphicsViewerSetView(FAddressRef address, int charWidth)
{
	GraphicsViewerGoToAddress(address);
	GraphicsViewerSetCharWidth(charWidth);
}


void FSpectrumEmu::FormatSpectrumMemory(FCodeAnalysisState& state) 
{
	// Format screen pixel memory if it hasn't already been
	if (state.GetLabelForAddress(kScreenPixMemStart) == nullptr)
	{
		AddLabel(state, 0x4000, "ScreenPixels", ELabelType::Data);

		FDataInfo* pScreenPixData = state.GetReadDataInfoForAddress(kScreenPixMemStart);
		pScreenPixData->DataType = EDataType::ScreenPixels;
		//pScreenPixData->Address = kScreenPixMemStart;
		pScreenPixData->ByteSize = kScreenPixMemSize;
	}

	// Format attribute memory if it hasn't already
	if (state.GetLabelForAddress(kScreenAttrMemStart) == nullptr)
	{
		AddLabel(state, 0x5800, "ScreenAttributes", ELabelType::Data);

		FDataFormattingOptions format;
		format.StartAddress = kScreenAttrMemStart;
		format.DataType = EDataType::ColAttr;
		format.ItemSize = 32;
		format.NoItems = 24;
		FormatData(state, format);
	}
}

class FScreenPixMemDescGenerator : public FMemoryRegionDescGenerator
{
public:
	FScreenPixMemDescGenerator()
	{
		RegionMin = kScreenPixMemStart;
		RegionMax = kScreenPixMemEnd;
	}

	const char* GenerateAddressString(uint16_t addr) override
	{
		int xp = 0, yp = 0;
		GetScreenAddressCoords(addr, xp, yp);
		sprintf(DescStr, "Screen Pix: %d,%d", xp, yp);
		return DescStr;
	}
private:
	char DescStr[32] = { 0 };
};


class FScreenAttrMemDescGenerator : public FMemoryRegionDescGenerator
{
public:
	FScreenAttrMemDescGenerator()
	{
		RegionMin = kScreenAttrMemStart;
		RegionMax = kScreenAttrMemEnd;
	}

	const char* GenerateAddressString(uint16_t addr) override
	{
		int xp = 0, yp = 0;
		GetAttribAddressCoords(addr, xp, yp);
		sprintf(DescStr, "Screen Attr: %d,%d", xp/8, yp/8);
		return DescStr;
	}
private:
	char DescStr[32] = { 0 };
};

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
	if(GetGlobalConfig().bEnableAudio)
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

int UIEvalBreakpoint(ui_dbg_t* dbg_win, uint16_t pc, int ticks, uint64_t pins, void* user_data)
{
	return 0;
}

uint64_t FSpectrumEmu::Z80Tick(int num, uint64_t pins)
{
	FCodeAnalysisState &state = CodeAnalysis;
	FDebugger& debugger = CodeAnalysis.Debugger;
	z80_t& cpu = ZXEmuState.cpu;
	const uint16_t pc = GetPC().Address;
	static uint64_t lastTickPins = 0;
	const uint64_t risingPins = pins & (pins ^ lastTickPins);
	lastTickPins = pins;
	const uint16_t scanlinePos = (uint16_t)ZXEmuState.scanline_y;

	if (scanlinePos == 0)	// clear scanline info on new frame
		debugger.ResetScanlineEvents();

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

		IOAnalysis.IOHandler(pc, pins);

		if (pins & Z80_RD)
		{
			if ((pins & Z80_A0) == 0)
				debugger.RegisterEvent((uint8_t)EEventType::KeyboardRead, pcAddrRef, addr , data, scanlinePos);
			else if ((pins & (Z80_A7 | Z80_A6 | Z80_A5)) == 0) // Kempston Joystick (........000.....)
				debugger.RegisterEvent((uint8_t)EEventType::KempstonJoystickRead, pcAddrRef, addr, data, scanlinePos);
			else if (pins & 0xff)
				debugger.RegisterEvent((uint8_t)EEventType::FloatingBusRead, pcAddrRef, addr, data, scanlinePos);
			// 128K specific
			else if (ZXEmuState.type == ZX_TYPE_128)
			{
				if ((pins & (Z80_A15 | Z80_A14 | Z80_A1)) == (Z80_A15 | Z80_A14))
					debugger.RegisterEvent((uint8_t)EEventType::SoundChipRead, pcAddrRef, addr, data, scanlinePos);
			}
		}
		else if (pins & Z80_WR)
		{
			// an IO write

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
					debugger.RegisterEvent((uint8_t)EEventType::OutputBeeper, pcAddrRef, Z80_GET_ADDR(pins), data, scanlinePos);

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
					}
				}
				else if ((pins & (Z80_A15 | Z80_A14 | Z80_A1)) == (Z80_A15 | Z80_A14))	// select AY-3-8912 register (11............0.)
					debugger.RegisterEvent((uint8_t)EEventType::SoundChipRegisterSelect, pcAddrRef, addr, data, scanlinePos);
				else if ((pins & (Z80_A15 | Z80_A14 | Z80_A1)) == Z80_A15)	// write to AY-3-8912 (10............0.) 
					debugger.RegisterEvent((uint8_t)EEventType::SoundChipRegisterWrite, pcAddrRef, addr, data, scanlinePos);
			}

		}
	}

	if (pins & Z80_INT)	// have we had a vblank interrupt?
	{
	}

	InstructionsTicks++;

	const bool bNewOp = z80_opdone(&ZXEmuState.cpu);

	if (bNewOp)
	{
		OnInstructionExecuted(InstructionsTicks, pins);
		InstructionsTicks = 0;
	}

	debugger.CPUTick(pins);
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
	CodeAnalysis.UnMapBank(CurROMBank, 0);
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
	CodeAnalysis.UnMapBank(CurRAMBank[slot], startPage);
	CodeAnalysis.MapBank(bankId, startPage);

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

const char* g_AYRegNames[] = 
{
	"CH A Period Fine",		// 0
	"CH A Period Coarse",	// 1
	"CH B Period Fine",		// 2
	"CH B Period Coarse",	// 3
	"CH C Period Fine",		// 4
	"CH C Period Coarse",	// 5
	"Noise Pitch",			// 6
	"Mixer",				// 7
	"CH A Volume",			// 8
	"CH B Volume",			// 9
	"CH C Volume",			// 10 (A)
	"Env Dur fine",			// 11 (B)
	"Env Dur coarse",		// 12 (C)
	"Env Shape",			// 13 (D)
	"I/O Port A",			// 14 (E)
	"I/O Port B",			// 15 (F)
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



bool FSpectrumEmu::Init(const FSpectrumConfig& config)
{
	SetWindowTitle(kAppTitle.c_str());
	SetWindowIcon("SALOGO.png");

	// Initialise Emulator
	LoadGlobalConfig(kGlobalConfigFilename);
	FGlobalConfig& globalConfig = GetGlobalConfig();
	SetNumberDisplayMode(globalConfig.NumberDisplayMode);
	CodeAnalysis.Config.bShowOpcodeValues = globalConfig.bShowOpcodeValues;
	CodeAnalysis.Config.BranchLinesDisplayMode = globalConfig.BranchLinesDisplayMode;
	CodeAnalysis.Config.bShowBanks = config.Model == ESpectrumModel::Spectrum128K;
	CodeAnalysis.Config.CharacterColourLUT = FZXGraphicsView::GetColourLUT();
	
	// setup emu
	zx_type_t type = config.Model == ESpectrumModel::Spectrum128K ? ZX_TYPE_128 : ZX_TYPE_48K;
	zx_joystick_type_t joy_type = ZX_JOYSTICKTYPE_NONE;

	zx_desc_t desc;
	memset(&desc, 0, sizeof(zx_desc_t));
	desc.type = type;
	desc.joystick_type = joy_type;
	//desc.pixel_buffer = FrameBuffer;
	//desc.pixel_buffer_size = pixelBufferSize;

	// audio
	desc.audio.callback.func = PushAudio;	// our audio callback
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


	GamesList.Init(this);
	if(config.Model == ESpectrumModel::Spectrum128K)
		GamesList.EnumerateGames(globalConfig.SnapshotFolder128.c_str());
	else
		GamesList.EnumerateGames(globalConfig.SnapshotFolder.c_str());

	RZXManager.Init(this);
	RZXGamesList.Init(this);
	RZXGamesList.EnumerateGames(globalConfig.RZXFolder.c_str());

	// Clear UI
	memset(&UIZX, 0, sizeof(ui_zx_t));

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

	// This is where we add the viewers we want
	//Viewers.push_back(new FBreakpointViewer(this));
	Viewers.push_back(new FOverviewViewer(this));

	// Initialise Viewers
	for (auto Viewer : Viewers)
	{
		if (Viewer->Init() == false)
		{
			// TODO: report error
		}
	}

	GraphicsViewer.pEmu = this;
	InitGraphicsViewer(GraphicsViewer);
	IOAnalysis.Init(this);
	SpectrumViewer.Init(this);
	FrameTraceViewer.Init(this);

	CodeAnalysis.ViewState[0].Enabled = true;	// always have first view enabled

	// Setup memory description handlers
	AddMemoryRegionDescGenerator(new FScreenPixMemDescGenerator());
	AddMemoryRegionDescGenerator(new FScreenAttrMemDescGenerator());	

	// register Viewers
	RegisterStarquakeViewer(this);
	RegisterGames(this);

	LoadGameConfigs(this);

	// create & register ROM banks
	for (int bankNo = 0; bankNo < kNoROMBanks; bankNo++)
	{
		char bankName[32];
		sprintf(bankName, "ROM %d", bankNo);
		ROMBanks[bankNo] = CodeAnalysis.CreateBank(bankName, 16,ZXEmuState.rom[bankNo], true);
		CodeAnalysis.GetBank(ROMBanks[bankNo])->PrimaryMappedPage = 0;
	}

	// create & register RAM banks
	for (int bankNo = 0; bankNo < kNoRAMBanks; bankNo++)
	{
		char bankName[32];
		sprintf(bankName, "RAM %d", bankNo);
		RAMBanks[bankNo] = CodeAnalysis.CreateBank(bankName, 16, ZXEmuState.ram[bankNo], false);
		CodeAnalysis.GetBank(RAMBanks[bankNo])->PrimaryMappedPage = 48;
	}

	// Setup initial machine memory config
	if (config.Model == ESpectrumModel::Spectrum48K)
	{
		CodeAnalysis.GetBank(RAMBanks[0])->PrimaryMappedPage = 16;
		CodeAnalysis.GetBank(RAMBanks[1])->PrimaryMappedPage = 32;
		CodeAnalysis.GetBank(RAMBanks[2])->PrimaryMappedPage = 48;

		SetROMBank(0);
		SetRAMBank(1, 0);	// 0x4000 - 0x7fff
		SetRAMBank(2, 1);	// 0x8000 - 0xBfff
		SetRAMBank(3, 2);	// 0xc000 - 0xffff
	}
	else
	{
		CodeAnalysis.GetBank(RAMBanks[5])->PrimaryMappedPage = 16;
		CodeAnalysis.GetBank(RAMBanks[2])->PrimaryMappedPage = 32;
		CodeAnalysis.GetBank(RAMBanks[0])->PrimaryMappedPage = 48;

		SetROMBank(0);
		SetRAMBank(1, 5);	// 0x4000 - 0x7fff
		SetRAMBank(2, 2);	// 0x8000 - 0xBfff
		SetRAMBank(3, 0);	// 0xc000 - 0xffff
	}


	// load the command line game if none specified then load the last game
	bool bLoadedGame = false;

	if (config.SpecificGame.empty() == false)
	{
		bLoadedGame = StartGame(config.SpecificGame.c_str());
	}
	else if (globalConfig.LastGame.empty() == false)
	{
		bLoadedGame = StartGame(globalConfig.LastGame.c_str());
	}
	
	// Start ROM if no game has been loaded
	if(bLoadedGame == false)
	{
		std::string romJsonFName = kRomInfo48JsonFile;

		if (config.Model == ESpectrumModel::Spectrum128K)
			romJsonFName = kRomInfo128JsonFile;

		CodeAnalysis.Init(this);

		if (FileExists(romJsonFName.c_str()))
			ImportAnalysisJson(CodeAnalysis, romJsonFName.c_str());
	}

	if(config.SkoolkitImport.empty() == false)
		ImportSkoolFile(config.SkoolkitImport.c_str());

	// Setup Debugger
	FDebugger& debugger = CodeAnalysis.Debugger;
	debugger.RegisterEventType((int)EEventType::None, "None", 0);
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

	debugger.SetScreenMemoryArea(kScreenPixMemStart, kScreenAttrMemEnd);

	bInitialised = true;
	return true;
}

void FSpectrumEmu::Shutdown()
{
	if (RZXManager.GetReplayMode() == EReplayMode::Off)
		SaveCurrentGameData();	// save on close

	// Save Global Config - move to function?
	FGlobalConfig& config = GetGlobalConfig();

	if (pActiveGame != nullptr)
		config.LastGame = pActiveGame->pConfig->Name;

	config.NumberDisplayMode = GetNumberDisplayMode();
	config.bShowOpcodeValues = CodeAnalysis.Config.bShowOpcodeValues;
	config.BranchLinesDisplayMode = CodeAnalysis.Config.BranchLinesDisplayMode;

	SaveGlobalConfig(kGlobalConfigFilename);
}

void FSpectrumEmu::StartGame(FGameConfig *pGameConfig, bool bLoadGameData /* =  true*/)
{
	// reset systems
	MemoryAccessHandlers.clear();	// remove old memory handlers
	ResetMemoryStats(MemStats);
	FrameTraceViewer.Reset();

	const std::string windowTitle = kAppTitle + " - " + pGameConfig->Name;
	SetWindowTitle(windowTitle.c_str());
	
	// start up game
	if(pActiveGame!=nullptr)
		delete pActiveGame->pViewerData;
	delete pActiveGame;
	
	FGame *pNewGame = new FGame;
	pGameConfig->Spectrum128KGame = ZXEmuState.type == ZX_TYPE_128;
	pNewGame->pConfig = pGameConfig;
	pNewGame->pViewerConfig = pGameConfig->pViewerConfig;
	assert(pGameConfig->pViewerConfig != nullptr);
	GraphicsViewer.pGame = pNewGame;
	pActiveGame = pNewGame;
	pNewGame->pViewerData = pNewGame->pViewerConfig->pInitFunction(this, pGameConfig);
	GenerateSpriteListsFromConfig(GraphicsViewer, pGameConfig);

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
		const std::string root = GetGlobalConfig().WorkspaceRoot;
		const std::string dataFName = root + "GameData/" + pGameConfig->Name + ".bin";
		std::string romJsonFName = kRomInfo48JsonFile;

		if (ZXEmuState.type == ZX_TYPE_128)
			romJsonFName = root + kRomInfo128JsonFile;

		const std::string analysisJsonFName = root + "AnalysisJson/" + pGameConfig->Name + ".json";
		const std::string analysisStateFName = root + "AnalysisState/" + pGameConfig->Name + ".astate";
		const std::string saveStateFName = root + "SaveStates/" + pGameConfig->Name + ".state";
		if (FileExists(analysisJsonFName.c_str()))
		{
			ImportAnalysisJson(CodeAnalysis, analysisJsonFName.c_str());
			ImportAnalysisState(CodeAnalysis, analysisStateFName.c_str());
		}
		else
			LoadGameData(this, dataFName.c_str());	// Load the old one - this needs to go in time

		LoadGameState(this, saveStateFName.c_str());

		if (FileExists(romJsonFName.c_str()))
			ImportAnalysisJson(CodeAnalysis, romJsonFName.c_str());

		// where do we want pokes to live?
		LoadPOKFile(*pGameConfig, std::string(GetGlobalConfig().PokesFolder + pGameConfig->Name + ".pok").c_str());
	}
	ReAnalyseCode(CodeAnalysis);
	GenerateGlobalInfo(CodeAnalysis);
	FormatSpectrumMemory(CodeAnalysis);
	CodeAnalysis.SetAddressRangeDirty();

	// Start in break mode so the memory will be in it's initial state. 
	// Otherwise, if we export a skool/asm file once the game is running the memory could be in an arbitrary state.
	// 
	// decode whole screen
	ZXDecodeScreen(&ZXEmuState);
	CodeAnalysis.Debugger.SetPC(CodeAnalysis.AddressRefFromPhysicalAddress(ZXEmuState.cpu.pc - 1));
	CodeAnalysis.Debugger.Break();

	CodeAnalysis.Debugger.RegisterNewStackPointer(ZXEmuState.cpu.sp, FAddressRef());
}

bool FSpectrumEmu::StartGame(const char *pGameName)
{
	for (const auto& pGameConfig : GetGameConfigs())
	{
		if (pGameConfig->Name == pGameName)
		{
			const std::string snapFolder = ZXEmuState.type == ZX_TYPE_128 ? GetGlobalConfig().SnapshotFolder128 : GetGlobalConfig().SnapshotFolder;
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
void FSpectrumEmu::SaveCurrentGameData()
{
	if (pActiveGame != nullptr)
	{
		FGameConfig *pGameConfig = pActiveGame->pConfig;
		if (pGameConfig->Name.empty())
		{
			
		}
		else
		{
			const std::string root = GetGlobalConfig().WorkspaceRoot;
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

			AddGameConfig(pGameConfig);
			SaveGameConfigToFile(*pGameConfig, configFName.c_str());
			//SaveGameData(this, dataFName.c_str());		// The Past

			// The Future
			SaveGameState(this, saveStateFName.c_str());
			ExportAnalysisJson(CodeAnalysis, analysisJsonFName.c_str());
			ExportAnalysisState(CodeAnalysis, analysisStateFName.c_str());
		}
	}

	// TODO: this could use
#if	SAVE_ROM_JSON
	const std::string romJsonFName = root + kRomInfoJsonFile;
	ExportAnalysisJson(CodeAnalysis, romJsonFName.c_str(), true);	// export ROMS only
#endif
}

bool FSpectrumEmu::NewGameFromSnapshot(int snapshotIndex)
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

void FSpectrumEmu::DrawMainMenu(double timeMS)
{
	ui_zx_t* pZXUI = &UIZX;
	assert(pZXUI && pZXUI->zx && pZXUI->boot_cb);

	bExportAsm = false;
	bReplaceGamePopup = false;

	if (ImGui::BeginMainMenuBar())
	{
		if (ImGui::BeginMenu("File"))
		{
			if (ImGui::BeginMenu("New Game from Snapshot File"))
			{
				const int numGames = GamesList.GetNoGames();
				if (!numGames)
				{
					const std::string snapFolder = ZXEmuState.type == ZX_TYPE_128 ? GetGlobalConfig().SnapshotFolder128 : GetGlobalConfig().SnapshotFolder;
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
								NewGameFromSnapshot(gameNo);
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
					ImGui::Text("No RZX files found in RZX directory:\n\n'%s'.\n\nRZX directory is set in GlobalConfig.json", GetGlobalConfig().RZXFolder.c_str());
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
								FGameConfig* pNewConfig = CreateNewGameConfigFromSnapshot(game);
								if (pNewConfig != nullptr)
									StartGame(pNewConfig);
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
							const std::string snapFolder = ZXEmuState.type == ZX_TYPE_128 ? GetGlobalConfig().SnapshotFolder128 : GetGlobalConfig().SnapshotFolder;
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
					const std::string dir = GetGlobalConfig().WorkspaceRoot + "OutputBin/";
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
				const std::string snapFolder = ZXEmuState.type == ZX_TYPE_128 ? GetGlobalConfig().SnapshotFolder128 : GetGlobalConfig().SnapshotFolder;
				const std::string gameFile = snapFolder + pActiveGame->pConfig->SnapshotFile;
				GamesList.LoadGame(gameFile.c_str());
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
			FGlobalConfig& config = GetGlobalConfig();

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
			ImGui::MenuItem("Scan Line Indicator", 0, &config.bShowScanLineIndicator);
			ImGui::MenuItem("Enable Audio", 0, &config.bEnableAudio);
			ImGui::MenuItem("Edit Mode", 0, &CodeAnalysis.bAllowEditing);
			ImGui::MenuItem("Show Opcode Values", 0, &CodeAnalysis.Config.bShowOpcodeValues);

			if (ImGui::BeginMenu("Display Branch Lines"))
			{
				if (ImGui::MenuItem("Off", 0, CodeAnalysis.Config.BranchLinesDisplayMode == 0))
				{
					CodeAnalysis.Config.BranchLinesDisplayMode = 0;
				}
				if (ImGui::MenuItem("Minimal", 0, CodeAnalysis.Config.BranchLinesDisplayMode == 1))
				{
					CodeAnalysis.Config.BranchLinesDisplayMode = 1;
				}
				if (ImGui::MenuItem("Full", 0, CodeAnalysis.Config.BranchLinesDisplayMode == 2))
				{
					CodeAnalysis.Config.BranchLinesDisplayMode = 2;
				}

				ImGui::EndMenu();
			}

			if(pActiveGame!=nullptr)
				ImGui::MenuItem("Save Snapshot with game", 0, &pActiveGame->pConfig->WriteSnapshot);

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
			if (ImGui::MenuItem("Find Ascii Strings"))
			{
				CodeAnalysis.FindAsciiStrings(0x4000);
			}
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
					const std::string dir = GetGlobalConfig().WorkspaceRoot + "OutputASM/";
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

void StoreRegisters_Z80(FCodeAnalysisState& state);

bool GetIOInputFunc(uint16_t port, uint8_t* pInVal, void* pUserData)
{
	FSpectrumEmu* pEmu = (FSpectrumEmu*)pUserData;
	return pEmu->RZXManager.GetInput(port, *pInVal);
}


void FSpectrumEmu::Tick()
{
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

	UpdateCharacterSets(CodeAnalysis);

	// Draw UI
	DrawDockingView();
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
			IOAnalysis.DrawUI();
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



void FSpectrumEmu::DrawUI()
{
	ui_zx_t* pZXUI = &UIZX;
	const double timeMS = 1000.0f / ImGui::GetIO().Framerate;
	
	//if(ExecThisFrame)
	//	ui_zx_after_exec(pZXUI);

	//const int instructionsThisFrame = (int)CodeAnalysis.FrameTrace.size();
	//static int maxInst = 0;
	//maxInst = std::max(maxInst, instructionsThisFrame);

	DrawMainMenu(timeMS);
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

	/*for (int i = 0; i < 4; i++)
	{
		ui_memedit_draw(&pZXUI->memedit[i]);
		ui_dasm_draw(&pZXUI->dasm[i]);
	}*/

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

	//DasmDraw(&pUI->FunctionDasm);
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
			ImGui::Text(pActiveGame->pConfig->Name.c_str());
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

	DrawGraphicsViewer(GraphicsViewer);
	DrawMemoryTools();

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

	if (bShowDebugLog)
		g_ImGuiLog.Draw("Debug Log", &bShowDebugLog);
}

bool FSpectrumEmu::DrawDockingView()
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

// Cheats
// TODO: move this out to another file/class
void FSpectrumEmu::DrawCheatsUI()
{
	if (pActiveGame == nullptr)
		return;
	
	if (pActiveGame->pConfig->Cheats.size() == 0)
	{
		ImGui::Text("No pokes loaded");
		return;
	}

	static int bAdvancedMode = 0;
    ImGui::RadioButton("Standard", &bAdvancedMode, 0);
	ImGui::SameLine();
    ImGui::RadioButton("Advanced", &bAdvancedMode, 1);
	ImGui::Separator();
    
	FGameConfig &config = *pActiveGame->pConfig;

	for (FCheat &cheat : config.Cheats)
	{
		ImGui::PushID(cheat.Description.c_str());
		bool bToggleCheat = false;
		bool bWasEnabled = cheat.bEnabled;
		int userDefinedCount = 0;
		
		if (cheat.bHasUserDefinedEntries)
		{
			ImGui::Text(cheat.Description.c_str());
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
				sprintf(tempStr, "##Value %d", ++userDefinedCount);
				
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
				FCodeInfo* pCodeInfo = CodeAnalysis.GetCodeInfoForAddress(entry.Address);
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

// Import a skool file to use in the disassembly.
// If a game is active the gamedata will be backed up.
// A skoolinfo file will be saved, which saves info that cannot be stored in the code analysis data.
// This skoolinfo file will be used when exporting to help replicate the original skool file.
// The name of the output skoolinfo file will be taken from the active game.
// If no game is active the filename of the output skoolinfo file must be passed in pOutSkoolInfoName.
// pSkoolInfo is optional. The skoolinfo data will be saved in pSkoolInfo if a pointer is passed in. 
bool FSpectrumEmu::ImportSkoolFile(const char* pFilename, const char* pOutSkoolInfoName /* = nullptr*/, FSkoolFileInfo* pSkoolInfo /* = nullptr*/)
{
	// one of these must be set
	if (!pActiveGame && !pOutSkoolInfoName)
		return false;

	LOGINFO("Importing skool file '%s'", pFilename);

	const std::string root = GetGlobalConfig().WorkspaceRoot;

	if (pActiveGame)
	{
		// backup their gamedata
		std::string dir = root + "AnalysisJson/";
		const std::string dataFName = dir + pActiveGame->pConfig->Name + ".json.bak";
		EnsureDirectoryExists(dir.c_str());
		/*if (!SaveGameData(this, dataFName.c_str()))
		{
			LOGERROR("Failed to import skool file. Could not save backup of analysis data to '%s'", dataFName.c_str());
			return false;
		}*/
	}
	// use FSkoolFileInfo pointer if it's passed in. Otherwise use a temporary local struct.
	FSkoolFileInfo skoolInfo;
	FSkoolFileInfo* pInfo = pSkoolInfo ? pSkoolInfo : &skoolInfo;
	if (!ImportSkoolKitFile(CodeAnalysis, pFilename, pSkoolInfo ? pSkoolInfo : pInfo))
	{
		LOGINFO("Failed to import '%s'", pFilename);
		return false;
	}

	const std::string gameName = pActiveGame ? pActiveGame->pConfig->Name.c_str() : pOutSkoolInfoName;
	const std::string dir = root + "OutputSkoolKit/";
	const std::string skoolInfoFname(dir + gameName + std::string(".skoolinfo"));
	EnsureDirectoryExists(dir.c_str());
	LOGINFO("Saving skoolinfo file '%s'", skoolInfoFname.c_str());
	if (!SaveSkoolFileInfo(*pInfo, skoolInfoFname.c_str()))
	{
		LOGINFO("Failed to save skoolinfo file '%s'", skoolInfoFname.c_str());
		return false;
	}

	LOGINFO("Imported skool file '%s' successfully for '%s'.", pFilename, gameName.c_str());
	LOGDEBUG("Disassembly range $%x-$%x. %d locations saved to skoolinfo file", pInfo->StartAddr, pInfo->EndAddr, pInfo->Locations.size());

	return true;
}

bool FSpectrumEmu::ExportSkoolFile(bool bHexadecimal, const char* pName /* = nullptr*/)
{
	if (!pActiveGame)
		return false;
	
	const std::string outputDir = "OutputSkoolKit/";
	EnsureDirectoryExists(outputDir.c_str());

	const std::string name = pName ? std::string(pName) : pActiveGame->pConfig->Name;
	FSkoolFileInfo skoolInfo;
	std::string skoolInfoFname = outputDir + name + ".skoolinfo";
	bool bLoadedSkoolFileInfo = LoadSkoolFileInfo(skoolInfo, skoolInfoFname.c_str());
	
	const std::string outFname = outputDir + name + ".skool";
	::ExportSkoolFile(CodeAnalysis, outFname.c_str(), bHexadecimal ? FSkoolFile::Base::Hexadecimal : FSkoolFile::Base::Decimal, bLoadedSkoolFileInfo ? &skoolInfo : nullptr);
	
	return true;
}

// Start a game, import a skool file and then export it, to test the SkoolKit importer and exporter are working properly.
// You can optionally pass a game to start in pGameName. The output skool file will have the same name as the game.
// If no game name is passed, then no game will be started and you must pass the name of the output skool file in pOutSkoolName.
// This can be used to import and export skool files for the spectrum rom. 
void FSpectrumEmu::DoSkoolKitTest(const char* pGameName, const char* pInSkoolFileName, bool bHexadecimal, const char* pOutSkoolName /* = nullptr*/)
{
	if (!pGameName && !pOutSkoolName)
		return;

	if (pGameName)
	{
		if (!StartGame(pGameName))
			return;
	}

	FSkoolFileInfo skoolInfo;
	std::string inSkoolPath = std::string("InputSkoolKit/") + pInSkoolFileName;
    if (!ImportSkoolFile(inSkoolPath.c_str(), pOutSkoolName, &skoolInfo))
		return;

	EnsureDirectoryExists("OutputSkoolKit/");
	std::string outFname = "OutputSkoolKit/" + std::string(pGameName ? pGameName : pOutSkoolName) + ".skool";
	FSkoolFile::Base base = bHexadecimal ? FSkoolFile::Base::Hexadecimal : FSkoolFile::Base::Decimal;
	::ExportSkoolFile(CodeAnalysis, outFname.c_str(), base, &skoolInfo);
}

void FSpectrumEmu::AppFocusCallback(int focused)
{
	if (focused)
	{
		if (ZXEmuState.type == ZX_TYPE_128)
			GamesList.EnumerateGames(GetGlobalConfig().SnapshotFolder128.c_str());
		else
			GamesList.EnumerateGames(GetGlobalConfig().SnapshotFolder.c_str());
	}
}

void FSpectrumConfig::ParseCommandline(int argc, char** argv)
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
			Model = ESpectrumModel::Spectrum128K;
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
		else if (*argIt == std::string("-skoolfile"))
		{
			if (++argIt == argList.end())
			{
				LOGERROR("-skoolfile : No skoolkit file specified");
				break;
			}
			SkoolkitImport = *++argIt;
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
