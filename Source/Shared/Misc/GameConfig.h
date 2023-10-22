#pragma once
#include <string>
#include <map>
#include <vector>

#include <Util/Misc.h>
#include <CodeAnalyser/CodeAnalyser.h>

#include <json_fwd.hpp>

class FCodeAnalysisState;
struct FGameViewerData;
class FSpectrumEmu;
struct FGame;
struct FZXSpectrumGameConfig;
struct FGameSnapshot;


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
	std::string		SnapshotFile;

	FCodeAnalysisViewConfig	ViewConfigs[FCodeAnalysisState::kNoViewStates];
};



bool AddGameConfig(FGameConfig *pConfig);
bool RemoveGameConfig(const char* pName);
const std::vector< FGameConfig *>& GetGameConfigs();

bool SaveGameConfigToFile(const FGameConfig &config, const char *fname);
bool LoadGameConfigFromFile(const FCodeAnalysisState& state, FGameConfig &config, const char *fname);



