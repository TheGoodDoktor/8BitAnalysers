
#include "SpeccyUI.h"
#include <windows.h>

#include "Speccy/Speccy.h"

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

void DrawSpeccyUI(FSpeccy &speccyInstance)
{
	ui_zx_draw(&g_UIZX, 16);//TODO
}