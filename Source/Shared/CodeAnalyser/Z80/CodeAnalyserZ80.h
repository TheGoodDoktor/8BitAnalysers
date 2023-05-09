#pragma once
#include <cstdint>
#include "../CodeAnalysisPage.h"

class ICPUInterface;
class FCodeAnalysisState;

struct FMachineStateZ80 : FMachineState
{
	union { uint16_t AF;	struct { uint8_t A; uint8_t F; }; };
	union { uint16_t BC;	struct { uint8_t B; uint8_t C; }; };
	uint16_t	DE;
	uint16_t	HL;
	uint16_t	AF_;
	uint16_t	BC_;
	uint16_t	DE_;
	uint16_t	HL_;
	uint16_t	IX;
	uint16_t	IY;
	uint16_t	SP;
	uint16_t	PC;
	uint8_t		I;
	uint8_t		R;
	uint8_t		IM;

	static const int	kNoStackEntries = 16;	// 16 should be enough - we can always increase it
	uint16_t	Stack[kNoStackEntries];
};

bool CheckPointerIndirectionInstructionZ80(FCodeAnalysisState& state, uint16_t pc, uint16_t* out_addr);
bool CheckPointerRefInstructionZ80(FCodeAnalysisState& state, uint16_t pc, uint16_t* out_addr);
bool CheckJumpInstructionZ80(FCodeAnalysisState& state, uint16_t pc, uint16_t* out_addr);
bool CheckCallInstructionZ80(FCodeAnalysisState& state, uint16_t pc);
bool CheckStopInstructionZ80(FCodeAnalysisState& state, uint16_t pc);
bool RegisterCodeExecutedZ80(FCodeAnalysisState& state, uint16_t pc, uint16_t oldpc);

FMachineStateZ80* AllocateMachineStateZ80();
void FreeMachineStatesZ80();
void CaptureMachineStateZ80(FMachineState* pMachineState, ICPUInterface* pCPUInterface);
