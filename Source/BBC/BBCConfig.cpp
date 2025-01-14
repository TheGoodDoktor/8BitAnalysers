#include "BBCConfig.h"

#include "json.hpp"

#include "BBCEmulator.h"
#include <Util/FileUtil.h>

void FBBCConfig::ReadFromJson(const nlohmann::json& jsonConfigFile)
{
	FGlobalConfig::ReadFromJson(jsonConfigFile);

	if (jsonConfigFile.contains("TapesFolder"))
		TapesFolder = jsonConfigFile["TapesFolder"];
	if (jsonConfigFile.contains("DisksFolder"))
		DisksFolder = jsonConfigFile["DisksFolder"];
	
	if (jsonConfigFile.contains("RomFolder"))
		RomFolder = jsonConfigFile["RomFolder"];	

	if (jsonConfigFile.contains("OSRom"))
		OSRom = jsonConfigFile["OSRom"];

	if (jsonConfigFile.contains("BasicRom"))
		BasicRom = jsonConfigFile["BasicRom"];

	// fixup paths
	if (TapesFolder.back() != '/')
		TapesFolder += "/";
	if (DisksFolder.back() != '/')
		DisksFolder += "/";
}

void FBBCConfig::WriteToJson(nlohmann::json& jsonConfigFile) const
{
	FGlobalConfig::WriteToJson(jsonConfigFile);
	jsonConfigFile["TapesFolder"] = TapesFolder;
	jsonConfigFile["DisksFolder"] = DisksFolder;
	jsonConfigFile["RomFolder"] = RomFolder;
	jsonConfigFile["OSRom"] = OSRom;
	jsonConfigFile["BasicRom"] = BasicRom;
}

void FBBCProjectConfig::LoadFromJson(const nlohmann::json& jsonConfig)
{
	FProjectConfig::LoadFromJson(jsonConfig);
}

void FBBCProjectConfig::SaveToJson(nlohmann::json& jsonConfig) const
{
	FProjectConfig::SaveToJson(jsonConfig);

}

bool LoadBBCProjectConfigs(FBBCEmulator* pBBCEmu)
{
	FDirFileList listing;

	const std::string root = pBBCEmu->GetGlobalConfig()->WorkspaceRoot;

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
						FBBCProjectConfig* pNewConfig = new FBBCProjectConfig;
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
			FBBCProjectConfig* pNewConfig = new FBBCProjectConfig;
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

FBBCProjectConfig* CreateNewBBCProjectFromEmuFile(const FEmulatorFile& emuFile)
{
	FBBCProjectConfig* pNewConfig = new FBBCProjectConfig;

	pNewConfig->Name = emuFile.DisplayName;
	pNewConfig->EmulatorFile = emuFile;

	return pNewConfig;
}

FBBCProjectConfig* CreateNewBBCBasicConfig(void)
{
	FBBCProjectConfig* pNewConfig = new FBBCProjectConfig;

	pNewConfig->Name = "BBCBasic";
	//pNewConfig->SnapshotFile = "";
	//pNewConfig->pViewerConfig = GetViewConfigForGame(pNewConfig->Name.c_str());

	return pNewConfig;
}