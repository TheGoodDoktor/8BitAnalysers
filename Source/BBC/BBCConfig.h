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
	std::string			PrgFolder = "./Prg/";
	std::string			CrtFolder = "./Crt/";

	bool		bShowHCounter = false;
	bool		bShowVICOverlay = false;

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

FBBCProjectConfig* CreateNewC64ProjectFromEmuFile(const FEmulatorFile& emuFile);