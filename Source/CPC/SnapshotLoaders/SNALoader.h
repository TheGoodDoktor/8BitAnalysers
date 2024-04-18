#pragma once

#include <cstddef>
#include <cinttypes>

class FCPCEmu;
enum class ECPCModel;

bool LoadSNAFile(FCPCEmu* pEmu, const char* fName, ECPCModel fallbackModel);
