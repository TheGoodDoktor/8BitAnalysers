/*
 * Geargrafx - PC Engine / TurboGrafx Emulator
 * Copyright (C) 2024  Ignacio Sanchez

 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * any later version.

 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.

 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see http://www.gnu.org/licenses/
 *
 */

#ifndef HUC6270_H
#define HUC6270_H

#include <iostream>
#include <fstream>
#include "huc6270_defines.h"
#include "common.h"

class HuC6202;
class HuC6260;
class HuC6280;

class HuC6270
{
public:
    enum HuC6270_Vertical_State
    {
        HuC6270_VERTICAL_STATE_VDS,
        HuC6270_VERTICAL_STATE_VDW,
        HuC6270_VERTICAL_STATE_VCR,
        HuC6270_VERTICAL_STATE_VSW,
        HuC6270_VERTICAL_STATE_COUNT
    };

    enum HuC6270_Horizontal_State
    {
        HuC6270_HORIZONTAL_STATE_HDS,
        HuC6270_HORIZONTAL_STATE_HDW,
        HuC6270_HORIZONTAL_STATE_HDE,
        HuC6270_HORIZONTAL_STATE_HSW,
        HuC6270_HORIZONTAL_STATE_COUNT
    };

    enum HuC6270_Line_Event
    {
        HuC6270_EVENT_NONE,
        HuC6270_EVENT_BYR,
        HuC6270_EVENT_BXR,
        HuC6270_EVENT_HDS,
        HuC6270_EVENT_RCR
    };

    struct HuC6270_State
    {
        u16* AR;
        u16* SR;
        u16* R;
        u16* READ_BUFFER;
        s32* HPOS;
        s32* VPOS;
        s32* V_STATE;
        s32* H_STATE;
    };

public:
    HuC6270(HuC6280* huC6280);
    ~HuC6270();
    void Init(HuC6260* huC6260, HuC6202* huC6202);
    void Reset();
    u16 Clock();
    void SetHSyncHigh();
    void SetVSyncLow();
    u8 ReadRegister(u16 address);
    void WriteRegister(u16 address, u8 value);
    HuC6270_State* GetState();
    u16* GetVRAM();
    u16* GetSAT();
    void SetNoSpriteLimit(bool no_sprite_limit);
    void SaveState(std::ostream& stream);
    void LoadState(std::istream& stream);

private:
    struct HuC6270_Sprite_Data
    {
        s32 index;
        u16 x;
        u16 flags;
        u8 palette;
        u16 data[4];
    };

private:
    HuC6202* m_huc6202;
    HuC6260* m_huc6260;
    HuC6280* m_huc6280;
    HuC6270_State m_state;
    u16 m_vram[HUC6270_VRAM_SIZE] = {};
    u16 m_address_register;
    u16 m_status_register;
    u16 m_register[20];
    u16 m_sat[HUC6270_SAT_SIZE] = {};
    u16 m_read_buffer;
    u16 m_vram_openbus;
    bool m_trigger_sat_transfer;
    u16 m_sat_transfer_pending;
    u32 m_vram_transfer_pending;
    u16 m_vram_transfer_src;
    u16 m_vram_transfer_dest;
    s32 m_hpos;
    s32 m_vpos;
    s32 m_bg_offset_y;
    s32 m_bg_counter_y;
    bool m_increment_bg_counter_y;
    bool m_need_to_increment_raster_line;
    s32 m_raster_line;
    u16 m_latched_bxr;
    u16 m_latched_hds;
    u16 m_latched_hdw;
    u16 m_latched_hde;
    u16 m_latched_hsw;
    u16 m_latched_vds;
    u16 m_latched_vdw;
    u16 m_latched_vcr;
    u16 m_latched_vsw;
    u16 m_latched_mwr;
    u16 m_latched_cr;
    s32 m_v_state;
    s32 m_h_state;
    s32 m_next_event;
    s32 m_lines_to_next_v_state;
    s32 m_clocks_to_next_h_state;
    s32 m_clocks_to_next_event;
    bool m_vblank_triggered;
    bool m_active_line;
    bool m_burst_mode;
    u16 m_line_buffer[HUC6270_MAX_BACKGROUND_WIDTH] = {};
    u16 m_line_buffer_sprites[HUC6270_MAX_BACKGROUND_WIDTH] = {};
    s32 m_line_buffer_index;
    bool m_no_sprite_limit;
    s32 m_sprite_count;
    bool m_sprite_overflow;
    HuC6270_Sprite_Data m_sprites[HUC6270_SPRITES * 2] = {};

private:
    void EndOfLine();
    void LineEvents();
    void HSyncStart();
    void IncrementRasterLine();
    void SATTransfer();
    void VRAMTransfer();
    void NextVerticalState();
    void NextHorizontalState();
    u16 ReadVRAM(u16 address);
    void VBlankIRQ();
    void RCRIRQ();
    void OverflowIRQ();
    void SpriteCollisionIRQ();
    void RenderLine();
    void RenderBackground(int width);
    void RenderSprites(int width);
    void FetchSprites();
};

static const u16 k_register_mask[20] = {
    0xFFFF, 0xFFFF, 0xFFFF, 0x0000, 0x0000,
    0x1FFF, 0x03FF, 0x03FF, 0x01FF, 0x00FF,
    0x7F1F, 0x7F7F, 0xFF1F, 0x01FF, 0x00FF,
    0x001F, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF };

static const int k_huc6270_screen_size_x[8] = { 32, 64, 128, 128, 32, 64, 128, 128 };
static const int k_huc6270_screen_size_y[8] = { 32, 32, 32, 32, 64, 64, 64, 64 };
static const int k_huc6270_screen_size_x_pixels[8] = { 32 * 8, 64 * 8, 128 * 8, 128 * 8, 32 * 8, 64 * 8, 128 * 8, 128 * 8 };
static const int k_huc6270_screen_size_y_pixels[8] = { 32 * 8, 32 * 8, 32 * 8, 32 * 8, 64 * 8, 64 * 8, 64 * 8, 64 * 8 };

static const int k_huc6270_screen_size_x_pixels_mask[8] = {
    k_huc6270_screen_size_x_pixels[0] - 1, k_huc6270_screen_size_x_pixels[1] - 1,
    k_huc6270_screen_size_x_pixels[2] - 1, k_huc6270_screen_size_x_pixels[3] - 1,
    k_huc6270_screen_size_x_pixels[4] - 1, k_huc6270_screen_size_x_pixels[5] - 1,
    k_huc6270_screen_size_x_pixels[6] - 1, k_huc6270_screen_size_x_pixels[7] - 1 };
static const int k_huc6270_screen_size_y_pixels_mask[8] = {
    k_huc6270_screen_size_y_pixels[0] - 1, k_huc6270_screen_size_y_pixels[1] - 1,
    k_huc6270_screen_size_y_pixels[2] - 1, k_huc6270_screen_size_y_pixels[3] - 1,
    k_huc6270_screen_size_y_pixels[4] - 1, k_huc6270_screen_size_y_pixels[5] - 1,
    k_huc6270_screen_size_y_pixels[6] - 1, k_huc6270_screen_size_y_pixels[7] - 1 };

static const int k_huc6270_read_write_increment[4] = { 0x01, 0x20, 0x40, 0x80 };

static const int k_huc6270_sprite_width[2] = { 16, 32 };
static const int k_huc6270_sprite_height[4] = { 16, 32, 64, 64 };
static const int k_huc6270_sprite_mask_width[2] = { 0xFFFF, 0xFFFE };
static const int k_huc6270_sprite_mask_height[4] = { 0xFFFF, 0xFFFD, 0xFFF9, 0xFFF9 };

static const char* const k_register_names_aligned[32] = {
    "MAWR ", "MARR ", "VWR  ", "???  ", "???  ", "CR   ", "RCR  ", "BXR  ",
    "BYR  ", "MWR  ", "HSR  ", "HDR  ", "VSR  ", "VDR  ", "VCR  ", "DCR  ",
    "SOUR ", "DESR ", "LENR ", "DVSSR", "???  ", "???  ", "???  ", "???  ",
    "???  ", "???  ", "???  ", "???  ", "???  ", "???  ", "???  ", "???  " };

static const char* const k_register_names[32] = {
    "MAWR", "MARR", "VWR",  "???",   "???",  "CR",   "RCR", "BXR",
    "BYR",  "MWR",  "HSR",  "HDR",   "VSR",  "VDR",  "VCR", "DCR",
    "SOUR", "DESR", "LENR", "DVSSR", "???" , "???" , "???", "???",
    "???",  "???",  "???",  "???",   "???",  "???",  "???", "???" };

#include "huc6270_inline.h"

#endif /* HUC6270_H */