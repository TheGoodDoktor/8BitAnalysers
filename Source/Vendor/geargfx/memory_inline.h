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

#ifndef MEMORY_INLINE_H
#define MEMORY_INLINE_H

#include <assert.h>
#include "memory.h"
#include "media.h"
#include "huc6260.h"
#include "huc6270.h"
#include "huc6280.h"
#include "input.h"
#include "audio.h"
#include "cdrom.h"
#include "mapper.h"
#include "sf2_mapper.h"
#include "arcade_card_mapper.h"

INLINE u8 Memory::Read(u16 address, bool block_transfer)
{
#if defined(GG_TESTING)
    return m_test_memory[address];
#endif

#if !defined(GG_DISABLE_DISASSEMBLER)
    m_huc6280->CheckMemoryBreakpoints(HuC6280::HuC6280_BREAKPOINT_TYPE_ROMRAM, address, true);
#endif

    u8 mpr_index = address >> 13;
    u8 bank = m_mpr[mpr_index];
    u16 offset = address & 0x1FFF;

    if (bank != 0xFF)
    {
        if (IsValidPointer(m_current_mapper) && (bank < 0x80))
            return m_current_mapper->Read(bank, offset);
        else 
            return m_memory_map[bank][offset];
    }
    else
    {
        // Hardware Page
        switch (offset & 0x1C00)
        {
            case 0x0000:
                // HuC6270
                m_huc6280->InjectCycles(block_transfer ? 2 : 1);
                return m_huc6202->ReadRegister(offset);
            case 0x0400:
                // HuC6260
                m_huc6280->InjectCycles(1);
                return m_huc6260->ReadRegister(offset);
            case 0x0800:
                // PSG
                return block_transfer ? 0x00 : m_io_buffer;
            case 0x0C00:
                // Timer Counter
                if (block_transfer)
                    return 0x00;
                else
                {
                    m_io_buffer = (m_huc6280->ReadTimerRegister() & 0x7F) | (m_io_buffer & 0x80);
                    return m_io_buffer;
                }
            case 0x1000:
                // I/O
                if (block_transfer)
                    return 0x00;
                else
                {
                    m_io_buffer = m_input->ReadK();
                    return m_io_buffer;
                }
            case 0x1400:
            {
                // Interrupt registers
                if (block_transfer)
                    return 0x00;
                else
                {
                    switch (address & 0x03)
                    {
                        case 0:
                        case 1:
                        {
                            Debug("Invalid interrupt register read at %04X", address);
                            break;
                        }
                        case 2:
                        case 3:
                        {
                            m_io_buffer = (m_huc6280->ReadInterruptRegister(offset) & 0x07) | (m_io_buffer & 0xF8);
                            break;
                        }
                    }
                    return m_io_buffer;
                }
            }
            case 0x1800:
                // CDROM
                if (m_media->IsCDROM())
                {
                    if (IsValidPointer(m_current_mapper) && (offset >= 0x1A00))
                        return m_current_mapper->ReadHardware(offset);
                    else
                        return m_cdrom->ReadRegister(offset);
                }
                else
                    return 0xFF;
            case 0x1C00:
                // Unused
                Debug("Unused hardware read at %04X", address);
                return 0xFF;
            default:
                Debug("Invalid hardware read at %04X", address);
                return 0xFF;
        }
    }

    Debug("Invalid memory read at %04X, bank: %02X", address, bank);
    return 0xFF;
}

INLINE void Memory::Write(u16 address, u8 value, bool block_transfer)
{
#if defined(GG_TESTING)
    m_test_memory[address] = value;
    return;
#endif

#if !defined(GG_DISABLE_DISASSEMBLER)
    m_huc6280->CheckMemoryBreakpoints(HuC6280::HuC6280_BREAKPOINT_TYPE_ROMRAM, address, false);
#endif

    u8 mpr_index = address >> 13;
    u8 bank = m_mpr[mpr_index];
    u16 offset = address & 0x1FFF;

    if (IsValidPointer(m_current_mapper) && bank < 0x80)
    {
        m_current_mapper->Write(bank, offset, value);
    }
    else if (bank == 0xF7)
    {
        if (m_memory_map_write[bank] && (offset < 0x800))
            m_memory_map[bank][offset] = value;
    }
    else if (bank != 0xFF)
    {
        if (m_memory_map_write[bank])
            m_memory_map[bank][offset] = value;
    }
    else
    {
        // Hardware Page
        switch (offset & 0x1C00)
        {
            case 0x0000:
                // HuC6270
                m_huc6280->InjectCycles(block_transfer ? 2 : 1);
                m_huc6202->WriteRegister(offset, value);
                break;
            case 0x0400:
                // HuC6260
                m_huc6280->InjectCycles(1);
                m_huc6260->WriteRegister(offset, value);
                break;
            case 0x0800:
                // PSG
                m_audio->WritePSG(offset, value);
                m_io_buffer = value;
                break;
            case 0x0C00:
                // Timer
                m_huc6280->WriteTimerRegister(offset, value);
                m_io_buffer = value;
                break;
            case 0x1000:
                // I/O
                m_input->WriteO(value);
                m_io_buffer = value;
                break;
            case 0x1400:
            {
                switch (address & 0x03)
                {
                    case 0:
                    case 1:
                    {
                        Debug("Invalid interrupt register write at %04X, value=%02X", address, value);
                        break;
                    }
                    case 2:
                    case 3:
                    {
                        m_huc6280->WriteInterruptRegister(offset, value);
                        break;
                    }
                }
                m_io_buffer = value;
                break;
            }
            case 0x1800:
                // CDROM
                if (m_media->IsCDROM())
                {
                    if (IsValidPointer(m_current_mapper) && (offset >= 0x1A00))
                        m_current_mapper->WriteHardware(offset, value);
                    else
                        m_cdrom->WriteRegister(offset, value);
                }
                break;
            case 0x1C00:
                // Unused
                Debug("Unused hardware write at %04X, value=%02X", address, value);
                break;
            default:
                Debug("Invalid hardware write at %04X, value=%02X", address, value);
                break;
        }
    }
}

INLINE void Memory::SetMpr(u8 index, u8 value)
{
    assert(index < 8);
    m_mpr[index] = value;
}

INLINE u8 Memory::GetMpr(u8 index)
{
    assert(index < 8);
    return m_mpr[index];
}

INLINE u32 Memory::GetPhysicalAddress(u16 address)
{
    return (GetBank(address) << 13) | (address & 0x1FFF);
}

INLINE u8 Memory::GetBank(u16 address)
{
    return m_mpr[(address >> 13) & 0x07];
}

INLINE GG_Disassembler_Record* Memory::GetDisassemblerRecord(u16 address)
{
    return m_disassembler[GetPhysicalAddress(address)];
}

INLINE u8* Memory::GetWorkingRAM()
{
    return m_wram;
}

INLINE u8* Memory::GetCardRAM()
{
    return m_card_ram;
}

INLINE u8* Memory::GetBackupRAM()
{
    return m_backup_ram;
}

INLINE u8* Memory::GetCDROMRAM()
{
    return m_cdrom_ram;
}

INLINE u8* Memory::GetArcadeRAM()
{
    return m_arcade_card_mapper->GetRAM();
}

INLINE int Memory::GetWorkingRAMSize()
{
    return m_media->IsSGX() ? 0x8000 : 0x2000;
}

INLINE int Memory::GetCardRAMSize()
{
    return m_card_ram_size;
}

INLINE int Memory::GetCardRAMStart()
{
    return m_card_ram_start;
}

INLINE int Memory::GetCardRAMEnd()
{
    return m_card_ram_end;
}

INLINE int Memory::GetBackupRAMSize()
{
    return 0x800;
}

INLINE int Memory::GetCDROMRAMSize()
{
    return m_cdrom_ram_size;
}

INLINE int Memory::GetArcadeCardRAMSize()
{
    if (m_media->IsArcadeCard())
        return 0x200000;
    else
        return 0;
}

INLINE ArcadeCardMapper* Memory::GetArcadeCardMapper()
{
    return m_arcade_card_mapper;
}

INLINE u8** Memory::GetMemoryMap()
{
    return m_memory_map;
}

INLINE bool* Memory::GetMemoryMapWrite()
{
    return m_memory_map_write;
}

INLINE GG_Disassembler_Record** Memory::GetAllDisassemblerRecords()
{
    return m_disassembler;
}

INLINE void Memory::EnableBackupRam(bool enable)
{
    m_backup_ram_enabled = enable;
}

INLINE bool Memory::IsBackupRamEnabled()
{
    return m_backup_ram_enabled;
}

INLINE bool Memory::IsBackupRamUsed()
{
    if(!m_backup_ram_enabled)
        return false;

    for(int i = 8; i < 0x800; i++)
        if(m_backup_ram[i])
        return true;

    return false;
}

INLINE void Memory::UpdateBackupRam(bool enable)
{
    if (m_backup_ram_enabled && enable)
    {
        //Debug("Backup RAM enabled");
        m_memory_map_write[0xF7] = true;
        m_memory_map[0xF7] = m_backup_ram;
    }
    else
    {
        //Debug("Backup RAM disabled");
        m_memory_map_write[0xF7] = false;
        m_memory_map[0xF7] = m_unused_memory;
    }
}

#endif /* MEMORY_INLINE_H */