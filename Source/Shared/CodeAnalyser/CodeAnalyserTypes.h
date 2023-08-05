#pragma once

#include <cstdint>

// CPU abstraction
enum class ECPUType
{
	Unknown,
	Z80,
	M6502
};

struct FAddressRef
{
	FAddressRef() :BankId(-1), Address(0) {}
	FAddressRef(int16_t bankId, uint16_t address) :BankId(bankId), Address(address) {}

	bool IsValid() const { return BankId != -1; }
	void SetInvalid() { BankId = -1; }
	bool operator<(const FAddressRef& other) const { return Val < other.Val; }
	bool operator==(const FAddressRef& other) const { return Val == other.Val; }
	bool operator!=(const FAddressRef& other) const { return Val != other.Val; }
	FAddressRef operator++(int) { Address++; return *this; }
	union
	{
		struct
		{
			int16_t		BankId;
			uint16_t	Address;
		};
		uint32_t	Val;
	};
};