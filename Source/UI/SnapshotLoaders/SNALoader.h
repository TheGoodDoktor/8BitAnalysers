#pragma once

#include <stdint.h>

class FSpectrumEmu;

bool LoadSNAFile(FSpectrumEmu* pEmu, const char* fName);
bool LoadSNAFromMemory(FSpectrumEmu* pEmu, const uint8_t* pData, size_t dataSize);
