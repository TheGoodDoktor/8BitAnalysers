#pragma once

#include <cstddef>
#include <cinttypes>

class FCpcEmu;

bool LoadSNAFile(FCpcEmu* pEmu, const char* fName);
bool LoadSNAFileCached(FCpcEmu* pEmu, const char* fName, uint8_t*& pData, size_t& dataSize);
bool LoadSNAFromMemory(FCpcEmu* pEmu, uint8_t* pData, size_t dataSize);
