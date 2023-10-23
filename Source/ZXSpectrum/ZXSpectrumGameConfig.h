#pragma once

#include <Misc/GameConfig.h>


struct FGame;
struct FGameViewerData;
struct FGameSnapshot;

struct FZXSpectrumGameConfig;
class FSpectrumEmu;

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
	FGameViewerData* (*pInitFunction)(FSpectrumEmu* pEmu, FZXSpectrumGameConfig* pGameConfig);
	void				(*pDrawFunction)(FSpectrumEmu* pEmu, FGame* pGame);
};


// spectrum specific
struct FZXSpectrumGameConfig : FGameConfig
{
	void	LoadFromJson(const nlohmann::json& jsonConfig) override;
	void	SaveToJson(nlohmann::json& jsonConfig) const override;

	bool			Spectrum128KGame = false;

	FViewerConfig* pViewerConfig = nullptr;

	std::map<std::string, FSpriteDefConfig> SpriteConfigs;
	std::vector< FCheat> Cheats;
};

FZXSpectrumGameConfig* CreateNewZXGameConfigFromSnapshot(const FGameSnapshot& snapshot);
bool LoadZXSpectrumGameConfigs(FSpectrumEmu* pUI);
bool LoadPOKFile(FZXSpectrumGameConfig& config, const char* fname);
