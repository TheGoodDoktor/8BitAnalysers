#pragma once

#include <stdint.h>

// Constants for Amstrad CPC Hardware
// The Z80 sees a 64K address space divided into four 16K slots. The Gate Array
// controls which RAM bank (or ROM) is visible in each slot at any given moment.
//
// Slot 0 (0x0000-0x3FFF): RAM bank 0 by default; overlaid by OS ROM when lower ROM enabled.
//                         RAM config 2 maps bank 4 here instead of bank 0.
// Slot 1 (0x4000-0x7FFF): Always some RAM bank, never ROM. Bank varies by RAM config (see below).
// Slot 2 (0x8000-0xBFFF): Always some RAM bank, never ROM. Usually bank 2; bank 6 in configs 2/3.
// Slot 3 (0xC000-0xFFFF): RAM bank 3 by default; overlaid by BASIC/expansion ROM when upper ROM enabled.
//                         Many RAM configs map bank 7 here instead (configs 1/2/3).
//
// CPC 6128 RAM configurations (Gate Array RAM management register, bits 2-0):
//   Config 0: slots -> banks [0, 1, 2, 3]  (standard / default)
//   Config 1: slots -> banks [0, 1, 2, 7]
//   Config 2: slots -> banks [4, 5, 6, 7]
//   Config 3: slots -> banks [0, 3, 2, 7]
//   Config 4: slots -> banks [0, 4, 2, 3]
//   Config 5: slots -> banks [0, 5, 2, 3]
//   Config 6: slots -> banks [0, 6, 2, 3]
//   Config 7: slots -> banks [0, 7, 2, 3]
// CPC 464 only has banks 0-3 and always uses config 0.
//
// Screen memory: default CRTC start address is 0xC000 (slot 3). The CRTC can
// relocate it anywhere in the address space in 2K increments. Double-buffered
// games commonly use 0x4000 (slot 1) as the secondary screen buffer.

constexpr uint16_t	kBlockSize = 0x4000;	// 16K per slot

// Full address space
constexpr uint16_t	kAddressSpaceStart = 0x0000;
constexpr uint16_t	kAddressSpaceEnd   = 0xFFFF;
