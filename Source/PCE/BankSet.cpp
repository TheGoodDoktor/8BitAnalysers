#include "BankSet.h"

#include <string.h> // for memset
#include <stdio.h> // for snprintf
#include <CodeAnalyser/CodeAnalysisState.h>
#include <CodeAnalyser/CodeAnalyser.h>
#include <CodeAnalyser/CodeAnalysisPage.h>

void FBankSet::SetPrimaryMappedPage(FCodeAnalysisState& state, int bankSetIndex, uint16_t pageAddr)
{
	FCodeAnalysisBank* pBank = state.GetBank(Banks[bankSetIndex].BankId);
	assert(pBank);
	pBank->PrimaryMappedPage = pageAddr;
}

int16_t FBankSet::GetFreeBank(uint8_t mprSlot)
{
	for (int i = 0; i < Banks.size(); i++)
	{
		FBankSetEntry& entry = Banks[i];
		if (!entry.bMapped)
		{
			entry.bMapped = true;
			assert(SlotBankId[mprSlot] == -1);
			SlotBankId[mprSlot] = i;
			return entry.BankId;
		}
	}

	return -1;
}
	
int16_t FBankSet::SetBankFreed(uint8_t mprSlot)
{
	assert(SlotBankId[mprSlot] != -1);
	FBankSetEntry& entry = Banks[SlotBankId[mprSlot]];
	entry.bMapped = false;
	SlotBankId[mprSlot] = -1;
	return entry.BankId;
}

void FBankSet::Reset()
{
	for (int i = 0; i < 8; i++)
		SlotBankId[i] = -1;
	// Drop any pool banks borrowed for duplicate mappings.
	// The pool itself forgets the bindings in FDupeBankPool::Reset().
	Banks.resize(NumFixedBanks);
	for (int i = 0; i < Banks.size(); i++)
		Banks[i].bMapped = false;
	MappedSlotsMask = 0;
	memset(SlotMapCount, 0, sizeof(SlotMapCount));
	memset(SlotFirstUseOrder, 0, sizeof(SlotFirstUseOrder));
	NextFirstUseOrder = 1;
}

void FBankSet::RecordSlotMapping(uint8_t mprSlot)
{
	if (SlotMapCount[mprSlot] < UINT32_MAX)
		SlotMapCount[mprSlot]++;
	if (!(MappedSlotsMask & (1 << mprSlot)))
	{
		MappedSlotsMask |= (1 << mprSlot);
		SlotFirstUseOrder[mprSlot] = NextFirstUseOrder++;
	}
}
	
void FBankSet::AddFixedBankId(int16_t bankId)
{
	// Fixed banks must all be added before any pool banks get attached.
	assert((int)Banks.size() == NumFixedBanks);
	Banks.push_back(FBankSetEntry({ bankId, false }));
	NumFixedBanks++;
}

void FBankSet::AttachPoolBank(int16_t bankId)
{
	Banks.push_back(FBankSetEntry({ bankId, false }));
}

void FBankSet::DetachPoolBank(int16_t bankId)
{
	for (int i = NumFixedBanks; i < (int)Banks.size(); i++)
	{
		if (Banks[i].BankId == bankId)
		{
			assert(!Banks[i].bMapped);
			Banks.erase(Banks.begin() + i);
			// SlotBankId holds indices into Banks so fix up any entries above the erased index.
			for (int s = 0; s < 8; s++)
			{
				assert(SlotBankId[s] != i);
				if (SlotBankId[s] > i)
					SlotBankId[s]--;
			}
			return;
		}
	}
	assert(0);	// bank is not attached to this set
}
	
int16_t FBankSet::GetBankId(int index /* = 0 */) const
{
	assert(!Banks.empty());
	if (index >= Banks.size())
		return -1;

	return Banks[index].BankId;
}

bool FBankSet::ClaimSpecificBank(int16_t bankId, int mprSlot)
{
	for (int i = 0; i < Banks.size(); i++)
	{
		FBankSetEntry& entry = Banks[i];
		if (entry.BankId == bankId)
		{
			assert(!entry.bMapped);
			if (entry.bMapped)
			{
				LOGERROR("ClaimSpecificBank: Entry already mapped! bankId=%d mprSlot=%d", bankId, mprSlot);
				return false; // Already mapped (corrupt state)
			}

			assert(SlotBankId[mprSlot] == -1);
			SlotBankId[mprSlot] = i;

			entry.bMapped = true;
			return true;
		}
	}
	LOGERROR("ClaimSpecificBank: Could not find bankId in bank set. bankId=%d mprSlot=%d", bankId, mprSlot);

	return false;
}

uint16_t FBankSet::GetMappedAddressFromUsage() const
{
	int bestSlot = -1;
	uint32_t bestCount = 0;
	for (int i = 0; i < 8; i++)
	{
		if (SlotMapCount[i] > bestCount)
		{
			bestCount = SlotMapCount[i];
			bestSlot = i;
		}
	}
	return bestSlot >= 0 ? (uint16_t)(bestSlot * 0x2000) : 0;
}

void FDupeBankPool::Create(FCodeAnalysisState& state, uint8_t* pInitialMem, uint16_t initialAddress)
{
	char bankName[32];
	for (int i = 0; i < kNumPoolBanks; i++)
	{
		snprintf(bankName, 32, "DUPE_%02d", i);
		const int16_t bankId = state.CreateBank(bankName, 8, pInitialMem, false, initialAddress);
		if (i == 0)
			FirstBankId = bankId;
		assert(bankId == FirstBankId + i);	// IsPoolBank() relies on pool bank ids being contiguous
		Entries[i].BankId = bankId;
	}
}

void FDupeBankPool::Reset(FCodeAnalysisState& state, uint8_t* pUnusedMem)
{
	char bankName[32];
	for (int i = 0; i < kNumPoolBanks; i++)
	{
		FPoolEntry& entry = Entries[i];
		entry.pBoundSet = nullptr;
		entry.LastUseCounter = 0;

		// Point the bank back at unused memory so it doesn't dangle into memory owned by unloaded media.
		if (FCodeAnalysisBank* pBank = state.GetBank(entry.BankId))
		{
			snprintf(bankName, 32, "DUPE_%02d", i);
			pBank->Name = bankName;
			pBank->Memory = pUnusedMem;
			pBank->ItemList.clear();
			pBank->bIsDirty = true;
			pBank->bMachineROM = false;
		}
	}
	UseCounter = 0;
}

int16_t FDupeBankPool::BindBankToSet(FCodeAnalysisState& state, FBankSet* pBankSet)
{
	// Prefer an unbound bank so we don't invalidate a warm binding another set could remount.
	FPoolEntry* pEntry = nullptr;
	for (int i = 0; i < kNumPoolBanks; i++)
	{
		if (Entries[i].pBoundSet == nullptr)
		{
			pEntry = &Entries[i];
			break;
		}
	}

	// Otherwise steal the least recently used bank that isn't currently mapped.
	if (pEntry == nullptr)
	{
		for (int i = 0; i < kNumPoolBanks; i++)
		{
			FPoolEntry& entry = Entries[i];
			const FCodeAnalysisBank* pBank = state.GetBank(entry.BankId);
			if (pBank && !pBank->IsMapped())
			{
				if (pEntry == nullptr || entry.LastUseCounter < pEntry->LastUseCounter)
					pEntry = &entry;
			}
		}
	}

	if (pEntry == nullptr)
	{
		// Every pool bank is mapped. Shouldn't be possible - at most 7 duplicates can be mapped at once.
		LOGERROR("FDupeBankPool: pool exhausted binding a bank for set with primary id %d", pBankSet->GetBankId());
		return -1;
	}

	BindEntry(state, *pEntry, pBankSet);
	pEntry->LastUseCounter = ++UseCounter;
	return pEntry->BankId;
}

bool FDupeBankPool::BindSpecificBankToSet(FCodeAnalysisState& state, int16_t bankId, FBankSet* pBankSet)
{
	FPoolEntry* pEntry = GetEntry(bankId);
	if (pEntry == nullptr)
		return false;

	BindEntry(state, *pEntry, pBankSet);
	pEntry->LastUseCounter = ++UseCounter;
	return true;
}

void FDupeBankPool::OnBankReleased(int16_t bankId)
{
	if (FPoolEntry* pEntry = GetEntry(bankId))
		pEntry->LastUseCounter = ++UseCounter;
}

bool FDupeBankPool::IsPoolBank(int16_t bankId) const
{
	return FirstBankId != -1 && bankId >= FirstBankId && bankId < FirstBankId + kNumPoolBanks;
}

FBankSet* FDupeBankPool::GetBoundBankSet(int16_t bankId) const
{
	return IsPoolBank(bankId) ? Entries[bankId - FirstBankId].pBoundSet : nullptr;
}

FDupeBankPool::FPoolEntry* FDupeBankPool::GetEntry(int16_t bankId)
{
	return IsPoolBank(bankId) ? &Entries[bankId - FirstBankId] : nullptr;
}

void FDupeBankPool::BindEntry(FCodeAnalysisState& state, FPoolEntry& entry, FBankSet* pBankSet)
{
	if (entry.pBoundSet == pBankSet)
		return;	// warm binding - the bank's analysis state is still valid for this set

	if (entry.pBoundSet != nullptr)
		entry.pBoundSet->DetachPoolBank(entry.BankId);

	FCodeAnalysisBank* pBank = state.GetBank(entry.BankId);
	const FCodeAnalysisBank* pPrimaryBank = state.GetBank(pBankSet->GetBankId());
	assert(pBank && pPrimaryBank);
	if (!pBank || !pPrimaryBank)
		return;

	// The bank now represents different memory so invalidate its transient analysis state.
	char bankName[32];
	snprintf(bankName, 32, "%s_%d", pPrimaryBank->Name.c_str(), (int)pBankSet->Banks.size() + 1);
	pBank->Name = bankName;
	pBank->Memory = pPrimaryBank->Memory;
	pBank->bMachineROM = pPrimaryBank->bMachineROM;
	pBank->ItemList.clear();
	pBank->bIsDirty = true;

	// Wipe the pages too. Physical-address analysis writes (e.g. WriteCodeInfoForAddress) land in
	// the mapped bank's pages, so the pages hold aliases of the old set's code/data info. The item
	// list gets rebuilt from the pages, so stale page data would resurrect the old set's items.
	// The pointers are just dropped, not freed - the canonical bank's pages own the objects.
	for (int pageNo = 0; pageNo < pBank->NoPages; pageNo++)
		pBank->Pages[pageNo].Initialise();

	pBankSet->AttachPoolBank(entry.BankId);
	entry.pBoundSet = pBankSet;
}