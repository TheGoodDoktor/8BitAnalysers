#include "CPCConfig.h"

#include "json.hpp"

void FCPCConfig::ReadFromJson(const nlohmann::json& jsonConfigFile)
{
	FGlobalConfig::ReadFromJson(jsonConfigFile);

	if (jsonConfigFile.contains("SnapshotFolder128"))
		SnapshotFolder128 = jsonConfigFile["SnapshotFolder128"];

	// fixup paths
	if (SnapshotFolder128.back() != '/')
		SnapshotFolder128 += "/";
}

void FCPCConfig::WriteToJson(nlohmann::json& jsonConfigFile)
{
	FGlobalConfig::WriteToJson(jsonConfigFile);
	jsonConfigFile["SnapshotFolder128"] = SnapshotFolder128;
}