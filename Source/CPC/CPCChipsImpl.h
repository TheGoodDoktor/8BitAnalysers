#pragma once

#include "chips/chips_common.h"
#include "chips/z80.h"
#include "chips/beeper.h"
#include "chips/ay38910.h"
#include "chips/i8255.h"
#include "chips/mc6845.h"
#include "chips/am40010.h"
#include "chips/upd765.h"
#include "chips/kbd.h"
#include "chips/clk.h"
#include "chips/mem.h"
#include "chips/fdd.h"
#include "chips/fdd_cpc.h"
#include "systems/cpc.h"

// This file will contain C function for compatibility reasons

#ifdef __cplusplus
extern "C" {
#endif	

// put any function definitions that need to be called from c++ here
uint32_t GetCPCColour(unsigned int index);

// This is a modified version of the Chips _cpc_bankswitch() code. It has been extended to allow mapping of external upper ROM banks.
void CPCBankSwitchCB(uint8_t ram_config, uint8_t rom_enable, uint8_t rom_select, void* user_data);

#ifdef __cplusplus
} // extern "C"
#endif
