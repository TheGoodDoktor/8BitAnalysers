#pragma once

#include "Misc/GlobalConfig.h"

// Spectrum specific config
struct FZXSpectrumConfig : public FGlobalConfig
{
    bool Init(void) override;
	std::string			SnapshotFolder128 = "./Games128/";
	std::string			PokesFolder = "./Pokes/";
	std::string			RZXFolder = "./RZX/";

protected:

	void ReadFromJson(const nlohmann::json& jsonConfigFile) override;
	void WriteToJson(nlohmann::json& jsonConfigFile) const override;

private:

	// Ensure that paths have a trailing slash.
	void FixupPaths(void)
	{
		FGlobalConfig::FixupPaths();

		if (SnapshotFolder128.back() != '/')
			SnapshotFolder128 += "/";
		if (PokesFolder.back() != '/')
			PokesFolder += "/";
		if (RZXFolder.back() != '/')
			RZXFolder += "/";
	}

};
