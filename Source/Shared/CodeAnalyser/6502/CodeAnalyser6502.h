#pragma once
#include <cstdint>

class ICPUInterface;
class FCodeAnalysisState;

bool CheckPointerIndirectionInstruction6502(const FCodeAnalysisState& state, uint16_t pc, uint16_t* out_addr);
bool CheckPointerRefInstruction6502(const FCodeAnalysisState& state, uint16_t pc, uint16_t* out_addr);
bool CheckJumpInstruction6502(const FCodeAnalysisState& state, uint16_t pc, uint16_t* out_addr);
bool CheckCallInstruction6502(const FCodeAnalysisState& state, uint16_t pc);
bool CheckStopInstruction6502(const FCodeAnalysisState& state, uint16_t pc);
bool RegisterCodeExecuted6502(FCodeAnalysisState& state, uint16_t pc, uint16_t oldpc);