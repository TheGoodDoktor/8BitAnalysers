#include "TAPLoader.h"
#include "../SpectrumEmu.h"

#include <cstdint>
#include <Util/FileUtil.h>
#include <cassert>
#include <systems/zx.h>

// https://sinclair.wiki.zxnet.co.uk/wiki/TAP_format

bool LoadTAPFile(FSpectrumEmu* pEmu, const char* fName)
{
	size_t byteCount = 0;
	uint8_t* pData = (uint8_t*)LoadBinaryFile(fName, byteCount);
	if (!pData)
		return false;
	const bool bSuccess = LoadTAPFromMemory(pEmu, pData, byteCount);
	free(pData);

	return bSuccess;
}

bool LoadTAPFromMemory(FSpectrumEmu* pEmu, const uint8_t* pData, size_t dataSize)
{
	return false;
}