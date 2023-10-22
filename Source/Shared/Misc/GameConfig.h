#pragma once
#include <string>
#include <map>
#include <vector>

#include <Util/Misc.h>
#include <CodeAnalyser/CodeAnalyser.h>

#include <json_fwd.hpp>

class FCodeAnalysisState;

struct FCodeAnalysisViewConfig
{
	bool		bEnabled = false;
	FAddressRef	ViewAddress;
};

struct FGameConfig
{
	FGameConfig()
	{
		ViewConfigs[0].bEnabled = true;
	}

	virtual void	LoadFromJson(const nlohmann::json& jsonConfig);
	virtual void	SaveToJson(nlohmann::json& jsonConfig) const;
	std::string		Name;
	std::string		Machine;
	std::string		SnapshotFile;

	FCodeAnalysisViewConfig	ViewConfigs[FCodeAnalysisState::kNoViewStates];
};

bool AddGameConfig(FGameConfig *pConfig);
bool RemoveGameConfig(const char* pName);
FGameConfig* GetGameConfigForName(const char* pName);
FGameConfig* GetGameConfigForSnapshot(const char* pName);
const std::vector< FGameConfig *>& GetGameConfigs();

bool SaveGameConfigToFile(const FGameConfig &config, const char *fname);
bool LoadGameConfigFromFile(FGameConfig &config, const char *fname);



