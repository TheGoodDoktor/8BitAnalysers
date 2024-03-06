#include "ExternalROMSupport.h"

#include "Util/FileUtil.h"
#include "CPCConfig.h"
#include "Debug/DebugLog.h"

#include <cassert>
#include <cstring>

extern unsigned char dump_cpc464_basic_bin[];

const int kUpperROMSize = 0x4000;
uint8_t* pUpperROM[kNumUpperROMSlots] = { 0 };
uint8_t* pUpperROMData = nullptr;

void InitExternalROMs(const FCPCConfig* pConfig)
{
	// sam todo: 6128 support
	pUpperROM[0] = dump_cpc464_basic_bin;

	for (int i = 1; i < kNumUpperROMSlots; i++)
	{
		pUpperROM[i] = nullptr;

		if (pConfig->UpperROMSlot.size() > i)
		{
			if (!pConfig->UpperROMSlot[i].empty())
			{
				const std::string romName = pConfig->UpperROMSlot[i];
				const std::string relPath = "Roms/" + romName;
				const char* fullPath = GetBundlePath(relPath.c_str());

				if (LoadExternalROM(fullPath, i))
				{
					LOGINFO("Loaded external ROM '%s' into slot %d", romName.c_str(), i);
				}
				else
				{
					LOGWARNING("Could not load external ROM file '%s' into slot %d", romName.c_str(), i);
				}
			}
		}
	}
	// Ensure we have some default upper rom data
	SelectUpperROM(0);
}

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

bool LoadExternalROM(const char* pFilename, int slotIndex)
{
	// todo use rom path
	if (slotIndex >= kNumUpperROMSlots)
		return false;

	size_t byteCount = 0;
	uint8_t* pData = (uint8_t*)LoadBinaryFile(pFilename, byteCount);
	
	if (!pData)
		return false;
	
	bool bOk = true;
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
		else
		{
			bOk = false;
		}
		free(pData);
	}
	return bOk;
}
