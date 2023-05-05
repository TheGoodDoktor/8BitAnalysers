#define CHIPS_IMPL

#include "chips/chips_common.h"
#include "chips/z80.h"
#include "chips/m6502.h"
#include "chips/m6526.h"
#include "chips/m6569.h"
#include "chips/m6581.h"
#include "chips/beeper.h"
#include "chips/kbd.h"
#include "chips/mem.h"
#include "chips/clk.h"
#include <chips/m6502.h>
#include <chips/m6522.h>
#include <chips/m6569.h>
#include <systems/c1530.h>
#include <systems/c1541.h>
#include <systems/c64.h>

#define CHIPS_UTIL_IMPL
#define UI_DASM_USE_Z80
#define UI_DASM_USE_M6502
//#include "ui.h"
#include "util/m6502dasm.h"
#include "util/z80dasm.h"