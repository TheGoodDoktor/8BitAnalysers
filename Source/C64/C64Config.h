#pragma once

#include "Misc/GlobalConfig.h"

// Spectrum specific config
struct FC64Config : public FGlobalConfig
{
	std::string			TapesFolder = "./Tapes/";
	std::string			DisksFolder = "./Disks/";
	std::string			PrgFolder = "./Prg/";

protected:

	void ReadFromJson(const nlohmann::json& jsonConfigFile) override;
	void WriteToJson(nlohmann::json& jsonConfigFile) override;
};
