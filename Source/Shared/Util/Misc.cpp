#include "Misc.h"
#include <stdio.h>
#include <cassert>
#include <sstream>
#include <vector>

static ENumberDisplayMode g_HexNumDispMode = ENumberDisplayMode::HexAitch;
static ENumberDisplayMode g_NumDispMode = ENumberDisplayMode::HexAitch;
static const int kTextLength = 24;
static const int kNoStrings = 8;
int g_StringIndex = 0;
static char g_TextWorkspace[kNoStrings][kTextLength];

char* GetStrPtr()
{
	const int index = g_StringIndex;
	g_StringIndex++;
	if (g_StringIndex == kNoStrings)
		g_StringIndex = 0;
	return g_TextWorkspace[index];
}

void SetNumberDisplayMode(ENumberDisplayMode mode)
{
	g_NumDispMode = mode;
}

void SetHexNumberDisplayMode(ENumberDisplayMode mode)
{
	g_HexNumDispMode = mode;
}
ENumberDisplayMode GetHexNumberDisplayMode()
{
	return g_HexNumDispMode;
}

ENumberDisplayMode GetNumberDisplayMode()
{
	return g_NumDispMode;
}

#define BYTE_TO_BINARY(byte)  \
  (byte & 0x80 ? '1' : '0'), \
  (byte & 0x40 ? '1' : '0'), \
  (byte & 0x20 ? '1' : '0'), \
  (byte & 0x10 ? '1' : '0'), \
  (byte & 0x08 ? '1' : '0'), \
  (byte & 0x04 ? '1' : '0'), \
  (byte & 0x02 ? '1' : '0'), \
  (byte & 0x01 ? '1' : '0') 

const char* NumStr(uint8_t num, ENumberDisplayMode numDispMode)
{
	char* pStrAddress = GetStrPtr();	

	switch (numDispMode)
	{
	case ENumberDisplayMode::Decimal:
		snprintf(pStrAddress,kTextLength, "%d", num);
		return pStrAddress;

	case ENumberDisplayMode::HexAitch:
		snprintf(pStrAddress,kTextLength, "%02Xh", num);
		return pStrAddress;
	case ENumberDisplayMode::HexDollar:
		snprintf(pStrAddress,kTextLength, "$%02X", num);
		return pStrAddress;
	case ENumberDisplayMode::HexAmpersand:
		snprintf(pStrAddress, kTextLength, "&%02X", num);
		return pStrAddress;

	case ENumberDisplayMode::Binary:
		snprintf(pStrAddress,kTextLength, "%c%c%c%c%c%c%c%c%c", '%', BYTE_TO_BINARY(num));
		return pStrAddress;
	case ENumberDisplayMode::Ascii:
		snprintf(pStrAddress, kTextLength, "'%c'", num);
		return pStrAddress;
	default:
		assert(0);
		return nullptr;
	}

}

const char* NumStr(uint8_t num)
{
	return NumStr(num, g_NumDispMode);
}

const char* NumStr(uint16_t num, ENumberDisplayMode numDispMode)
{
	char* pStrAddress = GetStrPtr();	

	switch (numDispMode)
	{
	case ENumberDisplayMode::Decimal:
		snprintf(pStrAddress,kTextLength, "%d", num);
		return pStrAddress;

	case ENumberDisplayMode::HexAitch:
		snprintf(pStrAddress,kTextLength, "%04Xh", num);
		return pStrAddress;

	case ENumberDisplayMode::HexDollar:
		snprintf(pStrAddress,kTextLength, "$%04X", num);
		return pStrAddress;

	case ENumberDisplayMode::HexAmpersand:
		snprintf(pStrAddress, kTextLength, "&%04X", num);
		return pStrAddress;

	case ENumberDisplayMode::Binary:
		snprintf(pStrAddress,kTextLength, "%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c", '%', BYTE_TO_BINARY(num>>8), BYTE_TO_BINARY(num));
		return pStrAddress;

	default:
		assert(0);
		return nullptr;
	}
}

const char* NumStr(uint16_t num)
{
	return NumStr(num, g_NumDispMode);
}


void Tokenize(const std::string& stringToSplit, const char token, std::vector<std::string>& splitStrings)
{
	std::stringstream stringStream(stringToSplit);
	std::string line;
	splitStrings.clear();

	if (stringToSplit.empty())
	{
		// special case: empty string
		splitStrings.push_back("");
		return;
	}

	while (std::getline(stringStream, line, token))
	{
		splitStrings.push_back(line);
	}
}
