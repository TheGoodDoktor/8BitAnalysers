#pragma once
#include <stdint.h>

#include "SkoolFile.h"

class FCodeAnalysisState;
struct FSkoolFileInfo;

bool ExportSkoolFile(FCodeAnalysisState& state, const char* pTextFileName, FSkoolFile::Base base = FSkoolFile::Base::Hexadecimal, const FSkoolFileInfo* pSkoolInfo = nullptr, uint16_t startAddr=0x4000, uint16_t endAddr=0xffff);
