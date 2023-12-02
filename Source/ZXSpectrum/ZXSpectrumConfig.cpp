#include "ZXSpectrumConfig.h"

#include "json.hpp"
#include "Util/FileUtil.h"

bool FZXSpectrumConfig::Init(void)
{
    if(FGlobalConfig::Init() == false)
        return false;
    
    SnapshotFolder = GetDocumentsPath("SpectrumGames/48K");
    SnapshotFolder128 = GetDocumentsPath("SpectrumGames/128K");
    WorkspaceRoot = GetDocumentsPath("SpectrumAnalyserProjects");
    
    return true;
}
void FZXSpectrumConfig::ReadFromJson(const nlohmann::json& jsonConfigFile)
{
	FGlobalConfig::ReadFromJson(jsonConfigFile);

	if (jsonConfigFile.contains("SnapshotFolder128"))
		SnapshotFolder128 = jsonConfigFile["SnapshotFolder128"];
	if (jsonConfigFile.contains("PokesFolder"))
		PokesFolder = jsonConfigFile["PokesFolder"];
	if (jsonConfigFile.contains("RZXFolder"))
		RZXFolder = jsonConfigFile["RZXFolder"];

	// fixup paths
	if (SnapshotFolder128.back() != '/')
		SnapshotFolder128 += "/";
	if (PokesFolder.back() != '/')
		PokesFolder += "/";
	if (RZXFolder.back() != '/')
		RZXFolder += "/";
}

void FZXSpectrumConfig::WriteToJson(nlohmann::json& jsonConfigFile) const
{
	FGlobalConfig::WriteToJson(jsonConfigFile);
	jsonConfigFile["SnapshotFolder128"] = SnapshotFolder128;
	jsonConfigFile["PokesFolder"] = PokesFolder;
	jsonConfigFile["RZXFolder"] = RZXFolder;
}
