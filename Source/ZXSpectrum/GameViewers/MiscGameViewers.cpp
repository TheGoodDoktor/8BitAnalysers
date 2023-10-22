#include "MiscGameViewers.h"
#include "../SpectrumEmu.h"
#include "../ZXSpectrumGameConfig.h"
#include "GameViewer.h"

FGameViewerData *InitMiscGameViewer(FSpectrumEmu*pEmu, FZXSpectrumGameConfig *pGameConfig)
{
	FGameViewerData* pGameViewerData = new FGameViewerData;
	pGameViewerData->pEmu = pEmu;

	InitGameViewer(pGameViewerData, pGameConfig);
	return pGameViewerData;
}

void DrawMiscGameViewer(FSpectrumEmu* pEmu, FGame *pGame)
{
	FGameViewerData* pGameViewer = pGame->pViewerData;

	ImGui::BeginTabBar("GameViewer");
	
	if (ImGui::BeginTabItem("Sprites"))
	{
		DrawSpriteListGUI(pEmu->GraphicsViewer, pGameViewer->pSpriteGraphicsView);
		ImGui::EndTabItem();
	}

	ImGui::EndTabBar();
}


// Firelord
/*
FGameConfig g_FirelordConfig =
{
	"Firelord",
	"Firelord.z80",
	InitMiscGameViewer,
	DrawMiscGameViewer,
{
	{{"Firelord_Font"}, {0xb240,32,1,1}},
	{{"Firelord_Sprites"}, {0xb3b0,52,3,2}},
	}
};*/
/*
// Cybernoid 2
FGameConfig g_Cybernoid2Config =
{
	"Cybernoid II",
	"Cybernoid II.z80",
	InitMiscGameViewer,
	DrawMiscGameViewer,
{
	{{"Sprites_2x2"}, {0xCE5F,52,2,2}},
	{{"Sprites_3x2"}, {0xC19F,52,3,2}},
	}
};*/

// Feud
/*
FGameConfig g_FeudConfig =
{
	"Feud",
	"Feud.z80",
	InitMiscGameViewer,
	DrawMiscGameViewer,
{
	{{"Font"}, {0xf55e,80,1,1}},
	{{"Zombie Villager"}, {0xf73e,16,3,4}},
	{{"Effects"}, {0xff1f,8,2,2}},
	//{{"Wizard Sprites"}, {0xd38d,80,3,4}},
	{{"Character Sprites"}, {0xd32a,72,3,4}},
	//{{"Wizard 2 Sprites"}, {0xdf2d,80,3,4}},
	{{"Compass"}, {0xee96,1,6,6}},
	}
};*/

FViewerConfig g_DefaultViewConfig = 
{
	"Default",
	InitMiscGameViewer,
	DrawMiscGameViewer,
};

void RegisterGames(FSpectrumEmu *pEmu)
{
	AddViewerConfig(&g_DefaultViewConfig);

	//pUI->GameConfigs.push_back(&g_FirelordConfig);
	//pUI->GameConfigs.push_back(&g_Cybernoid2Config);
	//pUI->GameConfigs.push_back(&g_FeudConfig);
}

