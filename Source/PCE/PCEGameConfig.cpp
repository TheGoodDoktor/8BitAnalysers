#include "PCEGameConfig.h"

#include <json.hpp>
#include <iomanip>
#include <fstream>
#include <sstream>

#include <Util/FileUtil.h>
#include "PCEConfig.h"
#include "PCEEmu.h"

// PCE specific

void FPCEGameConfig::LoadFromJson(const nlohmann::json& jsonConfigFile)
{
	FProjectConfig::LoadFromJson(jsonConfigFile);

	if(EmulatorFile.ListName.empty())
		EmulatorFile.ListName = "Snapshot File";
}

void FPCEGameConfig::SaveToJson(nlohmann::json& jsonConfigFile) const
{
	FProjectConfig::SaveToJson(jsonConfigFile);
}

FPCEGameConfig* CreateNewPCEGameConfigFromSnapshot(const FEmulatorFile& snapshot)
{
	FPCEGameConfig* pNewConfig = new FPCEGameConfig;

	pNewConfig->Name = RemoveFileExtension(snapshot.DisplayName.c_str());
	pNewConfig->EmulatorFile = snapshot;

	return pNewConfig;
}

FPCEGameConfig* CreateNewEmptyConfig(void)
{
	FPCEGameConfig* pNewConfig = new FPCEGameConfig;

	pNewConfig->Name = "No Project";

	return pNewConfig;
}

bool LoadPCEGameConfigs(FPCEEmu* pEmu)
{
	FDirFileList listing;
	
	const std::string root = pEmu->GetGlobalConfig()->WorkspaceRoot;

	// Search through each game directory
	if (EnumerateDirectory(root.c_str(), listing) == false)
		return false;

	for (const auto& file : listing)
	{
		if (file.FileType == FDirEntry::Directory)
		{
			if(file.FileName == "." || file.FileName == "..")
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
						FPCEGameConfig* pNewConfig = new FPCEGameConfig;
						if (LoadGameConfigFromFile(*pNewConfig, configFileName.c_str()))
						{
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

	return true;
}
