#include "CodeToolTipsHuC6280.h"
#include "RegisterView6502.h"
#include "../../6502/CodeAnalyserHuC6280.h"
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
	const char* Title;
	const char* Desc;
};

static const std::unordered_map<uint8_t, FHuCInstrInfo> g_HuCOpcodes =
{
	// ---- Implied / single-byte (standard 6502) ----
	{ 0x00, { "BRK", "Software interrupt" }},
	{ 0x08, { "PHP", "Push processor status onto stack" }},
	{ 0x18, { "CLC", "Clear carry flag" }},
	{ 0x28, { "PLP", "Pull processor status from stack" }},
	{ 0x38, { "SEC", "Set carry flag" }},
	{ 0x40, { "RTI", "Return from interrupt" }},
	{ 0x48, { "PHA", "Push A onto stack" }},
	{ 0x58, { "CLI", "Clear interrupt disable flag" }},
	{ 0x60, { "RTS", "Return from subroutine" }},
	{ 0x68, { "PLA", "Pull A from stack" }},
	{ 0x78, { "SEI", "Set interrupt disable flag" }},
	{ 0x88, { "DEY", "Decrement Y" }},
	{ 0x98, { "TYA", "Transfer Y to A" }},
	{ 0xA8, { "TAY", "Transfer A to Y" }},
	{ 0xB8, { "CLV", "Clear overflow flag" }},
	{ 0xC8, { "INY", "Increment Y" }},
	{ 0xCA, { "DEX", "Decrement X" }},
	{ 0xD8, { "CLD", "Clear decimal flag" }},
	{ 0xE8, { "INX", "Increment X" }},
	{ 0xEA, { "NOP", "No operation" }},
	{ 0xF8, { "SED", "Set decimal flag" }},
	{ 0x8A, { "TXA", "Transfer X to A" }},
	{ 0x9A, { "TXS", "Transfer X to stack pointer" }},
	{ 0xAA, { "TAX", "Transfer A to X" }},
	{ 0xBA, { "TSX", "Transfer stack pointer to X" }},

	// ---- Implied (65C02 additions) ----
	{ 0x1A, { "INC", "Increment A" }},
	{ 0x3A, { "DEC", "Decrement A" }},
	{ 0x5A, { "PHY", "Push Y onto stack" }},
	{ 0x7A, { "PLY", "Pull Y from stack" }},
	{ 0xDA, { "PHX", "Push X onto stack" }},
	{ 0xFA, { "PLX", "Pull X from stack" }},

	// ---- Branches ----
	{ 0x10, { "BPL", "Branch if plus (N clear)" }},
	{ 0x30, { "BMI", "Branch if minus (N set)" }},
	{ 0x50, { "BVC", "Branch if overflow clear" }},
	{ 0x70, { "BVS", "Branch if overflow set" }},
	{ 0x80, { "BRA", "Branch always" }},
	{ 0x90, { "BCC", "Branch if carry clear (unsigned <)" }},
	{ 0xB0, { "BCS", "Branch if carry set (unsigned >=)" }},
	{ 0xD0, { "BNE", "Branch if not equal (Z clear)" }},
	{ 0xF0, { "BEQ", "Branch if equal (Z set)" }},

	// ---- ORA ----
	{ 0x01, { "ORA", "OR A with (zp,X)" }},
	{ 0x05, { "ORA", "OR A with zero page" }},
	{ 0x09, { "ORA", "OR A with immediate" }},
	{ 0x0D, { "ORA", "OR A with absolute" }},
	{ 0x11, { "ORA", "OR A with (zp),Y" }},
	{ 0x12, { "ORA", "OR A with (zp)" }},
	{ 0x15, { "ORA", "OR A with zp,X" }},
	{ 0x19, { "ORA", "OR A with abs,Y" }},
	{ 0x1D, { "ORA", "OR A with abs,X" }},

	// ---- AND ----
	{ 0x21, { "AND", "AND A with (zp,X)" }},
	{ 0x25, { "AND", "AND A with zero page" }},
	{ 0x29, { "AND", "AND A with immediate" }},
	{ 0x2D, { "AND", "AND A with absolute" }},
	{ 0x31, { "AND", "AND A with (zp),Y" }},
	{ 0x32, { "AND", "AND A with (zp)" }},
	{ 0x35, { "AND", "AND A with zp,X" }},
	{ 0x39, { "AND", "AND A with abs,Y" }},
	{ 0x3D, { "AND", "AND A with abs,X" }},

	// ---- EOR ----
	{ 0x41, { "EOR", "Exclusive OR A with (zp,X)" }},
	{ 0x45, { "EOR", "Exclusive OR A with zero page" }},
	{ 0x49, { "EOR", "Exclusive OR A with immediate" }},
	{ 0x4D, { "EOR", "Exclusive OR A with absolute" }},
	{ 0x51, { "EOR", "Exclusive OR A with (zp),Y" }},
	{ 0x52, { "EOR", "Exclusive OR A with (zp)" }},
	{ 0x55, { "EOR", "Exclusive OR A with zp,X" }},
	{ 0x59, { "EOR", "Exclusive OR A with abs,Y" }},
	{ 0x5D, { "EOR", "Exclusive OR A with abs,X" }},

	// ---- ADC ----
	{ 0x61, { "ADC", "Add (zp,X) to A with carry" }},
	{ 0x65, { "ADC", "Add zero page to A with carry" }},
	{ 0x69, { "ADC", "Add immediate to A with carry" }},
	{ 0x6D, { "ADC", "Add absolute to A with carry" }},
	{ 0x71, { "ADC", "Add (zp),Y to A with carry" }},
	{ 0x72, { "ADC", "Add (zp) to A with carry" }},
	{ 0x75, { "ADC", "Add zp,X to A with carry" }},
	{ 0x79, { "ADC", "Add abs,Y to A with carry" }},
	{ 0x7D, { "ADC", "Add abs,X to A with carry" }},

	// ---- STA ----
	{ 0x81, { "STA", "Store A to (zp,X)" }},
	{ 0x85, { "STA", "Store A to zero page" }},
	{ 0x8D, { "STA", "Store A to absolute" }},
	{ 0x91, { "STA", "Store A to (zp),Y" }},
	{ 0x92, { "STA", "Store A to (zp)" }},
	{ 0x95, { "STA", "Store A to zp,X" }},
	{ 0x99, { "STA", "Store A to abs,Y" }},
	{ 0x9D, { "STA", "Store A to abs,X" }},

	// ---- LDA ----
	{ 0xA1, { "LDA", "Load A from (zp,X)" }},
	{ 0xA5, { "LDA", "Load A from zero page" }},
	{ 0xA9, { "LDA", "Load A with immediate" }},
	{ 0xAD, { "LDA", "Load A from absolute" }},
	{ 0xB1, { "LDA", "Load A from (zp),Y" }},
	{ 0xB2, { "LDA", "Load A from (zp)" }},
	{ 0xB5, { "LDA", "Load A from zp,X" }},
	{ 0xB9, { "LDA", "Load A from abs,Y" }},
	{ 0xBD, { "LDA", "Load A from abs,X" }},

	// ---- CMP ----
	{ 0xC1, { "CMP", "Compare A with (zp,X)" }},
	{ 0xC5, { "CMP", "Compare A with zero page" }},
	{ 0xC9, { "CMP", "Compare A with immediate" }},
	{ 0xCD, { "CMP", "Compare A with absolute" }},
	{ 0xD1, { "CMP", "Compare A with (zp),Y" }},
	{ 0xD2, { "CMP", "Compare A with (zp)" }},
	{ 0xD5, { "CMP", "Compare A with zp,X" }},
	{ 0xD9, { "CMP", "Compare A with abs,Y" }},
	{ 0xDD, { "CMP", "Compare A with abs,X" }},

	// ---- SBC ----
	{ 0xE1, { "SBC", "Subtract (zp,X) from A with borrow" }},
	{ 0xE5, { "SBC", "Subtract zero page from A with borrow" }},
	{ 0xE9, { "SBC", "Subtract immediate from A with borrow" }},
	{ 0xED, { "SBC", "Subtract absolute from A with borrow" }},
	{ 0xF1, { "SBC", "Subtract (zp),Y from A with borrow" }},
	{ 0xF2, { "SBC", "Subtract (zp) from A with borrow" }},
	{ 0xF5, { "SBC", "Subtract zp,X from A with borrow" }},
	{ 0xF9, { "SBC", "Subtract abs,Y from A with borrow" }},
	{ 0xFD, { "SBC", "Subtract abs,X from A with borrow" }},

	// ---- ASL ----
	{ 0x06, { "ASL", "Arithmetic shift left zero page" }},
	{ 0x0A, { "ASL", "Arithmetic shift left A" }},
	{ 0x0E, { "ASL", "Arithmetic shift left absolute" }},
	{ 0x16, { "ASL", "Arithmetic shift left zp,X" }},
	{ 0x1E, { "ASL", "Arithmetic shift left abs,X" }},

	// ---- ROL ----
	{ 0x26, { "ROL", "Rotate left zero page" }},
	{ 0x2A, { "ROL", "Rotate left A" }},
	{ 0x2E, { "ROL", "Rotate left absolute" }},
	{ 0x36, { "ROL", "Rotate left zp,X" }},
	{ 0x3E, { "ROL", "Rotate left abs,X" }},

	// ---- LSR ----
	{ 0x46, { "LSR", "Logical shift right zero page" }},
	{ 0x4A, { "LSR", "Logical shift right A" }},
	{ 0x4E, { "LSR", "Logical shift right absolute" }},
	{ 0x56, { "LSR", "Logical shift right zp,X" }},
	{ 0x5E, { "LSR", "Logical shift right abs,X" }},

	// ---- ROR ----
	{ 0x66, { "ROR", "Rotate right zero page" }},
	{ 0x6A, { "ROR", "Rotate right A" }},
	{ 0x6E, { "ROR", "Rotate right absolute" }},
	{ 0x76, { "ROR", "Rotate right zp,X" }},
	{ 0x7E, { "ROR", "Rotate right abs,X" }},

	// ---- STX / LDX ----
	{ 0x86, { "STX", "Store X to zero page" }},
	{ 0x8E, { "STX", "Store X to absolute" }},
	{ 0x96, { "STX", "Store X to zp,Y" }},
	{ 0xA2, { "LDX", "Load X with immediate" }},
	{ 0xA6, { "LDX", "Load X from zero page" }},
	{ 0xAE, { "LDX", "Load X from absolute" }},
	{ 0xB6, { "LDX", "Load X from zp,Y" }},
	{ 0xBE, { "LDX", "Load X from abs,Y" }},

	// ---- STY / LDY ----
	{ 0x84, { "STY", "Store Y to zero page" }},
	{ 0x8C, { "STY", "Store Y to absolute" }},
	{ 0x94, { "STY", "Store Y to zp,X" }},
	{ 0xA0, { "LDY", "Load Y with immediate" }},
	{ 0xA4, { "LDY", "Load Y from zero page" }},
	{ 0xAC, { "LDY", "Load Y from absolute" }},
	{ 0xB4, { "LDY", "Load Y from zp,X" }},
	{ 0xBC, { "LDY", "Load Y from abs,X" }},

	// ---- DEC / INC ----
	{ 0xC6, { "DEC", "Decrement zero page" }},
	{ 0xCE, { "DEC", "Decrement absolute" }},
	{ 0xD6, { "DEC", "Decrement zp,X" }},
	{ 0xDE, { "DEC", "Decrement abs,X" }},
	{ 0xE6, { "INC", "Increment zero page" }},
	{ 0xEE, { "INC", "Increment absolute" }},
	{ 0xF6, { "INC", "Increment zp,X" }},
	{ 0xFE, { "INC", "Increment abs,X" }},

	// ---- CPX / CPY ----
	{ 0xE0, { "CPX", "Compare X with immediate" }},
	{ 0xE4, { "CPX", "Compare X with zero page" }},
	{ 0xEC, { "CPX", "Compare X with absolute" }},
	{ 0xC0, { "CPY", "Compare Y with immediate" }},
	{ 0xC4, { "CPY", "Compare Y with zero page" }},
	{ 0xCC, { "CPY", "Compare Y with absolute" }},

	// ---- BIT ----
	{ 0x24, { "BIT (Bit Test)", "Perform AND between memory and A. Result is discarded." }}, // zp
	{ 0x2C, { "BIT (Bit Test)", "Perform AND between memory and A. Result is discarded." }}, // abs
	{ 0x34, { "BIT (Bit Test)", "Perform AND between memory and A. Result is discarded." }}, // zp,X
	{ 0x3C, { "BIT (Bit Test)", "Perform AND between memory and A. Result is discarded." }}, // abs,x
	{ 0x89, { "BIT (Bit Test)", "Perform AND between immediate value and A. Result is discarded." }}, // imm

	// ---- JMP / JSR ----
	{ 0x20, { "JSR", "Jump to subroutine" }},
	{ 0x44, { "BSR", "Branch to subroutine (relative)" }},
	{ 0x4C, { "JMP", "Jump to absolute address" }},
	{ 0x6C, { "JMP", "Jump to indirect address" }},
	{ 0x7C, { "JMP", "Jump to (abs,X) address" }},

	// ---- STZ / TSB / TRB (65C02) ----
	{ 0x64, { "STZ", "Store zero to zero page" }},
	{ 0x74, { "STZ", "Store zero to zp,X" }},
	{ 0x9C, { "STZ", "Store zero to absolute" }},
	{ 0x9E, { "STZ", "Store zero to abs,X" }},
	{ 0x04, { "TSB", "Test and set bits in zero page" }},
	{ 0x0C, { "TSB", "Test and set bits in absolute" }},
	{ 0x14, { "TRB", "Test and reset bits in zero page" }},
	{ 0x1C, { "TRB", "Test and reset bits in absolute" }},

	// ---- HuC6280: register swap ----
	{ 0x02, { "SXY", "Swap X and Y" }},
	{ 0x22, { "SAX", "Swap A and X" }},
	{ 0x42, { "SAY", "Swap A and Y" }},

	// ---- HuC6280: register clear ----
	{ 0x62, { "CLA", "Clear A" }},
	{ 0x82, { "CLX", "Clear X" }},
	{ 0xC2, { "CLY", "Clear Y" }},

	// ---- HuC6280: speed / T flag ----
	{ 0x54, { "CSL", "CPU speed low (1.79 MHz)" }},
	{ 0xD4, { "CSH", "CPU speed high (7.16 MHz)" }},
	{ 0xF4, { "SET", "Set T flag" }},

	// ---- HuC6280: MPR (memory page register / bank mapping) ----
	{ 0x43, { "TMA", "Transfer MPR to A" }},
	{ 0x53, { "TAM", "Transfer A to MPR" }},

	// ---- HuC6280: VDC I/O ----
	{ 0x03, { "ST0", "Store to VDC address register (ST0)" }},
	{ 0x13, { "ST1", "Store to VDC data register low (ST1)" }},
	{ 0x23, { "ST2", "Store to VDC data register high (ST2)" }},

	// ---- HuC6280: block transfer ----
	{ 0x73, { "TII", "Block transfer: source++, dest++" }},
	{ 0xC3, { "TDD", "Block transfer: source--, dest--" }},
	{ 0xD3, { "TIN", "Block transfer: source++, dest fixed" }},
	{ 0xF3, { "TAI", "Block transfer: alternating source" }},
	{ 0xE3, { "TIA", "Block transfer: alternating dest" }},

	// ---- HuC6280: TST ----
	{ 0x83, { "TST", "Test bits: imm & zp" }},
	{ 0x93, { "TST", "Test bits: imm & zp,X" }},
	{ 0xA3, { "TST", "Test bits: imm & abs" }},
	{ 0xB3, { "TST", "Test bits: imm & abs,X" }},

	// ---- HuC6280: RMB (reset memory bit) ----
	{ 0x07, { "RMB", "Reset memory bit 0" }},
	{ 0x17, { "RMB", "Reset memory bit 1" }},
	{ 0x27, { "RMB", "Reset memory bit 2" }},
	{ 0x37, { "RMB", "Reset memory bit 3" }},
	{ 0x47, { "RMB", "Reset memory bit 4" }},
	{ 0x57, { "RMB", "Reset memory bit 5" }},
	{ 0x67, { "RMB", "Reset memory bit 6" }},
	{ 0x77, { "RMB", "Reset memory bit 7" }},

	// ---- HuC6280: SMB (set memory bit) ----
	{ 0x87, { "SMB", "Set memory bit 0" }},
	{ 0x97, { "SMB", "Set memory bit 1" }},
	{ 0xA7, { "SMB", "Set memory bit 2" }},
	{ 0xB7, { "SMB", "Set memory bit 3" }},
	{ 0xC7, { "SMB", "Set memory bit 4" }},
	{ 0xD7, { "SMB", "Set memory bit 5" }},
	{ 0xE7, { "SMB", "Set memory bit 6" }},
	{ 0xF7, { "SMB", "Set memory bit 7" }},

	// ---- HuC6280: BBR (branch on bit reset) ----
	{ 0x0F, { "BBR", "Branch if bit 0 of zp is reset" }},
	{ 0x1F, { "BBR", "Branch if bit 1 of zp is reset" }},
	{ 0x2F, { "BBR", "Branch if bit 2 of zp is reset" }},
	{ 0x3F, { "BBR", "Branch if bit 3 of zp is reset" }},
	{ 0x4F, { "BBR", "Branch if bit 4 of zp is reset" }},
	{ 0x5F, { "BBR", "Branch if bit 5 of zp is reset" }},
	{ 0x6F, { "BBR", "Branch if bit 6 of zp is reset" }},
	{ 0x7F, { "BBR", "Branch if bit 7 of zp is reset" }},

	// ---- HuC6280: BBS (branch on bit set) ----
	{ 0x8F, { "BBS", "Branch if bit 0 of zp is set" }},
	{ 0x9F, { "BBS", "Branch if bit 1 of zp is set" }},
	{ 0xAF, { "BBS", "Branch if bit 2 of zp is set" }},
	{ 0xBF, { "BBS", "Branch if bit 3 of zp is set" }},
	{ 0xCF, { "BBS", "Branch if bit 4 of zp is set" }},
	{ 0xDF, { "BBS", "Branch if bit 5 of zp is set" }},
	{ 0xEF, { "BBS", "Branch if bit 6 of zp is set" }},
	{ 0xFF, { "BBS", "Branch if bit 7 of zp is set" }},
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
		ImGui::TextColored(col, "%s:$%02X  bin:%s", name, cur, NumStr(cur, ENumberDisplayMode::Binary));
	else
		ImGui::TextColored(col, "%s:%s", name, NumStr(cur));
}

static void DrawRegTable(const FTooltipInfo& info,
                         const ICPUEmulator6502* pCPU,
                         const F6502DisplayRegisters& old)
{
	if (info.numRegs == 0)
		return;

	static ImGuiTableFlags tflags = ImGuiTableFlags_Borders | ImGuiTableFlags_RowBg | ImGuiTableFlags_SizingStretchSame;
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

static void DrawMemOperand(FCodeAnalysisState& state, FCodeAnalysisViewState& viewState, uint16_t addr)
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

static void DrawTamTma(FCodeAnalysisState& state, uint16_t pc, uint8_t op, const ICPUEmulator6502* pCPU)
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
		ImGui::Text("Src=$%04X Dst=$%04X Len=%d bytes", src, dst, len);
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

	// Title + description
	const auto it = g_HuCOpcodes.find(op);
	if (it != g_HuCOpcodes.end())
	{
		const FHuCInstrInfo& info = it->second;
		const ImVec2 pos = ImGui::GetCursorScreenPos();
		const ImVec2 itemRectMax = ImGui::GetItemRectSize();
		ImGui::GetWindowDrawList()->AddRectFilled(
			ImVec2(pos.x - 20, pos.y - 4),
			ImVec2(pos.x + itemRectMax.x, pos.y + ImGui::GetTextLineHeightWithSpacing()),
			IM_COL32(64, 64, 64, 255));

		ImGui::PushStyleColor(ImGuiCol_Text, IM_COL32(255, 255, 255, 255));
		ImGui::TextUnformatted(info.Title);
		ImGui::PopStyleColor();
		ImGui::Separator();
		ImGui::TextUnformatted(info.Desc);
		const char* addressMode = GetAddressModeStringHuC6280(op);
		if (addressMode && addressMode[0] != '\0')
			ImGui::Text("Mode: %s", addressMode);
	}
	else
	{
		ImGui::Text("Unknown opcode $%02X", op);
		ImGui::EndTooltip();
		return;
	}

	// Special-case instructions handled separately
	/*if (op == 0x43 || op == 0x53)
	{
		ImGui::Separator();
		DrawTamTma(state, addr, op, pCPU);
		ImGui::EndTooltip();
		return;
	}*/
	if (DrawBlockTransfer(state, addr, op))  { ImGui::EndTooltip(); return; }
	//if (DrawTstOperands(state, addr, op))    { ImGui::EndTooltip(); return; }
	//if (DrawBitBranchOperands(state, addr, op)) { ImGui::EndTooltip(); return; }

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
	/*{
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
	}*/

	// Flags
	//if (info.bShowFlags)
	//	DrawFlagsRow(pCPU->GetP(), old.P);

	// Memory is destination: snippet first, registers below
	/*if (info.bHasMem && info.bMemIsDest)
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
	}*/

	ImGui::EndTooltip();
}
