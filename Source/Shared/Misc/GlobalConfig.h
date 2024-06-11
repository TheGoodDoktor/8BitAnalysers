#pragma once

#include <string>
#include "Util/Misc.h"
#include <json_fwd.hpp>

// Global system config base class
// Individual machine configs will derive from this
struct FGlobalConfig
{
	bool				bEnableAudio;
	bool				bShowScanLineIndicator = false;
	bool				bShowOpcodeValues = false;
	ENumberDisplayMode	NumberDisplayMode = ENumberDisplayMode::HexAitch;
	int					BranchLinesDisplayMode = 1;
	std::string			LastGame;

	std::string			WorkspaceRoot = "./Workspace/";
	std::string			SnapshotFolder = "./Games/";

	std::string			Font = "Cousine-Regular.ttf"; // if no font is specified the default font will be used
	int					FontSizePts = 13;
	int					ImageScale = 1;
    
	// Lua config
    bool                bEnableLua = false;
	bool				bEditLuaBaseFiles = false;
	std::vector<std::string>	LuaBaseFiles;

    virtual bool    Init(void);
	bool	Load(const char* filename);
	bool	Save(const char* filename);
protected:
	virtual void ReadFromJson(const nlohmann::json& jsonConfigFile);
	virtual void WriteToJson(nlohmann::json& jsonConfigFile) const;

	// Ensure that paths have a trailing slash.
	void FixupPaths(void)
	{
		if (WorkspaceRoot.back() != '/')
			WorkspaceRoot += "/";
		if (SnapshotFolder.back() != '/')
			SnapshotFolder += "/";
	}
};
