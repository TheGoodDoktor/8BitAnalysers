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

	if (jsonConfigFile.contains("BiosPath"))
		BiosPath = jsonConfigFile["BiosPath"];

	if (jsonConfigFile.contains("BiosFilename"))
		BiosFilename = jsonConfigFile["BiosFilename"];
#endif

	if (jsonConfigFile.contains("GameDbPath"))
		GameDbPath = jsonConfigFile["GameDbPath"];

	if (jsonConfigFile.contains("ValidatorPath"))
		ValidatorPath = jsonConfigFile["ValidatorPath"];
	
	if (jsonConfigFile.contains("AsmExportValidator"))
		bUseAsmExportValidator = jsonConfigFile["AsmExportValidator"];

	FixupPaths();
}

void FPCEConfig::WriteToJson(nlohmann::json& jsonConfigFile) const
{
	FGlobalConfig::WriteToJson(jsonConfigFile);

#if CDROM_SUPPORT
	jsonConfigFile["CdRomFolder"] = CdRomFolder;
	jsonConfigFile["BiosPath"] = BiosPath;
	jsonConfigFile["BiosFilename"] = BiosFilename;
#endif
	jsonConfigFile["GameDbPath"] = GameDbPath;

#ifndef NDEBUG
	jsonConfigFile["ValidatorPath"] = ValidatorPath;
	jsonConfigFile["AsmExportValidator"] = bUseAsmExportValidator;
#endif
}

void FPCEConfig::FixupPaths(void)
{
	FGlobalConfig::FixupPaths();

#if CDROM_SUPPORT
	if (CdRomFolder.back() != '/')
		CdRomFolder += "/";

	if (BiosPath.back() != '/')
		BiosPath += "/";
#endif
	if (ValidatorPath.back() != '/')
		ValidatorPath += "/";

	if (GameDbPath.back() != '/')
		GameDbPath += "/";
}
