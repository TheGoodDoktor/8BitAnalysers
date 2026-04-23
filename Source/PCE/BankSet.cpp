#include "BankSet.h"

#include <string.h> // for memset
#include <CodeAnalyser/CodeAnalysisState.h>
#include <CodeAnalyser/CodeAnalyser.h>

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
	
void FBankSet::SetBankFreed(uint8_t mprSlot)
{
	assert(SlotBankId[mprSlot] != -1);
	Banks[SlotBankId[mprSlot]].bMapped = false;
	SlotBankId[mprSlot] = -1;
}
	
void FBankSet::Reset()
{
	for (int i = 0; i < 8; i++)
		SlotBankId[i] = -1;
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
	
void FBankSet::AddBankId(int16_t bankId)
{
	Banks.push_back(FBankSetEntry({ bankId, false }));
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
				return false; // Already mapped (corrupt state)

			assert(SlotBankId[mprSlot] == -1);
			SlotBankId[mprSlot] = i;

			entry.bMapped = true;
			return true;
		}
	}
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