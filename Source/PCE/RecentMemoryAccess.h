#pragma once

#include "CodeAnalyser/CodeAnalyserTypes.h"

class FPCEEmu;

enum class EMemoryAccessType
{
	None = 0,
	Read,
	Write,
};

struct FMemoryAccessItem
{
	uint16_t GetStartAddress() const { return NumBytes == 1 ? Addr.GetAddress() : Addr.GetAddress() - NumBytes - 1; }

	EMemoryAccessType Type;
	FAddressRef Addr;
	int NumBytes;
};

class FRecentMemoryAccess
{
public:
	static const int kMaxCount = 16;

	FRecentMemoryAccess(FPCEEmu* pEmu);

	void Reset();
	void ClearUsage();

	void RegisterAccess(EMemoryAccessType type, FAddressRef addr);

	void SetEnabled(bool bEnabled);

	int GetCount() const { return Count; }
	const FMemoryAccessItem* GetItem(int index) const;

	int GetLastReadIndex() const { return LastReadIndex; }
	int GetLastWriteIndex() const { return LastWriteIndex; }
	int GetCurIndex() const { return CurIndex; }

private:

	// Circular buffer of memory accesses
	FMemoryAccessItem MemoryAccess[kMaxCount];
	int Count = 0;				
	int CurIndex = 0;		// Buffer write index

	int LastReadIndex = -1;
	int LastWriteIndex = -1;

	bool bEnabled = false;

	FPCEEmu* pPCEEmu = nullptr;
};

