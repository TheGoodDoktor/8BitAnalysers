#include "ExternalROMSupport.h"

#include "Util/FileUtil.h"

#include <cassert>

extern unsigned char dump_cpc464_basic_bin[];

const int kUpperROMSize = 0x4000;
uint8_t* pUpperROM[kNumUpperROMSlots] = { 0 };
uint8_t* pUpperROMData = nullptr;

void InitExternalROMs()
{
	for (int i = 0; i < kNumUpperROMSlots; i++)
	{
		pUpperROM[i] = nullptr;
	}

	// sam todo: 6128 support
	pUpperROM[0] = dump_cpc464_basic_bin;
}

// Returns index of rom slot that was selected.
int SelectUpperROM(int slotIndex)
{
	if (slotIndex >= kNumUpperROMSlots)
		return -1;

	if (!pUpperROM[slotIndex])
		slotIndex = 0;

	// Fallback to upper rom 0 if no external rom in selected slot
	pUpperROMData = pUpperROM[slotIndex];
	return slotIndex;
}

const uint8_t* GetUpperROMSlot(int slotIndex)
{
	if (slotIndex >= kNumUpperROMSlots)
		return nullptr;

	return pUpperROM[slotIndex];
}

const uint8_t* GetUpperROMData()
{
	return pUpperROMData;
}

void LoadExternalROM(const char* pFilename, int slotIndex)
{
	if (slotIndex >= kNumUpperROMSlots)
		return;

	size_t byteCount = 0;
	uint8_t* pData = (uint8_t*)LoadBinaryFile(pFilename, byteCount);
	
	if (pData)
	{
		assert(pUpperROM[slotIndex] == nullptr);

		if (byteCount <= kUpperROMSize)
		{
			pUpperROM[slotIndex] = (uint8_t*)malloc(kUpperROMSize);
			if (pUpperROM[slotIndex])
			{
				if (byteCount < kUpperROMSize)
					memset(pUpperROM[slotIndex], 0, kUpperROMSize);
				memcpy(pUpperROM[slotIndex], pData, byteCount);
			}
		}
		free(pData);
	}
}
