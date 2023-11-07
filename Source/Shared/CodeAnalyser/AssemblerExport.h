#pragma once

class FCodeAnalysisState;

// TODO: we should have a bank based approach?
bool ExportAssembler(FCodeAnalysisState& state, const char* pTextFileName, uint16_t startAddr, uint16_t endAddr);
