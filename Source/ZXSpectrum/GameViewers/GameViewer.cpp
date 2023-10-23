#include "GameViewer.h"

#include "../SpectrumEmu.h"
#include "../Viewers/ZXGraphicsView.h"
#include "../ZXSpectrumGameConfig.h"

static std::map<std::string, FViewerConfig *>	g_ViewerConfigs;

FGameViewerData::~FGameViewerData()
{
	delete pSpriteGraphicsView;
	delete pScreenGraphicsView;
}


void InitGameViewer(FGameViewerData *pGameViewer, FZXSpectrumGameConfig *pGameConfig)
{
	FSpectrumEmu *pEmu = pGameViewer->pEmu;

	pGameViewer->pSpriteGraphicsView = new FZXGraphicsView(64, 64);
	pGameViewer->pScreenGraphicsView = new FZXGraphicsView(256, 256);

	// add memory handlers for screen memory
	{
		FMemoryAccessHandler handler;
		handler.Name = "ScreenPixMemWrite";
		handler.MemStart = 0x4000;
		handler.MemEnd = 0x57ff;	
		handler.Type = MemoryAccessType::Write;

		pEmu->AddMemoryHandler(handler);
	}
	{
		FMemoryAccessHandler handler;
		handler.Name = "ScreenAttrMemWrite";
		handler.MemStart = 0x5800;
		handler.MemEnd = 0x5Aff;	
		handler.Type = MemoryAccessType::Write;

		pEmu->AddMemoryHandler(handler);
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

		pEmu->AddMemoryHandler(handler);
	}
}

bool AddViewerConfig(FViewerConfig* pConfig)
{
	g_ViewerConfigs[pConfig->Name] = pConfig;
	return true;
}

FViewerConfig*	GetViewConfigForGame(const char *pGameName)
{
	auto configIt = g_ViewerConfigs.find(pGameName);
	if (configIt == g_ViewerConfigs.end())
	{
		configIt = g_ViewerConfigs.find("Default");
		if (configIt == g_ViewerConfigs.end())
			return nullptr;
	}

	return configIt->second;		
}

