#pragma once
#include <string>
#include <map>
#include <vector>

#include <Util/Misc.h>
#include <CodeAnalyser/CodeAnalyser.h>

struct FCodeAnalysisState;
struct FGameViewerData;
class FSpectrumEmu;
struct FGame;
struct FGameConfig;
struct FGameSnapshot;

struct FCheatMemoryEntry
{
	uint16_t	Address;
	uint8_t		Value;
	uint8_t		OldValue;
};

struct FCheat
{
	std::string						Description;
	bool							bEnabled = false;
	std::vector< FCheatMemoryEntry>	Entries;
};

struct FSpriteDefConfig
{
	uint16_t	BaseAddress;
	int			Count;
	int			Width;
	int			Height;
};

struct FViewerConfig
{
	std::string			Name;
	FGameViewerData *	(*pInitFunction)(FSpectrumEmu*pEmu, FGameConfig *pGameConfig);
	void				(*pDrawFunction)(FSpectrumEmu* pEmu, FGame* pGame);
};

struct FGameConfig
{
	std::string		Name;
	std::string		SnapshotFile;

	FViewerConfig *pViewerConfig = nullptr;

	std::map<std::string, FSpriteDefConfig> SpriteConfigs;

	std::vector< FCheat> Cheats;

	bool	bCodeAnalysisViewEnabled[FCodeAnalysisState::kNoViewStates] = { true,false,false,false };
	bool	bShowScanLineIndicator = false;
	ENumberDisplayMode		NumberDisplayMode = ENumberDisplayMode::HexAitch;
};

bool AddGameConfig(FGameConfig *pConfig);
const std::vector< FGameConfig *>& GetGameConfigs();

FGameConfig *CreateNewGameConfigFromSnapshot(const FGameSnapshot& snapshot);
bool SaveGameConfigToFile(const FGameConfig &config, const char *fname);
bool LoadGameConfigFromFile(FGameConfig &config, const char *fname);
bool LoadGameConfigs(FSpectrumEmu*pUI);

bool LoadPOKFile(FGameConfig &config, const char *fname);


bool SaveGameData(const FCodeAnalysisState& state, const char *fname);
bool LoadGameData(FCodeAnalysisState& state, const char *fname);
bool SaveROMData(const FCodeAnalysisState& state, const char *fname);
bool LoadROMData(FCodeAnalysisState& state, const char *fname);