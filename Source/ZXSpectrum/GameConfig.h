#pragma once
#include <string>
#include <map>
#include <vector>

#include <Util/Misc.h>
#include <CodeAnalyser/CodeAnalyser.h>

class FCodeAnalysisState;
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
	bool		bUserDefined = false;
	bool		bUserDefinedValueDirty = false;
};

struct FCheat
{
	std::string						Description;
	bool							bEnabled = false;
	bool							bHasUserDefinedEntries = false;
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

struct FCodeAnalysisViewConfig
{
	bool		bEnabled = false;
	FAddressRef	ViewAddress;
};

struct FGameConfig
{
	FGameConfig()
	{
		ViewConfigs[0].bEnabled = true;
	}

	std::string		Name;
	std::string		SnapshotFile;
	bool			Spectrum128KGame = false;
	bool			WriteSnapshot = false;

	FViewerConfig *pViewerConfig = nullptr;

	std::map<std::string, FSpriteDefConfig> SpriteConfigs;

	std::vector< FCheat> Cheats;

	FCodeAnalysisViewConfig	ViewConfigs[FCodeAnalysisState::kNoViewStates];
};

bool AddGameConfig(FGameConfig *pConfig);
const std::vector< FGameConfig *>& GetGameConfigs();

FGameConfig *CreateNewGameConfigFromSnapshot(const FGameSnapshot& snapshot);
bool SaveGameConfigToFile(const FGameConfig &config, const char *fname);
bool LoadGameConfigFromFile(const FCodeAnalysisState& state, FGameConfig &config, const char *fname);
bool LoadGameConfigs(FSpectrumEmu*pUI);

bool LoadPOKFile(FGameConfig &config, const char *fname);


