#pragma once
#include <string>
#include <map>
#include <vector>

#include <Util/Misc.h>
#include <CodeAnalyser/CodeAnalyser.h>

class FCodeAnalysisState;

#if SPECCY
struct FGameViewerData;
#endif
class FCpcEmu;
struct FGame;
struct FGameConfig;
struct FGameSnapshot;

#if SPECCY
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
	FGameViewerData *	(*pInitFunction)(FCpcEmu*pEmu, FGameConfig *pGameConfig);
	void				(*pDrawFunction)(FCpcEmu* pEmu, FGame* pGame);
};
#endif // #if SPECCY

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
	bool			Cpc6128Game = false;
	bool			WriteSnapshot = false;

#if SPECCY
	FViewerConfig *pViewerConfig = nullptr;

	std::map<std::string, FSpriteDefConfig> SpriteConfigs;

	std::vector< FCheat> Cheats;
#endif
	FCodeAnalysisViewConfig	ViewConfigs[FCodeAnalysisState::kNoViewStates];
};

bool AddGameConfig(FGameConfig *pConfig);
bool RemoveGameConfig(const char* pName);

const std::vector< FGameConfig *>& GetGameConfigs();

FGameConfig *CreateNewGameConfigFromSnapshot(const FGameSnapshot& snapshot);
bool SaveGameConfigToFile(const FGameConfig &config, const char *fname);
bool LoadGameConfigs(FCpcEmu*pUI);

#if SPECCY
bool LoadPOKFile(FGameConfig &config, const char *fname);
#endif

