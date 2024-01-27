#include "Z80Loader.h"
#include "../SpectrumEmu.h"
#include <Util/FileUtil.h>

bool LoadZ80File(FSpectrumEmu* pEmu, const char* fName)
{
	size_t byteCount = 0;
	void* pData = LoadBinaryFile(fName, byteCount);

	if (pData == nullptr)
		return false;

	const bool bSuccess = LoadZ80FromMemory(pEmu, (const uint8_t*)pData, byteCount);
	free(pData);
	return bSuccess;
}

bool LoadZ80FromMemory(FSpectrumEmu* pSpectrumEmu, const uint8_t* pData, size_t dataSize)
{
	chips_range_t dataInfo;
	dataInfo.ptr = (void*)pData;
	dataInfo.size = dataSize;

	if (dataSize < 35)
	{
		pSpectrumEmu->SetLastError("Snapshot data size is invalid.");
		return false;
	}

	const uint16_t pc = *(pData + 7) << 8 | *(pData + 6);
	const bool bIsVersion1 = 0 != pc;
	if (!bIsVersion1) 
	{
		const uint8_t machineType = *(pData + 34);
		if (machineType < 3)
		{
			if (pSpectrumEmu->ZXEmuState.type == ZX_TYPE_128) 
			{
				pSpectrumEmu->SetLastError("Snapshot is a 48k machine snapshot. Only 128k machine snapshots can be used.");
				return false;
			}
		}
		else 
		{
			if (pSpectrumEmu->ZXEmuState.type == ZX_TYPE_48K) 
			{
				pSpectrumEmu->SetLastError("Snapshot is not a 48k machine snapshot. Only 48k machine snapshots can be used.");
				return false;
			}
		}
	}
	else 
	{
		// version 1 is 48k only
		if (pSpectrumEmu->ZXEmuState.type == ZX_TYPE_128) 
		{
			pSpectrumEmu->SetLastError("Snapshot is a 48k machine snapshot. Only 128k machine snapshots can be used.");
			return false;
		}
	}

	if (zx_quickload(&pSpectrumEmu->ZXEmuState, dataInfo) == false)
		return false;

	if (pSpectrumEmu->ZXEmuState.type == ZX_TYPE_128)
	{
		uint8_t memConfig = pSpectrumEmu->ZXEmuState.last_mem_config;

		// Set code analysis banks
		pSpectrumEmu->SetROMBank(memConfig & (1 << 4) ? 1 : 0);
		pSpectrumEmu->SetRAMBank(3, memConfig & 0x7);
	}
	return true;
}