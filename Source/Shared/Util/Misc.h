#pragma once
#include <cstdint>
#include <string>
#include <vector>

// note - config relies on these being a consistent numerical value
enum class ENumberDisplayMode
{
	Decimal = 0,
	HexDollar,
	HexAitch
};

void SetNumberDisplayMode(ENumberDisplayMode mode);
ENumberDisplayMode GetNumberDisplayMode();
const char* NumStr(uint8_t); 
const char* NumStr(uint16_t);
void Tokenize(const std::string& stringToSplit, const char token, std::vector<std::string>& splitStrings);
