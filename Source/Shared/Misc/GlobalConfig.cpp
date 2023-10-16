#include "GlobalConfig.h"

#include "json.hpp"

#include <iomanip>
#include <fstream>
#include <sstream>

using json = nlohmann::json;


FGlobalConfig	g_GlobalConfig;

void FGlobalConfig::ReadFromJson(const json& jsonConfigFile)
{
	bEnableAudio = jsonConfigFile["EnableAudio"];
	bShowScanLineIndicator = jsonConfigFile["ShowScanlineIndicator"];
	if (jsonConfigFile.contains("ShowOpcodeValues"))
		bShowOpcodeValues = jsonConfigFile["ShowOpcodeValues"];
	LastGame = jsonConfigFile["LastGame"];
	NumberDisplayMode = (ENumberDisplayMode)jsonConfigFile["NumberMode"];
	if (jsonConfigFile.contains("BranchLinesDisplayMode"))
		BranchLinesDisplayMode = jsonConfigFile["BranchLinesDisplayMode"];
	if (jsonConfigFile.contains("WorkspaceRoot"))
		WorkspaceRoot = jsonConfigFile["WorkspaceRoot"];
	if (jsonConfigFile.contains("SnapshotFolder"))
		SnapshotFolder = jsonConfigFile["SnapshotFolder"];

	if (jsonConfigFile.contains("Font"))
		Font = jsonConfigFile["Font"];
	if (jsonConfigFile.contains("FontSizePixels"))
		FontSizePixels = jsonConfigFile["FontSizePixels"];
	
	// fixup paths
	if (WorkspaceRoot.back() != '/')
		WorkspaceRoot += "/";
	if (SnapshotFolder.back() != '/')
		SnapshotFolder += "/";
}

void FGlobalConfig::WriteToJson(json& jsonConfigFile)
{
	jsonConfigFile["EnableAudio"] = bEnableAudio;
	jsonConfigFile["ShowScanlineIndicator"] = bShowScanLineIndicator;
	jsonConfigFile["ShowOpcodeValues"] = bShowOpcodeValues;
	jsonConfigFile["LastGame"] = LastGame;
	jsonConfigFile["NumberMode"] = (int)NumberDisplayMode;
	jsonConfigFile["BranchLinesDisplayMode"] = BranchLinesDisplayMode;
	jsonConfigFile["WorkspaceRoot"] = WorkspaceRoot;
	jsonConfigFile["SnapshotFolder"] = SnapshotFolder;
	jsonConfigFile["Font"] = Font;
	jsonConfigFile["FontSizePixels"] = FontSizePixels;
}


bool	FGlobalConfig::Load(const char* filename)
{
	std::ifstream inFileStream(filename);
	if (inFileStream.is_open() == false)
		return false;

	json jsonConfigFile;

	inFileStream >> jsonConfigFile;
	inFileStream.close();

	ReadFromJson(jsonConfigFile);
	return true;
}

bool	FGlobalConfig::Save(const char* filename)
{
	json jsonConfigFile;

	WriteToJson(jsonConfigFile);

	std::ofstream outFileStream(filename);
	if (outFileStream.is_open())
	{
		outFileStream << std::setw(4) << jsonConfigFile << std::endl;
		return true;
	}

	return false;
}



FGlobalConfig& GetGlobalConfig()
{
	return g_GlobalConfig;
}
#if 0
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
	if (jsonConfigFile.contains("BranchLinesDisplayMode"))
		config.BranchLinesDisplayMode = jsonConfigFile["BranchLinesDisplayMode"];
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

	if (jsonConfigFile.contains("Font"))
		config.Font = jsonConfigFile["Font"];
	if (jsonConfigFile.contains("FontSizePixels"))
		config.FontSizePixels = jsonConfigFile["FontSizePixels"];

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
	jsonConfigFile["BranchLinesDisplayMode"] = config.BranchLinesDisplayMode;
	jsonConfigFile["WorkspaceRoot"] = config.WorkspaceRoot;
	jsonConfigFile["SnapshotFolder"] = config.SnapshotFolder;
	jsonConfigFile["SnapshotFolder128"] = config.SnapshotFolder128;
	jsonConfigFile["PokesFolder"] = config.PokesFolder;
	jsonConfigFile["RZXFolder"] = config.RZXFolder;
	jsonConfigFile["Font"] = config.Font;
	jsonConfigFile["FontSizePixels"] = config.FontSizePixels;

	std::ofstream outFileStream(fileName);
	if (outFileStream.is_open())
	{
		outFileStream << std::setw(4) << jsonConfigFile << std::endl;
		return true;
	}

	return false;
}

#endif