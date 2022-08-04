#pragma once
#include <cstdint>

class ICPUInterface;

bool CheckPointerIndirectionInstructionZ80(ICPUInterface* pCPUInterface, uint16_t pc, uint16_t* out_addr);
bool CheckPointerRefInstructionZ80(ICPUInterface* pCPUInterface, uint16_t pc, uint16_t* out_addr);
bool CheckJumpInstructionZ80(ICPUInterface* pCPUInterface, uint16_t pc, uint16_t* out_addr);
bool CheckCallInstructionZ80(ICPUInterface* pCPUInterface, uint16_t pc);
bool CheckStopInstructionZ80(ICPUInterface* pCPUInterface, uint16_t pc);
