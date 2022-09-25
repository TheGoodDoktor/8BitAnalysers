
#define CHIPS_IMPL
#include <imgui.h>
#include "SpectrumEmu.h"
#include <windows.h>

#include "GameConfig.h"
#include <ImGuiSupport/imgui_impl_lucidextra.h>
#include "GameViewers/GameViewer.h"
#include "GameViewers/StarquakeViewer.h"
#include "GameViewers/MiscGameViewers.h"
#include "Viewers/SpectrumViewer.h"
#include "Viewers/GraphicsViewer.h"
#include "Viewers/BreakpointViewer.h"
#include "Viewers/OverviewViewer.h"
#include "Util/FileUtil.h"

#include "ui/ui_dbg.h"
#include "MemoryHandlers.h"
#include "FunctionHandlers.h"
#include "Disassembler.h"
#include "CodeAnalyser/CodeAnalyser.h"
#include "CodeAnalyser/CodeAnalyserUI.h"

#include "zx-roms.h"
#include "ROMLabels.h"
#include <algorithm>
#include <Vendor/sokol/sokol_audio.h>
#include "Exporters/SkoolkitExporter.h"
#include "Importers/SkoolkitImporter.h"
#include <cassert>

// Memory access functions

uint8_t* MemGetPtr(zx_t* zx, int layer, uint16_t addr)
{
	if (layer == 0)
	{
		/* ZX128 ROM, RAM 5, RAM 2, RAM 0 */
		if (addr < 0x4000)
			return &zx->rom[0][addr];
		else if (addr < 0x8000)
			return &zx->ram[5][addr - 0x4000];
		else if (addr < 0xC000)
			return &zx->ram[2][addr - 0x8000];
		else
			return &zx->ram[0][addr - 0xC000];
	}
	else if (layer == 1)
	{
		/* 48K ROM, RAM 1 */
		if (addr < 0x4000)
			return &zx->rom[1][addr];
		else if (addr >= 0xC000)
			return &zx->ram[1][addr - 0xC000];
	}
	else if (layer < 8)
	{
		if (addr >= 0xC000)
			return &zx->ram[layer][addr - 0xC000];
	}
	/* fallthrough: unmapped memory */
	return 0;
}

uint8_t MemReadFunc(int layer, uint16_t addr, void* user_data)
{
	assert(user_data);
	zx_t* zx = (zx_t*)user_data;
	if ((layer == 0) || (ZX_TYPE_48K == zx->type))
	{
		/* CPU visible layer */
		return mem_rd(&zx->mem, addr);
	}
	else
	{
		uint8_t* ptr = MemGetPtr(zx, layer - 1, addr);
		if (ptr)
			return *ptr;
		else
			return 0xFF;
	}
}

void MemWriteFunc(int layer, uint16_t addr, uint8_t data, void* user_data)
{
	assert(user_data);
	zx_t* zx = (zx_t*)user_data;
	if ((layer == 0) || (ZX_TYPE_48K == zx->type)) 
	{
		mem_wr(&zx->mem, addr, data);
	}
	else 
	{
		uint8_t* ptr = MemGetPtr(zx, layer - 1, addr);
		if (ptr) 
		{
			*ptr = data;
		}
	}
}

uint8_t		FSpectrumEmu::ReadByte(uint16_t address) const
{
	return MemReadFunc(CurrentLayer, address, const_cast<zx_t *>(&ZXEmuState));

}
uint16_t	FSpectrumEmu::ReadWord(uint16_t address) const 
{
	return ReadByte(address) | (ReadByte(address + 1) << 8);
}

const uint8_t* FSpectrumEmu::GetMemPtr(uint16_t address) const 
{
	const int bank = address >> 14;
	const int bankAddr = address & 0x3fff;

	if (bank == 0)
		return &ZXEmuState.rom[0][bankAddr];
	else
		return &ZXEmuState.ram[bank - 1][bankAddr];

	//return MemGetPtr(const_cast<zx_t*>(&ZXEmuState), CurrentLayer, address);
}


void FSpectrumEmu::WriteByte(uint16_t address, uint8_t value)
{
	MemWriteFunc(CurrentLayer, address, value, &ZXEmuState);
}


uint16_t	FSpectrumEmu::GetPC(void) 
{
	return z80_pc(&ZXEmuState.cpu);
} 

bool FSpectrumEmu::IsAddressBreakpointed(uint16_t addr)
{
	for (int i = 0; i < UIZX.dbg.dbg.num_breakpoints; i++) 
	{
		if (UIZX.dbg.dbg.breakpoints[i].addr == addr)
			return true;
	}

	return false;
}

bool FSpectrumEmu::ToggleExecBreakpointAtAddress(uint16_t addr)
{
	int index = _ui_dbg_bp_find(&UIZX.dbg, UI_DBG_BREAKTYPE_EXEC, addr);
	if (index >= 0) 
	{
		/* breakpoint already exists, remove */
		_ui_dbg_bp_del(&UIZX.dbg, index);
		return false;
	}
	else 
	{
		/* breakpoint doesn't exist, add a new one */
		return _ui_dbg_bp_add_exec(&UIZX.dbg, true, addr);
	}
}

bool FSpectrumEmu::ToggleDataBreakpointAtAddress(uint16_t addr, uint16_t dataSize)
{
	const int type = dataSize == 1 ? UI_DBG_BREAKTYPE_BYTE : UI_DBG_BREAKTYPE_WORD;
	int index = _ui_dbg_bp_find(&UIZX.dbg, type, addr);
	if (index >= 0)
	{
		// breakpoint already exists, remove 
		_ui_dbg_bp_del(&UIZX.dbg, index);
		return false;
	}
	else
	{
		// breakpoint doesn't exist, add a new one 
		if (UIZX.dbg.dbg.num_breakpoints < UI_DBG_MAX_BREAKPOINTS)
		{
			ui_dbg_breakpoint_t* bp = &UIZX.dbg.dbg.breakpoints[UIZX.dbg.dbg.num_breakpoints++];
			bp->type = type;
			bp->cond = UI_DBG_BREAKCOND_NONEQUAL;
			bp->addr = addr;
			bp->val = ReadByte(addr);
			bp->enabled = true;
			return true;
		}
		else 
		{
			return false;
		}
	}
}

void FSpectrumEmu::Break(void)
{
	_ui_dbg_break(&UIZX.dbg);
}

void FSpectrumEmu::Continue(void) 
{
	_ui_dbg_continue(&UIZX.dbg);
}

void FSpectrumEmu::StepOver(void)
{
	_ui_dbg_step_over(&UIZX.dbg);
}

void FSpectrumEmu::StepInto(void)
{
	_ui_dbg_step_into(&UIZX.dbg);
}

void FSpectrumEmu::StepFrame()
{
	_ui_dbg_continue(&UIZX.dbg);
	bStepToNextFrame = true;
}

void FSpectrumEmu::StepScreenWrite()
{
	_ui_dbg_continue(&UIZX.dbg);
	bStepToNextScreenWrite = true;
}

void FSpectrumEmu::GraphicsViewerSetAddress(uint16_t address) 
{
	GraphicsViewerGoToAddress(address);
}

bool	FSpectrumEmu::ShouldExecThisFrame(void) const
{
	return ExecThisFrame;
}

void FSpectrumEmu::InsertROMLabels(FCodeAnalysisState& state) 
{
	for (const auto& label : g_RomLabels)
	{
		AddLabel(state, label.Address, label.pLabelName, label.LabelType);

		// run static analysis on all code labels
		if (label.LabelType == LabelType::Code || label.LabelType == LabelType::Function)
			RunStaticCodeAnalysis(state, label.Address);
	}

	for (const auto& label : g_SysVariables)
	{
		AddLabel(state, label.Address, label.pLabelName, LabelType::Data);
		// TODO: Set up data?
	}
}

void FSpectrumEmu::InsertSystemLabels(FCodeAnalysisState& state) 
{
	// screen memory start
	AddLabel(state, 0x4000, "ScreenPixels", LabelType::Data);

	FDataInfo* pScreenPixData = state.GetReadDataInfoForAddress(0x4000);
	pScreenPixData->DataType = DataType::Graphics;
	pScreenPixData->Address = 0x4000;
	pScreenPixData->ByteSize = 0x1800;

	AddLabel(state, 0x5800, "ScreenAttributes", LabelType::Data);
	FDataInfo* pScreenAttrData = state.GetReadDataInfoForAddress(0x5800);
	pScreenAttrData->DataType = DataType::Blob;
	pScreenAttrData->Address = 0x5800;
	pScreenAttrData->ByteSize = 0x400;

	// system variables?
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
	return ImGui_ImplDX11_CreateTextureRGBA(nullptr, w, h);
}

void gfx_update_texture(void* h, void* data, int data_byte_size)
{
	ImGui_ImplDX11_UpdateTextureRGBA(h, (unsigned char *)data);
}

void gfx_destroy_texture(void* h)
{
	
}

/* audio-streaming callback */
static void PushAudio(const float* samples, int num_samples, void* user_data)
{
	//saudio_push(samples, num_samples);

}

int ZXSpectrumTrapCallback(uint16_t pc, int ticks, uint64_t pins, void* user_data)
{
	FSpectrumEmu* pEmu = (FSpectrumEmu*)user_data;
	return pEmu->TrapFunction(pc, ticks, pins);
}


// Note - you can't read register values in Trap function
// They are only written back at end of exec function
int	FSpectrumEmu::TrapFunction(uint16_t pc, int ticks, uint64_t pins)
{
	FCodeAnalysisState &state = CodeAnalysis;
	const uint16_t addr = Z80_GET_ADDR(pins);
	const bool bMemAccess = !!((pins & Z80_CTRL_MASK) & Z80_MREQ);
	const bool bWrite = (pins & Z80_CTRL_MASK) == (Z80_MREQ | Z80_WR);
	const bool irq = (pins & Z80_INT) && z80_iff1(&ZXEmuState.cpu);	

	const uint16_t nextpc = pc;
	// store program count in history
	const uint16_t prevPC = PCHistory[PCHistoryPos];
	PCHistoryPos = (PCHistoryPos + 1) % FSpectrumEmu::kPCHistorySize;
	PCHistory[PCHistoryPos] = pc;

	pc = prevPC;	// set PC to pc of instruction just executed

	if (irq)
	{
		FCPUFunctionCall callInfo;
		callInfo.CallAddr = prevPC;
		callInfo.FunctionAddr = pc;
		callInfo.ReturnAddr = prevPC;
		state.CallStack.push_back(callInfo);
	}

	bool bBreak = RegisterCodeExecuted(state, pc, nextpc);
	FCodeInfo* pCodeInfo = state.GetCodeInfoForAddress(pc);
	pCodeInfo->FrameLastAccessed = state.CurrentFrameNo;
	// check for breakpointed code line
	if (bBreak)
		return UI_DBG_BP_BASE_TRAPID;
	
	int trapId = MemoryHandlerTrapFunction(pc, ticks, pins, this);

	// break on screen memory write
	if (bWrite && addr >= 0x4000 && addr < 0x5800)
	{
		if (bStepToNextScreenWrite)
		{
			bStepToNextScreenWrite = false;
			return UI_DBG_BP_BASE_TRAPID;
		}
	}

	// work out stack size
	const uint16_t sp = z80_sp(&ZXEmuState.cpu);	// this won't get the proper stack pos (see comment above function)
	if (sp < state.StackMin)
		state.StackMin = sp;
	if (sp > state.StackMax)
		state.StackMax = sp;

	// work out instruction count
	int iCount = 1;
	uint8_t opcode = ReadByte(pc);
	if (opcode == 0xED || opcode == 0xCB)
		iCount++;

	RZXManager.RegisterInstructions(iCount);

	return trapId;
}

int UIEvalBreakpoint(ui_dbg_t* dbg_win, uint16_t pc, int ticks, uint64_t pins, void* user_data)
{
	return 0;
}

extern uint16_t g_PC;

// Note - you can't read the cpu vars during tick
// They are only written back at end of exec function
uint64_t FSpectrumEmu::Z80Tick(int num, uint64_t pins)
{
	FCodeAnalysisState &state = CodeAnalysis;
	const uint16_t pc = g_PC;	// hack because we can't get it another way

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
			if (state.bRegisterDataAccesses)
				RegisterDataRead(state, pc, addr);
		}
		else if (pins & Z80_WR) 
		{
			if (state.bRegisterDataAccesses)
				RegisterDataWrite(state, pc, addr);

			state.SetLastWriterForAddress(addr,pc);

			// Log screen pixel writes
			if (addr >= 0x4000 && addr < 0x5800)
			{
				FrameScreenPixWrites.push_back({ addr,value, pc });
				
			}
			// Log screen attribute writes
			if (addr >= 0x5800 && addr < 0x5800 + 0x400)
			{
				FrameScreenAttrWrites.push_back({ addr,value, pc });
			}
			FCodeInfo *pCodeWrittenTo = state.GetCodeInfoForAddress(addr);
			if (pCodeWrittenTo != nullptr && pCodeWrittenTo->bSelfModifyingCode == false)
				pCodeWrittenTo->bSelfModifyingCode = true;
		}
	}
	else if (pins & Z80_IORQ)
	{
		IOAnalysis.IOHandler(pc, pins);
	}

	pins =  OldTickCB(num, pins, OldTickUserData);

	// RZX playback
	if (RZXManager.GetReplayMode() == EReplayMode::Playback)
	{
		if ((pins & Z80_IORQ) && (pins & Z80_RD))
		{
			uint8_t inVal = 0;

			if (RZXManager.GetInput(inVal))
			{
				Z80_SET_DATA(pins, (uint64_t)inVal);
			}
		}
	}
	return pins;
}

static uint64_t Z80TickThunk(int num, uint64_t pins, void* user_data)
{
	FSpectrumEmu* pEmu = (FSpectrumEmu*)user_data;
	return pEmu->Z80Tick(num, pins);
}

bool FSpectrumEmu::Init(const FSpectrumConfig& config)
{
	// Initialise Emulator
		
	// setup pixel buffer
	const size_t pixelBufferSize = 320 * 256 * 4;
	FrameBuffer = new unsigned char[pixelBufferSize * 2];

	// setup texture
	Texture = ImGui_ImplDX11_CreateTextureRGBA(static_cast<unsigned char*>(FrameBuffer), 320, 256);
	// setup emu
	zx_type_t type = config.Model == ESpectrumModel::Spectrum128K ? ZX_TYPE_128 : ZX_TYPE_48K;
	zx_joystick_type_t joy_type = ZX_JOYSTICKTYPE_NONE;

	zx_desc_t desc;
	memset(&desc, 0, sizeof(zx_desc_t));
	desc.type = type;
	desc.joystick_type = joy_type;
	desc.pixel_buffer = FrameBuffer;
	desc.pixel_buffer_size = pixelBufferSize;
	desc.audio_cb = PushAudio;	// our audio callback
	desc.audio_sample_rate = saudio_sample_rate();
	desc.rom_zx48k = dump_amstrad_zx48k_bin;
	desc.rom_zx48k_size = sizeof(dump_amstrad_zx48k_bin);
	desc.rom_zx128_0 = dump_amstrad_zx128k_0_bin;
	desc.rom_zx128_0_size = sizeof(dump_amstrad_zx128k_0_bin);
	desc.rom_zx128_1 = dump_amstrad_zx128k_1_bin;
	desc.rom_zx128_1_size = sizeof(dump_amstrad_zx128k_1_bin);

	zx_init(&ZXEmuState, &desc);

	GamesList.Init(this);
	GamesList.EnumerateGames("./Games");

	RZXManager.Init(this);
	RZXGamesList.Init(this);
	RZXGamesList.EnumerateGames("./RZX");

	// Clear UI
	memset(&UIZX, 0, sizeof(ui_zx_t));

	// Trap callback needs to be set before we create the UI
	z80_trap_cb(&ZXEmuState.cpu, ZXSpectrumTrapCallback, this);

	// Setup out tick callback
	OldTickCB = ZXEmuState.cpu.tick_cb;
	OldTickUserData = ZXEmuState.cpu.user_data;
	ZXEmuState.cpu.tick_cb = Z80TickThunk;
	ZXEmuState.cpu.user_data = this;

	//ui_init(zxui_draw);
	{
		ui_zx_desc_t desc = { 0 };
		desc.zx = &ZXEmuState;
		desc.boot_cb = boot_cb;
		desc.create_texture_cb = gfx_create_texture;
		desc.update_texture_cb = gfx_update_texture;
		desc.destroy_texture_cb = gfx_destroy_texture;
		desc.dbg_keys.break_keycode = ImGui::GetKeyIndex(ImGuiKey_Space);
		desc.dbg_keys.break_name = "F5";
		desc.dbg_keys.continue_keycode = VK_F5;
		desc.dbg_keys.continue_name = "F5";
		desc.dbg_keys.step_over_keycode = VK_F6;
		desc.dbg_keys.step_over_name = "F6";
		desc.dbg_keys.step_into_keycode = VK_F7;
		desc.dbg_keys.step_into_name = "F7";
		desc.dbg_keys.toggle_breakpoint_keycode = VK_F9;
		desc.dbg_keys.toggle_breakpoint_name = "F9";
		ui_zx_init(&UIZX, &desc);
	}

	// additional debugger config
	//pUI->UIZX.dbg.ui.open = true;
	UIZX.dbg.break_cb = UIEvalBreakpoint;
	
	//UIZX.dbg.ui.show_breakpoints = true;

	// Setup Disassembler for function view
	{
		FDasmDesc desc;
		desc.LayerNames[0] = "CPU Mapped";
		desc.LayerNames[1] = "Layer 0";
		desc.LayerNames[2] = "Layer 1";
		desc.LayerNames[3] = "Layer 2";
		desc.LayerNames[4] = "Layer 3";
		desc.LayerNames[5] = "Layer 4";
		desc.LayerNames[6] = "Layer 5";
		desc.LayerNames[7] = "Layer 6";
		desc.CPUType = DasmCPUType::Z80;
		desc.StartAddress = 0x0000;
		desc.ReadCB = MemReadFunc;
		desc.pUserData = &ZXEmuState;
		desc.pEmulator = this;
		desc.Title = "FunctionDasm";
		DasmInit(&FunctionDasm, &desc);
	}

	// This is where we add the viewers we want
	Viewers.push_back(new FBreakpointViewer(this));
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
	

	// register Viewers
	RegisterStarquakeViewer(this);
	RegisterGames(this);

	LoadGameConfigs(this);

	// Set up code analysis
	// initialise code analysis pages
	
	// ROM
	for (int pageNo = 0; pageNo < kNoROMPages; pageNo++)
	{
		ROMPages[pageNo].Initialise(pageNo * FCodeAnalysisPage::kPageSize);
		CodeAnalysis.SetCodeAnalysisRWPage(pageNo, &ROMPages[pageNo], &ROMPages[pageNo]);	// Read Only
	}
	// RAM
	const uint16_t RAMStartAddr = kNoROMPages * FCodeAnalysisPage::kPageSize;
	for (int pageNo = 0; pageNo < kNoRAMPages; pageNo++)
	{
		RAMPages[pageNo].Initialise(RAMStartAddr + (pageNo * FCodeAnalysisPage::kPageSize));
		CodeAnalysis.SetCodeAnalysisRWPage(pageNo + kNoROMPages, &RAMPages[pageNo], &RAMPages[pageNo]);	// Read/Write
	}

	// run initial analysis
	InitialiseCodeAnalysis(CodeAnalysis,this);
	LoadROMData(CodeAnalysis, "GameData/RomInfo.bin");
	
	return true;
}

void FSpectrumEmu::Shutdown()
{
	SaveCurrentGameData();	// save on close
}



void FSpectrumEmu::StartGame(FGameConfig *pGameConfig)
{
	MemoryAccessHandlers.clear();	// remove old memory handlers

	ResetMemoryStats(MemStats);
	
	// Reset Functions
	FunctionStack.clear();
	Functions.clear();

	// start up game
	if(pActiveGame!=nullptr)
		delete pActiveGame->pViewerData;
	delete pActiveGame;
	
	FGame *pNewGame = new FGame;
	pNewGame->pConfig = pGameConfig;
	pNewGame->pViewerConfig = pGameConfig->pViewerConfig;
	assert(pGameConfig->pViewerConfig != nullptr);
	GraphicsViewer.pGame = pNewGame;
	pActiveGame = pNewGame;
	pNewGame->pViewerData = pNewGame->pViewerConfig->pInitFunction(this, pGameConfig);
	GenerateSpriteListsFromConfig(GraphicsViewer, pGameConfig);
	
	// Initialise code analysis
	InitialiseCodeAnalysis(CodeAnalysis, this);

	// load game data if we can
	std::string dataFName = "GameData/" + pGameConfig->Name + ".bin";
	LoadGameData(CodeAnalysis, dataFName.c_str());
	LoadROMData(CodeAnalysis, "GameData/RomInfo.bin");
	ReAnalyseCode(CodeAnalysis);
	GenerateGlobalInfo(CodeAnalysis);
}

bool FSpectrumEmu::StartGame(const char *pGameName)
{
	for (const auto& pGameConfig : GetGameConfigs())
	{
		if (pGameConfig->Name == pGameName)
		{
			std::string gameFile = pGameConfig->SnapshotFile;	
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
		const FGameConfig *pGameConfig = pActiveGame->pConfig;
		if (pGameConfig->Name.empty())
		{
			
		}
		else
		{
			const std::string configFName = "Configs/" + pGameConfig->Name + ".json";
			const std::string dataFName = "GameData/" + pGameConfig->Name + ".bin";
			EnsureDirectoryExists("Configs");
			EnsureDirectoryExists("GameData");

			SaveGameConfigToFile(*pGameConfig, configFName.c_str());
			SaveGameData(CodeAnalysis, dataFName.c_str());
		}
	}
	SaveROMData(CodeAnalysis, "GameData/RomInfo.bin");
}

void FSpectrumEmu::DrawMainMenu(double timeMS)
{
	ui_zx_t* pZXUI = &UIZX;
	assert(pZXUI && pZXUI->zx && pZXUI->boot_cb);
		
	if (ImGui::BeginMainMenuBar()) 
	{
		if (ImGui::BeginMenu("File"))
		{
			if (ImGui::BeginMenu("New Game from Snapshot File"))
			{
				for(int gameNo=0;gameNo<GamesList.GetNoGames();gameNo++)
				{
					const FGameSnapshot& game = GamesList.GetGame(gameNo);
					
					if (ImGui::MenuItem(game.DisplayName.c_str()))
					{
						if (GamesList.LoadGame(gameNo))
						{
							FGameConfig *pNewConfig = CreateNewGameConfigFromSnapshot(game);
							if(pNewConfig != nullptr)
								StartGame(pNewConfig);
						}
					}
				}

				ImGui::EndMenu();
			}

			if (ImGui::BeginMenu("New Game from RZX File"))
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
				ImGui::EndMenu();
			}

			if (ImGui::BeginMenu( "Open Game"))
			{
				for (const auto& pGameConfig : GetGameConfigs())
				{
					if (ImGui::MenuItem(pGameConfig->Name.c_str()))
					{
						const std::string gameFile = pGameConfig->SnapshotFile;

						if(GamesList.LoadGame(gameFile.c_str()))
						{
							StartGame(pGameConfig);
						}
					}
				}

				ImGui::EndMenu();
			}

			if (ImGui::MenuItem("Open POK File..."))
			{
				std::string pokFile;
				OpenFileDialog(pokFile, ".\\POKFiles", "POK\0*.pok\0");
			}

			if (ImGui::MenuItem("Import SkoolKit Skool File"))
			{
				std::string skoolFile;
				OpenFileDialog(skoolFile, ".\\SkoolFiles", "*.skool\0");
				ImportSkoolKitFile(CodeAnalysis, skoolFile.c_str());
			}
			
			if (ImGui::MenuItem("Save Game Data"))
			{
				SaveCurrentGameData();
			}
			if (ImGui::MenuItem("Export Binary File"))
			{
				if (pActiveGame != nullptr)
				{
					EnsureDirectoryExists("OutputBin/");
					std::string outBinFname = "OutputBin/" + pActiveGame->pConfig->Name + ".bin";
					uint8_t *pSpecMem = new uint8_t[65536];
					for (int i = 0; i < 65536; i++)
						pSpecMem[i] = ReadByte(i);
					SaveBinaryFile(outBinFname.c_str(), pSpecMem, 65536);
					delete pSpecMem;
				}
			}

			if (ImGui::MenuItem("Export ASM File"))
			{
				if (pActiveGame != nullptr)
				{
					EnsureDirectoryExists("OutputASM/");
					std::string outBinFname = "OutputASM/" + pActiveGame->pConfig->Name + ".asm";

					OutputCodeAnalysisToTextFile(CodeAnalysis, outBinFname.c_str(),0x4000,0xffff);
				}
			}

			if (ImGui::MenuItem("Export SkoolKit Control File"))
			{
				if (pActiveGame != nullptr)
				{
					EnsureDirectoryExists("OutputASM/");
					std::string gameName = pActiveGame->pConfig->Name;
					std::string outCtlFname = "OutputASM/" + gameName + ".ctl";

					ExportSkoolKitControlFile(CodeAnalysis, outCtlFname.c_str(), gameName.c_str(), 0x4000, 0xffff);
				}
			}

			// TODO: export data for skookit
			if (ImGui::MenuItem("Export Region Info File"))
			{
			}
			ImGui::EndMenu();
		}
		
		if (ImGui::BeginMenu("System")) 
		{
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
		if (ImGui::BeginMenu("Windows"))
		{
			for (auto Viewer : Viewers)
			{
				ImGui::MenuItem(Viewer->GetName(), 0, &Viewer->bOpen);

			}
			ImGui::EndMenu();
		}
		if (ImGui::BeginMenu("Debug")) 
		{
			//ImGui::MenuItem("CPU Debugger", 0, &pZXUI->dbg.ui.open);
			//ImGui::MenuItem("Breakpoints", 0, &pZXUI->dbg.ui.show_breakpoints);
			ImGui::MenuItem("Memory Heatmap", 0, &pZXUI->dbg.ui.show_heatmap);
			if (ImGui::BeginMenu("Memory Editor")) 
			{
				ImGui::MenuItem("Window #1", 0, &pZXUI->memedit[0].open);
				ImGui::MenuItem("Window #2", 0, &pZXUI->memedit[1].open);
				ImGui::MenuItem("Window #3", 0, &pZXUI->memedit[2].open);
				ImGui::MenuItem("Window #4", 0, &pZXUI->memedit[3].open);
				ImGui::EndMenu();
			}
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

void DrawDebuggerUI(ui_dbg_t *pDebugger)
{
	if (ImGui::Begin("CPU Debugger"))
	{
		ui_dbg_dbgwin_draw(pDebugger);
	}
	ImGui::End();
	ui_dbg_draw(pDebugger);
	/*
	if (!(pDebugger->ui.open || pDebugger->ui.show_heatmap || pDebugger->ui.show_breakpoints)) {
		return;
	}
	_ui_dbg_dbgwin_draw(pDebugger);
	_ui_dbg_heatmap_draw(pDebugger);
	_ui_dbg_bp_draw(pDebugger);*/
}

void FSpectrumEmu::Tick()
{
	ExecThisFrame = ui_zx_before_exec(&UIZX);

	if (ExecThisFrame)
	{
		const float frameTime = min(1000000.0f / ImGui::GetIO().Framerate, 32000.0f) * ExecSpeedScale;
		const uint32_t microSeconds = max(static_cast<uint32_t>(frameTime), uint32_t(1));
		
		// TODO: Start frame method in analyser
		CodeAnalysis.FrameTrace.clear();
		
		/*if (RZXManager.GetReplayMode() == EReplayMode::Playback)
		{
			assert(ZXEmuState.valid);
			uint32_t icount = RZXManager.Update();

			uint32_t ticks_to_run = clk_ticks_to_run(&ZXEmuState.clk, microSeconds);
			uint32_t ticks_executed = z80_exec(&ZXEmuState.cpu, ticks_to_run);
			clk_ticks_executed(&ZXEmuState.clk, ticks_executed);
			kbd_update(&ZXEmuState.kbd);
		}
		else*/
		{
			zx_exec(&ZXEmuState, microSeconds);
		}
		ImGui_ImplDX11_UpdateTextureRGBA(Texture, FrameBuffer);

		FrameTraceViewer.CaptureFrame();
		FrameScreenPixWrites.clear();
		FrameScreenAttrWrites.clear();

		if (bStepToNextFrame)
		{
			_ui_dbg_break(&UIZX.dbg);
			CodeAnalyserGoToAddress(CodeAnalysis, GetPC());
			bStepToNextFrame = false;
		}

		// on debug break send code analyser to address
		if (UIZX.dbg.dbg.z80->trap_id >= UI_DBG_STEP_TRAPID)
		{
			CodeAnalyserGoToAddress(CodeAnalysis, GetPC());
		}
	}

	

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
	
	if(ExecThisFrame)
		ui_zx_after_exec(pZXUI);

	const int instructionsThisFrame = (int)CodeAnalysis.FrameTrace.size();
	static int maxInst = 0;
	maxInst = max(maxInst, instructionsThisFrame);

	/*// Store trace and frame image
	if (ExecThisFrame)
	{
		FrameTraceViewer.CaptureFrame();
		CodeAnalysis.FrameTrace.clear();
	}*/
	DrawMainMenu(timeMS);
	if (pZXUI->memmap.open)
	{
		UpdateMemmap(pZXUI);
	}

	// call the Chips UI functions
	ui_audio_draw(&pZXUI->audio, pZXUI->zx->sample_pos);
	ui_z80_draw(&pZXUI->cpu);
	ui_ay38910_draw(&pZXUI->ay);
	ui_kbd_draw(&pZXUI->kbd);
	ui_memmap_draw(&pZXUI->memmap);

	for (int i = 0; i < 4; i++)
	{
		ui_memedit_draw(&pZXUI->memedit[i]);
		ui_dasm_draw(&pZXUI->dasm[i]);
	}

	DrawDebuggerUI(&pZXUI->dbg);

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
	if (ImGui::Begin("Cheats"))
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

	DrawGraphicsViewer(GraphicsViewer);
	DrawMemoryTools();
	if (ImGui::Begin("Code Analysis"))
	{
		DrawCodeAnalysisData(CodeAnalysis);
	}
	ImGui::End();

	if (ImGui::Begin("Execution"))
	{
		DrawExecutionInfo(CodeAnalysis);
	}
	ImGui::End();
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

void FSpectrumEmu::DrawCheatsUI()
{
	if (pActiveGame == nullptr)
		return;
	
	FGameConfig &config = *pActiveGame->pConfig;

	for (FCheat &cheat : config.Cheats)
	{
		ImGui::PushID(cheat.Description.c_str());
		ImGui::Text(cheat.Description.c_str());
		ImGui::SameLine();
		if (ImGui::Checkbox("##cheatBox", &cheat.bEnabled))
		{
			for (auto &entry : cheat.Entries)
			{
				if (cheat.bEnabled)	// cheat activated
				{
					// store old value
					entry.OldValue = ReadByte( entry.Address);
					WriteByte( entry.Address, entry.Value);
				}
				else
				{
					WriteByte( entry.Address, entry.OldValue);
				}
			}
		}
		ImGui::PopID();
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

void GetScreenAddressCoords(uint16_t addr, int& x, int &y)
{
	const int y02 = (addr >> 8) & 7;	// bits 0-2
	const int y35 = (addr >> 5) & 7;	// bits 3-5
	const int y67 = (addr >> 11) & 3;	// bits 6 & 7
	x = (addr & 31) * 8;
	y = y02 | (y35 << 3) | (y67 << 6);
}
