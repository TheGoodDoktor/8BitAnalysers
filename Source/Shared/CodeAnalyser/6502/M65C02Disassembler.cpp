#include "M65C02Disassembler.h"

#include "../CodeAnalyser.h"
#include "../Disassembler.h"

#include <assert.h>
#include <string.h>

// This is a modified version of the Chips 6502 Disassembler by Andre Weissflog
// I have kept the original comments from the header below
// As far as I know I've followed the license but I'm happy to make changes if necessary

// This was a single header lib but I've pasted it into a C++ file and made my own header
// I've modified the original because I wanted to modify the text output

#define CHIPS_ASSERT(c) assert(c)


/*#
    # m6502dasm.h

    A stateless MOS 6502 disassembler that doesn't call any CRT functions.

    Do this:
    ~~~C
    #define CHIPS_UTIL_IMPL
    ~~~
    before you include this file in *one* C or C++ file to create the
    implementation.

    Optionally provide the following macros with your own implementation

    ~~~C
    CHIPS_ASSERT(c)
    ~~~
        your own assert macro (default: assert(c))

    ## Usage

    There's only one function to call which consumes a stream of instruction bytes
    and produces a stream of ASCII characters for exactly one instruction:

    ~~~C
    uint16_t m6502dasm_op(uint16_t pc, m6502dasm_input_t in_cb, m6502dasm_output_t out_cb, void* user_data)
    ~~~

    pc      - the current 16-bit program counter, this is used to compute
              absolute target addresses for relative jumps
    in_cb   - this function is called when the disassembler needs the next
              instruction byte: uint8_t in_cb(void* user_data)
    out_cb  - (optional) this function is called when the disassembler produces a single
              ASCII character: void out_cb(char c, void* user_data)
    user_data   - a user-provided context pointer for the callbacks

    m6502dasm_op() returns the new program counter (pc), this should be
    used as input arg when calling m6502dasm_op() for the next instruction.

    NOTE that the output callback will never be called with a null character,
    you need to terminate the resulting string yourself if needed.

    Undocumented instructions are supported and are marked with a '*'.

    ## zlib/libpng license

    Copyright (c) 2018 Andre Weissflog
    This software is provided 'as-is', without any express or implied warranty.
    In no event will the authors be held liable for any damages arising from the
    use of this software.
    Permission is granted to anyone to use this software for any purpose,
    including commercial applications, and to alter it and redistribute it
    freely, subject to the following restrictions:
        1. The origin of this software must not be misrepresented; you must not
        claim that you wrote the original software. If you use this software in a
        product, an acknowledgment in the product documentation would be
        appreciated but is not required.
        2. Altered source versions must be plainly marked as such, and must not
        be misrepresented as being the original software.
        3. This notice may not be removed or altered from any source
        distribution.
#*/
#include <stdint.h>
#include <stdbool.h>

//typedef void (*m6502dasm_output_t)(char c, void* user_data);

/* fetch unsigned 8-bit value and track pc */
#define _FETCH_U8(v) v=in_cb(user_data);pc++;

/* fetch signed 8-bit value and track pc */
#define _FETCH_I8(v) v=(int8_t)in_cb(user_data);pc++;

/* fetch unsigned 16-bit value and track pc */
#define _FETCH_U16(v) v=in_cb(user_data);v|=in_cb(user_data)<<8;pc+=2;

/* output character */
#define _CHR(c) if (out_cb) { out_cb(c,user_data); }

/* output string */
#define _STR(s) _m6502dasm_str(s,out_cb,user_data);

/* output offset as signed 8-bit string (decimal) */
#define _STR_D8(d8) DasmOutputD8((int8_t)(d8),out_cb,user_data);

/* output number as unsigned 8-bit string (hex) */
#define _STR_U8(u8) DasmOutputU8((uint8_t)(u8),out_cb,user_data);

/* output number number as unsigned 16-bit string (hex) */
#define _STR_U16(u16) DasmOutputU16((uint16_t)(u16),out_cb,user_data);

/* addressing modes */
#define A____    (0)     /* no addressing mode */
#define A_IMM    (1)     /* # */
#define A_ZER    (2)     /* zp */
#define A_ZPX    (3)     /* zp,X */
#define A_ZPY    (4)     /* zp,Y */
#define A_ABS    (5)     /* abs */
#define A_ABX    (6)     /* abs,X */
#define A_ABY    (7)     /* abs,Y */
#define A_IDX    (8)     /* (zp,X) */
#define A_IDY    (9)     /*(zp),Y */
#define A_JMP    (10)    /* special JMP abs */
#define A_JSR    (11)    /* special JSR abs */
#define A_BRA    (12)    /* special relative branch */
#define A_INV    (13)    /* this is an invalid instruction */
#define A_ZPI    (14)	 /* (zp) new 65C02 addressing mode

/* opcode descriptions */
static uint8_t _m6502dasm_ops[4][8][8] = {
    /* cc = 00 */
    {
        //---  BIT   JMP   JMP() STY   LDY   CPY   CPX
        {A____,A_JSR,A____,A____,A_IMM,A_IMM,A_IMM,A_IMM},
        {A_ZER,A_ZER,A_ZER,A_ZER,A_ZER,A_ZER,A_ZER,A_ZER},
        {A____,A____,A____,A____,A____,A____,A____,A____},
        {A_ABS,A_ABS,A_JMP,A_JMP,A_ABS,A_ABS,A_ABS,A_ABS},
        {A_BRA,A_BRA,A_BRA,A_BRA,A_BRA,A_BRA,A_BRA,A_BRA},  /* relative branches */
        {A_ZPX,A_ZPX,A_ZPX,A_ZPX,A_ZPX,A_ZPX,A_ZPX,A_ZPX},
        {A____,A____,A____,A____,A____,A____,A____,A____},
        {A_ABX,A_ABX,A_ABS,A_ABS,A_INV,A_ABX,A_ABX,A_ABX}
    },
    /* cc = 01 */
    {
        //ORA  AND   EOR   ADC   STA   LDA   CMP   SBC
        {A_IDX,A_IDX,A_IDX,A_IDX,A_IDX,A_IDX,A_IDX,A_IDX},
        {A_ZER,A_ZER,A_ZER,A_ZER,A_ZER,A_ZER,A_ZER,A_ZER},
        {A_IMM,A_IMM,A_IMM,A_IMM,A_IMM,A_IMM,A_IMM,A_IMM},
        {A_ABS,A_ABS,A_ABS,A_ABS,A_ABS,A_ABS,A_ABS,A_ABS},
        {A_IDY,A_IDY,A_IDY,A_IDY,A_IDY,A_IDY,A_IDY,A_IDY},
        {A_ZPX,A_ZPX,A_ZPX,A_ZPX,A_ZPX,A_ZPX,A_ZPX,A_ZPX},
        {A_ABY,A_ABY,A_ABY,A_ABY,A_ABY,A_ABY,A_ABY,A_ABY},
        {A_ABX,A_ABX,A_ABX,A_ABX,A_ABX,A_ABX,A_ABX,A_ABX},
    },
    /* cc = 02 */
    {
        //ASL  ROL   LSR   ROR   STX   LDX   DEC   INC
        {A_INV,A_INV,A_INV,A_INV,A_IMM,A_IMM,A_IMM,A_IMM},	// 0
        {A_ZER,A_ZER,A_ZER,A_ZER,A_ZER,A_ZER,A_ZER,A_ZER},	// 1
        {A____,A____,A____,A____,A____,A____,A____,A____},	// 2
        {A_ABS,A_ABS,A_ABS,A_ABS,A_ABS,A_ABS,A_ABS,A_ABS},	// 3
        {A_INV,A_INV,A_INV,A_INV,A_INV,A_ZPI,A_INV,A_INV},	// 4
        {A_ZPX,A_ZPX,A_ZPX,A_ZPX,A_ZPY,A_ZPY,A_ZPX,A_ZPX},	// 5
        {A____,A____,A____,A____,A____,A____,A____,A____},	// 6
        {A_ABX,A_ABX,A_ABX,A_ABX,A_INV,A_ABY,A_ABX,A_ABX},	// 7
    },
    /* cc = 03 */
    {
        {A_IDX,A_IDX,A_IDX,A_IDX,A_IDX,A_IDX,A_IDX,A_IDX},
        {A_ZER,A_ZER,A_ZER,A_ZER,A_ZER,A_ZER,A_ZER,A_ZER},
        {A_INV,A_INV,A_INV,A_INV,A_INV,A_INV,A_INV,A_IMM},
        {A_ABS,A_ABS,A_ABS,A_ABS,A_ABS,A_ABS,A_ABS,A_ABS},
        {A_IDY,A_IDY,A_IDY,A_IDY,A_INV,A_IDY,A_IDY,A_IDY},
        {A_ZPX,A_ZPX,A_ZPX,A_ZPX,A_ZPY,A_ZPY,A_ZPX,A_ZPX},
        {A_ABY,A_ABY,A_ABY,A_ABY,A_INV,A_INV,A_ABY,A_ABY},
        {A_ABX,A_ABX,A_ABX,A_ABX,A_INV,A_ABY,A_ABX,A_ABX}
    } };

static const char* _m6502dasm_hex = "0123456789ABCDEF";

/* helper function to output string */
static void _m6502dasm_str(const char* str, dasm_output_t out_cb, void* user_data) {
    if (out_cb) {
        char c;
        while (0 != (c = *str++)) {
            out_cb(c, user_data);
        }
    }
}

/* helper function to output an unsigned 8-bit value as hex string */
static void _m6502dasm_u8(uint8_t val, dasm_output_t out_cb, void* user_data) {
    if (out_cb) {
        out_cb('$', user_data);
        for (int i = 1; i >= 0; i--) {
            out_cb(_m6502dasm_hex[(val >> (i * 4)) & 0xF], user_data);
        }
    }
}

/* helper function to output an unsigned 16-bit value as hex string */
static void _m6502dasm_u16(uint16_t val, dasm_output_t out_cb, void* user_data) {
    if (out_cb) {
        out_cb('$', user_data);
        for (int i = 3; i >= 0; i--) {
            out_cb(_m6502dasm_hex[(val >> (i * 4)) & 0xF], user_data);
        }
    }
}

/* main disassembler function */
uint16_t m65C02dasm_op(uint16_t pc, dasm_input_t in_cb, dasm_output_t out_cb, void* user_data) {
    CHIPS_ASSERT(in_cb);
    uint8_t op;
    _FETCH_U8(op);
    uint8_t cc = op & 0x03;
    uint8_t bbb = (op >> 2) & 0x07;
    uint8_t aaa = (op >> 5) & 0x07;

    /* opcode name */
    const char* n = "???";
    bool indirect = false;
    switch (cc) {
    case 0:
        switch (aaa) {
        case 0:
            switch (bbb) {
            case 0:  n = "BRK"; break;
            case 2:  n = "PHP"; break;
            case 4:  n = "BPL"; break;
            case 6:  n = "CLC"; break;
            default: n = "*NOP"; break;
            }
            break;
        case 1:
            switch (bbb) {
            case 0:  n = "JSR"; break;
            case 2:  n = "PLP"; break;
            case 4:  n = "BMI"; break;
            case 5:  n = "*NOP"; break;
            case 6:  n = "SEC"; break;
            case 7:  n = "*NOP"; break;
            default: n = "BIT"; break;
            }
            break;
        case 2:
            switch (bbb) {
            case 0:  n = "RTI"; break;
            case 2:  n = "PHA"; break;
            case 3:  n = "JMP"; break;
            case 4:  n = "BVC"; break;
            case 6:  n = "CLI"; break;
            default: n = "*NOP"; break;
            }
            break;
        case 3:
            switch (bbb) {
            case 0:  n = "RTS"; break;
            case 2:  n = "PLA"; break;
            case 3:  n = "JMP"; indirect = true; break;  /* jmp () */
            case 4:  n = "BVS"; break;
            case 6:  n = "SEI"; break;
            default: n = "*NOP"; break;
            }
            break;
        case 4:
            switch (bbb) {
            case 0:  n = "*NOP"; break;
            case 2:  n = "DEY"; break;
            case 4:  n = "BCC"; break;
            case 6:  n = "TYA"; break;
            default: n = "STY"; break;
            }
            break;
        case 5:
            switch (bbb) {
            case 2:  n = "TAY"; break;
            case 4:  n = "BCS"; break;
            case 6:  n = "CLV"; break;
            default: n = "LDY"; break;
            }
            break;
        case 6:
            switch (bbb) {
            case 2:  n = "INY"; break;
            case 4:  n = "BNE"; break;
            case 5:  n = "*NOP"; break;
            case 6:  n = "CLD"; break;
            case 7:  n = "*NOP"; break;
            default: n = "CPY"; break;
            }
            break;
        case 7:
            switch (bbb) {
            case 2:  n = "INX"; break;
            case 4:  n = "BEQ"; break;
            case 5:  n = "*NOP"; break;
            case 6:  n = "SED"; break;
            case 7:  n = "*NOP"; break;
            default: n = "CPX"; break;
            }
            break;
        }
        break;

    case 1:
        switch (aaa) {
        case 0: n = "ORA"; break;
        case 1: n = "AND"; break; /* AND A */
        case 2: n = "EOR"; break;
        case 3: n = "ADC"; break;
        case 4:
            switch (bbb) {
            case 2:  n = "*NOP"; break;
            default: n = "STA"; break;
            }
            break;
        case 5: n = "LDA"; break;
        case 6: n = "CMP"; break;
        case 7: n = "SBC"; break;
        }
        break;

    case 2:
        switch (aaa) {
        case 0:
            switch (bbb) {
            case 6:  n = "*NOP"; break;
            default: n = "ASL"; break;
            }
            break;
        case 1:
            switch (bbb) {
            case 6:  n = "*NOP"; break;
            default: n = "ROL"; break;
            }
            break;
        case 2:
            switch (bbb) {
            case 6:  n = "PHY"; break;	// 65C02
            default: n = "LSR"; break;
            }
            break;
        case 3:
            switch (bbb) {
            case 6:  n = "PLY"; break;	// 65C02
            default: n = "ROR"; break;
            }
            break;
        case 4:
            switch (bbb) {
            case 0:  n = "*NOP"; break;
            case 2:  n = "TXA"; break;
            case 6:  n = "TXS"; break;
            default: n = "STX"; break;
            }
            break;
        case 5:
            switch (bbb) {
            case 2:  n = "TAX"; break;
			case 4:  n = "LDA"; break;
            case 6:  n = "TSX"; break;
            default: n = "LDX"; break;
            }
            break;
        case 6:
            switch (bbb) {
            case 0:  n = "*NOP"; break;
            case 2:  n = "DEX"; break;
            case 6:  n = "PHX"; break;	// 65C02
            default: n = "DEC"; break;
            }
            break;
        case 7:
            switch (bbb) {
            case 0:  n = "*NOP"; break;
            case 2:  n = "NOP"; break;
            case 6:  n = "PLX"; break;	// 65C02
            default: n = "INC"; break;
            }
            break;
        }
        break;

    case 3:
        switch (aaa) {
        case 0: n = "*SLO"; break;
        case 1: n = "*RLA"; break;
        case 2: n = "*SRE"; break;
        case 3: n = "*RRA"; break;
        case 4: n = "*SAX"; break;
        case 5: n = "*LAX"; break;
        case 6: n = "*DCP"; break;
        case 7:
            switch (bbb) {
            case 2:  n = "*SBC"; break;
            default: n = "*ISB"; break;
            }
            break;
        }
    }
    _STR(n);

    uint8_t u8; int8_t i8; uint16_t u16;
    switch (_m6502dasm_ops[cc][bbb][aaa]) {
    case A_IMM:
        _CHR(' '); _FETCH_U8(u8); _CHR('\\'); _CHR('#'); _STR_U8(u8);
        break;
    case A_ZER:
        _CHR(' '); _FETCH_U8(u8); _STR_U8(u8);
        break;
    case A_ZPX:
        _CHR(' '); _FETCH_U8(u8); _STR_U8(u8); _STR(",X");
        break;
    case A_ZPY:
        _CHR(' '); _FETCH_U8(u8); _STR_U8(u8); _STR(",Y");
        break;
    case A_ABS:
    case A_JSR:
    case A_JMP:
        _CHR(' '); _FETCH_U16(u16);
        if (indirect) {
            _CHR('('); _STR_U16(u16); _CHR(')');
        }
        else {
            _STR_U16(u16);
        }
        break;
    case A_ABX:
        _CHR(' '); _FETCH_U16(u16); _STR_U16(u16); _STR(",X");
        break;
    case A_ABY:
        _CHR(' '); _FETCH_U16(u16); _STR_U16(u16); _STR(",Y");
        break;
    case A_IDX:
        _CHR(' '); _FETCH_U8(u8); _CHR('('); _STR_U8(u8); _STR(",X)");
        break;
    case A_IDY:
        _CHR(' '); _FETCH_U8(u8); _CHR('('); _STR_U8(u8); _STR("),Y");
        break;
    case A_BRA: /* relative branch, compute target address */
        _CHR(' '); _FETCH_I8(i8); _STR_U16(pc + i8);
        break;
	case A_ZPI:
		_CHR(' '); _FETCH_U8(u8); _CHR('('); _STR_U8(u8); _STR(")");
		break;
    }
    return pc;
}

// END CHIPS


// These functions were added to support the 8bit Analysers

// number output abstraction


#if 0
static IDasmNumberOutput* g_pNumberOutputObj = nullptr;
static IDasmNumberOutput* GetNumberOutput()
{
	return g_pNumberOutputObj;
}

static void SetNumberOutput(IDasmNumberOutput* pNumberOutputObj)
{
	g_pNumberOutputObj = pNumberOutputObj;
}

// output an unsigned 8-bit value as hex string 
static void DasmOutputU8(uint8_t val, dasm_output_t out_cb, void* user_data)
{
	IDasmNumberOutput* pNumberOutput = GetNumberOutput();
	if (pNumberOutput)
		pNumberOutput->OutputU8(val, out_cb);

}

// output an unsigned 16-bit value as hex string 
static void DasmOutputU16(uint16_t val, dasm_output_t out_cb, void* user_data)
{
	IDasmNumberOutput* pNumberOutput = GetNumberOutput();
	if (pNumberOutput)
		pNumberOutput->OutputU16(val, out_cb);
}

// output a signed 8-bit offset as hex string 
static void DasmOutputD8(int8_t val, dasm_output_t out_cb, void* user_data)
{
	IDasmNumberOutput* pNumberOutput = GetNumberOutput();
	if (pNumberOutput)
		pNumberOutput->OutputD8(val, out_cb);
}
// helper functions



// disassembler callback to fetch the next instruction byte 
static uint8_t AnalysisDasmInputCB(void* pUserData)
{
	FAnalysisDasmState* pDasmState = (FAnalysisDasmState*)pUserData;

	return pDasmState->CodeAnalysisState->ReadByte(pDasmState->CurrentAddress++);
}

// disassembler callback to output a character 
static void AnalysisOutputCB(char c, void* pUserData)
{
	FAnalysisDasmState* pDasmState = (FAnalysisDasmState*)pUserData;

	// add character to string
	pDasmState->Text += c;
}
#endif

// Helper function to generate the disassembly for a code info item
uint16_t M65C02DisassembleCodeInfoItem(uint16_t pc, FCodeAnalysisState& state, FCodeInfo* pCodeInfo)
{
	FAnalysisDasmState dasmState;
	dasmState.pCodeInfoItem = pCodeInfo;
	dasmState.CodeAnalysisState = &state;
	dasmState.CurrentAddress = pc;
	SetNumberOutput(&dasmState);
	const uint16_t newPC = m65C02dasm_op(pc, AnalysisDasmInputCB, AnalysisOutputCB, &dasmState);
	pCodeInfo->Text = dasmState.Text;
	SetNumberOutput(nullptr);
	return newPC;
}


struct FStepDasmData
{
	std::vector<uint8_t> Data;
	FCodeAnalysisState* pCodeAnalysis = nullptr;
	uint16_t    PC = 0;
};

static uint8_t StepOverDasmInCB(void* userData)
{
	FStepDasmData* pDasmData = (FStepDasmData*)userData;

	// Get Opcode bytes
	uint8_t opcodeByte = pDasmData->pCodeAnalysis->ReadByte(pDasmData->PC++);
	pDasmData->Data.push_back(opcodeByte);
	return opcodeByte;
}

static void StepOverDasmOutCB(char c, void* userData)
{
	FStepDasmData* pDasmData = (FStepDasmData*)userData;
	// do we need to do anything here?
}

uint16_t M65C02DisassembleGetNextPC(uint16_t pc, FCodeAnalysisState& state, std::vector<uint8_t>& opcodes)
{
	FStepDasmData dasmData;
	dasmData.PC = pc;
	dasmData.pCodeAnalysis = &state;
	const uint16_t nextPC = m65C02dasm_op(pc, StepOverDasmInCB, nullptr, &dasmData);
	opcodes = dasmData.Data;
	return nextPC;
}
#if 0
/* disassembler callback to fetch the next instruction byte */
static uint8_t ExportDasmInputCB(void* pUserData)
{
	FExportDasmState* pDasmState = (FExportDasmState*)pUserData;

	return pDasmState->CodeAnalysisState->CPUInterface->ReadByte(pDasmState->CurrentAddress++);
}

/* disassembler callback to output a character */
static void ExportOutputCB(char c, void* pUserData)
{
	FExportDasmState* pDasmState = (FExportDasmState*)pUserData;

	// add character to string
	pDasmState->Text += c;
}
#endif
/*
std::string M6502GenerateDasmStringForAddress(FCodeAnalysisState& state, uint16_t pc, ENumberDisplayMode hexMode)
{
	FExportDasmState dasmState;
	dasmState.CodeAnalysisState = &state;
	dasmState.CurrentAddress = pc;
	dasmState.HexDisplayMode = hexMode;
	dasmState.pCodeInfoItem = state.GetCodeInfoForPhysicalAddress(pc);
	SetNumberOutput(&dasmState);
	m6502dasm_op(pc, ExportDasmInputCB, ExportOutputCB, &dasmState);
	SetNumberOutput(nullptr);

	return dasmState.Text;
}*/

bool M65C02GenerateDasmExportString(FExportDasmState& exportState)
{
	SetNumberOutput(&exportState);
	m65C02dasm_op(exportState.CurrentAddress, ExportDasmInputCB, ExportOutputCB, &exportState);
	SetNumberOutput(nullptr);
	return true;
}


