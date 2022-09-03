#include "StarquakeViewer.h"
#include "UI/Viewers/SpriteViewer.h"
#include "UI/SpeccyUI.h"
#include <ImGuiSupport/imgui_impl_lucidextra.h>
#include <algorithm>
#include <Util/GraphicsView.h>
#include "GameViewer.h"
#include "UI/GameConfig.h"

// Starquake addresses

// Screen composition data
static const uint16_t kSmallPlatformGfxAddr = 0xeb23;
static const uint16_t kSmallPlatformGfxSize = 0x00ff;
static const uint16_t kSmallPlatformTypeInfoAddr = 0x9740;
static const uint16_t kSmallPlatformColourLUT = 0xA7F7;

static const uint16_t kBigPlatformDataAddr = 0x9840;
static const int kNoBigPlatforms = 40;

static const uint16_t kScreenDataAddr = 0x7530;
static const int kNoScreens = 512;

// game state addresses
static const uint16_t kCurrentScreen = 0xd2c8;
static const uint16_t kNoLives = 0xd2cc;

/*static const uint16_t kPlayerSpriteImagePtr = 0xdd1a;
static const uint16_t kPlayerXCharPos = 0xdd1c;	// X pos in character coords
static const uint16_t kPlayerX = 0xdd1d;
static const uint16_t kPlayerY = 0xdd1e;
static const uint16_t kPlayerPrevSpriteAddr = 0xdd1f;
static const uint16_t kPlayerCurrentCtrl = 0xdd23;
static const uint16_t kPlayerLastCtrl = 0xdd24;
static const uint16_t kPlayerAnimFrame = 0xdd25;
*/
static const uint16_t kSpriteList = 0xdd18;

// struct for sprite in ZX memory
#pragma pack(1)
struct FStarquakeSprite
{
	uint16_t	ScanlinePtr;
	uint16_t	SpriteImagePtr;
	uint8_t		XCharacterPos;
	uint8_t		XPixelPos;
	uint8_t		YPixelPos;
	uint16_t	SpriteImagePtr2;
	uint8_t		InkCol;
	uint8_t		Unknown1;
	uint8_t		CurrentControl;
	uint8_t		LastControl;
	uint8_t		AnimFrame;
	uint8_t		Unknown2;
	uint8_t		Unknown4;

	uint8_t		Pad[16];
};
static_assert( sizeof( FStarquakeSprite ) == 32 );

// Sprite Data
static const uint16_t kBlobSpritesAddr = 0xe074;
static const int kNoBlobSprites = 52;

// Platforms
// The thing the player drops with the down button
#pragma pack(1)
struct FPlatformState
{
	unsigned		XCharPos : 5;
	unsigned		PlatNo : 3;
	unsigned		YCharPos : 8;
	unsigned		Unknown : 8;
	unsigned		Timer : 8;
};
static_assert( sizeof( FPlatformState ) == 4 );

static const uint16_t kPlatformStates = 0xdbbb;	// address

// platform sprite data
static const uint16_t kPlatformSpritesAddr = 0xdc55;
static const int kNoPlatformSprites = 35;

// Items
// They can be picked up and dropped
#pragma pack(1)
struct FItemState
{
	unsigned	XPos : 5;	
	unsigned	Colour : 3;
	unsigned	YPos : 7;
	unsigned	RoomMSB : 1;
	unsigned	RoomNo : 8;
	unsigned	ItemNo : 8;
};
static_assert( sizeof( FItemState ) == 4 );

static const uint16_t kCollectableItemStates = 0x94E8;	// address of states in ZX memory

// item sprite data
static const uint16_t kItemSpriteDataAddr = 0x9088;
static const int kNoItems = 35;

// Electric Hazards
// Timed hazards
#pragma pack(1)
struct FElectricHazardState
{
	uint8_t	XPos;
	uint8_t YPos;
	uint8_t Unknown1;
	uint8_t Timer;
	uint8_t Unknown3;
	uint8_t Active;
	uint8_t Unknown5;
	uint8_t Unknown6;
};
static_assert( sizeof( FElectricHazardState ) == 8 );

static const uint16_t kElectricHazardStates = 0x9635;	// address of states in ZX memory
static const int kNoElectricHazards = 4;


// Current Game State
struct FSQGameState
{
	int	CurrentScreen = 0;
	int NoLives = 0;
	//int PlayerXPos = 0;
	//int PlayerYPos = 0;
};

void UpdateSQGameState(FSpeccy* pSpeccyState, FSQGameState& SQState)
{
	SQState.CurrentScreen = ReadSpeccyWord( pSpeccyState, kCurrentScreen );
	SQState.NoLives = ReadSpeccyByte( pSpeccyState, kNoLives );
	//SQState.PlayerXPos = ReadSpeccyByte( pSpeccyState, kPlayerX );
	//SQState.PlayerYPos = ReadSpeccyByte( pSpeccyState, kPlayerY );
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


FGameViewerData *InitStarquakeViewer(FSpectrumEmu *pEmu, FGameConfig *pGameConfig)
{
	FStarquakeViewerData* pStarquakeViewerData = new FStarquakeViewerData;
	pStarquakeViewerData->pEmu = pEmu;
	
	InitGameViewer(pStarquakeViewerData, pGameConfig);

	// cheats
	//WriteSpeccyByte( pUI->pSpeccy, 0x9ffc, 201 );
	//WriteSpeccyByte( pUI->pSpeccy, 0xdb50, 0 );
	WriteSpeccyByte(pEmu->pSpeccy, 54505, 201 );

	// Add specific memory handlers

	// Small platforms
	{
		FMemoryAccessHandler handler;

		handler.Name = "Small platforms";
		handler.MemStart = kSmallPlatformGfxAddr;
		handler.MemEnd = kSmallPlatformGfxAddr + kSmallPlatformGfxSize;	// 8 bytes per char
		handler.Type = MemoryAccessType::Read;

		pEmu->AddMemoryHandler(handler);
	}

	// Big platforms
	{
		FMemoryAccessHandler handler;

		handler.Name = "Big platforms";
		handler.MemStart = kBigPlatformDataAddr;
		handler.MemEnd = kBigPlatformDataAddr + (kNoBigPlatforms * 4);	// 4 bytes per big platform
		handler.Type = MemoryAccessType::Read;

		pEmu->AddMemoryHandler(handler);
	}

	// Screens
	{
		FMemoryAccessHandler handler;

		handler.Name = "Screens";
		handler.MemStart = kScreenDataAddr;
		handler.MemEnd = kScreenDataAddr + (kNoScreens * 12); // 12 bytes per screen
		handler.Type = MemoryAccessType::Read;

		pEmu->AddMemoryHandler(handler);
	}

	return pStarquakeViewerData;
}

static void DrawSmallPlatform(int platformNum, int xp,int yp,FStarquakeViewerData *pStarquakeViewer, FGraphicsView *pGraphicsView)
{
	FSpectrumEmu *pEmu = pStarquakeViewer->pEmu;
	//FGraphicsView *pGraphicsView = pStarquakeViewer->pSpriteGraphicsView;
	
	//const uint16_t kSmallPlatformLUT = 0xeb23;
	const uint16_t kPlatformPtr = kSmallPlatformGfxAddr + (platformNum * 2);

	//memset(pGraphicsView->PixelBuffer, 0, pGraphicsView->Width * pGraphicsView->Height * 4);
	uint8_t lutCol = ReadSpeccyByte( pEmu->pSpeccy, 0xea63 );
	
	// check the platform info of this platform
	const uint8_t typeInfo = ReadSpeccyByte(pEmu->pSpeccy, kSmallPlatformTypeInfoAddr + platformNum );
	if( (typeInfo & 0xf0) < 0x50 && (typeInfo & 0xf0) != 0)
	{
		const uint8_t lutIndex = typeInfo >> 4;
		lutCol = ReadSpeccyByte(pEmu->pSpeccy, kSmallPlatformColourLUT + lutIndex );
	}

	uint16_t kPlatformAddr = ReadSpeccyByte(pEmu->pSpeccy, kPlatformPtr);
	kPlatformAddr = kPlatformAddr | (ReadSpeccyByte(pEmu->pSpeccy, kPlatformPtr+1) << 8);
	uint16_t kPlatformCharPtr = kPlatformAddr + 6;	// pointer to char data
	uint16_t kPlatformColPtr = kPlatformAddr - 1;	// pointer to colour data
	for(int y=0;y<6;y++)
	{
		const uint8_t bits = ReadSpeccyByte(pEmu->pSpeccy, kPlatformAddr + y);
		for(int x=0;x<8;x++)
		{
			if((bits & (1<<(7-x))) !=0 )
			{
				const uint8_t *pImage = GetSpeccyMemPtr(pEmu->pSpeccy, kPlatformCharPtr);
				const uint8_t col = ReadSpeccyByte(pEmu->pSpeccy, kPlatformColPtr);
				uint8_t colVal = col & 0x3f;	// just the colour values - ink & paper
				if ( colVal == 0x36 )		// yellow paper, yellow ink
					colVal = ( col & 0xc0 ) | lutCol;// ReadSpeccyByte( pUI->pSpeccy, 0xea63 );	// use flash & bright from colour & read colour from 0xea63
				else if (colVal == 0)
					colVal = (col & 0xf8) | ReadSpeccyByte(pEmu->pSpeccy, 0xea62);	// use flash, bright & paper
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

	FSpectrumEmu*pEmu = pStarquakeViewer->pEmu;
	
	const uint8_t kPlatformNo1 = ReadSpeccyByte(pEmu->pSpeccy, kBigPlatformData + 0);
	DrawSmallPlatform(kPlatformNo1,xp + 32, yp + 24, pStarquakeViewer, pStarquakeViewer->pScreenGraphicsView);
	const uint8_t kPlatformNo2 = ReadSpeccyByte(pEmu->pSpeccy, kBigPlatformData + 1);
	DrawSmallPlatform(kPlatformNo2, xp, yp + 24, pStarquakeViewer, pStarquakeViewer->pScreenGraphicsView);
	const uint8_t kPlatformNo3 = ReadSpeccyByte(pEmu->pSpeccy, kBigPlatformData + 2);
	DrawSmallPlatform(kPlatformNo3, xp + 32, yp, pStarquakeViewer, pStarquakeViewer->pScreenGraphicsView);
	const uint8_t kPlatformNo4 = ReadSpeccyByte(pEmu->pSpeccy, kBigPlatformData + 3);
	DrawSmallPlatform(kPlatformNo4, xp, yp, pStarquakeViewer, pStarquakeViewer->pScreenGraphicsView);


}

static void DrawItem( int itemNum, int xp, int yp, uint8_t col, FStarquakeViewerData* pStarquakeViewer, FGraphicsView* pGraphicsView )
{
	FSpectrumEmu* pEmu = pStarquakeViewer->pEmu;
	const uint8_t* pImage = GetSpeccyMemPtr(pEmu->pSpeccy, kItemSpriteDataAddr + (itemNum * 32) );

	PlotImageAt( pImage, xp, yp, 1, 8, (uint32_t*)pGraphicsView->PixelBuffer, pGraphicsView->Width, col );
	PlotImageAt( pImage + 8, xp + 8, yp, 1, 8, (uint32_t*)pGraphicsView->PixelBuffer, pGraphicsView->Width, col );
	PlotImageAt( pImage + 16, xp, yp + 8, 1, 8, (uint32_t*)pGraphicsView->PixelBuffer, pGraphicsView->Width, col );
	PlotImageAt( pImage + 24, xp + 8, yp + 8, 1, 8, (uint32_t*)pGraphicsView->PixelBuffer, pGraphicsView->Width, col );
}

static void DrawPlatform( int platformNo, int xp, int yp, uint8_t col, FStarquakeViewerData* pStarquakeViewer, FGraphicsView* pGraphicsView )
{
	FSpectrumEmu* pEmu = pStarquakeViewer->pEmu;
	const uint8_t* pImage = GetSpeccyMemPtr(pEmu->pSpeccy, kPlatformSpritesAddr + ( platformNo * 16 ) );

	PlotImageAt( pImage, xp, yp, 1, 8, (uint32_t*)pGraphicsView->PixelBuffer, pGraphicsView->Width, col );
	PlotImageAt( pImage + 8, xp + 8, yp, 1, 8, (uint32_t*)pGraphicsView->PixelBuffer, pGraphicsView->Width, col );
}

static void DrawScreen(int screenNum, int xp, int yp, FStarquakeViewerData *pStarquakeViewer)
{
	FSpectrumEmu* pEmu = pStarquakeViewer->pEmu;
	uint16_t kScreenData = kScreenDataAddr + (screenNum * 12);

	for(int y=0;y<3;y++)
	{
		for(int x=0;x<4;x++)
		{
			const uint8_t kPlatformNo = ReadSpeccyByte(pEmu->pSpeccy, kScreenData);
			DrawBigPlatform(kPlatformNo, xp + (x * 64), yp + (y * 48), pStarquakeViewer);
			kScreenData++;
		}
	}

}

void DrawStarquakeViewer(FSpectrumEmu*pUI, FGame *pGame)
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
		const uint8_t SmallPlatformInfo = ReadSpeccyByte( pUI->pSpeccy, kSmallPlatformTypeInfoAddr + platformNo );
		ImGui::Text( "Additional Info %xh", SmallPlatformInfo );
		if ( SmallPlatformInfo < 0x50 )
		{
			ImGui::Text( "Colour Attrib: %d", SmallPlatformInfo & 7 );
		}
		else
		{
			const uint8_t type = SmallPlatformInfo >> 4;

			switch(type)
			{
			case 0x5:
				ImGui::Text( "Upward transport tube" );
				break;
			case 0x6:
				ImGui::Text( "Contains kill zone" );
				break;
			case 0x7:
				ImGui::Text( "Electric hazard" );
				break;
			case 0x8:
				ImGui::Text( "Patrolling enemy placement" );
				break;
			case 0x9:
				ImGui::Text( "Item placement" );
				break;
			case 0xb:
				ImGui::Text( "Locked door" );
				break;
			case 0xc:
				ImGui::Text( "Flying transport" );
				break;
			case 0xd:
				ImGui::Text( "Teleporter" );
				break;
			default:
				ImGui::Text( "Unknown" );
				break;
			}
		}

		ClearGraphicsView( *pGraphicsView, 0xff000000 );
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
		DrawItem( itemNo, 0, 0, 0x7, pStarquakeViewer, pGraphicsView );
		DrawGraphicsView( *pGraphicsView );
		ImGui::EndTabItem();
	}
	if ( ImGui::BeginTabItem( "Platforms" ) )
	{
		static int platformNo = 0;
		FGraphicsView* pGraphicsView = pStarquakeViewer->pScreenGraphicsView;
		ImGui::InputInt( "Platform No", &platformNo );
		ImGui::Text( "%xh", platformNo );
		ClearGraphicsView( *pGraphicsView, 0xff000000 );
		DrawPlatform( platformNo, 0, 0, 0x7, pStarquakeViewer, pGraphicsView);
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

		// TODO: draw UI panel
		// 
		// Draw Game screen
		DrawScreen( pStarquakeViewer->State.CurrentScreen, 0, 48, pStarquakeViewer );

		// Draw Moving Sprites
		for ( int sprNo = 0; sprNo < 6; sprNo++ )
		{
			FStarquakeSprite* pSprite = (FStarquakeSprite*)GetSpeccyMemPtr( pUI->pSpeccy, kSpriteList + ( sprNo * sizeof( FStarquakeSprite ) ) );

			const uint8_t* pImage = GetSpeccyMemPtr( pUI->pSpeccy, pSprite->SpriteImagePtr );

			if( pSprite->XPixelPos > 16 || pSprite->YPixelPos > 16)
				PlotImageAt( pImage, pSprite->XCharacterPos * 8, 192 - pSprite->YPixelPos, 3, 16, (uint32_t*)pGraphicsView->PixelBuffer, pGraphicsView->Width, pSprite->InkCol );
		}

		// draw platforms
		for ( int platNo = 0; platNo < 6; platNo++ )
		{
			FPlatformState* pPlatform = (FPlatformState*)GetSpeccyMemPtr( pUI->pSpeccy, kPlatformStates + ( platNo * sizeof( FPlatformState ) ) );

			int x = pPlatform->XCharPos;// AndPlatNo & 31;
			int y = pPlatform->YCharPos;
			if ( y != 0 )
			{
				DrawPlatform( pPlatform->PlatNo, x * 8, y * 8, 0x7, pStarquakeViewer, pGraphicsView );
			}
		}

		// draw electric hazards
		for(int elecHaz = 0;elecHaz < kNoElectricHazards; elecHaz++)
		{
			FElectricHazardState* pElecHazardState = (FElectricHazardState *)GetSpeccyMemPtr( pUI->pSpeccy, kElectricHazardStates + ( elecHaz * sizeof( FElectricHazardState) ) );
		
			if ( pElecHazardState->XPos != 0 && pElecHazardState->Active != 0 )
			{
				DrawPlatform( 0x5, pElecHazardState->XPos * 8, pElecHazardState->YPos * 8, 0x7, pStarquakeViewer, pGraphicsView );
			}
		}
		
		// draw items
		for ( int collectableNo = 0; collectableNo < 12; collectableNo++ )
		{
			FItemState* pCollectable = (FItemState*)GetSpeccyMemPtr( pUI->pSpeccy, kCollectableItemStates + ( collectableNo * 4 ) );
		
			const int roomNo = pCollectable->RoomNo | ( ( pCollectable->RoomMSB ) << 8 );
			if(roomNo == pStarquakeViewer->State.CurrentScreen )
			{
				const int xPos = pCollectable->XPos * 8;
				const int yPos = pCollectable->YPos * 8;
				const uint8_t col = pCollectable->Colour;
				DrawItem( pCollectable->ItemNo, xPos, yPos, col, pStarquakeViewer, pGraphicsView);
			}
		}

		// screen item
		uint8_t itemX = ReadSpeccyByte( pUI->pSpeccy, 0xd2c0 );
		uint8_t itemY = ReadSpeccyByte( pUI->pSpeccy, 0xd2c1 );
		uint8_t itemType = ReadSpeccyByte( pUI->pSpeccy, 0xd2c2 );
		uint8_t itemCol = 0x7;	// TODO:

		if(itemX!=0)
		{
			DrawItem( itemType, itemX * 8, itemY * 8, itemCol, pStarquakeViewer, pGraphicsView );
		}

		DrawGraphicsView( *pGraphicsView );

		static bool bDrawDebug = false;

		ImGui::Checkbox( "Draw Debug", &bDrawDebug );

		if ( bDrawDebug )
		{
			for ( int sprNo = 0; sprNo < 6; sprNo++ )
			{
				FStarquakeSprite* pSprite = (FStarquakeSprite*)GetSpeccyMemPtr( pUI->pSpeccy, kSpriteList + ( sprNo * 32 ) );

				const ImVec2 windowsPos( pos.x + pSprite->XPixelPos, pos.y + 192 - pSprite->XPixelPos );
				pDrawList->AddRect( windowsPos, ImVec2( windowsPos.x + 16, windowsPos.y + 16 ), 0xffffffff );
			}
		}
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

void RegisterStarquakeViewer(FSpectrumEmu* pEmu)
{
	AddViewerConfig(&g_StarQuakeViewConfig);

	//pUI->GameConfigs.push_back(&g_StarQuakeConfig);
}