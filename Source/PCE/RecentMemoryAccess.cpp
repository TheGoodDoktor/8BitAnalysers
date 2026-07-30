#include "RecentMemoryAccess.h"
#include "PCEEmu.h"

FRecentMemoryAccess::FRecentMemoryAccess(FPCEEmu* pEmu)
	: pPCEEmu(pEmu)
{
	Reset();
}

void FRecentMemoryAccess::Reset()
{
	Count = 0;
	CurIndex = 0;

	FAddressRef LastAccess = FAddressRef::Invalid();

	for (int i = 0; i < kMaxCount; i++)
	{
		MemoryAccess[i].Addr = FAddressRef::Invalid();
		MemoryAccess[i].NumBytes = 0;
		MemoryAccess[i].Type = EMemoryAccessType::None;
	}
}

void FRecentMemoryAccess::ClearUsage()
{
}

void FRecentMemoryAccess::RegisterAccess(EMemoryAccessType type, FAddressRef addr)
{
	assert(type != EMemoryAccessType::None);
	if (!bEnabled)
		return;

	int& lastIndex = type == EMemoryAccessType::Read ? LastReadIndex : LastWriteIndex;
	const FAddressRef& lastAccess = MemoryAccess[lastIndex].Addr;
	if (lastAccess.IsValid())
	{
		// todo: deal with crossing bank boundaries?
		
		// Is this a repeated access of >1 bytes?
		if (lastAccess.GetBankId() == addr.GetBankId() && lastAccess.GetAddress() == (addr.GetAddress() - 1))
		{
			MemoryAccess[lastIndex].Addr = addr;
			MemoryAccess[lastIndex].NumBytes++;
			return;
		}
	}

	int index = CurIndex;
	if (Count < kMaxCount)
	{
		index = Count;
		Count++;
	}

	MemoryAccess[index].Addr = addr;
	MemoryAccess[index].Type = type;
	MemoryAccess[index].NumBytes = 1;

	lastIndex = CurIndex;

	CurIndex++;
	if (CurIndex == kMaxCount)
		CurIndex = 0;
}

// todo: remove this? not sure we need it
void FRecentMemoryAccess::SetEnabled(bool bEnable)
{
	if (bEnabled)
	{
	}
	else
	{
	}
	bEnabled = bEnable;
}

const FMemoryAccessItem* FRecentMemoryAccess::GetItem(int index) const
{
	if (index < kMaxCount)
	{
		if (index < Count)
		{
			return &MemoryAccess[index];
		}
		else
		{
			int retIndex = (CurIndex + index) % kMaxCount;
			return &MemoryAccess[retIndex];
		}
	}
	return nullptr;
}
