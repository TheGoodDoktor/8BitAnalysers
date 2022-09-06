#include "RZXLoader.h"
#include <malloc.h>
#include "Util/FileUtil.h"

bool LoadRZXFile(FSpectrumEmu* pEmu, const char* fName)
{
	size_t byteCount = 0;
	uint8_t* pData = (uint8_t*)LoadBinaryFile(fName, byteCount);
	const bool bSuccess = LoadRZXFromMemory(pEmu, pData, byteCount);
	free(pData);

	return bSuccess;
}

bool LoadRZXFromMemory(FSpectrumEmu* pEmu, const uint8_t* pData, size_t dataSize)
{
	// TODO: implement
	return false;
}