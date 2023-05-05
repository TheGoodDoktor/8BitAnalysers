#pragma once

#include "chips/chips_common.h"
#include "chips/z80.h"
#include "chips/beeper.h"
#include "chips/ay38910.h"
#include "chips/kbd.h"
#include "chips/clk.h"
#include "chips/mem.h"
#include "systems/zx.h"

// This file will contain C function for compatibility reasons

#ifdef __cplusplus
extern "C" {
#endif
	
typedef bool(*GetIOInput)(uint16_t port, uint8_t* pInVal, void* pUserData);

void ZXDecodeScreen(zx_t* pZX);
uint32_t ZXExeEmu(zx_t* sys, uint32_t micro_seconds);
uint32_t ZXExeEmu_UseFetchCount(zx_t* sys, uint32_t noFetches, GetIOInput ioInputCB, void* pUserData);

#ifdef __cplusplus
} // extern "C"
#endif
