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

#ifndef MEMORY_H
#define MEMORY_H

#include <iostream>
#include <fstream>
#include "common.h"

class Media;
class HuC6260;
class HuC6202;
class HuC6280;
class Input;
class Audio;
class CdRom;
class Mapper;
class SF2Mapper;
class ArcadeCardMapper;

class Memory
{
public:
    enum MemoryBankType
    {
        MEMORY_BANK_TYPE_ROM,
        MEMORY_BANK_TYPE_BIOS,
        MEMORY_BANK_TYPE_CARD_RAM,
        MEMORY_BANK_TYPE_BACKUP_RAM,
        MEMORY_BANK_TYPE_WRAM,
        MEMORY_BANK_TYPE_CDROM_RAM,
        MEMORY_BANK_TYPE_UNUSED
    };

public:
    Memory(HuC6260* huc6260, HuC6202* huc6202, HuC6280* huc6280, Media* media, Input* input, Audio* audio, CdRom* cdrom);
    ~Memory();
    void Init();
    void Reset();
    u8 Read(u16 address, bool block_transfer = false);
    void Write(u16 address, u8 value, bool block_transfer = false);
    void SetMpr(u8 index, u8 value);
    u8 GetMpr(u8 index);
    void SetMprTAM(u8 bits, u8 value);
    u8 GetMprTMA(u8 bits);
    u32 GetPhysicalAddress(u16 address);
    u8 GetBank(u16 address);
    void SetResetValues(int mpr, int wram, int card_ram, int arcade_card);
    GG_Disassembler_Record* GetDisassemblerRecord(u16 address);
    GG_Disassembler_Record* GetOrCreateDisassemblerRecord(u16 address);
    void ResetDisassemblerRecords();
    u8* GetWorkingRAM();
    u8* GetCardRAM();
    u8* GetBackupRAM();
    u8* GetCDROMRAM();
    u8* GetArcadeRAM();
    int GetWorkingRAMSize();
    int GetCardRAMSize();
    int GetCardRAMStart();
    int GetCardRAMEnd();
    int GetBackupRAMSize();
    int GetCDROMRAMSize();
    int GetArcadeCardRAMSize();
    ArcadeCardMapper* GetArcadeCardMapper();
    u8** GetMemoryMap();
    bool* GetMemoryMapWrite();
    GG_Disassembler_Record** GetAllDisassemblerRecords();
    void EnableBackupRam(bool enable);
    bool IsBackupRamEnabled();
    bool IsBackupRamUsed();
    void UpdateBackupRam(bool enable);
    MemoryBankType GetBankType(u8 bank);
    void SaveRam(std::ostream &file);
    bool LoadRam(std::istream &file, s32 file_size);
    void SaveState(std::ostream& stream);
    void LoadState(std::istream& stream);

private:
    void ReloadMemoryMap();

private:
    HuC6260* m_huc6260;
    HuC6202* m_huc6202;
    HuC6280* m_huc6280;
    Media* m_media;
    SF2Mapper* m_sf2_mapper;
    ArcadeCardMapper* m_arcade_card_mapper;
    Input* m_input;
    Audio* m_audio;
    CdRom* m_cdrom;
    u8 m_mpr[8];
    u8* m_memory_map[0x100] = {};
    bool m_memory_map_write[0x100] = {};
    u8 m_unused_memory[0x2000];
    u8 m_wram[0x8000] = {};
    u8 m_card_ram[0x30000] = {};
    u8 m_cdrom_ram[0x10000] = {};
    u8 m_backup_ram[0x2000] = {};
    u32 m_cdrom_ram_size;
    u32 m_card_ram_size;
    u8 m_card_ram_start;
    u8 m_card_ram_end;
    bool m_backup_ram_enabled;
    GG_Disassembler_Record** m_disassembler;
    u8 m_io_buffer;
    u8 m_mpr_buffer;
    u8* m_test_memory;
    Mapper* m_current_mapper;
    int m_mpr_reset_value;
    int m_wram_reset_value;
    int m_card_ram_reset_value;
    int m_arcade_card_reset_value;
};

static const u8 k_backup_ram_init_string[8] = { 'H', 'U', 'B', 'M', 0x00, 0xA0, 0x10, 0x80 };

#include "memory_inline.h"

#endif /* MEMORY_H */