#include "ArcadeZ80Config.h"

void FArcadeZ80Config::ReadFromJson(const nlohmann::json& jsonConfigFile)
{
	FGlobalConfig::ReadFromJson(jsonConfigFile);
}

void FArcadeZ80Config::WriteToJson(nlohmann::json& jsonConfigFile) const
{
	FGlobalConfig::WriteToJson(jsonConfigFile);

}

void FArcadeZ80ProjectConfig::LoadFromJson(const nlohmann::json& jsonConfig)
{
	FProjectConfig::LoadFromJson(jsonConfig);
	// Load project-specific configuration from JSON
}

void FArcadeZ80ProjectConfig::SaveToJson(nlohmann::json& jsonConfig) const
{
	FProjectConfig::SaveToJson(jsonConfig);
	// Save project-specific configuration to JSON
}

FArcadeZ80ProjectConfig* CreateNewArcadeZ80Config(void)
{
	FArcadeZ80ProjectConfig* pNewConfig = new FArcadeZ80ProjectConfig;

	pNewConfig->Name = "ArcadeZ80";
	//pNewConfig->SnapshotFile = "";
	//pNewConfig->pViewerConfig = GetViewConfigForGame(pNewConfig->Name.c_str());

	return pNewConfig;
}

