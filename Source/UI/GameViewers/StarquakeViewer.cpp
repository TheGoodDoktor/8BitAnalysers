#include "StarquakeViewer.h"
#include "UI/SpriteViewer.h"
#include "UI/SpeccyUI.h"
#include "imgui_impl_lucidextra.h"
#include <algorithm>
#include "UI/GraphicsView.h"

// Starquake addresses
static const uint16_t kPlatformGfxAddr = 0xeb23;
static const uint16_t kPlatformTypeInfoAddr = 0x9740;
static const uint16_t kBigPlatformDataAddr = 0x9840;
static const uint16_t kScreenDataAddr = 0x7530;

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

// Base class for game viewer data
struct FGameViewerData
{
	FSpeccyUI*			pUI = nullptr;
	FGraphicsView*		pSpriteGraphicsView = nullptr;
	FGraphicsView*		pScreenGraphicsView = nullptr;
};

void InitGameViewer(FGameViewerData *pGameViewer, FGameConfig *pGameConfig)
{
	FSpeccyUI *pUI = pGameViewer->pUI;

	pGameViewer->pSpriteGraphicsView = CreateGraphicsView(64, 64);
	pGameViewer->pScreenGraphicsView = CreateGraphicsView(256, 256);

	// Could be moved to a generic function
	pUI->SpriteLists.clear();
	for (const auto &sprConfIt : pGameConfig->SpriteConfigs)
	{
		const FSpriteDefConfig& config = sprConfIt.second;
		FUISpriteList &sprites = pUI->SpriteLists[sprConfIt.first];
		GenerateSpriteList(sprites.SpriteList, config.BaseAddress, config.Count, config.Width, config.Height);
	}
}

// StarQuake Stuff
struct FStarquakeViewerData : FGameViewerData
{
	
};


void InitStarquakeViewer(FStarquakeViewerData *pStarquakeViewer, FGameConfig *pGameConfig)
{
	FSpeccyUI *pUI = pStarquakeViewer->pUI;
	
	InitGameViewer(pStarquakeViewer, pGameConfig);
	
	
}

void DrawSmallPlatform(int platformNum, int xp,int yp,FStarquakeViewerData *pStarquakeViewer, FGraphicsView *pGraphicsView)
{
	FSpeccyUI *pUI = pStarquakeViewer->pUI;
	//FGraphicsView *pGraphicsView = pStarquakeViewer->pSpriteGraphicsView;
	
	const uint16_t kSmallPlatformLUT = 0xeb23;
	const uint16_t kPlatformPtr = kSmallPlatformLUT + (platformNum * 2);

	//memset(pGraphicsView->PixelBuffer, 0, pGraphicsView->Width * pGraphicsView->Height * 4);

	uint16_t kPlatformAddr = ReadySpeccyByte(*pUI->pSpeccy, kPlatformPtr);
	kPlatformAddr = kPlatformAddr | (ReadySpeccyByte(*pUI->pSpeccy, kPlatformPtr+1) << 8);
	uint16_t kPlatformCharPtr = kPlatformAddr + 6;	// pointer to char data
	uint16_t kPlatformColPtr = kPlatformAddr - 1;	// pointer to char data
	for(int y=0;y<6;y++)
	{
		const uint8_t bits = ReadySpeccyByte(*pUI->pSpeccy, kPlatformAddr + y);
		for(int x=0;x<8;x++)
		{
			if((bits & (1<<(7-x))) !=0 )
			{
				const uint8_t *pImage = GetSpeccyMemPtr(*pUI->pSpeccy, kPlatformCharPtr);
				const uint8_t col = ReadySpeccyByte(*pUI->pSpeccy, kPlatformColPtr);
				uint8_t colVal = col;
				colVal &= 0x3f;
				if (colVal == 0x36) 
					colVal = (colVal & 0xc0) | ReadySpeccyByte(*pUI->pSpeccy, 0xea63);
				else if (colVal != 0)
					colVal = colVal;
				else 
					colVal = (colVal & 0xf8) | ReadySpeccyByte(*pUI->pSpeccy, 0xea62);
				
				PlotImageAt(pImage, xp + (x * 8), yp + (y * 8), 1, 1, (uint32_t*)pGraphicsView->PixelBuffer, pGraphicsView->Width, colVal);

				kPlatformCharPtr+=8;
				kPlatformColPtr--;
			}
		}
	}


}

void DrawBigPlatform(int platformNum, int xp, int yp, FStarquakeViewerData *pStarquakeViewer)
{
	const uint16_t kBigPlatformBase = 0x9840;
	uint16_t kBigPlatformData = kBigPlatformBase + (platformNum * 4);

	FSpeccyUI *pUI = pStarquakeViewer->pUI;
	
	const uint8_t kPlatformNo1 = ReadySpeccyByte(*pUI->pSpeccy, kBigPlatformData + 0);
	DrawSmallPlatform(kPlatformNo1,xp + 32, yp + 32, pStarquakeViewer, pStarquakeViewer->pScreenGraphicsView);
	const uint8_t kPlatformNo2 = ReadySpeccyByte(*pUI->pSpeccy, kBigPlatformData + 1);
	DrawSmallPlatform(kPlatformNo2, xp, yp + 32, pStarquakeViewer, pStarquakeViewer->pScreenGraphicsView);
	const uint8_t kPlatformNo3 = ReadySpeccyByte(*pUI->pSpeccy, kBigPlatformData + 2);
	DrawSmallPlatform(kPlatformNo3, xp + 32, yp, pStarquakeViewer, pStarquakeViewer->pScreenGraphicsView);
	const uint8_t kPlatformNo4 = ReadySpeccyByte(*pUI->pSpeccy, kBigPlatformData + 3);
	DrawSmallPlatform(kPlatformNo4, xp, yp, pStarquakeViewer, pStarquakeViewer->pScreenGraphicsView);


}

void DrawScreen(int screenNum, int xp, int yp, FStarquakeViewerData *pStarquakeViewer)
{
}

void DrawStarquakeViewer(FSpeccyUI *pUI, FGameConfig *pGameConfig)
{
	FStarquakeViewerData* pStarquakeViewer = (FStarquakeViewerData*)pGameConfig->pUserData;

	ImGui::BeginTabBar("StarquakeTabBar");

	if (ImGui::BeginTabItem("Small Platforms"))
	{
		static int platformNo = 0;
		FGraphicsView *pGraphicsView = pStarquakeViewer->pSpriteGraphicsView;
		ImGui::InputInt("Platform No", &platformNo);
		memset(pGraphicsView->PixelBuffer, 0xff000000, pGraphicsView->Width * pGraphicsView->Height * 4);
		DrawSmallPlatform(platformNo, 0,0, pStarquakeViewer, pGraphicsView);
		DrawGraphicsView(*pGraphicsView);

		ImGui::EndTabItem();
	}

	if (ImGui::BeginTabItem("Big Platforms"))
	{
		static int platformNo = 0;
		FGraphicsView *pGraphicsView = pStarquakeViewer->pScreenGraphicsView;
		ImGui::InputInt("Platform No", &platformNo);
		memset(pGraphicsView->PixelBuffer, 0xff000000, pGraphicsView->Width * pGraphicsView->Height * 4);
		DrawBigPlatform(platformNo, 0, 0, pStarquakeViewer);
		DrawGraphicsView(*pGraphicsView);
		ImGui::EndTabItem();
	}

	if (ImGui::BeginTabItem("Screens"))
	{
		static int screenNo = 0;
		FGraphicsView *pGraphicsView = pStarquakeViewer->pScreenGraphicsView;
		ImGui::InputInt("Screen No", &screenNo);
		memset(pGraphicsView->PixelBuffer, 0xff000000, pGraphicsView->Width * pGraphicsView->Height * 4);
		DrawScreen(screenNo, 0, 0, pStarquakeViewer);
		DrawGraphicsView(*pStarquakeViewer->pScreenGraphicsView);
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
	DrawStarquakeViewer,
{
	{{"Starquake_Blobs"}, {0xe074,52,3,2}},	// starquake blobs sprites
	}
};

void RegisterStarquakeViewer(FSpeccyUI *pUI)
{
	FGameConfig *pGameConfig = &g_StarQuakeConfig;
	FStarquakeViewerData* pData = new FStarquakeViewerData;
	pData->pUI = pUI;

	
	InitStarquakeViewer(pData, pGameConfig);

	g_StarQuakeConfig.pUserData = pData;

	pUI->GameConfigs.push_back(pGameConfig);
}