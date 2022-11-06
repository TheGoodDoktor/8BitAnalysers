#include "Misc.h"
#include <stdio.h>
#include <cassert>
#include <sstream>
#include <vector>

static ENumberDisplayMode g_NumDispMode = ENumberDisplayMode::HexAitch;
static const int kTextLength = 8;
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

ENumberDisplayMode GetNumberDisplayMode()
{
	return g_NumDispMode;
}


const char* NumStr(uint8_t num)
{
	char* pStrAddress = GetStrPtr();	

	switch (g_NumDispMode)
	{
	case ENumberDisplayMode::Decimal:
		sprintf_s(pStrAddress, kTextLength, "%d", num);
		return pStrAddress;

	case ENumberDisplayMode::HexAitch:
		sprintf_s(pStrAddress, kTextLength, "%02Xh", num);
		return pStrAddress;

	case ENumberDisplayMode::HexDollar:
		sprintf_s(pStrAddress, kTextLength, "$%02X", num);
		return pStrAddress;
	default:
		assert(0);
		return nullptr;
	}

}

const char* NumStr(uint16_t num)
{
	char* pStrAddress = GetStrPtr();	

	switch (g_NumDispMode)
	{
	case ENumberDisplayMode::Decimal:
		sprintf_s(pStrAddress, kTextLength, "%d", num);
		return pStrAddress;

	case ENumberDisplayMode::HexAitch:
		sprintf_s(pStrAddress, kTextLength, "%04Xh", num);
		return pStrAddress;

	case ENumberDisplayMode::HexDollar:
		sprintf_s(pStrAddress, kTextLength, "$%04X", num);
		return pStrAddress;
	default:
		assert(0);
		return nullptr;
	}
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