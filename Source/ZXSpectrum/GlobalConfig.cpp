#include "GlobalConfig.h"

#include "json.hpp"

#include <iomanip>
#include <fstream>
#include <sstream>

using json = nlohmann::json;


FGlobalConfig	g_GlobalConfig;

FGlobalConfig& GetGlobalConfig()
{
	return g_GlobalConfig;
}

bool LoadGlobalConfig(const char* fileName)
{
	FGlobalConfig& config = GetGlobalConfig();

	std::ifstream inFileStream(fileName);
	if (inFileStream.is_open() == false)
		return false;

	json jsonConfigFile;

	inFileStream >> jsonConfigFile;
	inFileStream.close();

	config.bEnableAudio = jsonConfigFile["EnableAudio"];
	config.bShowScanLineIndicator = jsonConfigFile["ShowScanlineIndicator"];
	if(jsonConfigFile.contains("ShowOpcodeValues"))
		config.bShowOpcodeValues = jsonConfigFile["ShowOpcodeValues"];
	config.LastGame = jsonConfigFile["LastGame"];
	config.NumberDisplayMode = (ENumberDisplayMode)jsonConfigFile["NumberMode"];

	if(jsonConfigFile.contains("WorkspaceRoot"))
		config.WorkspaceRoot = jsonConfigFile["WorkspaceRoot"];
	if (jsonConfigFile.contains("SnapshotFolder"))
		config.SnapshotFolder = jsonConfigFile["SnapshotFolder"];
	if (jsonConfigFile.contains("SnapshotFolder128"))
		config.SnapshotFolder128 = jsonConfigFile["SnapshotFolder128"];
	if (jsonConfigFile.contains("PokesFolder"))
		config.PokesFolder = jsonConfigFile["PokesFolder"];
	if (jsonConfigFile.contains("RZXFolder"))
		config.RZXFolder = jsonConfigFile["RZXFolder"];

	if(config.WorkspaceRoot.back() != '/')
		config.WorkspaceRoot += "/";
	if (config.SnapshotFolder.back() != '/')
		config.SnapshotFolder += "/";
	if (config.SnapshotFolder128.back() != '/')
		config.SnapshotFolder128 += "/";
	if (config.PokesFolder.back() != '/')
		config.PokesFolder += "/";
	if (config.RZXFolder.back() != '/')
		config.RZXFolder += "/";

	return true;
}

bool SaveGlobalConfig(const char* fileName)
{
	const FGlobalConfig& config = GetGlobalConfig();
	json jsonConfigFile;

	jsonConfigFile["EnableAudio"] = config.bEnableAudio;
	jsonConfigFile["ShowScanlineIndicator"] = config.bShowScanLineIndicator;
	jsonConfigFile["ShowOpcodeValues"] = config.bShowOpcodeValues;
	jsonConfigFile["LastGame"] = config.LastGame;
	jsonConfigFile["NumberMode"] = (int)config.NumberDisplayMode;
	jsonConfigFile["WorkspaceRoot"] = config.WorkspaceRoot;
	jsonConfigFile["SnapshotFolder"] = config.SnapshotFolder;
	jsonConfigFile["SnapshotFolder128"] = config.SnapshotFolder128;
	jsonConfigFile["PokesFolder"] = config.PokesFolder;
	jsonConfigFile["RZXFolder"] = config.RZXFolder;

	std::ofstream outFileStream(fileName);
	if (outFileStream.is_open())
	{
		outFileStream << std::setw(4) << jsonConfigFile << std::endl;
		return true;
	}

	return false;
}