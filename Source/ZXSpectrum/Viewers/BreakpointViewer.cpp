#include "BreakpointViewer.h"
#include "../SpectrumEmu.h"
#include <imgui.h>
#include <ui/ui_dbg.h>
#include <CodeAnalyser/UI/CodeAnalyserUI.h>

/* add an execution breakpoint */
static bool _ui_dbg_bp_add_exec(ui_dbg_t* win, bool enabled, uint16_t addr) {
    if (win->dbg.num_breakpoints < UI_DBG_MAX_BREAKPOINTS) {
        ui_dbg_breakpoint_t* bp = &win->dbg.breakpoints[win->dbg.num_breakpoints++];
        bp->type = UI_DBG_BREAKTYPE_EXEC;
        bp->cond = UI_DBG_BREAKCOND_EQUAL;
        bp->addr = addr;
        bp->val = 0;
        bp->enabled = enabled;
        return true;
    }
    else {
        /* no more breakpoint slots */
        return false;
    }
}

/* delete breakpoint by index */
static void _ui_dbg_bp_del(ui_dbg_t* win, int index) {
    if ((win->dbg.num_breakpoints > 0) && (index >= 0) && (index < win->dbg.num_breakpoints)) {
        for (int i = index; i < (win->dbg.num_breakpoints - 1); i++) {
            win->dbg.breakpoints[i] = win->dbg.breakpoints[i + 1];
        }
        win->dbg.num_breakpoints--;
    }
}

static inline uint8_t _ui_dbg_read_byte(ui_dbg_t* win, uint16_t addr) {
    return win->read_cb(win->read_layer, addr, win->user_data);
}

static inline uint16_t _ui_dbg_read_word(ui_dbg_t* win, uint16_t addr) {
    uint8_t l = win->read_cb(win->read_layer, addr, win->user_data);
    uint8_t h = win->read_cb(win->read_layer, addr + 1, win->user_data);
    return (uint16_t)(h << 8) | l;
}

static inline uint16_t _ui_dbg_get_pc(ui_dbg_t* win) {
#if defined(UI_DBG_USE_Z80)
    return win->dbg.z80->pc;
#elif defined(UI_DBG_USE_M6502)
    return m6502_pc(win->dbg.m6502);
#endif
}


/* return true if breakpoint is enabled, false is disabled, or index out of bounds */
static bool _ui_dbg_bp_enabled(ui_dbg_t* win, int index) {
    if ((index >= 0) && (index < win->dbg.num_breakpoints)) {
        const ui_dbg_breakpoint_t* bp = &win->dbg.breakpoints[index];
        return bp->enabled;
    }
    return false;
}

/* disable all breakpoints */
static void _ui_dbg_bp_disable_all(ui_dbg_t* win) {
    for (int i = 0; i < win->dbg.num_breakpoints; i++) {
        win->dbg.breakpoints[i].enabled = false;
    }
}

/* enable all breakpoints */
static void _ui_dbg_bp_enable_all(ui_dbg_t* win) {
    for (int i = 0; i < win->dbg.num_breakpoints; i++) {
        win->dbg.breakpoints[i].enabled = true;
    }
}

/* delete all breakpoints */
static void _ui_dbg_bp_delete_all(ui_dbg_t* win) {
    win->dbg.num_breakpoints = 0;
}

/* draw the "Delete all breakpoints" popup modal */
static void _ui_dbg_bp_draw_delete_all_modal(ui_dbg_t* win, const char* title) {
    if (ImGui::BeginPopupModal(title, 0, ImGuiWindowFlags_AlwaysAutoResize)) {
        ImGui::Text("Delete all breakpoints?");
        ImGui::Separator();
        if (ImGui::Button("Ok", ImVec2(120, 0))) {
            _ui_dbg_bp_delete_all(win);
            ImGui::CloseCurrentPopup();
        }
        ImGui::SameLine();
        if (ImGui::Button("Cancel", ImVec2(120, 0))) {
            ImGui::CloseCurrentPopup();
        }
        ImGui::EndPopup();
    }
}


void FBreakpointViewer::DrawUI(void)
{
    ui_dbg_t* win = &pSpectrumEmu->UIZX.dbg;
    FCodeAnalysisViewState& viewState = pSpectrumEmu->CodeAnalysis.GetFocussedViewState();

    bool scroll_down = false;
    if (ImGui::Button("Add..")) 
    {
        _ui_dbg_bp_add_exec(win, false, _ui_dbg_get_pc(win));
        scroll_down = true;
    }
    ImGui::SameLine();
    if (ImGui::Button("Disable All")) 
    {
        _ui_dbg_bp_disable_all(win);
    }
    ImGui::SameLine();
    if (ImGui::Button("Enable All")) 
    {
        _ui_dbg_bp_enable_all(win);
    }
    ImGui::SameLine();
    if (ImGui::Button("Delete All")) 
    {
        ImGui::OpenPopup("Delete All?");
    }
    _ui_dbg_bp_draw_delete_all_modal(win, "Delete All?");
    int del_bp_index = -1;
    ImGui::Separator();
    ImGui::BeginChild("##bp_list", ImVec2(0, 0), false);
    for (int i = 0; i < win->dbg.num_breakpoints; i++) 
    {
        ImGui::PushID(i);
        ui_dbg_breakpoint_t* bp = &win->dbg.breakpoints[i];
        assert((bp->type >= 0) && (bp->type < UI_DBG_MAX_BREAKTYPES));
        /* visualize the current breakpoint */
        const bool bp_active = (win->dbg.last_trap_id >= UI_DBG_BP_BASE_TRAPID) &&
            ((win->dbg.last_trap_id - UI_DBG_BP_BASE_TRAPID) == i);
        if (bp_active) 
            ImGui::PushStyleColor(ImGuiCol_CheckMark, 0xFF0000FF);
        
        ImGui::Checkbox("##enabled", &bp->enabled); ImGui::SameLine();

        if (bp_active) 
            ImGui::PopStyleColor();
        
        if (ImGui::IsItemHovered()) 
        {
            if (bp->enabled) 
                ImGui::SetTooltip("Disable Breakpoint");
            else 
                ImGui::SetTooltip("Enable Breakpoint");
        }

        ImGui::SameLine();
        bool upd_val = false;
        ImGui::PushItemWidth(112);
        if (ImGui::Combo("##type", &bp->type, win->ui.breaktype_combo_labels, win->ui.num_breaktypes)) {
            upd_val = true;
        }
        ui_dbg_breaktype_t* bt = &win->ui.breaktypes[bp->type];
        assert(bt->label);
        ImGui::PopItemWidth();
        if (bt->show_addr) 
        {
            ImGui::SameLine();
            uint16_t old_addr = bp->addr;
            bp->addr = ui_util_input_u16("##addr", old_addr);
            DrawAddressLabel(pSpectrumEmu->CodeAnalysis, viewState, bp->addr);
            if (upd_val || (old_addr != bp->addr)) 
            {
                /* if breakpoint type or address has changed, update the breakpoint's value from memory */
                switch (bp->type) {
                case UI_DBG_BREAKTYPE_BYTE:
                    bp->val = (int)_ui_dbg_read_byte(win, bp->addr);
                    break;
                case UI_DBG_BREAKTYPE_WORD:
                    bp->val = (int)_ui_dbg_read_word(win, bp->addr);
                    break;
#if defined(UI_DBG_USE_Z80)
                case UI_DBG_BREAKTYPE_OUT:
                case UI_DBG_BREAKTYPE_IN:
                    bp->val = 0x00FF;
                    break;
#endif
                default:
                    bp->val = 0;
                    break;
                }
            }
        }
        if (bt->show_cmp) 
        {
            ImGui::SameLine();
            ImGui::PushItemWidth(42);
            ImGui::Combo("##cond", &bp->cond, "==\0!=\0>\0<\0>=\0<=\0");
            ImGui::PopItemWidth();
        }
        if (bt->show_val8 || bt->show_val16) 
        {
            if (bt->val_label) 
            {
                ImGui::SameLine();
                ImGui::Text("%s", bt->val_label);
            }
            if (bt->show_val8) 
            {
                ImGui::SameLine();
                bp->val = (int)ui_util_input_u8("##byte", (uint8_t)bp->val);
            }
            else
            {
                ImGui::SameLine();
                bp->val = (int)ui_util_input_u16("##word", (uint16_t)bp->val);
            }
        }
        ImGui::SameLine();
        if (ImGui::Button("Del")) 
        {
            del_bp_index = i;
        }
        if (ImGui::IsItemHovered()) 
        {
            ImGui::SetTooltip("Delete");
        }
        ImGui::PopID();
    }
    if (del_bp_index != -1) 
    {
        ImGui::OpenPopup("Delete?");
        win->dbg.delete_breakpoint_index = del_bp_index;
    }
    if ((win->dbg.delete_breakpoint_index >= 0) && ImGui::BeginPopupModal("Delete?", 0, ImGuiWindowFlags_AlwaysAutoResize)) 
    {
        ImGui::Text("Delete breakpoint at %04X?", win->dbg.breakpoints[win->dbg.delete_breakpoint_index].addr);
        ImGui::Separator();
        if (ImGui::Button("Ok", ImVec2(120, 0))) 
        {
            _ui_dbg_bp_del(win, win->dbg.delete_breakpoint_index);
            ImGui::CloseCurrentPopup();
            win->dbg.delete_breakpoint_index = -1;
        }
        ImGui::SameLine();
        if (ImGui::Button("Cancel", ImVec2(120, 0))) 
        {
            ImGui::CloseCurrentPopup();
            win->dbg.delete_breakpoint_index = -1;
        }
        ImGui::EndPopup();
    }
    if (scroll_down) 
    {
        ImGui::SetScrollHereY(1.0f);
    }

    ImGui::EndChild();
}
