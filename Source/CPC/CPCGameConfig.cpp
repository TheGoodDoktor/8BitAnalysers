#include "CPCGameConfig.h"

#include <json.hpp>
#include <iomanip>
#include <fstream>
#include <sstream>

#include <Util/FileUtil.h>
#include "CPCConfig.h"
#include "CPCEmu.h"

// CPC specific

void FCPCGameConfig::LoadFromJson(const nlohmann::json& jsonConfigFile)
{
	FGameConfig::LoadFromJson(jsonConfigFile);

	if (jsonConfigFile.contains("128KGame"))
		bCPC6128Game = jsonConfigFile["128KGame"];

}

void FCPCGameConfig::SaveToJson(nlohmann::json& jsonConfigFile) const
{
	FGameConfig::SaveToJson(jsonConfigFile);

	jsonConfigFile["128KGame"] = bCPC6128Game;
}

FCPCGameConfig* CreateNewCPCGameConfigFromSnapshot(const FGameSnapshot& snapshot)
{
	FCPCGameConfig* pNewConfig = new FCPCGameConfig;

	pNewConfig->Name = RemoveFileExtension(snapshot.DisplayName.c_str());
	pNewConfig->SnapshotFile = GetFileFromPath(snapshot.FileName.c_str());

	return pNewConfig;
}

FCPCGameConfig* CreateNewAmstradBasicConfig(void)
{
	FCPCGameConfig* pNewConfig = new FCPCGameConfig;

	pNewConfig->Name = "AmstradBasic";
	pNewConfig->SnapshotFile = "";

	return pNewConfig;
}

bool LoadCPCGameConfigs(FCPCEmu* pEmu)
{
	FDirFileList listing;

	const std::string root = pEmu->GetGlobalConfig()->WorkspaceRoot;

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
						FCPCGameConfig* pNewConfig = new FCPCGameConfig;
						if (LoadGameConfigFromFile(*pNewConfig, configFileName.c_str()))
						{
							//if (pNewConfig->bCPC6128Game == (pEmu->CPCEmuState.type == CPC_TYPE_6128))
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

	// Old method - keep this in while we have need of it
	const std::string configDir = root + "Configs/";

	if (EnumerateDirectory(configDir.c_str(), listing) == false)
		return false;

	for (const auto& file : listing)
	{
		const std::string& fn = configDir + file.FileName;
		if ((fn.substr(fn.find_last_of(".") + 1) == "json"))
		{
			FCPCGameConfig* pNewConfig = new FCPCGameConfig;
			if (LoadGameConfigFromFile(*pNewConfig, fn.c_str()))
			{
				if (pNewConfig->bCPC6128Game == (pEmu->CPCEmuState.type == CPC_TYPE_6128))
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