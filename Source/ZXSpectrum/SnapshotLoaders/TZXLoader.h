#pragma once

#include <cstddef>
#include <cinttypes>

class FSpectrumEmu;

bool LoadTZXFile(FSpectrumEmu* pEmu, const char* fName);
bool LoadTZXFromMemory(FSpectrumEmu* pEmu, const uint8_t* pData, size_t dataSize);

