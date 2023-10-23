#pragma once

#include "Misc/GlobalConfig.h"
#include "Misc/GameConfig.h"

class FC64Emulator;
struct FGameInfo;

// C64 specific configs

struct FC64Config : public FGlobalConfig
{
	std::string			TapesFolder = "./Tapes/";
	std::string			DisksFolder = "./Disks/";
	std::string			PrgFolder = "./Prg/";

	bool		bShowHCounter = false;

protected:

	void ReadFromJson(const nlohmann::json& jsonConfigFile) override;
	void WriteToJson(nlohmann::json& jsonConfigFile) const override;
};

struct FC64GameConfig : public FGameConfig
{
	void	LoadFromJson(const nlohmann::json& jsonConfig)override;
	void	SaveToJson(nlohmann::json& jsonConfig) const override;
};

bool LoadC64GameConfigs(FC64Emulator* pC64Emu);

FC64GameConfig* CreateNewC64GameConfigFromGameInfo(const FGameInfo& gameInfo);