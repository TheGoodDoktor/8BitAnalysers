#pragma once

#include <Misc/GameConfig.h>

struct FGame;
struct FEmulatorFile;
class FCPCEmu;

// CPC specific
struct FCPCProjectConfig : FProjectConfig
{
	void	LoadFromJson(const nlohmann::json& jsonConfig) override;
	void	SaveToJson(nlohmann::json& jsonConfig) const override;

	bool	bCPC6128Game = false;
};

FCPCProjectConfig* CreateNewCPCProjectConfigFromEmulatorFile(const FEmulatorFile& snapshot);
FCPCProjectConfig* CreateNewAmstradBasicConfig(void);
bool LoadCPCGameConfigs(FCPCEmu* pUI);
