#pragma once
#include <cstdint>

class ICPUInterface;
struct FCodeAnalysisState;

bool CheckPointerIndirectionInstruction6502(ICPUInterface* pCPUInterface, uint16_t pc, uint16_t* out_addr);
bool CheckPointerRefInstruction6502(ICPUInterface* pCPUInterface, uint16_t pc, uint16_t* out_addr);
bool CheckJumpInstruction6502(ICPUInterface* pCPUInterface, uint16_t pc, uint16_t* out_addr);
bool CheckCallInstruction6502(ICPUInterface* pCPUInterface, uint16_t pc);
bool CheckStopInstruction6502(ICPUInterface* pCPUInterface, uint16_t pc);
void RegisterCodeExecuted6502(FCodeAnalysisState& state, uint16_t pc, uint16_t nextpc);