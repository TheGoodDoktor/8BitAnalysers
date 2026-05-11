#include "CodeToolTipsHuC6280.h"
#include "RegisterView6502.h"
#include "../../CodeAnalyser.h"
#include "../CodeAnalyserUI.h"
#include <Misc/EmuBase.h>
#include <Util/Misc.h>
#include <imgui.h>
#include <unordered_map>

// Instruction descriptions for the HuC6280 (PC Engine CPU).
// Covers standard 6502, 65C02 additions, and HuC6280 extensions.

struct FHuCInstrInfo
{
	const char* Desc;
	const char* Operation;
};

static const std::unordered_map<uint8_t, FHuCInstrInfo> g_HuCOpcodes =
{
	// ---- Implied / single-byte (standard 6502) ----
	{ 0x00, { "Software interrupt",                      "PC+2,P -> stack; [$FFF6] -> PC" }},
	{ 0x08, { "Push processor status onto stack",        "P -> stack" }},
	{ 0x18, { "Clear carry flag",                        "0 -> C" }},
	{ 0x28, { "Pull processor status from stack",        "stack -> P" }},
	{ 0x38, { "Set carry flag",                          "1 -> C" }},
	{ 0x40, { "Return from interrupt",                   "stack -> P,PC" }},
	{ 0x48, { "Push A onto stack",                       "A -> stack" }},
	{ 0x58, { "Clear interrupt disable flag",            "0 -> I" }},
	{ 0x60, { "Return from subroutine",                  "stack -> PC; PC+1 -> PC" }},
	{ 0x68, { "Pull A from stack",                       "stack -> A" }},
	{ 0x78, { "Set interrupt disable flag",              "1 -> I" }},
	{ 0x88, { "Decrement Y",                             "Y - 1 -> Y" }},
	{ 0x98, { "Transfer Y to A",                         "Y -> A" }},
	{ 0xA8, { "Transfer A to Y",                         "A -> Y" }},
	{ 0xB8, { "Clear overflow flag",                     "0 -> V" }},
	{ 0xC8, { "Increment Y",                             "Y + 1 -> Y" }},
	{ 0xCA, { "Decrement X",                             "X - 1 -> X" }},
	{ 0xD8, { "Clear decimal flag",                      "0 -> D" }},
	{ 0xE8, { "Increment X",                             "X + 1 -> X" }},
	{ 0xEA, { "No operation",                            nullptr }},
	{ 0xF8, { "Set decimal flag",                        "1 -> D" }},
	{ 0x8A, { "Transfer X to A",                         "X -> A" }},
	{ 0x9A, { "Transfer X to stack pointer",             "X -> SP" }},
	{ 0xAA, { "Transfer A to X",                         "A -> X" }},
	{ 0xBA, { "Transfer stack pointer to X",             "SP -> X" }},

	// ---- Implied (65C02 additions) ----
	{ 0x1A, { "Increment A",                             "A + 1 -> A" }},
	{ 0x3A, { "Decrement A",                             "A - 1 -> A" }},
	{ 0x5A, { "Push Y onto stack",                       "Y -> stack" }},
	{ 0x7A, { "Pull Y from stack",                       "stack -> Y" }},
	{ 0xDA, { "Push X onto stack",                       "X -> stack" }},
	{ 0xFA, { "Pull X from stack",                       "stack -> X" }},

	// ---- Branches ----
	{ 0x10, { "Branch if plus (N clear)",                "N=0: PC + rel -> PC" }},
	{ 0x30, { "Branch if minus (N set)",                 "N=1: PC + rel -> PC" }},
	{ 0x50, { "Branch if overflow clear",                "V=0: PC + rel -> PC" }},
	{ 0x70, { "Branch if overflow set",                  "V=1: PC + rel -> PC" }},
	{ 0x80, { "Branch always",                           "PC + rel -> PC" }},
	{ 0x90, { "Branch if carry clear (unsigned <)",      "C=0: PC + rel -> PC" }},
	{ 0xB0, { "Branch if carry set (unsigned >=)",       "C=1: PC + rel -> PC" }},
	{ 0xD0, { "Branch if not equal (Z clear)",           "Z=0: PC + rel -> PC" }},
	{ 0xF0, { "Branch if equal (Z set)",                 "Z=1: PC + rel -> PC" }},

	// ---- ORA ----
	{ 0x01, { "OR A with (zp,X)",                        "A | M -> A" }},
	{ 0x05, { "OR A with zero page",                     "A | M -> A" }},
	{ 0x09, { "OR A with immediate",                     "A | M -> A" }},
	{ 0x0D, { "OR A with absolute",                      "A | M -> A" }},
	{ 0x11, { "OR A with (zp),Y",                        "A | M -> A" }},
	{ 0x12, { "OR A with (zp)",                          "A | M -> A" }},
	{ 0x15, { "OR A with zp,X",                          "A | M -> A" }},
	{ 0x19, { "OR A with abs,Y",                         "A | M -> A" }},
	{ 0x1D, { "OR A with abs,X",                         "A | M -> A" }},

	// ---- AND ----
	{ 0x21, { "AND A with (zp,X)",                       "A & M -> A" }},
	{ 0x25, { "AND A with zero page",                    "A & M -> A" }},
	{ 0x29, { "AND A with immediate",                    "A & M -> A" }},
	{ 0x2D, { "AND A with absolute",                     "A & M -> A" }},
	{ 0x31, { "AND A with (zp),Y",                       "A & M -> A" }},
	{ 0x32, { "AND A with (zp)",                         "A & M -> A" }},
	{ 0x35, { "AND A with zp,X",                         "A & M -> A" }},
	{ 0x39, { "AND A with abs,Y",                        "A & M -> A" }},
	{ 0x3D, { "AND A with abs,X",                        "A & M -> A" }},

	// ---- EOR ----
	{ 0x41, { "Exclusive OR A with (zp,X)",              "A ^ M -> A" }},
	{ 0x45, { "Exclusive OR A with zero page",           "A ^ M -> A" }},
	{ 0x49, { "Exclusive OR A with immediate",           "A ^ M -> A" }},
	{ 0x4D, { "Exclusive OR A with absolute",            "A ^ M -> A" }},
	{ 0x51, { "Exclusive OR A with (zp),Y",              "A ^ M -> A" }},
	{ 0x52, { "Exclusive OR A with (zp)",                "A ^ M -> A" }},
	{ 0x55, { "Exclusive OR A with zp,X",                "A ^ M -> A" }},
	{ 0x59, { "Exclusive OR A with abs,Y",               "A ^ M -> A" }},
	{ 0x5D, { "Exclusive OR A with abs,X",               "A ^ M -> A" }},

	// ---- ADC ----
	{ 0x61, { "Add (zp,X) to A with carry",              "A + M + C -> A,C" }},
	{ 0x65, { "Add zero page to A with carry",           "A + M + C -> A,C" }},
	{ 0x69, { "Add immediate to A with carry",           "A + M + C -> A,C" }},
	{ 0x6D, { "Add absolute to A with carry",            "A + M + C -> A,C" }},
	{ 0x71, { "Add (zp),Y to A with carry",              "A + M + C -> A,C" }},
	{ 0x72, { "Add (zp) to A with carry",                "A + M + C -> A,C" }},
	{ 0x75, { "Add zp,X to A with carry",                "A + M + C -> A,C" }},
	{ 0x79, { "Add abs,Y to A with carry",               "A + M + C -> A,C" }},
	{ 0x7D, { "Add abs,X to A with carry",               "A + M + C -> A,C" }},

	// ---- STA ----
	{ 0x81, { "Store A to (zp,X)",                       "A -> M" }},
	{ 0x85, { "Store A to zero page",                    "A -> M" }},
	{ 0x8D, { "Store A to absolute",                     "A -> M" }},
	{ 0x91, { "Store A to (zp),Y",                       "A -> M" }},
	{ 0x92, { "Store A to (zp)",                         "A -> M" }},
	{ 0x95, { "Store A to zp,X",                         "A -> M" }},
	{ 0x99, { "Store A to abs,Y",                        "A -> M" }},
	{ 0x9D, { "Store A to abs,X",                        "A -> M" }},

	// ---- LDA ----
	{ 0xA1, { "Load A from (zp,X)",                      "M -> A" }},
	{ 0xA5, { "Load A from zero page",                   "M -> A" }},
	{ 0xA9, { "Load A with immediate",                   "M -> A" }},
	{ 0xAD, { "Load A from absolute",                    "M -> A" }},
	{ 0xB1, { "Load A from (zp),Y",                      "M -> A" }},
	{ 0xB2, { "Load A from (zp)",                        "M -> A" }},
	{ 0xB5, { "Load A from zp,X",                        "M -> A" }},
	{ 0xB9, { "Load A from abs,Y",                       "M -> A" }},
	{ 0xBD, { "Load A from abs,X",                       "M -> A" }},

	// ---- CMP ----
	{ 0xC1, { "Compare A with (zp,X)",                   "A - M" }},
	{ 0xC5, { "Compare A with zero page",                "A - M" }},
	{ 0xC9, { "Compare A with immediate",                "A - M" }},
	{ 0xCD, { "Compare A with absolute",                 "A - M" }},
	{ 0xD1, { "Compare A with (zp),Y",                   "A - M" }},
	{ 0xD2, { "Compare A with (zp)",                     "A - M" }},
	{ 0xD5, { "Compare A with zp,X",                     "A - M" }},
	{ 0xD9, { "Compare A with abs,Y",                    "A - M" }},
	{ 0xDD, { "Compare A with abs,X",                    "A - M" }},

	// ---- SBC ----
	{ 0xE1, { "Subtract (zp,X) from A with borrow",      "A - M - ~C -> A" }},
	{ 0xE5, { "Subtract zero page from A with borrow",   "A - M - ~C -> A" }},
	{ 0xE9, { "Subtract immediate from A with borrow",   "A - M - ~C -> A" }},
	{ 0xED, { "Subtract absolute from A with borrow",    "A - M - ~C -> A" }},
	{ 0xF1, { "Subtract (zp),Y from A with borrow",      "A - M - ~C -> A" }},
	{ 0xF2, { "Subtract (zp) from A with borrow",        "A - M - ~C -> A" }},
	{ 0xF5, { "Subtract zp,X from A with borrow",        "A - M - ~C -> A" }},
	{ 0xF9, { "Subtract abs,Y from A with borrow",       "A - M - ~C -> A" }},
	{ 0xFD, { "Subtract abs,X from A with borrow",       "A - M - ~C -> A" }},

	// ---- ASL ----
	{ 0x06, { "Arithmetic shift left zero page",         "C <- [76543210] <- 0" }},
	{ 0x0A, { "Arithmetic shift left A",                 "C <- [76543210] <- 0" }},
	{ 0x0E, { "Arithmetic shift left absolute",          "C <- [76543210] <- 0" }},
	{ 0x16, { "Arithmetic shift left zp,X",              "C <- [76543210] <- 0" }},
	{ 0x1E, { "Arithmetic shift left abs,X",             "C <- [76543210] <- 0" }},

	// ---- ROL ----
	{ 0x26, { "Rotate left zero page",                   "C <- [76543210] <- C" }},
	{ 0x2A, { "Rotate left A",                           "C <- [76543210] <- C" }},
	{ 0x2E, { "Rotate left absolute",                    "C <- [76543210] <- C" }},
	{ 0x36, { "Rotate left zp,X",                        "C <- [76543210] <- C" }},
	{ 0x3E, { "Rotate left abs,X",                       "C <- [76543210] <- C" }},

	// ---- LSR ----
	{ 0x46, { "Logical shift right zero page",           "0 -> [76543210] -> C" }},
	{ 0x4A, { "Logical shift right A",                   "0 -> [76543210] -> C" }},
	{ 0x4E, { "Logical shift right absolute",            "0 -> [76543210] -> C" }},
	{ 0x56, { "Logical shift right zp,X",                "0 -> [76543210] -> C" }},
	{ 0x5E, { "Logical shift right abs,X",               "0 -> [76543210] -> C" }},

	// ---- ROR ----
	{ 0x66, { "Rotate right zero page",                  "C -> [76543210] -> C" }},
	{ 0x6A, { "Rotate right A",                          "C -> [76543210] -> C" }},
	{ 0x6E, { "Rotate right absolute",                   "C -> [76543210] -> C" }},
	{ 0x76, { "Rotate right zp,X",                       "C -> [76543210] -> C" }},
	{ 0x7E, { "Rotate right abs,X",                      "C -> [76543210] -> C" }},

	// ---- STX / LDX ----
	{ 0x86, { "Store X to zero page",                    "X -> M" }},
	{ 0x8E, { "Store X to absolute",                     "X -> M" }},
	{ 0x96, { "Store X to zp,Y",                         "X -> M" }},
	{ 0xA2, { "Load X with immediate",                   "M -> X" }},
	{ 0xA6, { "Load X from zero page",                   "M -> X" }},
	{ 0xAE, { "Load X from absolute",                    "M -> X" }},
	{ 0xB6, { "Load X from zp,Y",                        "M -> X" }},
	{ 0xBE, { "Load X from abs,Y",                       "M -> X" }},

	// ---- STY / LDY ----
	{ 0x84, { "Store Y to zero page",                    "Y -> M" }},
	{ 0x8C, { "Store Y to absolute",                     "Y -> M" }},
	{ 0x94, { "Store Y to zp,X",                         "Y -> M" }},
	{ 0xA0, { "Load Y with immediate",                   "M -> Y" }},
	{ 0xA4, { "Load Y from zero page",                   "M -> Y" }},
	{ 0xAC, { "Load Y from absolute",                    "M -> Y" }},
	{ 0xB4, { "Load Y from zp,X",                        "M -> Y" }},
	{ 0xBC, { "Load Y from abs,X",                       "M -> Y" }},

	// ---- DEC / INC ----
	{ 0xC6, { "Decrement zero page",                     "M - 1 -> M" }},
	{ 0xCE, { "Decrement absolute",                      "M - 1 -> M" }},
	{ 0xD6, { "Decrement zp,X",                          "M - 1 -> M" }},
	{ 0xDE, { "Decrement abs,X",                         "M - 1 -> M" }},
	{ 0xE6, { "Increment zero page",                     "M + 1 -> M" }},
	{ 0xEE, { "Increment absolute",                      "M + 1 -> M" }},
	{ 0xF6, { "Increment zp,X",                          "M + 1 -> M" }},
	{ 0xFE, { "Increment abs,X",                         "M + 1 -> M" }},

	// ---- CPX / CPY ----
	{ 0xE0, { "Compare X with immediate",                "X - M" }},
	{ 0xE4, { "Compare X with zero page",                "X - M" }},
	{ 0xEC, { "Compare X with absolute",                 "X - M" }},
	{ 0xC0, { "Compare Y with immediate",                "Y - M" }},
	{ 0xC4, { "Compare Y with zero page",                "Y - M" }},
	{ 0xCC, { "Compare Y with absolute",                 "Y - M" }},

	// ---- BIT ----
	{ 0x24, { "Test bits in zero page",                  "A & M; M7->N, M6->V" }},
	{ 0x2C, { "Test bits in absolute",                   "A & M; M7->N, M6->V" }},
	{ 0x34, { "Test bits in zp,X",                       "A & M; M7->N, M6->V" }},
	{ 0x3C, { "Test bits in abs,X",                      "A & M; M7->N, M6->V" }},
	{ 0x89, { "Test bits with immediate",                "A & imm; sets Z only (N,V unchanged)" }},

	// ---- JMP / JSR ----
	{ 0x20, { "Jump to subroutine",                      "PC+2 -> stack; addr -> PC" }},
	{ 0x44, { "Branch to subroutine (relative)",         "PC+2 -> stack; PC + rel -> PC" }},
	{ 0x4C, { "Jump to absolute address",                "addr -> PC" }},
	{ 0x6C, { "Jump to indirect address",                "[addr] -> PC" }},
	{ 0x7C, { "Jump to (abs,X) address",                 "[addr+X] -> PC" }},

	// ---- STZ / TSB / TRB (65C02) ----
	{ 0x64, { "Store zero to zero page",                 "0 -> M" }},
	{ 0x74, { "Store zero to zp,X",                      "0 -> M" }},
	{ 0x9C, { "Store zero to absolute",                  "0 -> M" }},
	{ 0x9E, { "Store zero to abs,X",                     "0 -> M" }},
	{ 0x04, { "Test and set bits in zero page",          "A | M -> M; ~(A & M) -> Z" }},
	{ 0x0C, { "Test and set bits in absolute",           "A | M -> M; ~(A & M) -> Z" }},
	{ 0x14, { "Test and reset bits in zero page",        "~A & M -> M; ~(A & M) -> Z" }},
	{ 0x1C, { "Test and reset bits in absolute",         "~A & M -> M; ~(A & M) -> Z" }},

	// ---- HuC6280: register swap ----
	{ 0x02, { "Swap X and Y",                            "X <-> Y" }},
	{ 0x22, { "Swap A and X",                            "A <-> X" }},
	{ 0x42, { "Swap A and Y",                            "A <-> Y" }},

	// ---- HuC6280: register clear ----
	{ 0x62, { "Clear A",                                 "0 -> A" }},
	{ 0x82, { "Clear X",                                 "0 -> X" }},
	{ 0xC2, { "Clear Y",                                 "0 -> Y" }},

	// ---- HuC6280: speed / T flag ----
	{ 0x54, { "CPU speed low (1.79 MHz)",                nullptr }},
	{ 0xD4, { "CPU speed high (7.16 MHz)",               nullptr }},
	{ 0xF4, { "Set T flag",                              "1 -> T; next ALU op uses [zp] as source" }},

	// ---- HuC6280: MPR (memory page register / bank mapping) ----
	{ 0x43, { "Transfer MPR to A",                       "MPR[n] -> A" }},
	{ 0x53, { "Transfer A to MPR",                       "A -> MPR[n]" }},

	// ---- HuC6280: VDC I/O ----
	{ 0x03, { "Store to VDC address register (ST0)",     "#imm -> VDC AR (selects VDC register)" }},
	{ 0x13, { "Store to VDC data register low (ST1)",    "#imm -> VDC DL" }},
	{ 0x23, { "Store to VDC data register high (ST2)",   "#imm -> VDC DH" }},

	// ---- HuC6280: block transfer ----
	{ 0x73, { "Block transfer: source++, dest++",        "M[src++] -> M[dst++]; len-- until 0" }},
	{ 0xC3, { "Block transfer: source--, dest--",        "M[src--] -> M[dst--]; len-- until 0" }},
	{ 0xD3, { "Block transfer: source++, dest fixed",    "M[src++] -> M[dst]; len-- until 0" }},
	{ 0xF3, { "Block transfer: alternating source",      "alternates src/src+1 each byte; dst++" }},
	{ 0xE3, { "Block transfer: alternating dest",        "src++; alternates dst/dst+1 each byte" }},

	// ---- HuC6280: TST ----
	{ 0x83, { "Test bits: imm & zp",                     "imm & M[zp]; sets N,Z" }},
	{ 0x93, { "Test bits: imm & zp,X",                   "imm & M[zp+X]; sets N,Z" }},
	{ 0xA3, { "Test bits: imm & abs",                    "imm & M[abs]; sets N,Z" }},
	{ 0xB3, { "Test bits: imm & abs,X",                  "imm & M[abs+X]; sets N,Z" }},

	// ---- HuC6280: RMB (reset memory bit) ----
	{ 0x07, { "Reset memory bit 0",                      "M &= ~(1<<0)" }},
	{ 0x17, { "Reset memory bit 1",                      "M &= ~(1<<1)" }},
	{ 0x27, { "Reset memory bit 2",                      "M &= ~(1<<2)" }},
	{ 0x37, { "Reset memory bit 3",                      "M &= ~(1<<3)" }},
	{ 0x47, { "Reset memory bit 4",                      "M &= ~(1<<4)" }},
	{ 0x57, { "Reset memory bit 5",                      "M &= ~(1<<5)" }},
	{ 0x67, { "Reset memory bit 6",                      "M &= ~(1<<6)" }},
	{ 0x77, { "Reset memory bit 7",                      "M &= ~(1<<7)" }},

	// ---- HuC6280: SMB (set memory bit) ----
	{ 0x87, { "Set memory bit 0",                        "M |= (1<<0)" }},
	{ 0x97, { "Set memory bit 1",                        "M |= (1<<1)" }},
	{ 0xA7, { "Set memory bit 2",                        "M |= (1<<2)" }},
	{ 0xB7, { "Set memory bit 3",                        "M |= (1<<3)" }},
	{ 0xC7, { "Set memory bit 4",                        "M |= (1<<4)" }},
	{ 0xD7, { "Set memory bit 5",                        "M |= (1<<5)" }},
	{ 0xE7, { "Set memory bit 6",                        "M |= (1<<6)" }},
	{ 0xF7, { "Set memory bit 7",                        "M |= (1<<7)" }},

	// ---- HuC6280: BBR (branch on bit reset) ----
	{ 0x0F, { "Branch if bit 0 of zp is reset",          "M[zp].0=0: PC + rel -> PC" }},
	{ 0x1F, { "Branch if bit 1 of zp is reset",          "M[zp].1=0: PC + rel -> PC" }},
	{ 0x2F, { "Branch if bit 2 of zp is reset",          "M[zp].2=0: PC + rel -> PC" }},
	{ 0x3F, { "Branch if bit 3 of zp is reset",          "M[zp].3=0: PC + rel -> PC" }},
	{ 0x4F, { "Branch if bit 4 of zp is reset",          "M[zp].4=0: PC + rel -> PC" }},
	{ 0x5F, { "Branch if bit 5 of zp is reset",          "M[zp].5=0: PC + rel -> PC" }},
	{ 0x6F, { "Branch if bit 6 of zp is reset",          "M[zp].6=0: PC + rel -> PC" }},
	{ 0x7F, { "Branch if bit 7 of zp is reset",          "M[zp].7=0: PC + rel -> PC" }},

	// ---- HuC6280: BBS (branch on bit set) ----
	{ 0x8F, { "Branch if bit 0 of zp is set",            "M[zp].0=1: PC + rel -> PC" }},
	{ 0x9F, { "Branch if bit 1 of zp is set",            "M[zp].1=1: PC + rel -> PC" }},
	{ 0xAF, { "Branch if bit 2 of zp is set",            "M[zp].2=1: PC + rel -> PC" }},
	{ 0xBF, { "Branch if bit 3 of zp is set",            "M[zp].3=1: PC + rel -> PC" }},
	{ 0xCF, { "Branch if bit 4 of zp is set",            "M[zp].4=1: PC + rel -> PC" }},
	{ 0xDF, { "Branch if bit 5 of zp is set",            "M[zp].5=1: PC + rel -> PC" }},
	{ 0xEF, { "Branch if bit 6 of zp is set",            "M[zp].6=1: PC + rel -> PC" }},
	{ 0xFF, { "Branch if bit 7 of zp is set",            "M[zp].7=1: PC + rel -> PC" }},
};

// ============================================================
// Register operand system
// ============================================================

namespace HuCToolTip
{

// PCE zero page lives at MPR slot 1 = $2000
static constexpr uint16_t kZPBase = 0x2000;

enum class EReg { None, A, X, Y, P };

struct FTooltipInfo
{
	EReg    regs[3]   = { EReg::None, EReg::None, EReg::None };
	int     numRegs   = 0;
	bool    bBinary   = false;  // show registers in binary (logical ops)
	bool    bShowFlags = false; // show P flags breakdown

	bool     bHasMem    = false;
	uint16_t memAddr    = 0;
	bool     bMemIsDest = false; // memory is destination: snippet goes above regs
};

// ------------------------------------------------------------
// Effective address computation
// ------------------------------------------------------------

static uint16_t ReadZPWord(const FCodeAnalysisState& state, uint8_t zp)
{
	const uint16_t addr = kZPBase + zp;
	return state.ReadByte(addr) | (state.ReadByte(addr + 1) << 8);
}

// Returns {hasAddress, effectiveAddress}.
// Immediate-mode instructions don't have a memory address so bHasMem stays false.
static bool ComputeEffectiveAddress(const FCodeAnalysisState& state, uint16_t pc,
                                    uint8_t op, const ICPUEmulator6502* pCPU,
                                    uint16_t& outAddr)
{
	const uint8_t imm8  = state.ReadByte(pc + 1);
	const uint16_t imm16 = state.ReadByte(pc + 1) | (state.ReadByte(pc + 2) << 8);
	const uint8_t X = pCPU->GetX();
	const uint8_t Y = pCPU->GetY();

	switch (op)
	{
	// ---- Zero page ----
	case 0x05: case 0x06: case 0x07: case 0x24: case 0x25: case 0x26: case 0x27:
	case 0x45: case 0x46: case 0x47: case 0x64: case 0x65: case 0x66: case 0x67:
	case 0x84: case 0x85: case 0x86: case 0x87: case 0xA4: case 0xA5: case 0xA6:
	case 0xA7: case 0xC4: case 0xC5: case 0xC6: case 0xC7: case 0xE4: case 0xE5:
	case 0xE6: case 0xE7:
	// BIT zp:
	// STZ zp: 0x64
	// SMB: 0x87, 0x97, etc  (zp only, handled via fall-through above)
		outAddr = kZPBase + imm8;
		return true;

	// ---- Zero page, X ----
	case 0x15: case 0x16: case 0x34: case 0x35: case 0x36: case 0x55: case 0x56:
	case 0x74: case 0x75: case 0x76: case 0x94: case 0x95: case 0xB4: case 0xB5:
	case 0xD5: case 0xD6: case 0xF5: case 0xF6: case 0x3C:
		outAddr = kZPBase + ((imm8 + X) & 0xFF);
		return true;

	// ---- Zero page, Y ----
	case 0x96: case 0xB6:
		outAddr = kZPBase + ((imm8 + Y) & 0xFF);
		return true;

	// ---- Absolute ----
	case 0x0C: case 0x0D: case 0x0E: case 0x1C: case 0x2C: case 0x2D: case 0x2E:
	case 0x4C: case 0x4D: case 0x4E: case 0x6D: case 0x6E: case 0x8C: case 0x8D:
	case 0x8E: case 0xAC: case 0xAD: case 0xAE: case 0xCC: case 0xCD: case 0xCE:
	case 0xEC: case 0xED: case 0xEE: case 0x9C:
		outAddr = imm16;
		return true;

	// ---- Absolute, X ----
	case 0x1D: case 0x1E: case 0x3D: case 0x3E: case 0x5D: case 0x5E: case 0x7D:
	case 0x7E: case 0x9D: case 0x9E: case 0xBD: case 0xDD: case 0xDE: case 0xFD:
	case 0xFE: case 0xBC:
		outAddr = imm16 + X;
		return true;

	// ---- Absolute, Y ----
	case 0x19: case 0x39: case 0x59: case 0x79: case 0x99: case 0xB9: case 0xBE:
	case 0xD9: case 0xF9:
		outAddr = imm16 + Y;
		return true;

	// ---- (zp, X) ----
	case 0x01: case 0x21: case 0x41: case 0x61: case 0x81: case 0xA1: case 0xC1:
	case 0xE1:
		outAddr = ReadZPWord(state, (imm8 + X) & 0xFF);
		return true;

	// ---- (zp), Y ----
	case 0x11: case 0x31: case 0x51: case 0x71: case 0x91: case 0xB1: case 0xD1:
	case 0xF1:
		outAddr = ReadZPWord(state, imm8) + Y;
		return true;

	// ---- (zp) ----
	case 0x12: case 0x32: case 0x52: case 0x72: case 0x92: case 0xB2: case 0xD2:
	case 0xF2:
		outAddr = ReadZPWord(state, imm8);
		return true;

	default:
		return false;
	}
}

// ------------------------------------------------------------
// Per-opcode operand decoding
// ------------------------------------------------------------

static FTooltipInfo DecodeTooltipInfo(const FCodeAnalysisState& state, uint16_t pc,
                                      uint8_t op, const ICPUEmulator6502* pCPU)
{
	FTooltipInfo info;

	auto addReg = [&](EReg r) {
		if (info.numRegs < 3) info.regs[info.numRegs++] = r;
	};

	uint16_t memAddr = 0;
	const bool hasMem = ComputeEffectiveAddress(state, pc, op, pCPU, memAddr);

	switch (op)
	{
	// ---- LDA ----
	case 0xA1: case 0xA5: case 0xA9: case 0xAD: case 0xB1: case 0xB2:
	case 0xB5: case 0xB9: case 0xBD:
		addReg(EReg::A);
		info.bShowFlags = true;
		if (hasMem) { info.bHasMem = true; info.memAddr = memAddr; }
		break;

	// ---- STA ----
	case 0x81: case 0x85: case 0x8D: case 0x91: case 0x92: case 0x95:
	case 0x99: case 0x9D:
		addReg(EReg::A);
		if (hasMem) { info.bHasMem = true; info.memAddr = memAddr; info.bMemIsDest = true; }
		break;

	// ---- LDX ----
	case 0xA2: case 0xA6: case 0xAE: case 0xB6: case 0xBE:
		addReg(EReg::X);
		info.bShowFlags = true;
		if (hasMem) { info.bHasMem = true; info.memAddr = memAddr; }
		break;

	// ---- STX ----
	case 0x86: case 0x8E: case 0x96:
		addReg(EReg::X);
		if (hasMem) { info.bHasMem = true; info.memAddr = memAddr; info.bMemIsDest = true; }
		break;

	// ---- LDY ----
	case 0xA0: case 0xA4: case 0xAC: case 0xB4: case 0xBC:
		addReg(EReg::Y);
		info.bShowFlags = true;
		if (hasMem) { info.bHasMem = true; info.memAddr = memAddr; }
		break;

	// ---- STY ----
	case 0x84: case 0x8C: case 0x94:
		addReg(EReg::Y);
		if (hasMem) { info.bHasMem = true; info.memAddr = memAddr; info.bMemIsDest = true; }
		break;

	// ---- STZ ----
	case 0x64: case 0x74: case 0x9C: case 0x9E:
		if (hasMem) { info.bHasMem = true; info.memAddr = memAddr; info.bMemIsDest = true; }
		break;

	// ---- ORA / AND / EOR (logical) ----
	case 0x01: case 0x05: case 0x09: case 0x0D: case 0x11: case 0x12:
	case 0x15: case 0x19: case 0x1D: // ORA
	case 0x21: case 0x25: case 0x29: case 0x2D: case 0x31: case 0x32:
	case 0x35: case 0x39: case 0x3D: // AND
	case 0x41: case 0x45: case 0x49: case 0x4D: case 0x51: case 0x52:
	case 0x55: case 0x59: case 0x5D: // EOR
		addReg(EReg::A);
		info.bBinary = true;
		info.bShowFlags = true;
		if (hasMem) { info.bHasMem = true; info.memAddr = memAddr; }
		break;

	// ---- ADC / SBC ----
	case 0x61: case 0x65: case 0x69: case 0x6D: case 0x71: case 0x72:
	case 0x75: case 0x79: case 0x7D: // ADC
	case 0xE1: case 0xE5: case 0xE9: case 0xED: case 0xF1: case 0xF2:
	case 0xF5: case 0xF9: case 0xFD: // SBC
		addReg(EReg::A);
		info.bShowFlags = true;
		if (hasMem) { info.bHasMem = true; info.memAddr = memAddr; }
		break;

	// ---- CMP ----
	case 0xC1: case 0xC5: case 0xC9: case 0xCD: case 0xD1: case 0xD2:
	case 0xD5: case 0xD9: case 0xDD:
		addReg(EReg::A);
		info.bShowFlags = true;
		if (hasMem) { info.bHasMem = true; info.memAddr = memAddr; }
		break;

	// ---- CPX ----
	case 0xE0: case 0xE4: case 0xEC:
		addReg(EReg::X);
		info.bShowFlags = true;
		if (hasMem) { info.bHasMem = true; info.memAddr = memAddr; }
		break;

	// ---- CPY ----
	case 0xC0: case 0xC4: case 0xCC:
		addReg(EReg::Y);
		info.bShowFlags = true;
		if (hasMem) { info.bHasMem = true; info.memAddr = memAddr; }
		break;

	// ---- ASL / LSR on A ----
	case 0x0A: case 0x4A:
		addReg(EReg::A);
		info.bBinary = true;
		info.bShowFlags = true;
		break;

	// ---- ASL / LSR on memory ----
	case 0x06: case 0x0E: case 0x16: case 0x1E: // ASL mem
	case 0x46: case 0x4E: case 0x56: case 0x5E: // LSR mem
		info.bShowFlags = true;
		if (hasMem) { info.bHasMem = true; info.memAddr = memAddr; info.bMemIsDest = true; }
		break;

	// ---- ROL / ROR on A ----
	case 0x2A: case 0x6A:
		addReg(EReg::A);
		info.bBinary = true;
		info.bShowFlags = true;
		break;

	// ---- ROL / ROR on memory ----
	case 0x26: case 0x2E: case 0x36: case 0x3E: // ROL mem
	case 0x66: case 0x6E: case 0x76: case 0x7E: // ROR mem
		info.bShowFlags = true;
		if (hasMem) { info.bHasMem = true; info.memAddr = memAddr; info.bMemIsDest = true; }
		break;

	// ---- INC / DEC on memory ----
	case 0xC6: case 0xCE: case 0xD6: case 0xDE: // DEC mem
	case 0xE6: case 0xEE: case 0xF6: case 0xFE: // INC mem
		info.bShowFlags = true;
		if (hasMem) { info.bHasMem = true; info.memAddr = memAddr; info.bMemIsDest = true; }
		break;

	// ---- INA / DEA ----
	case 0x1A: case 0x3A:
		addReg(EReg::A);
		info.bShowFlags = true;
		break;

	// ---- INX / DEX ----
	case 0xCA: case 0xE8:
		addReg(EReg::X);
		info.bShowFlags = true;
		break;

	// ---- INY / DEY ----
	case 0x88: case 0xC8:
		addReg(EReg::Y);
		info.bShowFlags = true;
		break;

	// ---- BIT ----
	case 0x24: case 0x2C: case 0x34: case 0x3C: case 0x89:
		addReg(EReg::A);
		info.bBinary = true;
		info.bShowFlags = true;
		if (hasMem) { info.bHasMem = true; info.memAddr = memAddr; }
		break;

	// ---- TSB / TRB ----
	case 0x04: case 0x0C: case 0x14: case 0x1C:
		addReg(EReg::A);
		info.bBinary = true;
		info.bShowFlags = true;
		if (hasMem) { info.bHasMem = true; info.memAddr = memAddr; info.bMemIsDest = true; }
		break;

	// ---- Transfer A <-> X/Y ----
	case 0xAA: addReg(EReg::X); addReg(EReg::A); info.bShowFlags = true; break; // TAX
	case 0x8A: addReg(EReg::A); addReg(EReg::X); info.bShowFlags = true; break; // TXA
	case 0xA8: addReg(EReg::Y); addReg(EReg::A); info.bShowFlags = true; break; // TAY
	case 0x98: addReg(EReg::A); addReg(EReg::Y); info.bShowFlags = true; break; // TYA

	// ---- Transfer SP <-> X ----
	case 0x9A: addReg(EReg::X); break; // TXS
	case 0xBA: addReg(EReg::X); break; // TSX

	// ---- HuC: register swaps ----
	case 0x02: addReg(EReg::X); addReg(EReg::Y); break; // SXY
	case 0x22: addReg(EReg::A); addReg(EReg::X); break; // SAX
	case 0x42: addReg(EReg::A); addReg(EReg::Y); break; // SAY

	// ---- HuC: register clears ----
	case 0x62: addReg(EReg::A); break; // CLA
	case 0x82: addReg(EReg::X); break; // CLX
	case 0xC2: addReg(EReg::Y); break; // CLY

	// ---- Stack ops ----
	case 0x48: addReg(EReg::A); break; // PHA
	case 0x68: addReg(EReg::A); info.bShowFlags = true; break; // PLA
	case 0xDA: addReg(EReg::X); break; // PHX
	case 0xFA: addReg(EReg::X); info.bShowFlags = true; break; // PLX
	case 0x5A: addReg(EReg::Y); break; // PHY
	case 0x7A: addReg(EReg::Y); info.bShowFlags = true; break; // PLY
	case 0x08: info.bShowFlags = true; break; // PHP
	case 0x28: info.bShowFlags = true; break; // PLP

	// ---- Flag instructions ----
	case 0x18: case 0x38: case 0x58: case 0x78:
	case 0xB8: case 0xD8: case 0xF8: case 0xF4:
		info.bShowFlags = true;
		break;

	// ---- Branches (flags as input) ----
	case 0x10: case 0x30: case 0x50: case 0x70: case 0x80:
	case 0x90: case 0xB0: case 0xD0: case 0xF0:
		info.bShowFlags = true;
		break;

	default:
		break;
	}

	return info;
}

// ------------------------------------------------------------
// Rendering helpers
// ------------------------------------------------------------

static const ImVec4 kColNormal  = { 1.f, 1.f, 1.f, 1.f };
static const ImVec4 kColChanged = { 1.f, 1.f, 0.f, 1.f };

static void DrawFlagsRow(uint8_t p, uint8_t oldP)
{
	static ImGuiTableFlags flags = ImGuiTableFlags_Borders | ImGuiTableFlags_RowBg;
	if (ImGui::BeginTable("HuCFlags", 8, flags))
	{
		const char* names[] = { "N", "V", "T", "B", "D", "I", "Z", "C" };
		for (const char* n : names) ImGui::TableSetupColumn(n);
		ImGui::TableHeadersRow();
		ImGui::TableNextRow();
		for (int i = 7; i >= 0; i--)
		{
			ImGui::TableNextColumn();
			const bool cur = (p    >> i) & 1;
			const bool old = (oldP >> i) & 1;
			ImGui::TextColored(cur != old ? kColChanged : kColNormal, "%d", cur ? 1 : 0);
		}
		ImGui::EndTable();
	}
}

static void DrawRegValue(const char* name, uint8_t cur, uint8_t old, bool bBinary)
{
	const bool changed = cur != old;
	const ImVec4& col = changed ? kColChanged : kColNormal;
	if (bBinary)
		ImGui::TextColored(col, "%s:$%02X  bin:%s", name,
		                   cur, NumStr(cur, ENumberDisplayMode::Binary));
	else
		ImGui::TextColored(col, "%s:%s", name, NumStr(cur));
}

static void DrawRegTable(const FTooltipInfo& info,
                         const ICPUEmulator6502* pCPU,
                         const F6502DisplayRegisters& old)
{
	if (info.numRegs == 0)
		return;

	static ImGuiTableFlags tflags = ImGuiTableFlags_Borders | ImGuiTableFlags_RowBg
	                              | ImGuiTableFlags_SizingStretchSame;
	if (ImGui::BeginTable("HuCRegs", info.numRegs, tflags))
	{
		for (int i = 0; i < info.numRegs; i++)
			ImGui::TableNextColumn();

		// Re-enter first row
		ImGui::TableNextRow();
		for (int i = 0; i < info.numRegs; i++)
		{
			ImGui::TableSetColumnIndex(i);
			switch (info.regs[i])
			{
			case EReg::A: DrawRegValue("A", pCPU->GetA(), old.A, info.bBinary); break;
			case EReg::X: DrawRegValue("X", pCPU->GetX(), old.X, info.bBinary); break;
			case EReg::Y: DrawRegValue("Y", pCPU->GetY(), old.Y, info.bBinary); break;
			default: break;
			}
		}
		ImGui::EndTable();
	}
}

static void DrawMemOperand(FCodeAnalysisState& state, FCodeAnalysisViewState& viewState,
                           uint16_t addr)
{
	static ImGuiTableFlags tflags = ImGuiTableFlags_Borders | ImGuiTableFlags_RowBg;
	if (ImGui::BeginTable("HuCMem", 1, tflags))
	{
		ImGui::TableNextColumn();
		ImGui::Text("$%04X = $%02X", addr, state.ReadByte(addr));
		DrawAddressLabel(state, viewState, addr);
		const FAddressRef ref = state.AddressRefFromPhysicalAddress(addr);
		viewState.HighlightAddress = ref;
		ImGui::Spacing();
		DrawSnippetToolTip(state, viewState, ref, 5);
		if (ImGui::IsMouseDoubleClicked(0))
			viewState.GoToAddress(ref);
		ImGui::EndTable();
	}
}

// ------------------------------------------------------------
// TAM / TMA special display
// ------------------------------------------------------------

static void DrawTamTma(FCodeAnalysisState& state, uint16_t pc, uint8_t op,
                       const ICPUEmulator6502* pCPU)
{
	const uint8_t mask = state.ReadByte(pc + 1);
	const FEmuBase* pEmu = state.GetEmulator();

	static ImGuiTableFlags tflags = ImGuiTableFlags_Borders | ImGuiTableFlags_RowBg;

	if (op == 0x53) // TAM: A -> MPR[slots in mask]
	{
		const uint8_t bankIdx = pCPU->GetA();
		const char* bankName = nullptr;

		ImGui::Text("A = $%02X (%d)", bankIdx, bankIdx);
		ImGui::Spacing();

		if (ImGui::BeginTable("TAMSlots", 2, tflags))
		{
			ImGui::TableSetupColumn("MPR Slot");
			ImGui::TableSetupColumn("Bank mapped");
			ImGui::TableHeadersRow();

			for (int slot = 0; slot < 8; slot++)
			{
				if (!(mask & (1 << slot)))
					continue;
				ImGui::TableNextRow();
				ImGui::TableNextColumn();
				ImGui::Text("Slot %d ($%04X)", slot, slot * 0x2000);
				ImGui::TableNextColumn();
				if (bankName)
					ImGui::Text("$%02X  \"%s\"", bankIdx, bankName);
				else
					ImGui::Text("$%02X", bankIdx);
			}
			ImGui::EndTable();
		}
	}
	else // TMA (0x43): A <- MPR[slot in mask]
	{
		if (ImGui::BeginTable("TMASlots", 3, tflags))
		{
			ImGui::TableSetupColumn("MPR Slot");
			ImGui::TableSetupColumn("Bank index");
			//ImGui::TableSetupColumn("Bank name");
			ImGui::TableHeadersRow();

			for (int slot = 0; slot < 8; slot++)
			{
				if (!(mask & (1 << slot)))
					continue;
				ImGui::TableNextRow();
				ImGui::TableNextColumn();
				ImGui::Text("Slot %d ($%04X)", slot, slot * 0x2000);

				// Get the bank currently mapped at this slot's address.
				const uint16_t slotAddr = (uint16_t)(slot * 0x2000);
				const int16_t bankId = state.GetBankFromAddress(slotAddr);
				const FCodeAnalysisBank* pBank = state.GetBank(bankId);

				ImGui::TableNextColumn();
				if (pBank) ImGui::Text("$%02X", pBank->Id); else ImGui::Text("?");
				//ImGui::TableNextColumn();
				//if (pBank) ImGui::TextUnformatted(pBank->Name.c_str()); else ImGui::Text("?");
			}
			ImGui::EndTable();
		}
	}
}

// ------------------------------------------------------------
// Block transfer operands
// ------------------------------------------------------------

static bool DrawBlockTransfer(FCodeAnalysisState& state, uint16_t pc, uint8_t op)
{
	static const uint8_t kBlockOps[] = { 0x73, 0xC3, 0xD3, 0xF3, 0xE3 };
	for (uint8_t bop : kBlockOps)
	{
		if (op != bop) continue;
		const uint16_t src = state.ReadWord(pc + 1);
		const uint16_t dst = state.ReadWord(pc + 3);
		const uint16_t len = state.ReadWord(pc + 5);
		ImGui::Text("src=$%04X  dst=$%04X  len=$%04X (%d bytes)", src, dst, len, len);
		return true;
	}
	return false;
}

// ------------------------------------------------------------
// TST operands
// ------------------------------------------------------------

static bool DrawTstOperands(FCodeAnalysisState& state, uint16_t pc, uint8_t op)
{
	uint8_t imm; uint16_t target;
	switch (op)
	{
	case 0x83:
		imm = state.ReadByte(pc + 1); target = state.ReadByte(pc + 2);
		ImGui::Text("mask=$%02X (%s)  zp=$%02X (=$%02X)",
		            imm, NumStr(imm, ENumberDisplayMode::Binary),
		            target, state.ReadByte(kZPBase + target));
		return true;
	case 0x93:
		imm = state.ReadByte(pc + 1); target = state.ReadByte(pc + 2);
		ImGui::Text("mask=$%02X (%s)  zp=$%02X,X",
		            imm, NumStr(imm, ENumberDisplayMode::Binary), target);
		return true;
	case 0xA3:
		imm = state.ReadByte(pc + 1); target = state.ReadWord(pc + 2);
		ImGui::Text("mask=$%02X (%s)  abs=$%04X (=$%02X)",
		            imm, NumStr(imm, ENumberDisplayMode::Binary),
		            target, state.ReadByte(target));
		return true;
	case 0xB3:
		imm = state.ReadByte(pc + 1); target = state.ReadWord(pc + 2);
		ImGui::Text("mask=$%02X (%s)  abs=$%04X,X",
		            imm, NumStr(imm, ENumberDisplayMode::Binary), target);
		return true;
	}
	return false;
}

// ------------------------------------------------------------
// BBR / BBS operands
// ------------------------------------------------------------

static bool DrawBitBranchOperands(FCodeAnalysisState& state, uint16_t pc, uint8_t op)
{
	if ((op & 0x0F) != 0x0F) return false;
	const uint8_t  zp     = state.ReadByte(pc + 1);
	const int8_t   rel    = static_cast<int8_t>(state.ReadByte(pc + 2));
	const uint16_t target = static_cast<uint16_t>(pc + 3 + rel);
	const uint8_t  live   = state.ReadByte(kZPBase + zp);
	const int bit = (op >> 4) & 7;
	const bool isBBS = (op & 0x80) != 0;
	ImGui::Text("zp=$%02X (=$%02X  bit%d=%d)  target=$%04X",
	            zp, live, bit, (live >> bit) & 1, target);
	ImGui::Text("Branch %staken", ((live >> bit) & 1) == (isBBS ? 1 : 0) ? "" : "NOT ");
	return true;
}

} // namespace HuCToolTip

// ============================================================
// Public entry point
// ============================================================

void ShowCodeToolTipHuC6280(FCodeAnalysisState& state, uint16_t addr)
{
	using namespace HuCToolTip;

	const uint8_t op = state.ReadByte(addr);
	ICPUEmulator6502* pCPU = (ICPUEmulator6502*)state.CPUInterface->GetCPUEmulator();

	ImGui::BeginTooltip();

	// Description + operation string
	const auto it = g_HuCOpcodes.find(op);
	if (it != g_HuCOpcodes.end())
	{
		const FHuCInstrInfo& info = it->second;
		ImGui::TextUnformatted(info.Desc);
		if (info.Operation)
		{
			ImGui::Separator();
			ImGui::TextUnformatted(info.Operation);
		}
	}
	else
	{
		ImGui::Text("Unknown opcode $%02X", op);
		ImGui::EndTooltip();
		return;
	}

	// Special-case instructions handled separately
	if (op == 0x43 || op == 0x53)
	{
		ImGui::Separator();
		DrawTamTma(state, addr, op, pCPU);
		ImGui::EndTooltip();
		return;
	}
	if (DrawBlockTransfer(state, addr, op))  { ImGui::EndTooltip(); return; }
	if (DrawTstOperands(state, addr, op))    { ImGui::EndTooltip(); return; }
	if (DrawBitBranchOperands(state, addr, op)) { ImGui::EndTooltip(); return; }

	// General register / memory display
	if (pCPU == nullptr) { ImGui::EndTooltip(); return; }

	const FTooltipInfo info = DecodeTooltipInfo(state, addr, op, pCPU);
	const F6502DisplayRegisters& old = GetStoredRegisters_6502();
	FCodeAnalysisViewState& viewState = state.GetFocussedViewState();

	const bool hasAnything = info.bShowFlags || info.numRegs > 0 || info.bHasMem;
	if (!hasAnything) { ImGui::EndTooltip(); return; }

	ImGui::Separator();

	// Show immediate value in the description when it's present
	// (instruction already named it, but show the current actual value)
	{
		const uint8_t imm8 = state.ReadByte(addr + 1);
		switch (op)
		{
		// Instructions where the first operand byte IS the immediate value
		case 0xA9: case 0xA2: case 0xA0: // LDA/LDX/LDY #imm
		case 0x09: case 0x29: case 0x49: case 0x69: case 0xC9: case 0xE9: // ALU #imm
		case 0xE0: case 0xC0:            // CPX/CPY #imm
		case 0x89:                       // BIT #imm
		{
			const bool bBin = (op == 0x09 || op == 0x29 || op == 0x49 || op == 0x89);
			if (bBin)
				ImGui::Text("#imm = $%02X  %s", imm8, NumStr(imm8, ENumberDisplayMode::Binary));
			else
				ImGui::Text("#imm = $%02X", imm8);
			break;
		}
		default: break;
		}
	}

	// Flags
	if (info.bShowFlags)
		DrawFlagsRow(pCPU->GetP(), old.P);

	// Memory is destination: snippet first, registers below
	if (info.bHasMem && info.bMemIsDest)
	{
		DrawMemOperand(state, viewState, info.memAddr);
		DrawRegTable(info, pCPU, old);
	}
	else
	{
		// Registers first (leftmost = destination), memory snippet below
		DrawRegTable(info, pCPU, old);
		if (info.bHasMem)
			DrawMemOperand(state, viewState, info.memAddr);
	}

	ImGui::EndTooltip();
}
