#pragma once

#include "Misc/GlobalConfig.h"

// PCE specific config
struct FPCEConfig : public FGlobalConfig
{
    bool Init(void) override;

protected:

	void ReadFromJson(const nlohmann::json& jsonConfigFile) override;
	void WriteToJson(nlohmann::json& jsonConfigFile) const override;

private:

	// Ensure that paths have a trailing slash.
	void FixupPaths(void)
	{
		FGlobalConfig::FixupPaths();
	}

};
