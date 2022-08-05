/*
    UI implementation for c64.c, this must live in a .cc file.
*/
//#include "common.h"
#define CHIPS_IMPL

#include "imgui.h"
#include "chips/z80.h"
#include "chips/m6502.h"
#include "chips/m6526.h"
#include "chips/m6569.h"
#include "chips/m6581.h"
#include "chips/beeper.h"
#include "chips/kbd.h"
#include "chips/mem.h"
#include "chips/clk.h"
#include "systems/c64.h"
#define UI_DASM_USE_Z80
#define UI_DASM_USE_M6502
#define UI_DBG_USE_M6502
//#include "ui.h"
#include "util/m6502dasm.h"
#include "util/z80dasm.h"
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

#include "CodeAnalyser/CodeAnalyser.h"
#include "CodeAnalyser/CodeAnalyserUI.h"

class FC64Emulator : public IInputEventHandler , public ICPUInterface
{
public:

    bool    Init();
    void    Shutdown();
    void    Tick();

    // Begin IInputEventHandler interface implementation
    void	OnKeyUp(int keyCode) override;
    void	OnKeyDown(int keyCode) override;
    void	OnChar(int charCode) override;
    // End IInputEventHandler interface implementation

    // Begin ICPUInterface interface implementation
    uint8_t		ReadByte(uint16_t address) override
    {
        return mem_rd(&C64Emu.mem_cpu, address);
    }
    uint16_t	ReadWord(uint16_t address) override
    {
        return mem_rd16(&C64Emu.mem_cpu, address);
    }

    uint16_t	GetPC(void) override
    {
        return m6502_pc(&C64Emu.cpu);
    }

    void	Break(void) override
    {
        C64UI.dbg.dbg.stopped = true;
        C64UI.dbg.dbg.step_mode = UI_DBG_STEPMODE_NONE;
    }

    void	Continue(void) override
    {
        C64UI.dbg.dbg.stopped = false;
        C64UI.dbg.dbg.step_mode = UI_DBG_STEPMODE_NONE;
    }

    void	GraphicsViewerSetAddress(uint16_t address) override
    {
    }

    bool	ExecThisFrame(void) override { return true; }

    void InsertROMLabels(struct FCodeAnalysisState& state) override {}
    void InsertSystemLabels(struct FCodeAnalysisState& state) override {}
    // End ICPUInterface interface implementation

    c64_desc_t GenerateC64Desc(c64_joystick_type_t joy_type);

    // Emulator Event Handlers
    void    OnBoot(void);
    int     OnCPUTrap(uint16_t pc, int ticks, uint64_t pins);
    uint64_t    OnCPUTick(uint64_t pins);


private:
    c64_t       C64Emu;
    ui_c64_t    C64UI;
    double      ExecTime;

    size_t          FramePixelBufferSize = 0;
    unsigned char*  FramePixelBuffer = nullptr;
    ImTextureID     FrameBufferTexture = nullptr;

    m6502_tick_t    OldTickCB = nullptr;
};

FC64Emulator g_C64Emu;

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
    FC64Emulator* pC64Emu = (FC64Emulator*)sys->user_data;
    pC64Emu->OnBoot();
    
}

static int CPUTrapCallback(uint16_t pc, int ticks, uint64_t pins, void* user_data)
{
    FC64Emulator* pC64Emu = (FC64Emulator*)user_data;
    return pC64Emu->OnCPUTrap(pc, ticks, pins);
}

uint64_t CPUTickCallback(uint64_t pins, void* user_data)
{
    c64_t* sys = (c64_t*)user_data;
    FC64Emulator* pC64Emu = (FC64Emulator*)sys->user_data;
    return pC64Emu->OnCPUTick(pins);
}

/* audio-streaming callback */
static void push_audio(const float* samples, int num_samples, void* user_data) 
{
    saudio_push(samples, num_samples);
}

/* get c64_desc_t struct based on joystick type */
c64_desc_t FC64Emulator::GenerateC64Desc(c64_joystick_type_t joy_type)
{
    c64_desc_t desc;
    memset(&desc, 0, sizeof(c64_desc_t));
    desc.joystick_type = joy_type;
    desc.pixel_buffer = FramePixelBuffer;
    desc.pixel_buffer_size = FramePixelBufferSize;
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

bool FC64Emulator::Init()
{
    SetInputEventHandler(this);
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
    FramePixelBufferSize = _C64_DISPLAY_SIZE;
    FramePixelBuffer = new unsigned char[FramePixelBufferSize * 2];

    // setup texture
    FrameBufferTexture = ImGui_ImplDX11_CreateTextureRGBA(static_cast<unsigned char*>(FramePixelBuffer), _C64_STD_DISPLAY_WIDTH, _C64_STD_DISPLAY_HEIGHT);

    // Setup C64 Emulator
    c64_joystick_type_t joy_type = C64_JOYSTICKTYPE_NONE;
    c64_desc_t desc = GenerateC64Desc(joy_type);
    c64_init(&C64Emu, &desc);
    C64Emu.user_data = this;

    // trap callback
    m6502_trap_cb(&C64Emu.cpu, CPUTrapCallback, this);

    // Tick callback - why isn't this working?
    OldTickCB = C64Emu.cpu.tick_cb;
    C64Emu.cpu.tick_cb = CPUTickCallback;

    // Setup C64 UI
    ui_c64_desc_t uiDesc;
    memset(&desc, 0, sizeof(ui_c64_desc_t));
    uiDesc.c64 = &C64Emu;
    uiDesc.boot_cb = C64BootCallback;
    uiDesc.create_texture_cb = gfx_create_texture;
    uiDesc.update_texture_cb = gfx_update_texture;
    uiDesc.destroy_texture_cb = gfx_destroy_texture;
    uiDesc.dbg_keys.break_keycode = ImGui::GetKeyIndex(ImGuiKey_Space);
    uiDesc.dbg_keys.break_name = "F5";
    uiDesc.dbg_keys.continue_keycode = VK_F5;
    uiDesc.dbg_keys.continue_name = "F5";
    uiDesc.dbg_keys.step_over_keycode = VK_F6;
    uiDesc.dbg_keys.step_over_name = "F6";
    uiDesc.dbg_keys.step_into_keycode = VK_F7;
    uiDesc.dbg_keys.step_into_name = "F7";
    uiDesc.dbg_keys.toggle_breakpoint_keycode = VK_F9;
    uiDesc.dbg_keys.toggle_breakpoint_name = "F9";
    ui_c64_init(&C64UI, &uiDesc);
    return true;
}

void FC64Emulator::Shutdown()
{
    ui_c64_discard(&C64UI);
    c64_discard(&C64Emu);
}

void FC64Emulator::Tick()
{
    const float frameTime = min(1000000.0f / ImGui::GetIO().Framerate, 32000.0f) * 1.0f;// speccyInstance.ExecSpeedScale;

    if (ui_c64_before_exec(&C64UI))
    {
        c64_exec(&C64Emu, max(static_cast<uint32_t>(frameTime), uint32_t(1)));
        ui_c64_after_exec(&C64UI);
    }

    ui_c64_draw(&C64UI, ExecTime);

    c64_display_width(&C64Emu);
    ImGui_ImplDX11_UpdateTextureRGBA(FrameBufferTexture, FramePixelBuffer, _C64_STD_DISPLAY_WIDTH, _C64_STD_DISPLAY_HEIGHT);

    if (ImGui::Begin("C64 Screen"))
    {
        ImGui::Text("Frame buffer size = %d x %d", _C64_STD_DISPLAY_WIDTH, _C64_STD_DISPLAY_HEIGHT);
        if (ImGui::Button("Load"))
        {
            // TODO: load game data
            FILE* fp = nullptr;
            fopen_s(&fp, "Games/Paradroid.prg", "rb");
            if (fp != nullptr)
            {
                uint8_t* gameData = nullptr;
                size_t gameDataSize = 0;
                fseek(fp, 0, SEEK_END);
                gameDataSize = ftell(fp);
                fseek(fp, 0, SEEK_SET);
                gameData = (uint8_t*)malloc(gameDataSize);
                fread(gameData, 1, gameDataSize, fp);
                c64_quickload(&C64Emu, gameData, gameDataSize);
                free(gameData);
                fclose(fp);
            }
        }
        ImGui::Image(FrameBufferTexture, ImVec2(_C64_STD_DISPLAY_WIDTH, _C64_STD_DISPLAY_HEIGHT));
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


int GetC64KeyFromKeyCode(int keyCode)
{
    int c = 0;
    bool bShift = false;
    switch (keyCode) 
    {
        case VK_SPACE:        c = 0x20; break;
        case VK_LEFT:         c = 0x08; break;
        case VK_RIGHT:        c = 0x09; break;
        case VK_DOWN:         c = 0x0A; break;
        case VK_UP:           c = 0x0B; break;
        case VK_RETURN:        c = 0x0D; break;
        case VK_BACK:           c = bShift ? 0x0C : 0x01; break;
        case VK_ESCAPE:       c = bShift ? 0x13 : 0x03; break;
        case VK_F1:           c = 0xF1; break;
        case VK_F2:           c = 0xF2; break;
        case VK_F3:           c = 0xF3; break;
        case VK_F4:           c = 0xF4; break;
        case VK_F5:           c = 0xF5; break;
        case VK_F6:           c = 0xF6; break;
        case VK_F7:           c = 0xF7; break;
        case VK_F8:           c = 0xF8; break;
        default:                        c = 0; break;
    }
    return c;
}

void FC64Emulator::OnKeyUp(int keyCode)
{
    c64_key_up(&C64Emu, GetC64KeyFromKeyCode(keyCode));
}

void FC64Emulator::OnKeyDown(int keyCode)
{
    c64_key_down(&C64Emu, GetC64KeyFromKeyCode(keyCode));
}

void FC64Emulator::OnChar(int charCode)
{
    int c = charCode;

    if ((c > 0x20) && (c < 0x7F)) 
    {
        /* need to invert case (unshifted is upper caps, shifted is lower caps */
        if (isupper(c)) 
            c = tolower(c);
        
        else if (islower(c)) 
            c = toupper(c);
     
        c64_key_down(&C64Emu, c);
        c64_key_up(&C64Emu, c);
    }
}

// Emulator Event Handers
void    FC64Emulator::OnBoot(void)
{
    c64_desc_t desc = GenerateC64Desc(C64Emu.joystick_type);
    c64_init(&C64Emu, &desc);
}

int    FC64Emulator::OnCPUTrap(uint16_t pc, int ticks, uint64_t pins)
{
    // TODO: Implement - use Speccy one as guide
    return 0;
}

uint64_t FC64Emulator::OnCPUTick(uint64_t pins)
{
    // TODO: Implement - use Speccy one as guide

    return OldTickCB(pins, &C64Emu);
}


void C64ChipsInit()
{
    g_C64Emu.Init();
}

/* per frame stuff, tick the emulator, handle input, decode and draw emulator display */
void C64ChipsTick(void) 
{
    g_C64Emu.Tick();
}

/* application cleanup callback */
void C64ChipsShutdown(void) 
{
    g_C64Emu.Shutdown();
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


