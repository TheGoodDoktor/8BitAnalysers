#pragma once

class FCodeAnalysisState;

// Export assembler to a file given an optional start address and end address.
// If no start or end address are specified it will export the entire 64k of RAM.
bool ExportAssembler(FCodeAnalysisState& state, const char* pTextFileName, uint16_t startAddr = 0, uint16_t endAddr=0xffff);
