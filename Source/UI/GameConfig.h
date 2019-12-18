#pragma once
#include <string>
#include <map>

struct FCodeAnalysisState;
struct FGameViewerData;
struct FSpeccyUI;
struct FGame;
struct FGameConfig;

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
	FGameViewerData *	(*pInitFunction)(FSpeccyUI *pUI, FGameConfig *pGameConfig);
	void				(*pDrawFunction)(FSpeccyUI *pSpeccyUI, FGame* pGame);
};

struct FGameConfig
{
	std::string		Name;
	std::string		Z80File;

	FGameViewerData *(*pInitFunction)(FSpeccyUI *pUI, FGameConfig *pGameConfig);
	void(*pDrawFunction)(FSpeccyUI *pSpeccyUI, FGame* pGame);

	//FViewerConfig *pViewerConfig = nullptr;

	std::map<std::string, FSpriteDefConfig> SpriteConfigs;
};

FGameConfig *CreateNewGameConfigFromZ80File(const char *pZ80FileName);
bool SaveGameConfigToFile(const FGameConfig &config, const char *fname);
bool LoadGameConfigFromFile(FGameConfig &config, const char *fname);
bool LoadGameConfigs(FSpeccyUI *pUI);

bool SaveGameData(const FCodeAnalysisState& state, const char *fname);
bool LoadGameData(FCodeAnalysisState& state, const char *fname);
bool SaveROMData(const FCodeAnalysisState& state, const char *fname);
bool LoadROMData(FCodeAnalysisState& state, const char *fname);