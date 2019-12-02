#include "GameViewer.h"

#include "UI/SpeccyUI.h"
#include "UI/GraphicsView.h"
#include "UI/GameConfig.h"

void InitGameViewer(FGameViewerData *pGameViewer, FGameConfig *pGameConfig)
{
	FSpeccyUI *pUI = pGameViewer->pUI;

	pGameViewer->pSpriteGraphicsView = CreateGraphicsView(64, 64);
	pGameViewer->pScreenGraphicsView = CreateGraphicsView(256, 256);

	// Generate sprite lists from configs
	pUI->SpriteLists.clear();
	for (const auto &sprConfIt : pGameConfig->SpriteConfigs)
	{
		const FSpriteDefConfig& config = sprConfIt.second;
		FUISpriteList &sprites = pUI->SpriteLists[sprConfIt.first];
		GenerateSpriteList(sprites.SpriteList, config.BaseAddress, config.Count, config.Width, config.Height);
	}

	// add memory handlers for screen memory
	{
		FMemoryAccessHandler handler;
		handler.Name = "ScreenPixMemWrite";
		handler.MemStart = 0x4000;
		handler.MemEnd = 0x57ff;	
		handler.Type = MemoryAccessType::Write;

		AddMemoryHandler(pUI, handler);
	}
	{
		FMemoryAccessHandler handler;
		handler.Name = "ScreenAttrMemWrite";
		handler.MemStart = 0x5800;
		handler.MemEnd = 0x5Aff;	
		handler.Type = MemoryAccessType::Write;

		AddMemoryHandler(pUI, handler);
	}

	// add memory handlers for sprite configs
	for (const auto &spriteConfIt : pGameConfig->SpriteConfigs)
	{
		FMemoryAccessHandler handler;
		const FSpriteDefConfig &sprConf = spriteConfIt.second;

		handler.Name = spriteConfIt.first;
		handler.MemStart = sprConf.BaseAddress;
		handler.MemEnd = sprConf.BaseAddress + (sprConf.Count * sprConf.Width * sprConf.Height * 8);	// 8 bytes per char
		handler.Type = MemoryAccessType::Read;

		AddMemoryHandler(pUI, handler);
	}
}

