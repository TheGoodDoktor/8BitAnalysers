#pragma once

#include "CodeAnalyser/CodeAnalyserTypes.h"

struct FMemoryAccessItem
{
	uint16_t GetStartAddress() const { return NumBytes == 1 ? Addr.GetAddress() : Addr.GetAddress() - NumBytes - 1; }
	FAddressRef Addr;
	int NumBytes;
};

static const int kMaxMemAccessCount = 16;

// todo rename
struct FMemoryAccessBuf
{
	void RegisterAccess(FAddressRef addr);
	void Reset();
	const FMemoryAccessItem* GetItem(int index) const;

	// Circular buffer of memory accesses
	FMemoryAccessItem MemoryAccess[kMaxMemAccessCount];
	int Count = 0;
	int CurIndex = 0;		// Buffer write index

	int LastIndex = -1;
	bool bTrackStack = false;
};

class FRecentMemoryAccess
{
public:
	FRecentMemoryAccess();

	void Reset();
	void SetStackTracking(bool bEnable) { Reads.bTrackStack = bEnable; Writes.bTrackStack = bEnable; }

	//void SetEnabled(bool bEnabled);

	FMemoryAccessBuf Reads;
	FMemoryAccessBuf Writes;

private:
	//bool bEnabled = false;
	bool bTrackStack = false;
};

