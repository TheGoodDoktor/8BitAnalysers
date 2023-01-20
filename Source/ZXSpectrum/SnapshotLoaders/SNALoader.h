#pragma once

#include <cstddef>
#include <cinttypes>

class FSpectrumEmu;

bool LoadSNAFile(FSpectrumEmu* pEmu, const char* fName);
bool LoadSNAFromMemory(FSpectrumEmu* pEmu, const uint8_t* pData, size_t dataSize);
