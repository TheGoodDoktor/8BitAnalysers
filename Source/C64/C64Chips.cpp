/*
    UI implementation for c64.c, this must live in a .cc file.
*/
//#include "common.h"
#define CHIPS_IMPL

#include "imgui.h"
#include "chips/m6502.h"
#include "chips/m6526.h"
#include "chips/m6569.h"
#include "chips/m6581.h"
#include "chips/beeper.h"
#include "chips/kbd.h"
#include "chips/mem.h"
#include "chips/clk.h"
#include "systems/c64.h"
#define UI_DASM_USE_M6502
#define UI_DBG_USE_M6502
//#include "ui.h"
#include "util/m6502dasm.h"
#include "ui/ui_util.h"
#include "ui/ui_chip.h"
#include "ui/ui_memedit.h"
#include "ui/ui_memmap.h"
#include "ui/ui_dasm.h"
#include "ui/ui_dbg.h"
#include "ui/ui_m6502.h"
#include "ui/ui_m6526.h"
#include "ui/ui_m6581.h"
#include "ui/ui_m6569.h"
#include "ui/ui_audio.h"
#include "ui/ui_kbd.h"
#include "ui/ui_c64.h"

#include "c64-roms.h"

#ifdef __clang__
#pragma clang diagnostic ignored "-Wmissing-field-initializers"
#endif
#include <ImGuiSupport/imgui_impl_lucidextra.h>
#include <ImGuiSupport/imgui_impl_win32.h>

#define SOKOL_IMPL
#include "sokol_audio.h"


c64_desc_t c64_desc(c64_joystick_type_t joy_type);

// globals : TODO - encapsulate
static c64_t c64;
static ui_c64_t ui_c64;
static double exec_time;

static size_t g_FramePixelBufferSize = 0;
static unsigned char* g_FramePixelBuffer = nullptr;
ImTextureID g_FrameBufferTexture = nullptr;

void* gfx_create_texture(int w, int h)
{
    return ImGui_ImplDX11_CreateTextureRGBA(nullptr, w, h);
}

void gfx_update_texture(void* h, void* data, int data_byte_size)
{
    ImGui_ImplDX11_UpdateTextureRGBA(h, (unsigned char*)data);
}

void gfx_destroy_texture(void* h)
{

}


/* reboot callback */
static void C64BootCallback(c64_t* sys) 
{
    c64_desc_t desc = c64_desc(sys->joystick_type);
    c64_init(sys, &desc);
}

void c64ui_draw(void) 
{
    ui_c64_draw(&ui_c64, exec_time);
}

void c64ui_init(c64_t* c64) 
{
    //ui_init(c64ui_draw);
    ui_c64_desc_t desc;
    memset(&desc, 0, sizeof(ui_c64_desc_t));
    desc.c64 = c64;
    desc.boot_cb = C64BootCallback;
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
    ui_c64_init(&ui_c64, &desc);
}

void c64ui_discard(void) 
{
    ui_c64_discard(&ui_c64);
}

void c64ui_exec(c64_t* c64)
{
    const float frameTime = min(1000000.0f / ImGui::GetIO().Framerate, 32000.0f) * 1.0f;// speccyInstance.ExecSpeedScale;

    if (ui_c64_before_exec(&ui_c64))
    {
        c64_exec(c64, max(static_cast<uint32_t>(frameTime), uint32_t(1)));
        ui_c64_after_exec(&ui_c64);
    }
}



/* audio-streaming callback */
static void push_audio(const float* samples, int num_samples, void* user_data) 
{
    saudio_push(samples, num_samples);
}

/* get c64_desc_t struct based on joystick type */
c64_desc_t c64_desc(c64_joystick_type_t joy_type) 
{
    c64_desc_t desc;
    memset(&desc, 0, sizeof(c64_desc_t));
    desc.joystick_type = joy_type;
    desc.pixel_buffer = g_FramePixelBuffer;
    desc.pixel_buffer_size = g_FramePixelBufferSize;
    desc.audio_cb = push_audio;
    desc.audio_sample_rate = saudio_sample_rate();
    desc.audio_tape_sound = false;// sargs_boolean("tape_sound"),
    desc.rom_char = dump_c64_char_bin;
    desc.rom_char_size = sizeof(dump_c64_char_bin);
    desc.rom_basic = dump_c64_basic_bin;
    desc.rom_basic_size = sizeof(dump_c64_basic_bin);
    desc.rom_kernal = dump_c64_kernalv3_bin;
    desc.rom_kernal_size = sizeof(dump_c64_kernalv3_bin);
     
    return desc;
}

/* one-time application init */
void C64ChipsInit(void) 
{
#if 0
    gfx_init(&(gfx_desc_t) 
    {
#ifdef CHIPS_USE_UI
        .draw_extra_cb = ui_draw,
#endif
            .top_offset = ui_extra_height
    });
    keybuf_init(5);
    clock_init();
    saudio_setup(&(saudio_desc) { 0 });
    fs_init();
    bool delay_input = false;
    if (sargs_exists("file")) {
        delay_input = true;
        if (!fs_load_file(sargs_value("file"))) {
            gfx_flash_error();
        }
    }
    c64_joystick_type_t joy_type = C64_JOYSTICKTYPE_NONE;
    if (sargs_exists("joystick")) {
        if (sargs_equals("joystick", "digital_1")) {
            joy_type = C64_JOYSTICKTYPE_DIGITAL_1;
        }
        else if (sargs_equals("joystick", "digital_2")) {
            joy_type = C64_JOYSTICKTYPE_DIGITAL_2;
        }
        else if (sargs_equals("joystick", "digital_12")) {
            joy_type = C64_JOYSTICKTYPE_DIGITAL_12;
        }
    }
#endif
    saudio_desc audiodesc;
    memset(&audiodesc, 0, sizeof(saudio_desc));
    saudio_setup(&audiodesc);

    // setup pixel buffer
    g_FramePixelBufferSize = _C64_DISPLAY_SIZE;
    g_FramePixelBuffer = new unsigned char[g_FramePixelBufferSize * 2];

    // setup texture
    g_FrameBufferTexture = ImGui_ImplDX11_CreateTextureRGBA(static_cast<unsigned char*>(g_FramePixelBuffer), _C64_STD_DISPLAY_WIDTH, _C64_STD_DISPLAY_HEIGHT);

    c64_joystick_type_t joy_type = C64_JOYSTICKTYPE_NONE;
    c64_desc_t desc = c64_desc(joy_type);
    c64_init(&c64, &desc);
    c64ui_init(&c64);
    /*
    if (!delay_input) {
        if (sargs_exists("input")) {
            keybuf_put(sargs_value("input"));
        }
    }*/
}

static void ReadKeys(void)
{
    ImGuiIO& io = ImGui::GetIO();
    for (int i = 0; i < 10; i++)
    {
        if (io.KeysDown[0x30 + i] == 1)
        {
            c64_key_down(&c64, '0' + i);
            c64_key_up(&c64, '0' + i);
        }
    }

    for (int i = 0; i < 26; i++)
    {
        if (io.KeysDown[0x41 + i] == 1)
        {
            c64_key_down(&c64, 'a' + i);
            c64_key_up(&c64, 'a' + i);
        }
    }
}



/* per frame stuff, tick the emulator, handle input, decode and draw emulator display */
void C64ChipsTick(void) 
{
    ReadKeys();

    c64ui_exec(&c64);
    ui_c64_draw(&ui_c64, exec_time);

    c64_display_width(&c64);
    ImGui_ImplDX11_UpdateTextureRGBA(g_FrameBufferTexture, g_FramePixelBuffer, _C64_STD_DISPLAY_WIDTH, _C64_STD_DISPLAY_HEIGHT);

    if (ImGui::Begin("C64 Screen"))
    {
        ImGui::Text("Frame buffer size = %d x %d", _C64_STD_DISPLAY_WIDTH, _C64_STD_DISPLAY_HEIGHT);
        ImGui::Image(g_FrameBufferTexture, ImVec2(_C64_STD_DISPLAY_WIDTH, _C64_STD_DISPLAY_HEIGHT));
    }
    ImGui::End();
#if 0
    gfx_draw(c64_display_width(&c64), c64_display_height(&c64));
    const uint32_t load_delay_frames = 180;
    if (fs_ptr() && clock_frame_count() > load_delay_frames) {
        bool load_success = false;
        if (fs_ext("txt") || fs_ext("bas")) {
            load_success = true;
            keybuf_put((const char*)fs_ptr());
        }
        else if (fs_ext("tap")) {
            load_success = c64_insert_tape(&c64, fs_ptr(), fs_size());
        }
        else if (fs_ext("bin") || fs_ext("prg") || fs_ext("")) {
            load_success = c64_quickload(&c64, fs_ptr(), fs_size());
        }
        if (load_success) {
            if (clock_frame_count() > (load_delay_frames + 10)) {
                gfx_flash_success();
            }
            if (fs_ext("tap")) {
                c64_start_tape(&c64);
            }
            if (sargs_exists("input")) {
                keybuf_put(sargs_value("input"));
            }
            else if (fs_ext("tap")) {
                keybuf_put("LOAD\n");
            }
        }
        else {
            gfx_flash_error();
        }
        fs_free();
    }
    uint8_t key_code;
    if (0 != (key_code = keybuf_get())) 
    {
        /* FIXME: this is ugly */
        c64_joystick_type_t joy_type = c64.joystick_type;
        c64.joystick_type = C64_JOYSTICKTYPE_NONE;
        c64_key_down(&c64, key_code);
        c64_key_up(&c64, key_code);
        c64.joystick_type = joy_type;
    }
#endif
}

#if 0
/* keyboard input handling */
void app_input(const sapp_event* event) 
{
    if (ui_input(event)) {
        /* input was handled by UI */
        return;
    }

    const bool shift = event->modifiers & SAPP_MODIFIER_SHIFT;
    switch (event->type) {
        int c;
    case SAPP_EVENTTYPE_CHAR:
        c = (int)event->char_code;
        if ((c > 0x20) && (c < 0x7F)) {
            /* need to invert case (unshifted is upper caps, shifted is lower caps */
            if (isupper(c)) {
                c = tolower(c);
            }
            else if (islower(c)) {
                c = toupper(c);
            }
            c64_key_down(&c64, c);
            c64_key_up(&c64, c);
        }
        break;
    case SAPP_EVENTTYPE_KEY_DOWN:
    case SAPP_EVENTTYPE_KEY_UP:
        switch (event->key_code) {
        case SAPP_KEYCODE_SPACE:        c = 0x20; break;
        case SAPP_KEYCODE_LEFT:         c = 0x08; break;
        case SAPP_KEYCODE_RIGHT:        c = 0x09; break;
        case SAPP_KEYCODE_DOWN:         c = 0x0A; break;
        case SAPP_KEYCODE_UP:           c = 0x0B; break;
        case SAPP_KEYCODE_ENTER:        c = 0x0D; break;
        case SAPP_KEYCODE_BACKSPACE:    c = shift ? 0x0C : 0x01; break;
        case SAPP_KEYCODE_ESCAPE:       c = shift ? 0x13 : 0x03; break;
        case SAPP_KEYCODE_F1:           c = 0xF1; break;
        case SAPP_KEYCODE_F2:           c = 0xF2; break;
        case SAPP_KEYCODE_F3:           c = 0xF3; break;
        case SAPP_KEYCODE_F4:           c = 0xF4; break;
        case SAPP_KEYCODE_F5:           c = 0xF5; break;
        case SAPP_KEYCODE_F6:           c = 0xF6; break;
        case SAPP_KEYCODE_F7:           c = 0xF7; break;
        case SAPP_KEYCODE_F8:           c = 0xF8; break;
        default:                        c = 0; break;
        }
        if (c) {
            if (event->type == SAPP_EVENTTYPE_KEY_DOWN) {
                c64_key_down(&c64, c);
            }
            else {
                c64_key_up(&c64, c);
            }
        }
        break;
    case SAPP_EVENTTYPE_TOUCHES_BEGAN:
        sapp_show_keyboard(true);
        break;
    default:
        break;
    }
}
#endif

/* application cleanup callback */
void C64ChipsShutdown(void) 
{
    c64ui_discard();
    c64_discard(&c64);
}
