#pragma once
#include <cstdint>
#include "../CodeAnalysisPage.h"

enum class EInstructionType;

class ICPUInterface;
class FCodeAnalysisState;
struct FCodeInfo;

bool CheckPointerIndirectionInstructionHuC6280(const FCodeAnalysisState& state, uint16_t pc, uint16_t* out_addr);
bool CheckPointerRefInstructionHuC6280(const FCodeAnalysisState& state, uint16_t pc, uint16_t* out_addr);
bool CheckJumpInstructionHuC6280(const FCodeAnalysisState& state, uint16_t pc, uint16_t* out_addr);
bool CheckCallInstructionHuC6280(const FCodeAnalysisState& state, uint16_t pc);
bool CheckStopInstructionHuC6280(const FCodeAnalysisState& state, uint16_t pc);
bool RegisterCodeExecutedHuC6280(FCodeAnalysisState& state, uint16_t pc, uint16_t oldpc);
void FillCodeInfoOperandsHuC6280(FCodeAnalysisState& state, uint16_t pc, FCodeInfo* pCodeInfo);

EInstructionType GetInstructionTypeHuC6280(FCodeAnalysisState& state, FAddressRef addr);
