#pragma once

#include <Misc/GameConfig.h>

struct FGame;
struct FGameSnapshot;
class FCpcEmu;

// CPC specific
struct FCPCGameConfig : FGameConfig
{
	void	LoadFromJson(const nlohmann::json& jsonConfig) override;
	void	SaveToJson(nlohmann::json& jsonConfig) const override;

	bool	bCPC6128Game = false;
};

FCPCGameConfig* CreateNewCPCGameConfigFromSnapshot(const FGameSnapshot& snapshot);
bool LoadCPCGameConfigs(FCpcEmu* pUI);
