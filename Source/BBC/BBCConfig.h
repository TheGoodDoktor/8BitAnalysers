#pragma once

#include "Misc/GlobalConfig.h"
#include "Misc/GameConfig.h"

class FBBCEmulator;
struct FGameSnapshot;

// C64 specific configs

struct FBBCConfig : public FGlobalConfig
{
	std::string			TapesFolder = "./Tapes/";
	std::string			DisksFolder = "./Disks/";
	std::string			RomFolder = "./Roms/";

	std::string			OSRom = "OS-1.2.rom";
	std::string			BasicRom = "BASIC2.rom";

protected:

	void ReadFromJson(const nlohmann::json& jsonConfigFile) override;
	void WriteToJson(nlohmann::json& jsonConfigFile) const override;
};

struct FBBCProjectConfig : public FProjectConfig
{
	void	LoadFromJson(const nlohmann::json& jsonConfig)override;
	void	SaveToJson(nlohmann::json& jsonConfig) const override;
};

bool LoadBBCProjectConfigs(FBBCEmulator* pC64Emu);

FBBCProjectConfig* CreateNewBBCProjectFromEmuFile(const FEmulatorFile& emuFile);
FBBCProjectConfig* CreateNewBBCBasicConfig(void);
