#pragma once

#include "Misc/GlobalConfig.h"
#include "Misc/GameConfig.h"

class FTubeEliteConfig : public FGlobalConfig
{
protected:

	void ReadFromJson(const nlohmann::json& jsonConfigFile) override;
	void WriteToJson(nlohmann::json& jsonConfigFile) const override;
};

struct FTubeEliteProjectConfig : public FProjectConfig
{
	void	LoadFromJson(const nlohmann::json& jsonConfig) override;
	void	SaveToJson(nlohmann::json& jsonConfig) const override;
};