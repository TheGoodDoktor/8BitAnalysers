#pragma once

#include "Misc/GlobalConfig.h"
#include "Misc/GameConfig.h"

class FC64Emulator;
struct FGameSnapshot;

// C64 specific configs

struct FC64Config : public FGlobalConfig
{
	std::string			TapesFolder = "./Tapes/";
	std::string			DisksFolder = "./Disks/";
	std::string			PrgFolder = "./Prg/";
	std::string			CrtFolder = "./Crt/";

	bool		bShowHCounter = false;
	bool		bShowVICOverlay = false;

protected:

	void ReadFromJson(const nlohmann::json& jsonConfigFile) override;
	void WriteToJson(nlohmann::json& jsonConfigFile) const override;
};

struct FC64ProjectConfig : public FProjectConfig
{
	void	LoadFromJson(const nlohmann::json& jsonConfig)override;
	void	SaveToJson(nlohmann::json& jsonConfig) const override;
};

bool LoadC64ProjectConfigs(FC64Emulator* pC64Emu);

FC64ProjectConfig* CreateNewC64ProjectFromEmuFile(const FEmulatorFile& emuFile);