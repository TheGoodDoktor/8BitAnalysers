
#pragma once

#include <stdint.h>
#include <vector>

class FCodeAnalysisState;

// This is code to workaround the fact that 8BA doesn't currently support memory aliasing.
// As in, mapping a single logical memory bank to multiple physical memory locations.

// The default number of banks in a bank set.
// The number includes the primary bank and any extra banks for duplicates.
// For example, a value of 4 means 1 primary and 3 duplicates.
// If this value is set to 1 then any duplicates will use UNUSED banks.
static const int kNumBankSetIds = 5;

// A set of bank ids that all represent the same logical memory.
// PCE games can map the same bank to different physical memory ranges.
// Eg. ROM1 being mapped to 0x4000-0x6000 and 0x8000-0xa000 simultaneously.
// This happens when the same bank index is present in >1 mpr slot.
// It is technically possible to map the same bank across the entire physical memory range.
// 8BA doesn't support a bank being mapped into >1 memory location at the same time, so 
// we need a set of banks that all point to the same memory.
struct FBankSet
{
	void SetPrimaryMappedPage(FCodeAnalysisState& state, int bankSetIndex, uint16_t pageAddr);
	int16_t GetFreeBank(uint8_t mprSlot);
	bool ClaimSpecificBank(int16_t bankId, int mprSlot);
	void SetBankFreed(uint8_t mprSlot);
	void Reset();
	void AddBankId(int16_t bankId);
	int16_t GetBankId(int index = 0) const;

	struct FBankSetEntry
	{
		int16_t BankId = -1;
		bool bMapped = false;
	};
	
	// Bank set index for each mpr slot. Only will be set to a bank id when a bank gets mapped.
	int SlotBankId[8] = { -1, -1, -1, -1, -1, -1, -1, -1 };

	// List of all bank ids.
	// Entry 0 is the primary bank. It should always have a primary mapped page.
	// Entries >1 are the dupe banks and will get a primary mapped page if they get mapped in.
	std::vector<FBankSetEntry> Banks;
};