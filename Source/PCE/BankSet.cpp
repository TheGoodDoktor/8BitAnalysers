#include "BankSet.h"

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
