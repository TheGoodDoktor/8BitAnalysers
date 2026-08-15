#include "RecentMemoryAccess.h"

#include "CodeAnalyser/CodeAnalyser.h"

FRecentMemoryAccess::FRecentMemoryAccess()
{
	Reads.Reset();
	Writes.Reset();
}

void FRecentMemoryAccess::Reset()
{
	Reads.Reset();
	Writes.Reset();
}

// todo: remove this? not sure we need it
/*void FRecentMemoryAccess::SetEnabled(bool bEnable)
{
	if (bEnabled)
	{
	}
	else
	{
	}
	bEnabled = bEnable;
}*/

void FMemoryAccessBuf::Reset()
{
	Count = 0;
	CurIndex = 0;

	for (int i = 0; i < kMaxCount; i++)
	{
		MemoryAccess[i].Addr = FAddressRef::Invalid();
		MemoryAccess[i].NumBytes = 0;
	}
}

void FMemoryAccessBuf::RegisterAccess(FAddressRef addr)
{
	//if (!bEnabled)
	//	return;
	const uint16_t physAddr = addr.GetAddress();
	if (!bTrackStack && (physAddr >= 0x2100 && physAddr <= 0x21ff))
	{
		return;
	}

	if (LastIndex != -1)
	{
		const FAddressRef& lastAccess = MemoryAccess[LastIndex].Addr;
		if (lastAccess.IsValid())
		{
			// Is this a repeated access of >1 bytes?
			if (lastAccess.GetBankId() == addr.GetBankId())
			{
				if (abs(lastAccess.GetAddress() - addr.GetAddress()) == 1)
				{
					MemoryAccess[LastIndex].Addr = addr;
					MemoryAccess[LastIndex].NumBytes++;
					return;
				}
			}
		}
	}

	int index = CurIndex;
	if (Count < kMaxCount)
	{
		index = Count;
		Count++;
	}

	MemoryAccess[index].Addr = addr;
	MemoryAccess[index].NumBytes = 1;

	LastIndex = CurIndex;

	CurIndex++;
	if (CurIndex == kMaxCount)
		CurIndex = 0;
}

const FMemoryAccessItem* FMemoryAccessBuf::GetItem(int index) const
{
	if (index < 0 || index >= Count)
		return nullptr;

	// Buffer hasn't wrapped yet.
	if (Count < kMaxCount)
		return &MemoryAccess[index];

	// Buffer has wrapped.
	const int retIndex = (CurIndex + index) % kMaxCount;
	return &MemoryAccess[retIndex];
}
