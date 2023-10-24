#include "SNALoader.h"

#include "../CPCEmu.h"

#include <Util/FileUtil.h>
#include <systems/cpc.h>

bool LoadSNAFile(FCpcEmu* pEmu, const char* fName)
{
	size_t byteCount = 0;
	uint8_t* pData = (uint8_t*)LoadBinaryFile(fName, byteCount);
	if (!pData)
		return false;

	const bool bSuccess = LoadSNAFromMemory(pEmu, pData, byteCount);
	free(pData);

	return bSuccess;
}

bool LoadSNAFileCached(FCpcEmu* pEmu, const char* fName, uint8_t*& pData , size_t& dataSize)
{
	if (pData == nullptr)
	{
		pData = (uint8_t*)LoadBinaryFile(fName, dataSize);
		if (!pData)
			return false;
	}
	return LoadSNAFromMemory(pEmu, pData, dataSize);
}

bool LoadSNAFromMemory(FCpcEmu * pEmu, uint8_t * pData, size_t dataSize)
{	
	chips_range_t range;
	range.ptr = static_cast<void*>(pData);
	range.size = dataSize;


	if (dataSize <= 0x100)
		return false;
	
	static uint8_t magic[8] = { 'M', 'V', 0x20, '-', 0x20, 'S', 'N', 'A' };
	for (size_t i = 0; i < 8; i++) 
	{
		if (magic[i] != pData[i]) 
			return false;
	}

	uint8_t sizeL = *(pData + 0x6b);
	uint8_t sizeH = *(pData + 0x6c);
	const uint16_t dump_size = sizeH << 8 | sizeL;

	bool bResult = cpc_quickload(&pEmu->CpcEmuState, range);
	if (bResult == true)
	{
		if (pEmu->CpcEmuState.type == CPC_TYPE_6128)
		{
			// todo: set rom bank here

			pEmu->SetRAMBanksPreset(pEmu->CpcEmuState.ga.ram_config & 7);
		}
		// todo: maybe set rom and ram banks here for 464 too

	}
	return bResult;
}

