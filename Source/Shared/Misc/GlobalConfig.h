#pragma once

#include <string>
#include <list>
#include "Util/Misc.h"
#include <json_fwd.hpp>

// Global system config base class
// Individual machine configs will derive from this
struct FGlobalConfig
{
	static const int kNumRecentProjects = 10;

	bool				bEnableAudio;
	bool				bShowScanLineIndicator = false;
	bool				bShowOpcodeValues = false;
	ENumberDisplayMode	NumberDisplayMode = ENumberDisplayMode::HexAitch;
	int					BranchLinesDisplayMode = 1;
	std::string			LastGame;

	std::list<std::string>	RecentProjects;

	std::string			ExportAssembler;	// which assembler to exort to
	std::string			DefaultAsmExportPath;	// default path for assembler export

	std::string			WorkspaceRoot = "./Workspace/";
	std::string			SnapshotFolder = "./Games/";

	std::string			Font = "Cousine-Regular.ttf";
	bool				bBuiltInFont = false;
	int					FontSizePts = 13;
	int					ImageScale = 1;
	int					OverviewScale = 2;
	int					GfxViewerScale = 1;
	int					GfxScreenScale = 1;

	bool				bEnableExperimentalFeatures = false;	// for WIP & experimental features

	// Lua config
	bool				bEditLuaBaseFiles = false;
	std::vector<std::string>	LuaBaseFiles;

	virtual bool    Init(void);
	bool	Load(const char* filename);
	bool	Save(const char* filename);
	void	AddProjectToRecents(const std::string& name);
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
