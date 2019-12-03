#include "StarquakeViewer.h"
#include "UI/SpriteViewer.h"
#include "UI/SpeccyUI.h"
#include "imgui_impl_lucidextra.h"
#include <algorithm>
#include "UI/GraphicsView.h"
#include "GameViewer.h"
#include "UI/GameConfig.h"

// Starquake addresses
static const uint16_t kPlatformGfxAddr = 0xeb23;
static const uint16_t kPlatformGfxSize = 0x00ff;
static const uint16_t kPlatformTypeInfoAddr = 0x9740;
static const uint16_t kBigPlatformDataAddr = 0x9840;
static const int kNoBigPlatforms = 40;
static const uint16_t kScreenDataAddr = 0x7530;
static const int kNoScreens = 512;

static const uint16_t kBlobSpritesAddr = 0xe074;
static const int kNoBlobSprites = 52;

// Firelord
namespace Firelord
{
	static const uint16_t kFontAddr = 0xb240;
	static const uint16_t kBlobSpritesAddr = 0xb3b0;
}

// Cybernoid II
namespace CybernoidII
{
	static uint16_t kSprites3x2 = 0xc19f;
	static uint16_t kSprites2x2 = 0xce5f;
	//bfc7

}

// graphics maps for various games - TODO: tag the game in the config
std::map<std::string, FSpriteDefConfig> g_SpriteConfigs =
{
	{{"Starquake_Blobs"}, {0xe074,52,3,2}},	// starquake blobs sprites

	{{"CybernoidII_2x2"}, {0xc19f,52,3,2}},	// 
	{{"CybernoidII_3x2"}, {0xce5f,52,2,2}},	// 

	{{"Firelord_Font"}, {0xb240,30,1,1}},	// 
	{{"Firelord_Blobs"}, {0xb3b0,52,3,2}},	// 
};



// Sprite instance
struct FSprite
{
	FSpriteDef*	pDef;
	int			XPos;
	int			YPos;
};

// StarQuake Stuff
struct FStarquakeViewerData : FGameViewerData
{
	
};


FGameViewerData *InitStarquakeViewer(FSpeccyUI *pUI, FGameConfig *pGameConfig)
{
	FStarquakeViewerData* pStarquakeViewerData = new FStarquakeViewerData;
	pStarquakeViewerData->pUI = pUI;
	
	InitGameViewer(pStarquakeViewerData, pGameConfig);

	// Add specific memory handlers

	// Small platforms
	{
		FMemoryAccessHandler handler;

		handler.Name = "Small platforms";
		handler.MemStart = kPlatformGfxAddr;
		handler.MemEnd = kPlatformGfxAddr + kPlatformGfxSize;	// 8 bytes per char
		handler.Type = MemoryAccessType::Read;

		AddMemoryHandler(pUI, handler);
	}

	// Big platforms
	{
		FMemoryAccessHandler handler;

		handler.Name = "Big platforms";
		handler.MemStart = kBigPlatformDataAddr;
		handler.MemEnd = kBigPlatformDataAddr + (kNoBigPlatforms * 4);	// 4 bytes per big platform
		handler.Type = MemoryAccessType::Read;

		AddMemoryHandler(pUI, handler);
	}

	// Screens
	{
		FMemoryAccessHandler handler;

		handler.Name = "Screens";
		handler.MemStart = kScreenDataAddr;
		handler.MemEnd = kScreenDataAddr + (kNoScreens * 12); // 12 bytes per screen
		handler.Type = MemoryAccessType::Read;

		AddMemoryHandler(pUI, handler);
	}

	return pStarquakeViewerData;
}

static void DrawSmallPlatform(int platformNum, int xp,int yp,FStarquakeViewerData *pStarquakeViewer, FGraphicsView *pGraphicsView)
{
	FSpeccyUI *pUI = pStarquakeViewer->pUI;
	//FGraphicsView *pGraphicsView = pStarquakeViewer->pSpriteGraphicsView;
	
	const uint16_t kSmallPlatformLUT = 0xeb23;
	const uint16_t kPlatformPtr = kSmallPlatformLUT + (platformNum * 2);

	//memset(pGraphicsView->PixelBuffer, 0, pGraphicsView->Width * pGraphicsView->Height * 4);

	uint16_t kPlatformAddr = ReadySpeccyByte(pUI->pSpeccy, kPlatformPtr);
	kPlatformAddr = kPlatformAddr | (ReadySpeccyByte(pUI->pSpeccy, kPlatformPtr+1) << 8);
	uint16_t kPlatformCharPtr = kPlatformAddr + 6;	// pointer to char data
	uint16_t kPlatformColPtr = kPlatformAddr - 1;	// pointer to char data
	for(int y=0;y<6;y++)
	{
		const uint8_t bits = ReadySpeccyByte(pUI->pSpeccy, kPlatformAddr + y);
		for(int x=0;x<8;x++)
		{
			if((bits & (1<<(7-x))) !=0 )
			{
				const uint8_t *pImage = GetSpeccyMemPtr(pUI->pSpeccy, kPlatformCharPtr);
				const uint8_t col = ReadySpeccyByte(pUI->pSpeccy, kPlatformColPtr);
				uint8_t colVal = col & 0x3f;	// just the colour values
				if (colVal == 0x36) 
					colVal = (col & 0xc0) | ReadySpeccyByte(pUI->pSpeccy, 0xea63);
				else if (colVal != 0)
					colVal = col;
				else 
					colVal = (col & 0xf8) | ReadySpeccyByte(pUI->pSpeccy, 0xea62);

				colVal &= ~0x40;
				
				PlotImageAt(pImage, xp + (x * 8), yp + (y * 8), 1, 1, (uint32_t*)pGraphicsView->PixelBuffer, pGraphicsView->Width, colVal);

				kPlatformCharPtr+=8;
				kPlatformColPtr--;
			}
		}
	}


}

static void DrawBigPlatform(int platformNum, int xp, int yp, FStarquakeViewerData *pStarquakeViewer)
{
	uint16_t kBigPlatformData = kBigPlatformDataAddr + (platformNum * 4);

	FSpeccyUI *pUI = pStarquakeViewer->pUI;
	
	const uint8_t kPlatformNo1 = ReadySpeccyByte(pUI->pSpeccy, kBigPlatformData + 0);
	DrawSmallPlatform(kPlatformNo1,xp + 32, yp + 24, pStarquakeViewer, pStarquakeViewer->pScreenGraphicsView);
	const uint8_t kPlatformNo2 = ReadySpeccyByte(pUI->pSpeccy, kBigPlatformData + 1);
	DrawSmallPlatform(kPlatformNo2, xp, yp + 24, pStarquakeViewer, pStarquakeViewer->pScreenGraphicsView);
	const uint8_t kPlatformNo3 = ReadySpeccyByte(pUI->pSpeccy, kBigPlatformData + 2);
	DrawSmallPlatform(kPlatformNo3, xp + 32, yp, pStarquakeViewer, pStarquakeViewer->pScreenGraphicsView);
	const uint8_t kPlatformNo4 = ReadySpeccyByte(pUI->pSpeccy, kBigPlatformData + 3);
	DrawSmallPlatform(kPlatformNo4, xp, yp, pStarquakeViewer, pStarquakeViewer->pScreenGraphicsView);


}

static void DrawScreen(int screenNum, int xp, int yp, FStarquakeViewerData *pStarquakeViewer)
{
	FSpeccyUI *pUI = pStarquakeViewer->pUI;
	uint16_t kScreenData = kScreenDataAddr + (screenNum * 12);

	for(int y=0;y<3;y++)
	{
		for(int x=0;x<4;x++)
		{
			const uint8_t kPlatformNo = ReadySpeccyByte(pUI->pSpeccy, kScreenData);
			DrawBigPlatform(kPlatformNo, x * 64, y * 48, pStarquakeViewer);
			kScreenData++;
		}
	}

}

void DrawStarquakeViewer(FSpeccyUI *pUI, FGame *pGame)
{
	FStarquakeViewerData* pStarquakeViewer = (FStarquakeViewerData*)pGame->pViewerData;

	ImGui::BeginTabBar("StarquakeTabBar");

	if (ImGui::BeginTabItem("Small Platforms"))
	{
		static int platformNo = 0;
		FGraphicsView *pGraphicsView = pStarquakeViewer->pSpriteGraphicsView;
		ImGui::InputInt("Platform No", &platformNo);
		ClearGraphicsView(*pGraphicsView, 0xff000000);
		DrawSmallPlatform(platformNo, 0,0, pStarquakeViewer, pGraphicsView);
		DrawGraphicsView(*pGraphicsView);

		ImGui::EndTabItem();
	}

	if (ImGui::BeginTabItem("Big Platforms"))
	{
		static int platformNo = 0;
		FGraphicsView *pGraphicsView = pStarquakeViewer->pScreenGraphicsView;
		ImGui::InputInt("Platform No", &platformNo);
		ClearGraphicsView(*pGraphicsView, 0xff000000);
		DrawBigPlatform(platformNo, 0, 0, pStarquakeViewer);
		DrawGraphicsView(*pGraphicsView);
		ImGui::EndTabItem();
	}

	if (ImGui::BeginTabItem("Screens"))
	{
		static int screenNo = 0;
		static bool getScreenFromGame = false;
		FGraphicsView *pGraphicsView = pStarquakeViewer->pScreenGraphicsView;
		ImGui::InputInt("Screen No", &screenNo);
		ImGui::Checkbox("Get from game", &getScreenFromGame);
		ClearGraphicsView(*pGraphicsView, 0xff000000);
		if(getScreenFromGame)
			screenNo = ReadySpeccyByte(pUI->pSpeccy, 0xd2c8);
		DrawScreen(screenNo, 0, 0, pStarquakeViewer);
		DrawGraphicsView(*pGraphicsView);
		ImGui::EndTabItem();
	}

	if (ImGui::BeginTabItem("Sprites"))
	{
		DrawSpriteListGUI(pUI, pStarquakeViewer->pSpriteGraphicsView);
		ImGui::EndTabItem();
	}

	ImGui::EndTabBar();
}

FGameConfig g_StarQuakeConfig =
{
	"Starquake",
	"Starquake.z80",
	InitStarquakeViewer,
	DrawStarquakeViewer,
{
	{{"Starquake_Blobs"}, {0xe074,52,3,2}},	// starquake blobs sprites
	{{"Enemies"}, {0xafc8,84,3,2}},	// starquake blobs sprites
	}
};

void RegisterStarquakeViewer(FSpeccyUI *pUI)
{
	pUI->GameConfigs.push_back(&g_StarQuakeConfig);
}