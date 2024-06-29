
#include "SkoolFileInfo.h"
#include "Util/Misc.h"
#include "Debug/DebugLog.h"
#include "Util/FileUtil.h"

#include "json.hpp"
#include <iomanip>
#include <fstream>
#include <cassert>

using json = nlohmann::json;

const std::string kBlockDirectives = "bcgistuw";
const std::string kSubBlockDirectives = "BCTW";

bool LoadSkoolFileInfo(FSkoolFileInfo& skoolInfo, const char* fname)
{
	std::ifstream inFileStream(fname);
	if (inFileStream.is_open() == false)
		return false;

	json jsonSkoolInfoFile;

	inFileStream >> jsonSkoolInfoFile;
	inFileStream.close();

	skoolInfo.StartAddr = ParseHexString16bit(jsonSkoolInfoFile["StartAddress"].get<std::string>());
	skoolInfo.EndAddr = ParseHexString16bit(jsonSkoolInfoFile["EndAddress"].get<std::string>());

	for (const auto& jsonItem : jsonSkoolInfoFile["Locations"])
	{
		FSkoolFileLocation location;
		const std::string& directives = jsonItem["Directives"].get<std::string>();
		
		for (char c : directives)
		{
			if (c == '*')
				location.bBranchDestination = true;
			else if (kBlockDirectives.find(c) != std::string::npos)
				location.BlockDirective = GetDirectiveFromChar(c);
			else if (kSubBlockDirectives.find(c) != std::string::npos)
				location.SubBlockDirective = GetDirectiveFromChar(c);
			else
				LOGWARNING("Unknown directive '%c' in skoolinfo file", c);
		}

		// todo support decimal
		uint16_t address = ParseHexString16bit(jsonItem["Address"].get<std::string>());

		skoolInfo.Locations[address] = location;
	}
	
	return true;
}

bool SaveSkoolFileInfo(const FSkoolFileInfo &skoolInfo, const char *fname)
{
	json jsonSkoolInfoFile;

	jsonSkoolInfoFile["StartAddress"] = MakeHexString(skoolInfo.StartAddr);
	jsonSkoolInfoFile["EndAddress"] = MakeHexString(skoolInfo.EndAddr);

	for (const auto &locIt : skoolInfo.Locations)
	{
		json jsonLocation;
		std::string directives;
		const FSkoolFileLocation& location = locIt.second;
		if (location.BlockDirective != SkoolDirective::None)
			directives.push_back(GetCharFromDirective(location.BlockDirective));
		if (location.SubBlockDirective != SkoolDirective::None)
			directives.push_back(toupper(GetCharFromDirective(location.SubBlockDirective)));
		if (location.bBranchDestination)
			directives.push_back('*');
		assert(!directives.empty());
		jsonLocation["Directives"] = directives;
		jsonLocation["Address"] = MakeHexString(locIt.first);

		jsonSkoolInfoFile["Locations"].push_back(jsonLocation);
	}

	std::ofstream outFileStream(fname);
	if (outFileStream.is_open())
	{
		outFileStream << std::setw(4) << jsonSkoolInfoFile << std::endl;
		return true;
	}

	return false;
}
