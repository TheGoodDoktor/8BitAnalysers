#include "ExternalROMSupport.h"

#include "Util/FileUtil.h"
#include "CPCConfig.h"
#include "Debug/DebugLog.h"

#include <cassert>
#include <cstring>

extern unsigned char dump_cpc464_basic_bin[];
extern unsigned char dump_cpc6128_amsdos_bin[];

const int kUpperROMSize = 0x4000;
uint8_t* pUpperROM[kNumUpperROMSlots] = { 0 };
uint8_t* pUpperROMData = nullptr;
// temp
bool bExternalROMSupport = false;

void InitExternalROMs(const FCPCConfig* pConfig, bool bIs6128)
{
	if (!bExternalROMSupport)
		return;

	pUpperROM[0] = dump_cpc464_basic_bin;

	// sam todo: test 6128 support
	if (bIs6128)
		pUpperROM[7] = dump_cpc6128_amsdos_bin;

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
	if (!bExternalROMSupport || slotIndex >= kNumUpperROMSlots)
		return -1;

	if (!pUpperROM[slotIndex])
		slotIndex = 0;

	// Fallback to upper rom 0 (BASIC) if no external rom in selected slot
	pUpperROMData = pUpperROM[slotIndex];
	return slotIndex;
}

const uint8_t* GetUpperROMSlot(int slotIndex)
{
	if (!bExternalROMSupport || slotIndex >= kNumUpperROMSlots)
		return nullptr;

	return pUpperROM[slotIndex];
}

const uint8_t* GetUpperROMData()
{
	return pUpperROMData;
}

bool LoadExternalROM(const char* pFilename, int slotIndex)
{
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

void SetExternalROMSupportEnabled(bool bEnabled)
{
	bExternalROMSupport = bEnabled;
}

bool IsExternalROMSupportEnabled()
{
	return bExternalROMSupport;
}