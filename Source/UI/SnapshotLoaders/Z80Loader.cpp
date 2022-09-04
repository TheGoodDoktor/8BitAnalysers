#include "Z80Loader.h"
#include "../SpeccyUI.h"
#include <Shared/Util/FileUtil.h>

bool LoadZ80File(FSpectrumEmu* pEmu, const char* fName)
{
	size_t byteCount = 0;
	void* pData = LoadBinaryFile(fName, byteCount);

	const bool bSuccess = zx_quickload(&pEmu->ZXEmuState, (const uint8_t*)pData, (int)byteCount);
	free(pData);
	return bSuccess;
}