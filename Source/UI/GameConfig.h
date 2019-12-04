#pragma once
#include <string>
#include <map>

struct FGameViewerData;
struct FSpeccyUI;
struct FGame;

struct FSpriteDefConfig
{
	uint16_t	BaseAddress;
	int			Count;
	int			Width;
	int			Height;
};

struct FGameConfig
{
	std::string		Name;
	std::string		Z80File;

	FGameViewerData *(*pInitFunction)(FSpeccyUI *pUI, FGameConfig *pGameConfig);
	void(*pDrawFunction)(FSpeccyUI *pSpeccyUI, FGame* pGame);

	std::map<std::string, FSpriteDefConfig> SpriteConfigs;
};

bool SaveGameConfigToFile(const FGameConfig &config, const char *fname);
bool LoadGameConfigFromFile(FGameConfig &config, const char *fname);
bool LoadGameConfigs(FSpeccyUI *pUI);

