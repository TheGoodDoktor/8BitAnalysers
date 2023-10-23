#pragma once

#include "Misc/GlobalConfig.h"

// Spectrum specific config
struct FZXSpectrumConfig : public FGlobalConfig
{
	std::string			SnapshotFolder128 = "./Games128/";
	std::string			PokesFolder = "./Pokes/";
	std::string			RZXFolder = "./RZX/";

protected:

	void ReadFromJson(const nlohmann::json& jsonConfigFile) override;
	void WriteToJson(nlohmann::json& jsonConfigFile) const override;
};
