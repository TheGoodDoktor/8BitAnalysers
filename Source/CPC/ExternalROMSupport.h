#pragma once

#include <stdint.h>

static const int kNumUpperROMSlots = 16;

#ifdef __cplusplus
extern "C" {
#endif	

	// Populate rom slots by loading any rom files we find in the global config. 
	void InitExternalROMs(const struct FCPCConfig* pConfig, bool bIs6128);
	bool LoadExternalROM(const char* pFilename, int slotIndex);
	
	// Get the memory for the currently selected upper ROM.
	const uint8_t* GetUpperROMData();

	// Get the memory for a specific ROM slot.
	const uint8_t* GetUpperROMSlot(int slotIndex);
	
	// Try to select a ROM slot. Returns index of rom slot that was selected, or -1 on failure. 
	int SelectUpperROM(int slotIndex);

	void SetExternalROMSupportEnabled(bool bEnabled);
	bool IsExternalROMSupportEnabled();

#ifdef __cplusplus
} // extern "C"
#endif