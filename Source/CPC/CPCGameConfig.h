#pragma once

#include <Misc/GameConfig.h>
#include "CPCEmu.h"    // for ECPCModel enum

struct FGame;
struct FEmulatorFile;
class FCPCEmu;

// CPC specific
struct FCPCProjectConfig : FProjectConfig
{
	void	LoadFromJson(const nlohmann::json& jsonConfig) override;
	void	SaveToJson(nlohmann::json& jsonConfig) const override;

	ECPCModel  GetCPCModel() const { return bCPC6128Game ? ECPCModel::CPC_6128 : ECPCModel::CPC_464; }

	bool	bCPC6128Game = true;
};

FCPCProjectConfig* CreateNewCPCProjectConfigFromEmulatorFile(const FEmulatorFile& snapshot);
FCPCProjectConfig* CreateNewAmstradBasicConfig(bool bIs6128);

bool LoadCPCGameConfigs(FCPCEmu* pUI);
