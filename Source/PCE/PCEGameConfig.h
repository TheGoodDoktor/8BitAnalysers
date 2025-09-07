#pragma once

#include <Misc/GameConfig.h>

struct FGame;
struct FEmulatorFile;

struct FPCEGameConfig;
class FPCEEmu;

// PCE specific
struct FPCEGameConfig : FProjectConfig
{
	void	LoadFromJson(const nlohmann::json& jsonConfig) override;
	void	SaveToJson(nlohmann::json& jsonConfig) const override;
};

FPCEGameConfig* CreateNewPCEGameConfigFromSnapshot(const FEmulatorFile& snapshot);
FPCEGameConfig* CreateNewEmptyConfig(void);
bool LoadPCEGameConfigs(FPCEEmu* pUI);
