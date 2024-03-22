#pragma once

#include "CodeAnalyser/CodeAnalyserTypes.h"
#include "CodeAnalyser/IOAnalyser.h"

#include <cstdint>
#include <unordered_map>
#include <unordered_set>
#include <vector>

class FCodeAnalysisState;
class FC64Emulator;

class FC64IODevice : public FIODevice
{
public:
	FC64Emulator* GetC64() { return pC64Emu;}

protected:
	FC64Emulator* pC64Emu = nullptr;
};

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
	void		(*UIDrawFunction)(FC64IODevice* pDevice, uint8_t val);
	EDataItemDisplayType	DisplayType = EDataItemDisplayType::Decimal;
};




void DrawRegValueHex(FC64IODevice* pDevice, uint8_t val);
void DrawRegValueDecimal(FC64IODevice* pDevice, uint8_t val);

int DrawRegSelectList(std::vector<FRegDisplayConfig>& regList, int selection);
void DrawRegDetails(FC64IODevice* pDevice, FC64IORegisterInfo& reg, const FRegDisplayConfig& regConfig, FCodeAnalysisState* pCodeAnalysis);
