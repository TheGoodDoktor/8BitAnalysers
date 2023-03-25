#pragma once

#include <cstdint>

struct FAddressRef
{
	FAddressRef() = default;
	FAddressRef(int16_t bankId, uint16_t address) :BankId(bankId), Address(address) {}

	bool IsValid() const { return BankId != -1; }
	void SetInvalid() { BankId = -1; }
	bool operator==(const FAddressRef& other) const { return Address == other.Address && BankId == other.BankId; }
	int16_t		BankId = -1;
	uint16_t	Address = 0;
};