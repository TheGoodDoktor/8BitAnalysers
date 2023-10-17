#include "C64Config.h"

#include "json.hpp"

void FC64Config::ReadFromJson(const nlohmann::json& jsonConfigFile)
{
	FGlobalConfig::ReadFromJson(jsonConfigFile);

	if (jsonConfigFile.contains("TapesFolder"))
		TapesFolder = jsonConfigFile["TapesFolder"];
	if (jsonConfigFile.contains("DisksFolder"))
		DisksFolder = jsonConfigFile["DisksFolder"];

	// fixup paths
	if (TapesFolder.back() != '/')
		TapesFolder += "/";
	if (DisksFolder.back() != '/')
		DisksFolder += "/";
}

void FC64Config::WriteToJson(nlohmann::json& jsonConfigFile)
{
	FGlobalConfig::WriteToJson(jsonConfigFile);
	jsonConfigFile["TapesFolder"] = TapesFolder;
	jsonConfigFile["DisksFolder"] = DisksFolder;
}