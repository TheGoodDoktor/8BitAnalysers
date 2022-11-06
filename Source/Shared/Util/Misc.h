#pragma once
#include <cstdint>
#include <string>
#include <vector>

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
void Tokenize(const std::string& stringToSplit, const char token, std::vector<std::string>& splitStrings);
