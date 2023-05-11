#include "TZXLoader.h"


#include "../SpectrumEmu.h"

#include <cstdint>
#include <Util/FileUtil.h>
#include <cassert>
#include <systems/zx.h>

// http://k1.spdns.de/Develop/Projects/zasm/Info/TZX%20format.html

bool LoadTZXFile(FSpectrumEmu* pEmu, const char* fName)
{
	size_t byteCount = 0;
	uint8_t* pData = (uint8_t*)LoadBinaryFile(fName, byteCount);
	if (!pData)
		return false;
	const bool bSuccess = LoadTZXFromMemory(pEmu, pData, byteCount);
	free(pData);

	return bSuccess;
}

bool LoadTZXFromMemory(FSpectrumEmu* pEmu, const uint8_t* pData, size_t dataSize)
{
	return false;
}
