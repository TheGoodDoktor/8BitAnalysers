#include "PCERegistersViewer.h"

#include <imgui.h>

#include "../PCEEmu.h"
#include <geargrafx_core.h>

void GetBankName(GeargrafxCore* pCore, uint8_t mpr, uint8_t mpr_value, char* name);

FPCERegistersViewer::FPCERegistersViewer(FEmuBase* pEmu)
: FViewerBase(pEmu) 
{ 
	Name = "PCE Registers";
	pPCEEmu = static_cast<FPCEEmu*>(pEmu);
}

bool FPCERegistersViewer::Init()
{
	return true;
}

void FPCERegistersViewer::DrawUI()
{
	Memory* pMemory = pPCEEmu->GetCore()->GetMemory();

	char bankName[16] = { };

	ImGui::SeparatorText("MPR");
	for (int i = 0; i < 8; i++)
	{
		const uint8_t mprVal = pMemory->GetMpr(i);
		GetBankName(pPCEEmu->GetCore(), i, mprVal, bankName);
		ImGui::Text("MPR%d: %s '%s'", i, NumStr(mprVal), bankName);
	}
}

void GetBankName(GeargrafxCore* pCore, uint8_t mpr, uint8_t mpr_value, char* name)
{
 //u16 cpu_address = mpr << 13;

	Memory* memory = pCore->GetMemory();
	Media* media = pCore->GetMedia();
	const Memory::MemoryBankType bank_type = memory->GetBankType(mpr_value);

	switch (bank_type)
	{
		// ROM
		case Memory::MEMORY_BANK_TYPE_ROM:
		{
			const u32 rom_address = mpr_value << 13;
			//snprintf(name, 16, "ROM %s", NumStr(mpr_value));
			snprintf(name, 16, "ROM %02d", mpr_value);
			//snprintf(tooltip, 128, "Range (CPU) $%04X-$%04X \nRange (ROM) $%06X-$%06X",
			//	cpu_address, cpu_address + 0x1FFF, rom_address, rom_address + 0x1FFF);
		}
		break;
		// BIOS
		case Memory::MEMORY_BANK_TYPE_BIOS:
		{
			const u32 rom_address = mpr_value << 13;
			snprintf(name, 16, "BIOS $%02X", mpr_value);
			//snprintf(tooltip, 128, "Range (CPU) $%04X-$%04X \nRange (BIOS) $%06X-$%06X",
			//	cpu_address, cpu_address + 0x1FFF, rom_address, rom_address + 0x1FFF);
		}
		break;
		// Card RAM
		case Memory::MEMORY_BANK_TYPE_CARD_RAM:
		{
			const int card_ram_start = memory->GetCardRAMStart();
			const int card_ram_size = memory->GetCardRAMSize();
			const u32 card_ram_address = ((mpr_value - card_ram_start) * 0x2000) % card_ram_size;
			snprintf(name, 16, "CARD RAM");
			//snprintf(tooltip, 128, "Range (CPU) $%04X-$%04X \nRange (CARD RAM) $%06X-$%06X",
			//	cpu_address, cpu_address + 0x1FFF, card_ram_address, card_ram_address + 0x1FFF);
		}
		break;
		// Backup RAM
		case Memory::MEMORY_BANK_TYPE_BACKUP_RAM:
			snprintf(name, 16, "BRAM");
			//snprintf(tooltip, 128, "Range (CPU) $%04X-$%04X \nBackup RAM",
			//	cpu_address, cpu_address + 0x1FFF);
		break;
		// WRAM
		case Memory::MEMORY_BANK_TYPE_WRAM:
		{
			const u8 ram_bank = mpr_value - 0xF8;
			const u16 ram_address = ram_bank << 13;

			if (media->IsSGX())
			{
				snprintf(name, 16, "WRAM %02d", ram_bank);
				//snprintf(name, 16, "WRAM $%02X", ram_bank);
				//snprintf(tooltip, 128, "Range (CPU) $%04X-$%04X \nRange (WRAM) $%04X-$%04X",
				//	cpu_address, cpu_address + 0x1FFF, ram_address, ram_address + 0x1FFF);
			}
			else
			{
				snprintf(name, 16, "WRAM 00");
				//snprintf(tooltip, 128, "Range (CPU) $%04X-$%04X \nRange (WRAM) $0000-$1FFF",
				//	cpu_address, cpu_address + 0x1FFF);
			}
		}
		break;
		// CDROM RAM
		case Memory::MEMORY_BANK_TYPE_CDROM_RAM:
		{
			const u32 cdrom_ram_address = (mpr_value - 0x80) * 0x2000;
			snprintf(name, 16, "CD RAM");
			//snprintf(tooltip, 128, "Range (CPU) $%04X-$%04X \nRange (CDROM RAM) $%06X-$%06X",
			//	cpu_address, cpu_address + 0x1FFF, cdrom_ram_address, cdrom_ram_address + 0x1FFF);
		}
		break;
		// Hardware registers at 0xFF or unused
		case Memory::MEMORY_BANK_TYPE_UNUSED:
		default:
			if (mpr_value == 0xFF)
			{
				snprintf(name, 16, "HARDWARE");
				//snprintf(tooltip, 128, "Range (CPU) $%04X-$%04X", cpu_address, cpu_address + 0x1FFF);
			}
			else
			{
				snprintf(name, 16, "UNUSED");
				//snprintf(tooltip, 128, "Range (CPU) $%04X-$%04X", cpu_address, cpu_address + 0x1FFF);
			}
		break;
	}
}