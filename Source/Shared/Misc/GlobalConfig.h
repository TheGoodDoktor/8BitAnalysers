#pragma once

#include <string>
#include "Util/Misc.h"
#include <json_fwd.hpp>

struct FGlobalConfig
{
	bool				bEnableAudio;
	bool				bShowScanLineIndicator = false;
	bool				bShowOpcodeValues = false;
	ENumberDisplayMode	NumberDisplayMode = ENumberDisplayMode::HexAitch;
	int					BranchLinesDisplayMode = 1;
	std::string			LastGame;

	std::string			WorkspaceRoot = "./";
	std::string			SnapshotFolder = "./Games/";

	std::string			Font = ""; // if no font is specified the default font will be used
	uint32_t			FontSizePixels = 13;

	bool	Load(const char* filename);
	bool	Save(const char* filename);
protected:
	virtual void ReadFromJson(const nlohmann::json& jsonConfigFile);
	virtual void WriteToJson(nlohmann::json& jsonConfigFile);

	//virtual bool	Load(const char* pFileName);
	//virtual bool	Save(const char *pFileName);
};



//FGlobalConfig& GetGlobalConfig();
//bool LoadGlobalConfig(const char* fileName);
//bool SaveGlobalConfig(const char* fileName);
