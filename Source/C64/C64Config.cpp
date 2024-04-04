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
	if (jsonConfigFile.contains("CrtFolder"))
		CrtFolder = jsonConfigFile["CrtFolder"];
	if (jsonConfigFile.contains("ShowHCounter"))
		bShowHCounter = jsonConfigFile["ShowHCounter"];
	if (jsonConfigFile.contains("ShowVICOverlay"))
		bShowVICOverlay = jsonConfigFile["ShowVICOverlay"];

	// fixup paths
	if (TapesFolder.back() != '/')
		TapesFolder += "/";
	if (DisksFolder.back() != '/')
		DisksFolder += "/";
	if (PrgFolder.back() != '/')
		PrgFolder += "/";
	if (CrtFolder.back() != '/')
		CrtFolder += "/";
}

void FC64Config::WriteToJson(nlohmann::json& jsonConfigFile) const
{
	FGlobalConfig::WriteToJson(jsonConfigFile);
	jsonConfigFile["TapesFolder"] = TapesFolder;
	jsonConfigFile["DisksFolder"] = DisksFolder;
	jsonConfigFile["PrgFolder"] = PrgFolder;
	jsonConfigFile["CrtFolder"] = CrtFolder;
	jsonConfigFile["ShowHCounter"] = bShowHCounter;
	jsonConfigFile["ShowVICOverlay"] = bShowVICOverlay;
}

void FC64ProjectConfig::LoadFromJson(const nlohmann::json& jsonConfig)
{
	FProjectConfig::LoadFromJson(jsonConfig);
}

void FC64ProjectConfig::SaveToJson(nlohmann::json& jsonConfig) const 
{
	FProjectConfig::SaveToJson(jsonConfig);

}

bool LoadC64ProjectConfigs(FC64Emulator* pC64Emu)
{
	FDirFileList listing;

	const std::string root = pC64Emu->GetGlobalConfig()->WorkspaceRoot;

	// New method - search through each game directory
	if (EnumerateDirectory(root.c_str(), listing) == false)
		return false;

	for (const auto& file : listing)
	{
		if (file.FileType == FDirEntry::Directory)
		{
			if (file.FileName == "." || file.FileName == "..")
				continue;

			FDirFileList directoryListing;
			std::string gameDir = root + file.FileName;
			if (EnumerateDirectory(gameDir.c_str(), directoryListing))
			{
				for (const auto& gameFile : directoryListing)
				{
					if (gameFile.FileName == "Config.json")
					{
						const std::string& configFileName = gameDir + "/" + gameFile.FileName;
						FC64ProjectConfig* pNewConfig = new FC64ProjectConfig;
						if (LoadGameConfigFromFile(*pNewConfig, configFileName.c_str()))
						{
							//if (pNewConfig->Spectrum128KGame == (pEmu->ZXEmuState.type == ZX_TYPE_128))
							AddGameConfig(pNewConfig);
						}
						else
						{
							delete pNewConfig;
						}

					}
				}
			}
		}
	}

	// Keep old method in for now
	const std::string configDir = root + "Configs/";

	if (EnumerateDirectory(configDir.c_str(), listing) == false)
		return false;

	for (const auto& file : listing)
	{
		const std::string& fn = configDir + file.FileName;
		if ((fn.substr(fn.find_last_of(".") + 1) == "json"))
		{
			FC64ProjectConfig* pNewConfig = new FC64ProjectConfig;
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

FC64ProjectConfig* CreateNewC64ProjectFromEmuFile(const FEmulatorFile& emuFile)
{
	FC64ProjectConfig* pNewConfig = new FC64ProjectConfig;

	pNewConfig->Name = emuFile.DisplayName;
	pNewConfig->EmulatorFile = emuFile;

	return pNewConfig;
}