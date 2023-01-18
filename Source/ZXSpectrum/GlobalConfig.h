#pragma once

#include <string>
#include "Util/Misc.h"

struct FGlobalConfig
{
	bool				bEnableAudio;
	bool				bShowScanLineIndicator = false;
	bool				bShowOpcodeValues = false;
	ENumberDisplayMode	NumberDisplayMode = ENumberDisplayMode::HexAitch;
	std::string			LastGame;
};

FGlobalConfig& GetGlobalConfig();
bool LoadGlobalConfig(const char* fileName);
bool SaveGlobalConfig(const char* fileName);
