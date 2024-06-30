#pragma once
#include <stdint.h>
#include <map>

#include "SkoolFile.h"

// Information about a single memory location from the skool file.
struct FSkoolFileLocation
{
	bool operator ==(const FSkoolFileLocation& other)
	{ 
		return BlockDirective == other.BlockDirective 
			&& SubBlockDirective == other.SubBlockDirective
			&& bBranchDestination == other.bBranchDestination; 
	}
	
	SkoolDirective BlockDirective = SkoolDirective::None;
	SkoolDirective SubBlockDirective = SkoolDirective::None;
	bool bBranchDestination = false;
};

// This holds metadata about a SkoolKit skool file.
// We store information here that can't be stored in the code analysis.
// This will be used when exporting a skool file.
struct FSkoolFileInfo
{
	const FSkoolFileLocation* GetLocation(uint16_t address) const
	{
		TLocationMap::const_iterator it = Locations.find(address);
		if (it != Locations.end())
			return &(it->second);
		return nullptr;
	}
	uint16_t StartAddr = 0x0;
	uint16_t EndAddr = 0x0;

	typedef std::map<uint16_t, FSkoolFileLocation> TLocationMap;
	TLocationMap Locations;
};

bool LoadSkoolFileInfo(FSkoolFileInfo& skoolInfo, const char* fname);
bool SaveSkoolFileInfo(const FSkoolFileInfo &skoolInfo, const char *fname);
