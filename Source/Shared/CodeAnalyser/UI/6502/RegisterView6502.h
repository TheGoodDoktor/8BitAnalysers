#pragma once
#include <cstdint>

class FCodeAnalysisState;

struct F6502DisplayRegisters
{
	F6502DisplayRegisters() {}
	F6502DisplayRegisters(class ICPUEmulator6502* pCPU);

	uint8_t  A = 0;
	uint8_t  X = 0;
	uint8_t  Y = 0;
	uint8_t  S = 0;
	uint8_t  P = 0;
	uint16_t PC = 0;

	bool CarryFlag            = false;
	bool ZeroFlag             = false;
	bool InterruptDisableFlag = false;
	bool DecimalModeFlag      = false;
	bool BreakFlag            = false;
	bool OverflowFlag         = false;
	bool NegativeFlag         = false;
};

void StoreRegisters_6502(FCodeAnalysisState& state);
const F6502DisplayRegisters& GetStoredRegisters_6502();
