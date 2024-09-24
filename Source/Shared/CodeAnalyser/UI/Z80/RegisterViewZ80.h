#include <chips/z80.h>
#include "../../CodeAnalyser.h"

// structure to hold registers for display purposes
struct FZ80DisplayRegisters
{
	FZ80DisplayRegisters() {}
	FZ80DisplayRegisters(z80_t* pCPU);

	uint8_t	A = 0, F = 0, B = 0, C = 0, D = 0, E = 0, H = 0, L = 0;
	uint8_t	A2 = 0, F2 = 0, B2 = 0, C2 = 0, D2 = 0, E2 = 0, H2 = 0, L2 = 0;
	uint8_t I = 0, R = 0, IM = 0;
	uint16_t BC = 0, DE = 0, HL = 0, IX = 0, IY = 0;
	uint16_t BC2 = 0, DE2 = 0, HL2 = 0;
	uint16_t SP = 0, PC = 0;

	// Flags
	bool	CarryFlag = false;
	bool	AddSubtractFlag = false;
	bool	ParityOverflowFlag = false;
	bool	HalfCarryFlag = false;
	bool	ZeroFlag = false;
	bool	SignFlag = false;

	bool	IFF1 = false;
	bool	IFF2 = false;
};

void StoreRegisters_Z80(FCodeAnalysisState& state);
const FZ80DisplayRegisters& GetStoredRegisters_Z80(void);