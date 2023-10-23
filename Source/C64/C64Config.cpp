#include "C64Config.h"

#include "json.hpp"
#include "C64GamesList.h"
#include "C64Emulator.h"
#include <Util/FileUtil.h>

void FC64Config::ReadFromJson(const nlohmann::json& jsonConfigFile)
{
	FGlobalConfig::ReadFromJson(jsonConfigFile);

	if (jsonConfigFile.contains("TapesFolder"))
		TapesFolder = jsonConfigFile["TapesFolder"];
	if (jsonConfigFile.contains("DisksFolder"))
		DisksFolder = jsonConfigFile["DisksFolder"];
	if (jsonConfigFile.contains("PrgFolder"))
		PrgFolder = jsonConfigFile["PrgFolder"];
	if (jsonConfigFile.contains("ShowHCounter"))
		bShowHCounter = jsonConfigFile["ShowHCounter"];

	// fixup paths
	if (TapesFolder.back() != '/')
		TapesFolder += "/";
	if (DisksFolder.back() != '/')
		DisksFolder += "/";
	if (PrgFolder.back() != '/')
		PrgFolder += "/";
}

void FC64Config::WriteToJson(nlohmann::json& jsonConfigFile) const
{
	FGlobalConfig::WriteToJson(jsonConfigFile);
	jsonConfigFile["TapesFolder"] = TapesFolder;
	jsonConfigFile["DisksFolder"] = DisksFolder;
	jsonConfigFile["PrgFolder"] = PrgFolder;
	jsonConfigFile["ShowHCounter"] = bShowHCounter;
}

void FC64GameConfig::LoadFromJson(const nlohmann::json& jsonConfig)
{
	FGameConfig::LoadFromJson(jsonConfig);
}

void FC64GameConfig::SaveToJson(nlohmann::json& jsonConfig) const 
{
	FGameConfig::SaveToJson(jsonConfig);

}

bool LoadC64GameConfigs(FC64Emulator* pC64Emu)
{
	FDirFileList listing;

	const std::string root = pC64Emu->GetGlobalConfig()->WorkspaceRoot;
	const std::string configDir = root + "Configs/";

	if (EnumerateDirectory(configDir.c_str(), listing) == false)
		return false;

	for (const auto& file : listing)
	{
		const std::string& fn = configDir + file.FileName;
		if ((fn.substr(fn.find_last_of(".") + 1) == "json"))
		{
			FC64GameConfig* pNewConfig = new FC64GameConfig;
			if (LoadGameConfigFromFile(*pNewConfig, fn.c_str()))
			{
				AddGameConfig(pNewConfig);
			}
			else
			{
				delete pNewConfig;
			}
		}
	}
	return true;
}

FC64GameConfig* CreateNewC64GameConfigFromGameInfo(const FGameInfo& gameInfo)
{
	FC64GameConfig* pNewConfig = new FC64GameConfig;

	pNewConfig->Name = gameInfo.Name;
	pNewConfig->SnapshotFile = GetFileFromPath(gameInfo.PRGFile.c_str());
	//pNewConfig->pViewerConfig = GetViewConfigForGame(pNewConfig->Name.c_str());

	return pNewConfig;
}