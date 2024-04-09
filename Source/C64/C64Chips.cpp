/*
    UI implementation for c64.c, this must live in a .cc file.
*/
//#include "common.h"
//#define CHIPS_IMPL
#define CHIPS_UI_IMPL
#include <cinttypes>

#include <imgui.h>
#include "C64Emulator.h"

#ifdef __clang__
#pragma clang diagnostic ignored "-Wmissing-field-initializers"
#endif
#include <ImGuiSupport/ImGuiTexture.h>
#include <backends/imgui_impl_win32.h>


#include "CodeAnalyser/CodeAnalyser.h"
#include "CodeAnalyser/UI/CodeAnalyserUI.h"
#include "Util/MemoryBuffer.h"
#include "Util/FileUtil.h"
#include "IOAnalysis/C64IOAnalysis.h"
#include "GraphicsViewer/C64GraphicsViewer.h"
#include "C64Display.h"
#include "C64GamesList.h"
#include <Util/Misc.h>
#include <algorithm>



//FC64Emulator g_C64Emu;

#if 0
static int CPUTrapCallback(uint16_t pc, int ticks, uint64_t pins, void* user_data)
{
    FC64Emulator* pC64Emu = (FC64Emulator*)user_data;
    return pC64Emu->OnCPUTrap(pc, ticks, pins);
}

uint64_t CPUTickCallback(uint64_t pins, void* user_data)
{
    c64_t* sys = (c64_t*)user_data;

    // FIXME: no such struct field
    return 0;
    //FC64Emulator* pC64Emu = (FC64Emulator*)sys->user_data;
    //return pC64Emu->OnCPUTick(pins);
}
#endif




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


