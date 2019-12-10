
#include "SpeccyUI.h"
#include <windows.h>

#include "GameConfig.h"
#include "imgui_impl_lucidextra.h"
#include "GameViewers/GameViewer.h"
#include "GameViewers/StarquakeViewer.h"
#include "GameViewers/MiscGameViewers.h"
#include "GraphicsView.h"
#include "Util/FileUtil.h"

#include "ui/ui_dbg.h"
#include "MemoryHandlers.h"
#include "FunctionHandlers.h"
#include "Disassembler.h"
#include "CodeAnalyser/CodeAnalyser.h"
#include "CodeAnalyser/CodeAnalyserUI.h"

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

int UITrapCallback(uint16_t pc, int ticks, uint64_t pins, void* user_data)
{
	FSpeccyUI *pUI = (FSpeccyUI *)user_data;
	FCodeAnalysisState &state = pUI->CodeAnalysis;

	const uint16_t nextpc = pc;
	// store program count in history
	const uint16_t prevPC = pUI->PCHistory[pUI->PCHistoryPos];
	pUI->PCHistoryPos = (pUI->PCHistoryPos + 1) % FSpeccyUI::kPCHistorySize;
	pUI->PCHistory[pUI->PCHistoryPos] = pc;

	pc = prevPC;	// set PC to pc of instruction just executed

	RunStaticCodeAnalysis(state, pc);
	state.CodeInfo[pc]->FrameLastAccessed = state.CurrentFrameNo;

	// labels
	//GenerateLabelsForAddress(pUI, pc,LabelType::Code);

	int trapId = MemoryHandlerTrapFunction(pc, ticks, pins, pUI);

	//if(trapId == 0)
		//trapId = FunctionTrapFunction(pc,nextpc, ticks, pins, pUI);
	
	
	return trapId;
}

int UIEvalBreakpoint(ui_dbg_t* dbg_win, uint16_t pc, int ticks, uint64_t pins, void* user_data)
{
	return 0;
}


FSpeccyUI* InitSpeccyUI(FSpeccy *pSpeccy)
{
	FSpeccyUI *pUI = new FSpeccyUI;
	memset(&pUI->UIZX, 0, sizeof(ui_zx_t));

	// Trap callback needs to be set before we create the UI
	z80_trap_cb(&pSpeccy->CurrentState.cpu, UITrapCallback, pUI);

	pUI->pSpeccy = pSpeccy;
	//ui_init(zxui_draw);
	{
		ui_zx_desc_t desc = { 0 };
		desc.zx = &pSpeccy->CurrentState;
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
		ui_zx_init(&pUI->UIZX, &desc);
	}

	// additional debugger config
	pUI->UIZX.dbg.ui.open = true;
	pUI->UIZX.dbg.break_cb = UIEvalBreakpoint;

	// Setup Disassembler for function view
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
	desc.pUserData = &pSpeccy->CurrentState;
	desc.pUI = pUI;
	desc.Title = "FunctionDasm";
	DasmInit(&pUI->FunctionDasm, &desc);

	// setup pixel buffer
	pUI->pGraphicsViewerView = CreateGraphicsView(64, 64);
	/*const int graphicsViewSize = 64;
	const size_t pixelBufferSize = graphicsViewSize * graphicsViewSize * 4;
	pUI->GraphicsViewPixelBuffer = new unsigned char[pixelBufferSize];

	pUI->GraphicsViewTexture = ImGui_ImplDX11_CreateTextureRGBA(pUI->GraphicsViewPixelBuffer, graphicsViewSize, graphicsViewSize);
	*/
	// register Viewers
	RegisterStarquakeViewer(pUI);
	RegisterGames(pUI);

	LoadGameConfigs(pUI);

	FCodeAnalysisState &state = pUI->CodeAnalysis;
	memset(state.Labels, 0, sizeof(state.Labels));
	memset(state.CodeInfo, 0, sizeof(state.CodeInfo));
	memset(state.DataInfo, 0, sizeof(state.DataInfo));

	for (int addr = 0; addr < (1 << 16); addr++)
	{
		// set up data entry for address
		FDataInfo *pDataInfo = new FDataInfo;
		pDataInfo->Address = (uint16_t)addr;
		pDataInfo->ByteSize = 1;
		pDataInfo->DataType = DataType::Byte;
		state.DataInfo[addr] = pDataInfo;
	}

	// run initial analysis
	InitialiseCodeAnalysis(pUI->CodeAnalysis,pUI->pSpeccy);
	LoadROMData(pUI->CodeAnalysis, "GameData/RomInfo.bin");
	
	return pUI;
}

void ShutdownSpeccyUI(FSpeccyUI* pUI)
{

}



void StartGame(FSpeccyUI* pUI, FGameConfig *pGameConfig)
{
	pUI->MemoryAccessHandlers.clear();	// remove old memory handlers

	ResetMemoryStats(pUI->MemStats);
	
	// Reset Functions
	pUI->FunctionStack.clear();
	pUI->Functions.clear();

	// start up game
	if(pUI->pActiveGame!=nullptr)
		delete pUI->pActiveGame->pViewerData;
	delete pUI->pActiveGame;
	
	pUI->pActiveGame = new FGame;
	pUI->pActiveGame->pConfig = pGameConfig;
	pUI->pActiveGame->pViewerData = pGameConfig->pInitFunction(pUI, pGameConfig);

	// Initialise code analysis
	InitialiseCodeAnalysis(pUI->CodeAnalysis, pUI->pSpeccy);

	// load game data if we can
	std::string dataFName = "GameData/" + pGameConfig->Name + ".bin";
	LoadGameData(pUI->CodeAnalysis, dataFName.c_str());
	LoadROMData(pUI->CodeAnalysis, "GameData/RomInfo.bin");

}

bool StartGame(FSpeccyUI* pUI, const char *pGameName)
{
	for (const auto& pGameConfig : pUI->GameConfigs)
	{
		if (pGameConfig->Name == pGameName)
		{
			if (LoadZ80File(*pUI->pSpeccy, pGameConfig->Z80File.c_str()))
			{
				StartGame(pUI, pGameConfig);
				return true;
			}
		}
	}

	return false;
}

// save config & data
void SaveCurrentGameData(FSpeccyUI *pUI)
{
	if (pUI->pActiveGame != nullptr)
	{
		const FGameConfig *pGameConfig = pUI->pActiveGame->pConfig;
		const std::string configFName = "Configs/" + pGameConfig->Name + ".json";
		const std::string dataFName = "GameData/" + pGameConfig->Name + ".bin";
		EnsureDirectoryExists("Configs");
		EnsureDirectoryExists("GameData");

		SaveGameConfigToFile(*pGameConfig, configFName.c_str());
		SaveGameData(pUI->CodeAnalysis, dataFName.c_str());
	}
	SaveROMData(pUI->CodeAnalysis, "GameData/RomInfo.bin");
}

static void DrawMainMenu(FSpeccyUI* pUI, double timeMS)
{
	ui_zx_t* pZXUI = &pUI->UIZX;
	FSpeccy *pSpeccy = pUI->pSpeccy;
	assert(pZXUI && pZXUI->zx && pZXUI->boot_cb);
	
	if (ImGui::BeginMainMenuBar()) 
	{
		if (ImGui::BeginMenu("File"))
		{
			
			if (ImGui::BeginMenu( "Open Game"))
			{
				for (const auto& pGameConfig : pUI->GameConfigs)
				{
					if (ImGui::MenuItem(pGameConfig->Name.c_str()))
					{
						if(LoadZ80File(*pSpeccy, pGameConfig->Z80File.c_str()))
						{
							StartGame(pUI,pGameConfig);
						}
					}
				}

				ImGui::EndMenu();
			}

			if (ImGui::BeginMenu("Open Z80 File"))
			{
				for (const auto& game : GetGameList())
				{
					if (ImGui::MenuItem(game.c_str()))
					{
						if (LoadZ80File(*pSpeccy, game.c_str()))
						{
							pUI->pActiveGame = nullptr;
						}
					}
				}

				ImGui::EndMenu();
			}
			if (ImGui::MenuItem("Save Game Data"))
			{
				SaveCurrentGameData(pUI);
			}
			if (ImGui::MenuItem("Export Binary File"))
			{
				if (pUI->pActiveGame != nullptr)
				{
					EnsureDirectoryExists("OutputBin/");
					std::string outBinFname = "OutputBin/" + pUI->pActiveGame->pConfig->Name + ".bin";
					uint8_t *pSpecMem = new uint8_t[65536];
					for (int i = 0; i < 65536; i++)
						pSpecMem[i] = ReadySpeccyByte(pSpeccy, i);
					SaveBinaryFile(outBinFname.c_str(), pSpecMem, 65536);
					delete pSpecMem;
				}
			}

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
			if (ImGui::MenuItem("ZX Spectrum 48K", 0, (pZXUI->zx->type == ZX_TYPE_48K)))
			{
				pZXUI->boot_cb(pZXUI->zx, ZX_TYPE_48K);
				ui_dbg_reboot(&pZXUI->dbg);
			}
			if (ImGui::MenuItem("ZX Spectrum 128", 0, (pZXUI->zx->type == ZX_TYPE_128)))
			{
				pZXUI->boot_cb(pZXUI->zx, ZX_TYPE_128);
				ui_dbg_reboot(&pZXUI->dbg);
			}
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
		if (ImGui::BeginMenu("Debug")) 
		{
			ImGui::MenuItem("CPU Debugger", 0, &pZXUI->dbg.ui.open);
			ImGui::MenuItem("Breakpoints", 0, &pZXUI->dbg.ui.show_breakpoints);
			ImGui::MenuItem("Memory Heatmap", 0, &pZXUI->dbg.ui.show_heatmap);
			if (ImGui::BeginMenu("Memory Editor")) 
			{
				ImGui::MenuItem("Window #1", 0, &pZXUI->memedit[0].open);
				ImGui::MenuItem("Window #2", 0, &pZXUI->memedit[1].open);
				ImGui::MenuItem("Window #3", 0, &pZXUI->memedit[2].open);
				ImGui::MenuItem("Window #4", 0, &pZXUI->memedit[3].open);
				ImGui::EndMenu();
			}
			if (ImGui::BeginMenu("Disassembler")) 
			{
				ImGui::MenuItem("Window #1", 0, &pZXUI->dasm[0].open);
				ImGui::MenuItem("Window #2", 0, &pZXUI->dasm[1].open);
				ImGui::MenuItem("Window #3", 0, &pZXUI->dasm[2].open);
				ImGui::MenuItem("Window #4", 0, &pZXUI->dasm[3].open);
				ImGui::EndMenu();
			}
			ImGui::EndMenu();
		}
		if (ImGui::BeginMenu("ImGui"))
		{
			ImGui::MenuItem("Show Demo", 0, &pUI->bShowImGuiDemo);
			ImGui::EndMenu();
		}
		

		/*if (ImGui::BeginMenu("Game Viewers"))
		{
			for (auto &viewerIt : pUI->GameViewers)
			{
				FGameViewer &viewer = viewerIt.second;
				ImGui::MenuItem(viewerIt.first.c_str(), 0, &viewer.bOpen);
			}
			ImGui::EndMenu();
		}*/
		
		ui_util_options_menu(timeMS, pZXUI->dbg.dbg.stopped);

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
	ui_dbg_draw(pDebugger);
	/*
	if (!(pDebugger->ui.open || pDebugger->ui.show_heatmap || pDebugger->ui.show_breakpoints)) {
		return;
	}
	_ui_dbg_dbgwin_draw(pDebugger);
	_ui_dbg_heatmap_draw(pDebugger);
	_ui_dbg_bp_draw(pDebugger);*/
}

void UpdatePreTickSpeccyUI(FSpeccyUI* pUI)
{
	pUI->pSpeccy->ExecThisFrame = ui_zx_before_exec(&pUI->UIZX);
}

static const uint32_t g_kColourLUT[8]=
{
	0xFF000000,     // black
	0xFFFF0000,     // blue
	0xFF0000FF,     // red
	0xFFFF00FF,     // magenta
	0xFF00FF00,     // green
	0xFFFFFF00,     // cyan
	0xFF00FFFF,     // yellow
	0xFFFFFFFF,     // white
};

// coords are in pixel units
// w & h in characters
void PlotImageAt(const uint8_t *pSrc, int xp,int yp,int w,int h,uint32_t *pDest, int destWidth, uint8_t colAttr)
{
	uint32_t* pBase = pDest + (xp + (yp * destWidth));
	uint32_t inkCol = g_kColourLUT[colAttr & 7];
	uint32_t paperCol = g_kColourLUT[(colAttr>>3) & 7];

	if (0 == (colAttr & (1 << 6))) 
	{
		// standard brightness
		inkCol &= 0xFFD7D7D7;
		paperCol &= 0xFFD7D7D7;
	}
	
	*pBase = 0;
	for(int y=0;y<h*8;y++)
	{
		for (int x = 0; x < w; x++)
		{
			const uint8_t charLine = *pSrc++;

			for (int xpix = 0; xpix < 8; xpix++)
			{
				const bool bSet = (charLine & (1 << (7 - xpix))) != 0;
				const uint32_t col = bSet ? inkCol : paperCol;
				*(pBase + xpix + (x * 8)) = col;
			}
		}

		pBase += destWidth;
	}
}

/*void PlotCharacterBlockAt(const FSpeccy *pSpeccy,uint16_t addr, int xp, int yp,int w,int h, uint32_t *pDest, int destWidth)
{
	uint16_t currAddr = addr;

	for (int y = yp; y < yp + h; y++)
	{
		for (int x = xp; x < xp + w; x++)
		{
			const uint8_t *pChar = GetSpeccyMemPtr(*pSpeccy, currAddr);
			PlotImageAt(pChar, x, y,1,1 pDest, destWidth);
			currAddr += 8;
		}
	}
}*/

void DrawGraphicsView(FSpeccyUI* pUI)
{
	FGraphicsView *pGraphicsView = pUI->pGraphicsViewerView;
	static int memOffset = 0;
	static int xs = 1;
	static int ys = 1;

	int byteOff = 0;
	int offsetMax = 0xffff - (64 * 8);
	
	if (ImGui::Begin("Graphics View") == false)
		return;
	
	ImGui::Text("Memory Map Address: 0x%x", memOffset);
	DrawGraphicsView(*pGraphicsView);
	ImGui::SameLine();
	ImGui::VSliderInt("##int", ImVec2(64, 256), &memOffset, 0, offsetMax);//,"0x%x");
	ImGui::InputInt("Address", &memOffset,1,8, ImGuiInputTextFlags_CharsHexadecimal);
	if (ImGui::Button("<<"))
		memOffset -= xs * ys * 8;
	ImGui::SameLine();
	if (ImGui::Button(">>"))
		memOffset += xs * ys * 8;

	ClearGraphicsView(*pGraphicsView, 0xff000000);

	// view 1 - straight character
	// draw 64 * 8 bytes
	ImGui::InputInt("XSize", &xs, 1, 4);
	ImGui::InputInt("YSize", &ys, 1, 4);

	static char configName[64];
	ImGui::Separator();
	ImGui::InputText("Config Name", configName, 64);
	ImGui::SameLine();
	if (ImGui::Button("Store"))
	{
		// TODO: store this in the config map
	}

	xs = min(max(1, xs), 8);
	ys = min(max(1, ys), 8);
	
	const int xcount = 8 / xs;
	const int ycount = 8 / ys;

	uint16_t speccyAddr = memOffset;
	int y = 0;
	for (int y = 0; y < ycount; y++)
	{
		for (int x = 0; x < xcount; x++)
		{
			const uint8_t *pImage = GetSpeccyMemPtr(pUI->pSpeccy, speccyAddr);
			PlotImageAt(pImage, x * xs * 8, y * ys * 8, xs, ys, (uint32_t*)pGraphicsView->PixelBuffer, 64);
			speccyAddr += xs * ys * 8;
		}
	}

	ImGui::End();
}




void DrawMemoryTools(FSpeccyUI* pUI)
{
	if (ImGui::Begin("Tools") == false)
		return;

	if (ImGui::BeginTabBar("MemoryToolsTabBar"))
	{

		if (ImGui::BeginTabItem("Memory Handlers"))
		{
			DrawMemoryHandlers(pUI);
			ImGui::EndTabItem();
		}

		if (ImGui::BeginTabItem("Memory Analysis"))
		{
			DrawMemoryAnalysis(pUI);
			ImGui::EndTabItem();
		}
		
		if (ImGui::BeginTabItem("Functions"))
		{
			DrawFunctionInfo(pUI);
			ImGui::EndTabItem();
		}

		ImGui::EndTabBar();
	}

	ImGui::End();
}


void ReadSpeccyKeys(FSpeccy *pSpeccy)
{
	ImGuiIO &io = ImGui::GetIO();
	for(int i=0;i<10;i++)
	{
		if (io.KeysDown[0x30+i] == 1)
		{
			zx_key_down(&pSpeccy->CurrentState, '0' + i);
			zx_key_up(&pSpeccy->CurrentState, '0' + i);
		}
	}

	for (int i = 0; i < 26; i++)
	{
		if (io.KeysDown[0x41 + i] == 1)
		{
			zx_key_down(&pSpeccy->CurrentState, 'a' + i);
			zx_key_up(&pSpeccy->CurrentState, 'a' + i);
		}
	}
}


void DrawSpeccyUI(FSpeccyUI* pUI)
{
	ui_zx_t* pZXUI = &pUI->UIZX;
	FSpeccy *pSpeccy = pUI->pSpeccy;
	const double timeMS = 1000.0f / ImGui::GetIO().Framerate;
	
	if(pSpeccy->ExecThisFrame)
		ui_zx_after_exec(pZXUI);
	
	DrawMainMenu(pUI, timeMS);

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

	//DasmDraw(&pUI->FunctionDasm);


	// show spectrum window
	ImGui::Begin("Spectrum View");
	ImGui::Image(pSpeccy->Texture, ImVec2(320, 256));
	ImGui::Text("Application average %.3f ms/frame (%.1f FPS)", 1000.0f / ImGui::GetIO().Framerate, ImGui::GetIO().Framerate);

	// read keys
	if (ImGui::IsWindowFocused())
	{
		ReadSpeccyKeys(pUI->pSpeccy);
	}
	ImGui::End();
	
	if (ImGui::Begin("Game Viewer"))
	{
		if (pUI->pActiveGame != nullptr)
		{
			ImGui::Text(pUI->pActiveGame->pConfig->Name.c_str());
			pUI->pActiveGame->pConfig->pDrawFunction(pUI, pUI->pActiveGame);
		}
		
		ImGui::End();
	}
	
	DrawGraphicsView(pUI);
	DrawMemoryTools(pUI);

	if (ImGui::Begin("Code Analysis"))
	{
		DrawCodeAnalysisData(pUI);
		ImGui::End();
	}
}

bool DrawDockingView(FSpeccyUI *pUI)
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
		DrawSpeccyUI(pUI);
		ImGui::End();
	}
	else
	{
		ImGui::PopStyleVar();
		bQuit = true;
	}

	return bQuit;
}

void UpdatePostTickSpeccyUI(FSpeccyUI* pUI)
{
	DrawDockingView(pUI);
	
}
/*
FGameViewer &AddGameViewer(FSpeccyUI *pUI,const char *pName)
{
	FGameViewer &gameViewer = pUI->GameViewers[pName];
	gameViewer.Name = pName;
	return gameViewer;
}*/


