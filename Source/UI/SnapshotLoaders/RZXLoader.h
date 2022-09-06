#pragma once

#include <stdint.h>

class FSpectrumEmu;

bool LoadRZXFile(FSpectrumEmu* pEmu, const char* fName);
bool LoadRZXFromMemory(FSpectrumEmu* pEmu, const uint8_t* pData, size_t dataSize);
