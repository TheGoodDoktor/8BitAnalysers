
#include "SpeccyUI.h"
#include <windows.h>

#include "Speccy/Speccy.h"
#define UI_DBG_USE_Z80
#define UI_DASM_USE_Z80

#include "chips/z80.h"
#include "chips/beeper.h"
#include "chips/ay38910.h"
#include "chips/mem.h"
#include "chips/kbd.h"
#include "chips/clk.h"
#include "systems/zx.h"
#include "chips/mem.h"
#include "ui/ui_chip.h"
#include "ui/ui_util.h"
#include "ui/ui_z80.h"
#include "ui/ui_ay38910.h"
#include "ui/ui_audio.h"
#include "ui/ui_kbd.h"
#include "ui/ui_dasm.h"
#include "ui/ui_dbg.h"
#include "ui/ui_memedit.h"
#include "ui/ui_memmap.h"
#include "ui/ui_zx.h"
#include "imgui_impl_lucidextra.h"


static ui_zx_t g_UIZX;

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

void InitSpeccyUI(const FSpeccy &speccyInstance)
{
	//ui_init(zxui_draw);
	ui_zx_desc_t desc = { 0 };
	desc.zx = (zx_t*)speccyInstance.EmuState;
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
	ui_zx_init(&g_UIZX, &desc);
}

void ShutdownSpeccyUI(const FSpeccy &speccyInstance)
{

}

static void DrawMainMenu(FSpeccy &speccyInstance, ui_zx_t* pUI , double timeMS)
{
	assert(pUI && pUI->zx && pUI->boot_cb);
	
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
						LoadZ80File(speccyInstance, file.c_str());
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
				zx_reset(pUI->zx);
				ui_dbg_reset(&pUI->dbg);
			}
			if (ImGui::MenuItem("ZX Spectrum 48K", 0, (pUI->zx->type == ZX_TYPE_48K)))
			{
				pUI->boot_cb(pUI->zx, ZX_TYPE_48K);
				ui_dbg_reboot(&pUI->dbg);
			}
			if (ImGui::MenuItem("ZX Spectrum 128", 0, (pUI->zx->type == ZX_TYPE_128)))
			{
				pUI->boot_cb(pUI->zx, ZX_TYPE_128);
				ui_dbg_reboot(&pUI->dbg);
			}
			if (ImGui::BeginMenu("Joystick")) 
			{
				if (ImGui::MenuItem("None", 0, (pUI->zx->joystick_type == ZX_JOYSTICKTYPE_NONE)))
				{
					pUI->zx->joystick_type = ZX_JOYSTICKTYPE_NONE;
				}
				if (ImGui::MenuItem("Kempston", 0, (pUI->zx->joystick_type == ZX_JOYSTICKTYPE_KEMPSTON)))
				{
					pUI->zx->joystick_type = ZX_JOYSTICKTYPE_KEMPSTON;
				}
				if (ImGui::MenuItem("Sinclair #1", 0, (pUI->zx->joystick_type == ZX_JOYSTICKTYPE_SINCLAIR_1)))
				{
					pUI->zx->joystick_type = ZX_JOYSTICKTYPE_SINCLAIR_1;
				}
				if (ImGui::MenuItem("Sinclair #2", 0, (pUI->zx->joystick_type == ZX_JOYSTICKTYPE_SINCLAIR_2)))
				{
					pUI->zx->joystick_type = ZX_JOYSTICKTYPE_SINCLAIR_2;
				}
				ImGui::EndMenu();
			}
			ImGui::EndMenu();
		}
		if (ImGui::BeginMenu("Hardware")) 
		{
			ImGui::MenuItem("Memory Map", 0, &pUI->memmap.open);
			ImGui::MenuItem("Keyboard Matrix", 0, &pUI->kbd.open);
			ImGui::MenuItem("Audio Output", 0, &pUI->audio.open);
			ImGui::MenuItem("Z80 CPU", 0, &pUI->cpu.open);
			if (pUI->zx->type == ZX_TYPE_128)
			{
				ImGui::MenuItem("AY-3-8912", 0, &pUI->ay.open);
			}
			else 
			{
				pUI->ay.open = false;
			}
			ImGui::EndMenu();
		}
		if (ImGui::BeginMenu("Debug")) 
		{
			ImGui::MenuItem("CPU Debugger", 0, &pUI->dbg.ui.open);
			ImGui::MenuItem("Breakpoints", 0, &pUI->dbg.ui.show_breakpoints);
			ImGui::MenuItem("Memory Heatmap", 0, &pUI->dbg.ui.show_heatmap);
			if (ImGui::BeginMenu("Memory Editor")) 
			{
				ImGui::MenuItem("Window #1", 0, &pUI->memedit[0].open);
				ImGui::MenuItem("Window #2", 0, &pUI->memedit[1].open);
				ImGui::MenuItem("Window #3", 0, &pUI->memedit[2].open);
				ImGui::MenuItem("Window #4", 0, &pUI->memedit[3].open);
				ImGui::EndMenu();
			}
			if (ImGui::BeginMenu("Disassembler")) 
			{
				ImGui::MenuItem("Window #1", 0, &pUI->dasm[0].open);
				ImGui::MenuItem("Window #2", 0, &pUI->dasm[1].open);
				ImGui::MenuItem("Window #3", 0, &pUI->dasm[2].open);
				ImGui::MenuItem("Window #4", 0, &pUI->dasm[3].open);
				ImGui::EndMenu();
			}
			ImGui::EndMenu();
		}
		
		ui_util_options_menu(timeMS, pUI->dbg.dbg.stopped);

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

void DrawSpeccyUI(FSpeccy &speccyInstance)
{
	ui_zx_t* pUI = &g_UIZX;
	const double timeMS = 1000.0f / ImGui::GetIO().Framerate;
	
	if (ui_zx_before_exec(&g_UIZX))
	{
		zx_exec(g_UIZX.zx, static_cast<uint32_t>(1000000.0f / ImGui::GetIO().Framerate));
		ui_zx_after_exec(&g_UIZX);
	}
	
	DrawMainMenu(speccyInstance, pUI, timeMS);

	if (pUI->memmap.open)
	{
		UpdateMemmap(pUI);
	}

	// call the Chips UI functions
	ui_audio_draw(&pUI->audio, pUI->zx->sample_pos);
	ui_z80_draw(&pUI->cpu);
	ui_ay38910_draw(&pUI->ay);
	ui_kbd_draw(&pUI->kbd);
	ui_memmap_draw(&pUI->memmap);

	for (int i = 0; i < 4; i++)
	{
		ui_memedit_draw(&pUI->memedit[i]);
		ui_dasm_draw(&pUI->dasm[i]);
	}

	ui_dbg_draw(&pUI->dbg);

	// show spectrum window
	ImGui::Begin("Spectrum View");
	ImGui::Image(speccyInstance.Texture, ImVec2(320, 256));
	ImGui::Text("Application average %.3f ms/frame (%.1f FPS)", 1000.0f / ImGui::GetIO().Framerate, ImGui::GetIO().Framerate);
	ImGui::End();
}