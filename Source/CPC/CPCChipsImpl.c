
#define CHIPS_IMPL
#include "CPCChipsImpl.h"

#define CHIPS_UTIL_IMPL
#include "util/z80dasm.h"
#include "ExternalROMSupport.h"

uint32_t GetCPCColour(unsigned int index)
{
    return _am40010_cpc_colors[index];
}

// This will be called when either of these occur:
// - lower/upper ROM enable/disable
// - RAM bank configuration switch (6128 only)
// - Upper ROM bank select
void CPCBankSwitchCB(uint8_t ram_config, uint8_t rom_enable, uint8_t rom_select, void* user_data)
{
	cpc_t* sys = (cpc_t*)user_data;
	int ram_config_index;
	const uint8_t* rom0_ptr;
	const uint8_t* rom1_ptr;
	if (CPC_TYPE_6128 == sys->type) {
		ram_config_index = ram_config & 7;
		rom0_ptr = sys->rom_os;
		rom1_ptr = (rom_select == 7) ? sys->rom_amsdos : sys->rom_basic;
	}
	else {
		ram_config_index = 0;
		rom0_ptr = sys->rom_os;
		rom1_ptr = sys->rom_basic;
	}
	const int i0 = _cpc_ram_config[ram_config_index][0];
	const int i1 = _cpc_ram_config[ram_config_index][1];
	const int i2 = _cpc_ram_config[ram_config_index][2];
	const int i3 = _cpc_ram_config[ram_config_index][3];

	// 0x0000 .. 0x3FFF
	if (rom_enable & AM40010_CONFIG_LROMEN) {
		// read/write RAM
		mem_map_ram(&sys->mem, 0, 0x0000, 0x4000, sys->ram[i0]);
	}
	else {
		// RAM-behind-ROM
		mem_map_rw(&sys->mem, 0, 0x0000, 0x4000, rom0_ptr, sys->ram[i0]);
	}
	// 0x4000 .. 0x7FFF
	mem_map_ram(&sys->mem, 0, 0x4000, 0x4000, sys->ram[i1]);
	// 0x8000 .. 0xBFFF
	mem_map_ram(&sys->mem, 0, 0x8000, 0x4000, sys->ram[i2]);
	// 0xC000 .. 0xFFFF
	if (rom_enable & AM40010_CONFIG_HROMEN) {
		// read/write RAM
		mem_map_ram(&sys->mem, 0, 0xC000, 0x4000, sys->ram[i3]);
	}
	else {
		// RAM-behind-ROM
		// This will switch to the current upper ROM bank slot, including external rom banks.
		assert(GetUpperROMData());
		mem_map_rw(&sys->mem, 0, 0xC000, 0x4000, GetUpperROMData(), sys->ram[i3]);
	}
}
