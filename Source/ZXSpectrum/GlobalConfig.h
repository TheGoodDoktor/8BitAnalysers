#pragma once

#include <string>
#include "Util/Misc.h"

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
	std::string			SnapshotFolder128 = "./Games128/";
	std::string			PokesFolder = "./Pokes/";
	std::string			RZXFolder = "./RZX/";

	std::string			Font = ""; // if no font is specified the default font will be used
	uint32_t			FontSizePixels = 13;
};

FGlobalConfig& GetGlobalConfig();
bool LoadGlobalConfig(const char* fileName);
bool SaveGlobalConfig(const char* fileName);
