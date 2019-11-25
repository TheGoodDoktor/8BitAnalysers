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
	FGraphicsView*		pGraphicsView = nullptr;
};

// StarQuake Stuff
struct FStarquakeViewerData : FGameViewerData
{
	
};


void InitStarquakeViewer(FStarquakeViewerData *pStarquakeViewer)
{
	FSpeccyUI *pUI = pStarquakeViewer->pUI;
	
	pStarquakeViewer->pGraphicsView = CreateGraphicsView(256, 256);

	for (const auto &sprConfIt : g_SpriteConfigs)
	{
		const FSpriteDefConfig& config = sprConfIt.second;
		FUISpriteList &sprites = pUI->SpriteLists[sprConfIt.first];
		GenerateSpriteList(sprites.SpriteList, config.BaseAddress, config.Count, config.Width, config.Height);
	}
	
}

void DrawSmallPlatform(int platformNum, int xp,int yp,FStarquakeViewerData *pStarquakeViewer)
{
	FSpeccyUI *pUI = pStarquakeViewer->pUI;
	FGraphicsView *pGraphicsView = pStarquakeViewer->pGraphicsView;
	
	const uint16_t kSmallPlatformLUT = 0xeb23;
	const uint16_t kPlatformPtr = kSmallPlatformLUT + (platformNum * 2);

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

	DrawGraphicsView(*pGraphicsView);

}

void DrawStarquakeViewer(FSpeccyUI *pUI, FGameViewer &viewer)
{
	FStarquakeViewerData* pStarquakeViewer = (FStarquakeViewerData*)viewer.pUserData;

	ImGui::BeginTabBar("StarquakeTabBar");

	if (ImGui::BeginTabItem("Small Platforms"))
	{
		DrawSmallPlatform(0, 0,0, pStarquakeViewer);
		ImGui::EndTabItem();
	}

	if (ImGui::BeginTabItem("Sprites"))
	{
		DrawSpriteListGUI(pUI, pStarquakeViewer->pGraphicsView);
		ImGui::EndTabItem();
	}

	ImGui::EndTabBar();
}


void RegisterStarquakeViewer(FSpeccyUI *pUI)
{
	FGameViewer &viewer = AddGameViewer(pUI, "Starquake");
	viewer.pDrawFunction = DrawStarquakeViewer;

	FStarquakeViewerData* pData = new FStarquakeViewerData;

	pData->pUI = pUI;
	InitStarquakeViewer(pData);
	viewer.pUserData = pData;
}