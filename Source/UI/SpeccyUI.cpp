
#include "SpeccyUI.h"
#include <windows.h>


#include "imgui_impl_lucidextra.h"


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

FSpeccyUI* InitSpeccyUI(FSpeccy *pSpeccy)
{
	FSpeccyUI *pUI = new FSpeccyUI;
	memset(pUI, 0, sizeof(FSpeccyUI));

	pUI->pSpeccy = pSpeccy;
	//ui_init(zxui_draw);
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

	// setup pixel buffer
	const int graphicsViewSize = 64;
	const size_t pixelBufferSize = graphicsViewSize * graphicsViewSize * 4;
	pUI->GraphicsViewPixelBuffer = new unsigned char[pixelBufferSize];

	pUI->GraphicsViewTexture = ImGui_ImplDX11_CreateTextureRGBA(pUI->GraphicsViewPixelBuffer, graphicsViewSize, graphicsViewSize);


	return pUI;
}

void ShutdownSpeccyUI(FSpeccyUI* pUI)
{

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
			if (ImGui::BeginMenu( "Open Z80 File"))
			{
				for (const std::string& file : GetGameList())
				{
					if (ImGui::MenuItem(file.c_str()))
					{
						LoadZ80File(*pSpeccy, file.c_str());
					}
				}

				ImGui::EndMenu();
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

// coords are in character units
void PlotCharacterAt(const uint8_t *pSrc, int x,int y,uint32_t *pDest, int destWidth)
{
	uint32_t* pBase = pDest + ((x*8) + ((y*8) * destWidth));
	
	for(int y=0;y<8;y++)
	{
		uint8_t charLine = *pSrc++;
		
		for(int x=0;x<8;x++)
		{
			const bool bSet = (charLine & (1 << (7 - x))) != 0;
			const uint32_t col = bSet ? 0xffffffff : 0;
			*(pBase + x) = col;
		}

		pBase += destWidth;
	}
}

void DrawGraphicsView(FSpeccyUI* pUI)
{
	static int memBank = 0;
	static int memOffset = 0;
	int byteOff = 0;
	int offsetMax = 0x4000 - (64 * 8);

	int realAddr = 0x4000 + (memBank * 0x40000) + memOffset;
	
	ImGui::Begin("Graphics View");
	ImGui::Text("Memory Map Address: 0x%x", realAddr);
	ImGui::Image(pUI->GraphicsViewTexture, ImVec2(256, 256));
	ImGui::SameLine();
	ImGui::VSliderInt("##int", ImVec2(64, 256), &memOffset, offsetMax, 0);//,"0x%x");
	ImGui::InputInt("Bank", &memBank, 1,1);
	ImGui::InputInt("Bank Address", &memOffset,8*8,64*8, ImGuiInputTextFlags_CharsHexadecimal);

	memBank = min(memBank, 7);
	memBank = max(memBank, 0);
	uint8_t *pGfxMem = &pUI->pSpeccy->CurrentState.ram[memBank][memOffset];
	uint32_t *pPix = (uint32_t*)pUI->GraphicsViewPixelBuffer;

	// view 1 - straight character
	// draw 64 * 8 bytes
	for (int y = 0; y < 8; y++)
	{
		for (int x = 0; x < 8; x++)
		{
			PlotCharacterAt(pGfxMem, x, y, pPix, 64);
			pGfxMem += 8;
		}
	}

	ImGui::End();

	/*

	for (int y = 0; y < 64; y++)
	{ 
		for (int x = 0; x < 64; x++)
		{
			uint8_t memVal = pUI->pSpeccy->CurrentState.ram[0][memOffset + byteOff];
			bool bSet = memVal & (1<<(x & 7)) !=0;

			*pPix = bSet ? 0 : 0xffffffff;
			pPix++;
		}
	}*/
	ImGui_ImplDX11_UpdateTextureRGBA(pUI->GraphicsViewTexture, pUI->GraphicsViewPixelBuffer);
}

void UpdatePostTickSpeccyUI(FSpeccyUI* pUI)
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
	

	// show spectrum window
	ImGui::Begin("Spectrum View");
	ImGui::Image(pSpeccy->Texture, ImVec2(320, 256));
	ImGui::Text("Application average %.3f ms/frame (%.1f FPS)", 1000.0f / ImGui::GetIO().Framerate, ImGui::GetIO().Framerate);
	ImGui::End();


	DrawGraphicsView(pUI);
}

