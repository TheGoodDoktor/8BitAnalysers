#include "PCEConfig.h"

#include "json.hpp"
#include "Util/FileUtil.h"
#include "ProjectDefines.h"

bool FPCEConfig::Init(void)
{
	if(FGlobalConfig::Init() == false)
		return false;

	//LuaBaseFiles.push_back("Lua/PCEBase.lua");
	SnapshotFolder = GetDocumentsPath("PCEGames");
#if CDROM_SUPPORT
	CdRomFolder = GetDocumentsPath("PCECdRoms");
#endif
	WorkspaceRoot = GetDocumentsPath("PCEAnalyserProjects");

	FixupPaths();

	return true;
}

void FPCEConfig::ReadFromJson(const nlohmann::json& jsonConfigFile)
{
	FGlobalConfig::ReadFromJson(jsonConfigFile);

#if CDROM_SUPPORT
	if (jsonConfigFile.contains("CdRomFolder"))
		CdRomFolder = jsonConfigFile["CdRomFolder"];

	if (jsonConfigFile.contains("BiosFilePath"))
		BiosFilePath = jsonConfigFile["BiosFilePath"];
#endif

	if (jsonConfigFile.contains("GameDbPath"))
		GameDbFolder = jsonConfigFile["GameDbPath"];

	if (jsonConfigFile.contains("ValidatorFolder"))
		ValidatorFolder = jsonConfigFile["ValidatorFolder"];
	
	if (jsonConfigFile.contains("AsmExportValidator"))
		bUseAsmExportValidator = jsonConfigFile["AsmExportValidator"];

	FixupPaths();
}

void FPCEConfig::WriteToJson(nlohmann::json& jsonConfigFile) const
{
	FGlobalConfig::WriteToJson(jsonConfigFile);

#if CDROM_SUPPORT
	jsonConfigFile["CdRomFolder"] = CdRomFolder;
	jsonConfigFile["BiosFilePath"] = BiosFilePath;
#endif
	jsonConfigFile["GameDbFolder"] = GameDbFolder;

#ifndef NDEBUG
	jsonConfigFile["ValidatorFolder"] = ValidatorFolder;
	jsonConfigFile["AsmExportValidator"] = bUseAsmExportValidator;
#endif
}

void FPCEConfig::FixupPaths(void)
{
	FGlobalConfig::FixupPaths();

#if CDROM_SUPPORT
	if (CdRomFolder.back() != '/')
		CdRomFolder += "/";
#endif
	if (ValidatorFolder.back() != '/')
		ValidatorFolder += "/";

	if (GameDbFolder.back() != '/')
		GameDbFolder += "/";
}
