#include "Z80Loader.h"
#include "../SpectrumEmu.h"
#include <Util/FileUtil.h>

bool LoadZ80File(FSpectrumEmu* pEmu, const char* fName)
{
	size_t byteCount = 0;
	void* pData = LoadBinaryFile(fName, byteCount);

	const bool bSuccess = LoadZ80FromMemory(pEmu, (const uint8_t*)pData, byteCount);
	free(pData);
	return bSuccess;
}

bool LoadZ80FromMemory(FSpectrumEmu* pEmu, const uint8_t* pData, size_t dataSize)
{
	chips_range_t dataInfo;
	dataInfo.ptr = (void*)pData;
	dataInfo.size = dataSize;
	return zx_quickload(&pEmu->ZXEmuState, dataInfo);
}