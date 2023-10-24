#include "GameConfig.h"

#include "Util/FileUtil.h"
#include "json.hpp"
#include <iomanip>
#include <fstream>
#include <sstream>

#include "CPCEmu.h"
#if SPECCY
#include "GameViewers/GameViewer.h"
#include "SnapshotLoaders/GamesList.h"
#endif
#include "Debug/DebugLog.h"
#include "Util/Misc.h"
#include <Util/GraphicsView.h>
#include "CPCConfig.h"

using json = nlohmann::json;
static std::vector< FGameConfig *>	g_GameConfigs;

bool AddGameConfig(FGameConfig *pConfig)
{
	for (const auto& pGameConfig : GetGameConfigs())
	{
		// Dont add game configs with identical names
		if (pGameConfig->Name == pConfig->Name)
			return false;
	}

	g_GameConfigs.push_back(pConfig);
	return true;
}

const std::vector< FGameConfig *>& GetGameConfigs()
{
	return g_GameConfigs;
}

bool RemoveGameConfig(const char* pName)
{
	for (std::vector< FGameConfig*>::iterator it = g_GameConfigs.begin(); it != g_GameConfigs.end(); ++it)
	{
		FGameConfig* pConfig = *it;
		if (pConfig->Name == pName)
		{
			g_GameConfigs.erase(it);
			return true;
		}
	}
	return false;
}

FGameConfig * CreateNewGameConfigFromSnapshot(const FGameSnapshot& snapshot)
{
	FGameConfig *pNewConfig = new FGameConfig;

	pNewConfig->Name = RemoveFileExtension(snapshot.DisplayName.c_str());
	pNewConfig->SnapshotFile = GetFileFromPath(snapshot.FileName.c_str());
	
#if SPECCY
	pNewConfig->pViewerConfig = GetViewConfigForGame(pNewConfig->Name.c_str());
#endif

	return pNewConfig;
}

bool SaveGameConfigToFile(const FGameConfig &config, const char *fname)
{
	json jsonConfigFile;
	jsonConfigFile["Name"] = config.Name;

	jsonConfigFile["SnapshotFile"] = config.SnapshotFile;
	jsonConfigFile["128KGame"] = config.Cpc6128Game;

#if SPECCY
	for (const auto&sprConfigIt : config.SpriteConfigs)
	{
		json spriteConfig;
		const FSpriteDefConfig& spriteDefConfig = sprConfigIt.second;
		
		spriteConfig["Name"] = sprConfigIt.first;
		spriteConfig["BaseAddress"] = MakeHexString(spriteDefConfig.BaseAddress);
		spriteConfig["Count"] = spriteDefConfig.Count;
		spriteConfig["Width"] = spriteDefConfig.Width;
		spriteConfig["Height"] = spriteDefConfig.Height;

		jsonConfigFile["SpriteConfigs"].push_back(spriteConfig);
	}
#endif

	// save character sets

	// save options
	json optionsJson;

#if SPECCY
	optionsJson["WriteSnapshot"] = config.WriteSnapshot;
#endif

	// Output view options
	for (int i = 0; i < FCodeAnalysisState::kNoViewStates; i++)
	{
		const FCodeAnalysisViewConfig& viewConfig = config.ViewConfigs[i];
		json viewConfigJson;
		viewConfigJson["Enabled"] = viewConfig.bEnabled;
		viewConfigJson["ViewAddress"] = viewConfig.ViewAddress.Address;
		viewConfigJson["ViewAddressBank"] = viewConfig.ViewAddress.BankId;

		optionsJson["ViewConfigs"].push_back(viewConfigJson);
	}

	jsonConfigFile["Options"] = optionsJson;

	std::ofstream outFileStream(fname);
	if (outFileStream.is_open())
	{
		outFileStream << std::setw(4) << jsonConfigFile << std::endl;
		return true;
	}

	return false;
}


bool LoadGameConfigFromFile(const FCodeAnalysisState& state, FGameConfig& config, const char* fname)
{
	std::ifstream inFileStream(fname);
	if (inFileStream.is_open() == false)
		return false;

	json jsonConfigFile;

	inFileStream >> jsonConfigFile;
	inFileStream.close();

	config.Name = jsonConfigFile["Name"].get<std::string>();
	config.Cpc6128Game = jsonConfigFile["128KGame"];

	if (jsonConfigFile["SnapshotFile"].is_null() == false)
	{
		config.SnapshotFile = GetFileFromPath(jsonConfigFile["SnapshotFile"].get<std::string>().c_str());
	}

#if SPECCY
	config.pViewerConfig = GetViewConfigForGame(config.Name.c_str());

	for(const auto & jsonSprConfig : jsonConfigFile["SpriteConfigs"])
	{
		const std::string &name = jsonSprConfig["Name"].get<std::string>();
		
		FSpriteDefConfig &sprConfig = config.SpriteConfigs[name];
		sprConfig.BaseAddress = ParseHexString16bit(jsonSprConfig["BaseAddress"].get<std::string>());
		sprConfig.Count = jsonSprConfig["Count"].get<int>();
		sprConfig.Width = jsonSprConfig["Width"].get<int>();
		sprConfig.Height = jsonSprConfig["Height"].get<int>();
	}
#endif

	// load options
	if (jsonConfigFile.contains("Options"))
	{
		const json& optionsJson = jsonConfigFile["Options"];
#if SPECCY
		if(optionsJson.contains("WriteSnapshot"))
			config.WriteSnapshot = optionsJson["WriteSnapshot"];
#endif
		if (optionsJson.contains("EnableCodeAnalysisView"))
		{
			for (int i = 0; i < FCodeAnalysisState::kNoViewStates; i++)
				config.ViewConfigs[i].bEnabled = optionsJson["EnableCodeAnalysisView"][i];
		}
		else if (optionsJson.contains("ViewConfigs"))
		{
			for (int i = 0; i < FCodeAnalysisState::kNoViewStates; i++)
			{
				FCodeAnalysisViewConfig& viewConfig = config.ViewConfigs[i];
				const json& viewConfigJson = optionsJson["ViewConfigs"][i];
				viewConfig.bEnabled = viewConfigJson["Enabled"];
				viewConfig.ViewAddress.Address = viewConfigJson["ViewAddress"];
				if (viewConfigJson.contains("ViewAddressBank"))
					viewConfig.ViewAddress.BankId = viewConfigJson["ViewAddressBank"];
				else
					viewConfig.ViewAddress.BankId = -1;
			}
		}
	}

	return true;
}

bool LoadGameConfigs(FCpcEmu *pEmu)
{
	FDirFileList listing;

	const std::string root = pEmu->pGlobalConfig->WorkspaceRoot;
	const std::string configDir = root + "Configs/";

	if (EnumerateDirectory(configDir.c_str(), listing) == false)
		return false;

	for (const auto &file : listing)
	{
		const std::string &fn = configDir + file.FileName;
		if ((fn.substr(fn.find_last_of(".") + 1) == "json"))
		{
			FGameConfig* pNewConfig = new FGameConfig;
			if (LoadGameConfigFromFile(pEmu->CodeAnalysis, *pNewConfig, fn.c_str()))
			{
				if (pNewConfig->Cpc6128Game == (pEmu->CpcEmuState.type == CPC_TYPE_6128))
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