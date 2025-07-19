#include "TubeEliteConfig.h"

void FTubeEliteConfig::ReadFromJson(const nlohmann::json& jsonConfigFile)
{
	FGlobalConfig::ReadFromJson(jsonConfigFile);
}

void FTubeEliteConfig::WriteToJson(nlohmann::json& jsonConfigFile) const
{
	FGlobalConfig::WriteToJson(jsonConfigFile);

}

void FTubeEliteProjectConfig::LoadFromJson(const nlohmann::json& jsonConfig)
{
	FProjectConfig::LoadFromJson(jsonConfig);
	// Load project-specific configuration from JSON
}

void FTubeEliteProjectConfig::SaveToJson(nlohmann::json& jsonConfig) const
{
	FProjectConfig::SaveToJson(jsonConfig);
	// Save project-specific configuration to JSON
}

FTubeEliteProjectConfig* CreateNewTubeEliteConfig(void)
{
	FTubeEliteProjectConfig* pNewConfig = new FTubeEliteProjectConfig;

	pNewConfig->Name = "TubeElite";
	//pNewConfig->SnapshotFile = "";
	//pNewConfig->pViewerConfig = GetViewConfigForGame(pNewConfig->Name.c_str());

	return pNewConfig;
}

FTubeEliteProjectConfig* CreateNewBBCBasicConfig(void)
{
	FTubeEliteProjectConfig* pNewConfig = new FTubeEliteProjectConfig;

	pNewConfig->Name = "BBCBasic";
	//pNewConfig->SnapshotFile = "";
	//pNewConfig->pViewerConfig = GetViewConfigForGame(pNewConfig->Name.c_str());

	return pNewConfig;
}