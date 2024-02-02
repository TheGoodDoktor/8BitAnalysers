#include "ZXSpectrumConfig.h"

#include "json.hpp"
#include "Util/FileUtil.h"

bool FZXSpectrumConfig::Init(void)
{
	if(FGlobalConfig::Init() == false)
		return false;

	LuaBaseFiles.push_back("Lua/ZXBase.lua");
	SnapshotFolder = GetDocumentsPath("SpectrumGames");
	//SnapshotFolder128 = GetDocumentsPath("SpectrumGames/128K");
	WorkspaceRoot = GetDocumentsPath("SpectrumAnalyserProjects");

	FixupPaths();

	return true;
}

void FZXSpectrumConfig::ReadFromJson(const nlohmann::json& jsonConfigFile)
{
	FGlobalConfig::ReadFromJson(jsonConfigFile);

	//if (jsonConfigFile.contains("SnapshotFolder128"))
	//	SnapshotFolder128 = jsonConfigFile["SnapshotFolder128"];
	if (jsonConfigFile.contains("PokesFolder"))
		PokesFolder = jsonConfigFile["PokesFolder"];
	if (jsonConfigFile.contains("RZXFolder"))
		RZXFolder = jsonConfigFile["RZXFolder"];

	FixupPaths();
}

void FZXSpectrumConfig::WriteToJson(nlohmann::json& jsonConfigFile) const
{
	FGlobalConfig::WriteToJson(jsonConfigFile);
	//jsonConfigFile["SnapshotFolder128"] = SnapshotFolder128;
	jsonConfigFile["PokesFolder"] = PokesFolder;
	jsonConfigFile["RZXFolder"] = RZXFolder;
}
