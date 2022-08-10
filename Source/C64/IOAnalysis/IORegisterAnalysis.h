#pragma once

#include <cstdint>
#include <map>
#include <set>

struct FC64IORegisterAccessInfo
{
	std::set<uint8_t>	WriteVals;
};

struct FC64IORegisterInfo
{
	std::map<uint16_t, FC64IORegisterAccessInfo>	Accesses;
	uint8_t			LastVal;
};

struct FRegDisplayConfig
{
	const char* Name;
	void		(*UIDrawFunction)(uint8_t val);
};

void DrawRegValueDefault(uint8_t val);
void DrawRegValueDecimal(uint8_t val);