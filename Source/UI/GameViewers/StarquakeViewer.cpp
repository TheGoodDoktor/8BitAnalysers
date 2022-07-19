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
static const uint16_t kPlatformColourLUT = 0xA7F7;

static const uint16_t kBigPlatformDataAddr = 0x9840;
static const int kNoBigPlatforms = 40;

static const uint16_t kScreenDataAddr = 0x7530;
static const int kNoScreens = 512;

static const uint16_t kItemDataAddr = 0x9088;
static const int kNoItems = 35;

static const uint16_t kBlobSpritesAddr = 0xe074;
static const int kNoBlobSprites = 52;

// game state addresses
static const uint16_t kCurrentScreen = 0xd2c8;
static const uint16_t kNoLives = 0xd2cc;

static const uint16_t kPlayerX = 0xdd1d;
static const uint16_t kPlayerY = 0xdd1e;
static const uint16_t kPlayerXOffset = 0xdd1c;

// Current Game State
struct FSQGameState
{
	int	CurrentScreen = 0;
	int NoLives = 0;
	int PlayerXPos = 0;
	int PlayerYPos = 0;
};

void UpdateSQGameState(FSpeccy* pSpeccyState, FSQGameState& SQState)
{
	SQState.CurrentScreen = ReadSpeccyByte( pSpeccyState, kCurrentScreen );
	SQState.NoLives = ReadSpeccyByte( pSpeccyState, kNoLives );
	SQState.PlayerXPos = ReadSpeccyByte( pSpeccyState, kPlayerX );
	SQState.PlayerYPos = ReadSpeccyByte( pSpeccyState, kPlayerY );
}

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
	FSQGameState	State;	// global game state
};


FGameViewerData *InitStarquakeViewer(FSpeccyUI *pUI, FGameConfig *pGameConfig)
{
	FStarquakeViewerData* pStarquakeViewerData = new FStarquakeViewerData;
	pStarquakeViewerData->pUI = pUI;
	
	InitGameViewer(pStarquakeViewerData, pGameConfig);

	// cheats
	WriteSpeccyByte( pUI->pSpeccy, 0x9ffc, 201 );

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
	
	//const uint16_t kSmallPlatformLUT = 0xeb23;
	const uint16_t kPlatformPtr = kPlatformGfxAddr + (platformNum * 2);

	//memset(pGraphicsView->PixelBuffer, 0, pGraphicsView->Width * pGraphicsView->Height * 4);
	uint8_t lutCol = ReadSpeccyByte( pUI->pSpeccy, 0xea63 );
	
	// check the platform info of this platform
	const uint8_t typeInfo = ReadSpeccyByte( pUI->pSpeccy, kPlatformTypeInfoAddr + platformNum );
	if( (typeInfo & 0xf0) < 0x50 && (typeInfo & 0xf0) != 0)
	{
		const uint8_t lutIndex = typeInfo >> 4;
		lutCol = ReadSpeccyByte( pUI->pSpeccy, kPlatformColourLUT + lutIndex );
	}

	uint16_t kPlatformAddr = ReadSpeccyByte(pUI->pSpeccy, kPlatformPtr);
	kPlatformAddr = kPlatformAddr | (ReadSpeccyByte(pUI->pSpeccy, kPlatformPtr+1) << 8);
	uint16_t kPlatformCharPtr = kPlatformAddr + 6;	// pointer to char data
	uint16_t kPlatformColPtr = kPlatformAddr - 1;	// pointer to colour data
	for(int y=0;y<6;y++)
	{
		const uint8_t bits = ReadSpeccyByte(pUI->pSpeccy, kPlatformAddr + y);
		for(int x=0;x<8;x++)
		{
			if((bits & (1<<(7-x))) !=0 )
			{
				const uint8_t *pImage = GetSpeccyMemPtr(pUI->pSpeccy, kPlatformCharPtr);
				const uint8_t col = ReadSpeccyByte(pUI->pSpeccy, kPlatformColPtr);
				uint8_t colVal = col & 0x3f;	// just the colour values - ink & paper
				if ( colVal == 0x36 )		// yellow paper, yellow ink
					colVal = ( col & 0xc0 ) | lutCol;// ReadSpeccyByte( pUI->pSpeccy, 0xea63 );	// use flash & bright from colour & read colour from 0xea63
				else if (colVal == 0)
					colVal = (col & 0xf8) | ReadSpeccyByte(pUI->pSpeccy, 0xea62);	// use flash, bright & paper
				else
					colVal = col;

				colVal &= ~0x40;	// clear bright bit
				
				PlotImageAt(pImage, xp + (x * 8), yp + (y * 8), 1, 8, (uint32_t*)pGraphicsView->PixelBuffer, pGraphicsView->Width, colVal);

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
	
	const uint8_t kPlatformNo1 = ReadSpeccyByte(pUI->pSpeccy, kBigPlatformData + 0);
	DrawSmallPlatform(kPlatformNo1,xp + 32, yp + 24, pStarquakeViewer, pStarquakeViewer->pScreenGraphicsView);
	const uint8_t kPlatformNo2 = ReadSpeccyByte(pUI->pSpeccy, kBigPlatformData + 1);
	DrawSmallPlatform(kPlatformNo2, xp, yp + 24, pStarquakeViewer, pStarquakeViewer->pScreenGraphicsView);
	const uint8_t kPlatformNo3 = ReadSpeccyByte(pUI->pSpeccy, kBigPlatformData + 2);
	DrawSmallPlatform(kPlatformNo3, xp + 32, yp, pStarquakeViewer, pStarquakeViewer->pScreenGraphicsView);
	const uint8_t kPlatformNo4 = ReadSpeccyByte(pUI->pSpeccy, kBigPlatformData + 3);
	DrawSmallPlatform(kPlatformNo4, xp, yp, pStarquakeViewer, pStarquakeViewer->pScreenGraphicsView);


}

static void DrawItem( int itemNum, int xp, int yp, FStarquakeViewerData* pStarquakeViewer, FGraphicsView* pGraphicsView )
{
	FSpeccyUI* pUI = pStarquakeViewer->pUI;
	const uint8_t* pImage = GetSpeccyMemPtr( pUI->pSpeccy, kItemDataAddr + (itemNum * 32) );

	PlotImageAt( pImage, xp, yp, 1, 8, (uint32_t*)pGraphicsView->PixelBuffer, pGraphicsView->Width );
	PlotImageAt( pImage + 8, xp + 8, yp, 1, 8, (uint32_t*)pGraphicsView->PixelBuffer, pGraphicsView->Width );
	PlotImageAt( pImage + 16, xp, yp + 8, 1, 8, (uint32_t*)pGraphicsView->PixelBuffer, pGraphicsView->Width );
	PlotImageAt( pImage + 24, xp + 8, yp + 8, 1, 8, (uint32_t*)pGraphicsView->PixelBuffer, pGraphicsView->Width );
}

static void DrawScreen(int screenNum, int xp, int yp, FStarquakeViewerData *pStarquakeViewer)
{
	FSpeccyUI *pUI = pStarquakeViewer->pUI;
	uint16_t kScreenData = kScreenDataAddr + (screenNum * 12);

	for(int y=0;y<3;y++)
	{
		for(int x=0;x<4;x++)
		{
			const uint8_t kPlatformNo = ReadSpeccyByte(pUI->pSpeccy, kScreenData);
			DrawBigPlatform(kPlatformNo, xp + (x * 64), yp + (y * 48), pStarquakeViewer);
			kScreenData++;
		}
	}

}

void DrawStarquakeViewer(FSpeccyUI *pUI, FGame *pGame)
{
	FStarquakeViewerData* pStarquakeViewer = (FStarquakeViewerData*)pGame->pViewerData;

	UpdateSQGameState( pUI->pSpeccy, pStarquakeViewer->State );

	ImGui::BeginTabBar("StarquakeTabBar");

	if (ImGui::BeginTabItem("Small Platforms"))
	{
		static int platformNo = 0;
		FGraphicsView *pGraphicsView = pStarquakeViewer->pSpriteGraphicsView;
		ImGui::InputInt("Platform No", &platformNo);
		ImGui::Text( "%xh", platformNo );
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
		ImGui::Text( "%xh", platformNo );
		ClearGraphicsView(*pGraphicsView, 0xff000000);
		DrawBigPlatform(platformNo, 0, 0, pStarquakeViewer);
		DrawGraphicsView(*pGraphicsView);
		ImGui::EndTabItem();
	}

	if ( ImGui::BeginTabItem( "Items" ) )
	{
		static int itemNo = 0;
		FGraphicsView* pGraphicsView = pStarquakeViewer->pScreenGraphicsView;
		ImGui::InputInt( "Item No", &itemNo );
		ImGui::Text( "%xh", itemNo );
		ClearGraphicsView( *pGraphicsView, 0xff000000 );
		DrawItem( itemNo, 0, 0, pStarquakeViewer, pGraphicsView );
		DrawGraphicsView( *pGraphicsView );
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
			screenNo = pStarquakeViewer->State.CurrentScreen;
		DrawScreen(screenNo, 0, 0, pStarquakeViewer);
		DrawGraphicsView(*pGraphicsView);
		ImGui::EndTabItem();
	}

	if (ImGui::BeginTabItem("Sprites"))
	{
		DrawSpriteListGUI(pUI->GraphicsViewer, pStarquakeViewer->pSpriteGraphicsView);
		ImGui::EndTabItem();
	}

	if ( ImGui::BeginTabItem( "Game View" ) )
	{
		FGraphicsView *pGraphicsView = pStarquakeViewer->pScreenGraphicsView;
		ImDrawList* pDrawList = ImGui::GetWindowDrawList();
		ImVec2 pos = ImGui::GetCursorScreenPos();

		ClearGraphicsView( *pGraphicsView, 0xff000000 );

		// TODO: draw panel
		// 
		// Draw Game screen
		DrawScreen( pStarquakeViewer->State.CurrentScreen, 0, 48, pStarquakeViewer );

		// Draw player
		uint16_t spriteAddr = ReadSpeccyByte( pUI->pSpeccy, 0xdd1a );
		spriteAddr = spriteAddr | ( ReadSpeccyByte( pUI->pSpeccy, 0xdd1a + 1 ) << 8 );

		const uint8_t* pImage = GetSpeccyMemPtr( pUI->pSpeccy, spriteAddr );

		int x = ReadSpeccyByte( pUI->pSpeccy, kPlayerX );
		int y = ReadSpeccyByte( pUI->pSpeccy, kPlayerY );
		int xOff = ReadSpeccyByte( pUI->pSpeccy, kPlayerXOffset );

		PlotImageAt( pImage, x , 192 - y, 3, 16, (uint32_t*)pGraphicsView->PixelBuffer, pGraphicsView->Width );

		// TODO: draw enemies
		
		// TODO: draw items

		DrawGraphicsView( *pGraphicsView );

		ImVec2 windowsPos( pos.x + x, pos.y + 192 - y );
		pDrawList->AddRect( windowsPos, ImVec2( windowsPos.x + 16, windowsPos.y + 16 ), 0xffffffff );

		ImGui::EndTabItem();
	}

	ImGui::EndTabBar();
}

/*FGameConfig g_StarQuakeConfig =
{
	"Starquake",
	"Starquake.z80",
	InitStarquakeViewer,
	DrawStarquakeViewer,
{
	{{"Starquake_Blobs"}, {0xe074,52,3,2}},	// starquake blobs sprites
	{{"Enemies"}, {0xafc8,84,3,2}},	// starquake blobs sprites
	}
};*/

FViewerConfig g_StarQuakeViewConfig =
{
	"Starquake",
	InitStarquakeViewer,
	DrawStarquakeViewer,
};

void RegisterStarquakeViewer(FSpeccyUI *pUI)
{
	AddViewerConfig(&g_StarQuakeViewConfig);

	//pUI->GameConfigs.push_back(&g_StarQuakeConfig);
}