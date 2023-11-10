#pragma once

#include <cstddef>
#include <cinttypes>

class FCPCEmu;

bool LoadSNAFile(FCPCEmu* pEmu, const char* fName);
bool LoadSNAFileCached(FCPCEmu* pEmu, const char* fName, uint8_t*& pData, size_t& dataSize);
bool LoadSNAFromMemory(FCPCEmu* pEmu, uint8_t* pData, size_t dataSize);
