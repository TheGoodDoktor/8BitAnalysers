#pragma once

#include "chips/chips_common.h"
#include "chips/z80.h"
#include "chips/beeper.h"
#include "chips/ay38910.h"
#include "chips/kbd.h"
#include "chips/clk.h"
#include "chips/mem.h"
#include "systems/zx.h"

// This file will contain C function for compatability reasons

#ifdef __cplusplus
extern "C" {
#endif

void DecodeScreen(zx_t* pZX);

#ifdef __cplusplus
} // extern "C"
#endif
