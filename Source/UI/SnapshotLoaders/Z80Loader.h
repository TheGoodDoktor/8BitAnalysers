#pragma once

#include <stdint.h>

class FSpectrumEmu;

bool LoadZ80File(FSpectrumEmu* pEmu, const char* fName); 
bool LoadZ80FromMemory(FSpectrumEmu* pEmu, const uint8_t* pData, size_t dataSize);