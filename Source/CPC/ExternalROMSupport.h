#pragma once

#include <stdint.h>

static const int kNumUpperROMSlots = 16;

#ifdef __cplusplus
extern "C" {
#endif	

	void InitExternalROMs();
	void LoadExternalROM(const char* pFilename, int slotIndex);
	const uint8_t* GetUpperROMData();
	const uint8_t* GetUpperROMSlot(int slotIndex);
	int SelectUpperROM(int slotIndex);

#ifdef __cplusplus
} // extern "C"
#endif