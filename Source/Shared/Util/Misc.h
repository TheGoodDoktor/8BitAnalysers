#pragma once
#include <cstdint>

enum class ENumberDisplayMode
{
	Decimal,
	HexDollar,
	HexAitch
};

void SetNumberDisplayMode(ENumberDisplayMode mode);
ENumberDisplayMode GetNumberDisplayMode();
const char* NumStr(uint8_t); 
const char* NumStr(uint16_t);