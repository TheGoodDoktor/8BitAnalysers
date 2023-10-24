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
uint32_t GetCpcColour(unsigned int index);

#ifdef __cplusplus
} // extern "C"
#endif
