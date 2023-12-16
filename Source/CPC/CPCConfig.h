#pragma once

#include "Misc/GlobalConfig.h"

// CPC specific config
struct FCPCConfig : public FGlobalConfig
{
	FCPCConfig();

	std::string					SnapshotFolder128 = "./Games128/";
	std::vector<std::string>	UpperROMSlot;
protected:

	void ReadFromJson(const nlohmann::json& jsonConfigFile) override;
	void WriteToJson(nlohmann::json& jsonConfigFile) const override;
};
