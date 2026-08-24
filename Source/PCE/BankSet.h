
#pragma once

#include <stdint.h>
#include <vector>

class FCodeAnalysisState;

// This is code to workaround the fact that 8BA doesn't currently support memory aliasing.
// As in, mapping a single logical memory bank to multiple physical memory locations.

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
	int16_t SetBankFreed(uint8_t mprSlot);	// returns the id of the bank that was freed
	void Reset();
	void AddFixedBankId(int16_t bankId);	// Add a fixed canonical bank. Fixed banks survive Reset()
	void AttachPoolBank(int16_t bankId);	// Attach a borrowed pool bank to represent a duplicate mapping
	void DetachPoolBank(int16_t bankId);	// Detach a borrowed pool bank. It must not be currently mapped
	int16_t GetBankId(int index = 0) const;

	struct FBankSetEntry
	{
		int16_t BankId = -1;
		bool bMapped = false;
	};
	
	// Bank set index for each mpr slot. Only will be set to a bank id when a bank gets mapped.
	int SlotBankId[8] = { -1, -1, -1, -1, -1, -1, -1, -1 };

	// MPR slot usage tracking.
	uint8_t  MappedSlotsMask = 0;        // bitfield: bit i set if slot i has ever been mapped
	uint32_t SlotMapCount[8] = {};       // how many times each slot has been mapped
	uint8_t  SlotFirstUseOrder[8] = {};  // order (1-8) of first use per slot, 0 = never used
	uint8_t  NextFirstUseOrder = 1;      // next order value to assign on first use

	void RecordSlotMapping(uint8_t mprSlot);
	uint16_t GetMappedAddressFromUsage() const;

	// List of all bank ids.
	// Entry 0 is the primary bank. It should always have a primary mapped page.
	// Entries >1 are the dupe banks and will get a primary mapped page if they get mapped in.
	std::vector<FBankSetEntry> Banks;

	// Number of fixed banks at the start of the Banks list (added with AddBankId).
	// Entries beyond this are pool banks borrowed for duplicate mappings and are dropped by Reset().
	int NumFixedBanks = 0;
};

// A shared pool of banks used to represent duplicate mappings (the same logical bank in >1 mpr slot).
// At most 7 duplicates can be mapped simultaneously (8 mpr slots, one of them holds the primary),
// so the pool size is a hard ceiling with headroom. The headroom acts as a cache: a released bank
// keeps its binding so the same bank set can remount it without invalidating its analysis state.
// A bank is only rebound (wiped, renamed and repointed at different memory) when no unbound bank
// is available and one has to be stolen from another set.
class FDupeBankPool
{
public:
	static const int kNumPoolBanks = 16;

	// Create the pool's analysis banks. Call once at startup so bank ids stay deterministic.
	void Create(FCodeAnalysisState& state, uint8_t* pInitialMem, uint16_t initialAddress);

	// Drop all bindings and return the pool banks to their initial state.
	// Call after the bank sets have been Reset() - it doesn't detach banks from their old sets.
	void Reset(FCodeAnalysisState& state, uint8_t* pUnusedMem);

	// Bind a pool bank to a set and attach it, so it can represent a duplicate mapping.
	// Returns -1 if every pool bank is currently mapped.
	// The caller is responsible for updating any bankId->bankset/canonical bank id lookups.
	int16_t BindBankToSet(FCodeAnalysisState& state, FBankSet* pBankSet);

	// Bind a specific pool bank to a set. Used at project load to restore a saved mapping.
	bool BindSpecificBankToSet(FCodeAnalysisState& state, int16_t bankId, FBankSet* pBankSet);

	// The bank's mpr slot was freed. Keeps the binding so the set can remount it cheaply.
	void OnBankReleased(int16_t bankId);

	bool IsPoolBank(int16_t bankId) const;
	FBankSet* GetBoundBankSet(int16_t bankId) const;
	int16_t GetBankIdByIndex(int index) const { return Entries[index].BankId; }

private:
	struct FPoolEntry
	{
		int16_t BankId = -1;
		FBankSet* pBoundSet = nullptr;
		uint32_t LastUseCounter = 0;
	};

	FPoolEntry* GetEntry(int16_t bankId);
	void BindEntry(FCodeAnalysisState& state, FPoolEntry& entry, FBankSet* pBankSet);

	FPoolEntry Entries[kNumPoolBanks];
	uint32_t UseCounter = 0;
	int16_t FirstBankId = -1;	// pool bank ids are contiguous, starting here
};