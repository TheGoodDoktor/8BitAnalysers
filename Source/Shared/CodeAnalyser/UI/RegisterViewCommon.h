#pragma once

#include <imgui.h>

class FCodeAnalysisState;

void EditByte(uint8_t* pByteData);
void EditWord(uint16_t* pWordData);
void DrawByteRegister(FCodeAnalysisState& state, uint8_t* curByte, uint8_t oldByte, const char* fmt);
void DrawWordRegister(FCodeAnalysisState& state, uint16_t* curWord, uint16_t oldWord, const char* fmt);
void DoByteRegisterTooltip(uint8_t byteValue);
void DrawFlag(FCodeAnalysisState& state, bool* curFlag, bool oldFlag);

ImVec4 GetRegNormalCol();
ImVec4 GetRegChangedCol();
