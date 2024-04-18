#pragma once
#include <string>
#include <map>
#include <vector>

#include <Util/Misc.h>
#include <CodeAnalyser/CodeAnalyser.h>
#include "GamesList.h"

#include <json_fwd.hpp>

class FCodeAnalysisState;

struct FCodeAnalysisViewConfig
{
	bool		bEnabled = false;
	FAddressRef	ViewAddress;
};

struct FProjectConfig
{
	FProjectConfig()
	{
		ViewConfigs[0].bEnabled = true;
	}

	virtual void	LoadFromJson(const nlohmann::json& jsonConfig);
	virtual void	SaveToJson(nlohmann::json& jsonConfig) const;
	virtual void	FixupAddressRefs(FCodeAnalysisState& state);

	bool AddLuaSourceFile(const char *pFilename);
	std::string		Name;
	std::string		Machine;
	FEmulatorFile	EmulatorFile;

	std::vector<std::string>	LuaSourceFiles;

	FCodeAnalysisViewConfig	ViewConfigs[FCodeAnalysisState::kNoViewStates];
};

bool AddGameConfig(FProjectConfig *pConfig);
bool RemoveGameConfig(const char* pName);
FProjectConfig* GetGameConfigForName(const char* pName);
FProjectConfig* GetGameConfigForSnapshot(const char* pName);
const std::vector< FProjectConfig *>& GetGameConfigs();

bool SaveGameConfigToFile(const FProjectConfig &config, const char *fname);
bool LoadGameConfigFromFile(FProjectConfig &config, const char *fname);



