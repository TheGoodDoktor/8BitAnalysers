#pragma once

#include "Misc/GlobalConfig.h"

// CPC specific config
struct FCPCConfig : public FGlobalConfig
{
	std::string			SnapshotFolder128 = "./Games128/";

protected:

	void ReadFromJson(const nlohmann::json& jsonConfigFile) override;
	void WriteToJson(nlohmann::json& jsonConfigFile) override;
};
