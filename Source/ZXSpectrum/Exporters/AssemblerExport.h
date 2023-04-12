#pragma once

class FCodeAnalysisState;

// Export assembler to a file given an optional start address and end address.
// If no start or end address are specified it will export from the the end of attrib memory to the top of RAM.
bool ExportAssembler(FCodeAnalysisState& state, const char* pTextFileName, uint16_t startAddr = kScreenAttrMemEnd + 1, uint16_t endAddr=0xffff);
