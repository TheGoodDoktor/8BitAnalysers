#pragma once

#include <cstddef>
#include <cinttypes>

class FSpectrumEmu;

bool LoadTAPFile(FSpectrumEmu* pEmu, const char* fName);
bool LoadTAPFromMemory(FSpectrumEmu* pEmu, const uint8_t* pData, size_t dataSize);

