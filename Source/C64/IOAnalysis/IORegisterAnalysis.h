#pragma once

#include "CodeAnalyser/CodeAnalyserTypes.h"

#include <cstdint>
#include <unordered_map>
#include <unordered_set>
#include <vector>

class FCodeAnalysisState;

struct FC64IORegisterAccessInfo
{
	std::unordered_set<uint8_t>	WriteVals;
};

struct FC64IORegisterInfo
{
	void Reset() { Accesses.clear(); LastVal = 0; }
	std::unordered_map<FAddressRef, FC64IORegisterAccessInfo>	Accesses;
	uint8_t			LastVal = 0;
};

struct FRegDisplayConfig
{
	const char* Name;
	void		(*UIDrawFunction)(uint8_t val);
};

void DrawRegValueHex(uint8_t val);
void DrawRegValueDecimal(uint8_t val);

int DrawRegSelectList(std::vector<FRegDisplayConfig>& regList, int selection);
void DrawRegDetails(FC64IORegisterInfo& reg, const FRegDisplayConfig& regConfig, FCodeAnalysisState* pCodeAnalysis);
