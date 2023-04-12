#include "StarquakeViewer.h"
#include "../Viewers/SpriteViewer.h"
#include "../SpectrumEmu.h"
#include <ImGuiSupport/ImGuiTexture.h>
#include <algorithm>
#include "GameViewer.h"
#include "../GameConfig.h"
#include "../Viewers/ZXGraphicsView.h"
#include <CodeAnalyser/UI/CodeAnalyserUI.h>

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
//static_assert( sizeof( FStarquakeSprite ) == 32 );

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
//static_assert( sizeof( FPlatformState ) == 4 );

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
//static_assert( sizeof( FItemState ) == 4 );

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
//static_assert( sizeof( FElectricHazardState ) == 8 );

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

void UpdateSQGameState(FSpectrumEmu* pEmu, FSQGameState& SQState)
{
	SQState.CurrentScreen = pEmu->ReadWord( kCurrentScreen );
	SQState.NoLives = pEmu->ReadByte( kNoLives );
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

static void DrawSmallPlatform(int platformNum, int xp,int yp,FStarquakeViewerData *pStarquakeViewer, FZXGraphicsView *pGraphicsView)
{
	FSpectrumEmu *pEmu = pStarquakeViewer->pEmu;
	//FGraphicsView *pGraphicsView = pStarquakeViewer->pSpriteGraphicsView;
	
	//const uint16_t kSmallPlatformLUT = 0xeb23;
	const uint16_t kPlatformPtr = kSmallPlatformGfxAddr + (platformNum * 2);

	//memset(pGraphicsView->PixelBuffer, 0, pGraphicsView->Width * pGraphicsView->Height * 4);
	uint8_t lutCol = pEmu->ReadByte( 0xea63 );
	
	// check the platform info of this platform
	const uint8_t typeInfo = pEmu->ReadByte( kSmallPlatformTypeInfoAddr + platformNum );
	if( (typeInfo & 0xf0) < 0x50 && (typeInfo & 0xf0) != 0)
	{
		const uint8_t lutIndex = typeInfo >> 4;
		lutCol = pEmu->ReadByte(kSmallPlatformColourLUT + lutIndex );
	}

	uint16_t kPlatformAddr = pEmu->ReadByte(kPlatformPtr);
	kPlatformAddr = kPlatformAddr | (pEmu->ReadByte(kPlatformPtr+1) << 8);
	uint16_t kPlatformCharPtr = kPlatformAddr + 6;	// pointer to char data
	uint16_t kPlatformColPtr = kPlatformAddr - 1;	// pointer to colour data
	for(int y=0;y<6;y++)
	{
		const uint8_t bits = pEmu->ReadByte( kPlatformAddr + y);
		for(int x=0;x<8;x++)
		{
			if((bits & (1<<(7-x))) !=0 )
			{
				const uint8_t *pImage = pEmu->GetMemPtr( kPlatformCharPtr);
				const uint8_t col = pEmu->ReadByte(kPlatformColPtr);
				uint8_t colVal = col & 0x3f;	// just the colour values - ink & paper
				if ( colVal == 0x36 )		// yellow paper, yellow ink
					colVal = ( col & 0xc0 ) | lutCol;// ReadSpeccyByte( pUI->pSpeccy, 0xea63 );	// use flash & bright from colour & read colour from 0xea63
				else if (colVal == 0)
					colVal = (col & 0xf8) | pEmu->ReadByte(0xea62);	// use flash, bright & paper
				else
					colVal = col;

				colVal &= ~0x40;	// clear bright bit
				
				pGraphicsView->DrawBitImage(pImage, xp + (x * 8), yp + (y * 8), 1, 1, colVal);

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
	
	const uint8_t kPlatformNo1 = pEmu->ReadByte(kBigPlatformData + 0);
	DrawSmallPlatform(kPlatformNo1,xp + 32, yp + 24, pStarquakeViewer, pStarquakeViewer->pScreenGraphicsView);
	const uint8_t kPlatformNo2 = pEmu->ReadByte(kBigPlatformData + 1);
	DrawSmallPlatform(kPlatformNo2, xp, yp + 24, pStarquakeViewer, pStarquakeViewer->pScreenGraphicsView);
	const uint8_t kPlatformNo3 = pEmu->ReadByte(kBigPlatformData + 2);
	DrawSmallPlatform(kPlatformNo3, xp + 32, yp, pStarquakeViewer, pStarquakeViewer->pScreenGraphicsView);
	const uint8_t kPlatformNo4 = pEmu->ReadByte(kBigPlatformData + 3);
	DrawSmallPlatform(kPlatformNo4, xp, yp, pStarquakeViewer, pStarquakeViewer->pScreenGraphicsView);


}

static void DrawItem( int itemNum, int xp, int yp, uint8_t col, FStarquakeViewerData* pStarquakeViewer, FZXGraphicsView* pGraphicsView )
{
	FSpectrumEmu* pEmu = pStarquakeViewer->pEmu;
	const uint8_t* pImage = pEmu->GetMemPtr(kItemSpriteDataAddr + (itemNum * 32) );

	pGraphicsView->DrawBitImageChars(pImage, xp, yp, 2, 2, col);
}

static void DrawPlatform( int platformNo, int xp, int yp, uint8_t col, FStarquakeViewerData* pStarquakeViewer, FZXGraphicsView* pGraphicsView )
{
	FSpectrumEmu* pEmu = pStarquakeViewer->pEmu;
	const uint8_t* pImage = pEmu->GetMemPtr(kPlatformSpritesAddr + ( platformNo * 16 ) );

	pGraphicsView->DrawBitImageChars(pImage, xp, yp, 2, 1, col);
}

static void DrawScreen(int screenNum, int xp, int yp, FStarquakeViewerData *pStarquakeViewer)
{
	FSpectrumEmu* pEmu = pStarquakeViewer->pEmu;
	uint16_t kScreenData = kScreenDataAddr + (screenNum * 12);

	for(int y=0;y<3;y++)
	{
		for(int x=0;x<4;x++)
		{
			const uint8_t kPlatformNo = pEmu->ReadByte(kScreenData);
			DrawBigPlatform(kPlatformNo, xp + (x * 64), yp + (y * 48), pStarquakeViewer);
			kScreenData++;
		}
	}

}

void FormatSmallPlatformMemory(FSpectrumEmu* pEmu, int platformNo)
{
	FCodeAnalysisState& state = pEmu->CodeAnalysis;
	const uint16_t kPlatformPtr = kSmallPlatformGfxAddr + (platformNo * 2);
	const uint16_t kPlatformAddr = state.CPUInterface->ReadWord(kPlatformPtr);

	char labelName[32];

	// TODO: Format the memory for this platform
	// Create Label
	//if (state.GetLabelForAddress(kPlatformAddr) == nullptr)
	{
		sprintf(labelName, "SmallPlatform_%d", platformNo);
		FLabelInfo* pLabel = AddLabel(state, kPlatformAddr, labelName, ELabelType::Data);
		pLabel->Global = true;
	}
	// Format Mask - 6 bytes bitmap
	FDataFormattingOptions format;
	format.SetupForBitmap(kPlatformAddr, 8, 6);
	FormatData(state, format);

	// find number of chars using mask
	int noPlatformChars = 0;
	int charCount[6] = { 0,0,0,0,0,0 };
	for (int maskNo = 0; maskNo < 6; maskNo++)
	{
		const uint8_t maskByte = pEmu->ReadByte(kPlatformAddr + maskNo);
		for (int bit = 0; bit < 8; bit++)
		{
			if (maskByte & (1 << bit))
			{
				noPlatformChars++;
				charCount[maskNo]++;
			}
		}
	}

	// Add attribute label
	if(noPlatformChars > 0)
	{
		sprintf(labelName, "SmallPlatform_%d_Attributes", platformNo);
		FLabelInfo* pLabel = AddLabel(state, kPlatformAddr - noPlatformChars, labelName, ELabelType::Data);
		pLabel->Global = true;

		format.StartAddress = kPlatformAddr - noPlatformChars;

		for (int i = 0; i < 6; i++)
		{
			if (charCount[i] > 0)
			{
				format.DataType = EDataType::ColAttr;
				format.ItemSize = charCount[i];
				format.NoItems = 1;
				FormatData(state, format);
				format.StartAddress += charCount[i];
			}
		}
	}

	// Format chars 
	uint16_t platCharAddr = kPlatformAddr + 6;
	for (int platChar = 0; platChar < noPlatformChars; platChar++)
	{
		sprintf(labelName, "SmallPlatform_%d_Char_%d", platformNo, platChar);
		FLabelInfo* pLabel = AddLabel(state, platCharAddr, labelName, ELabelType::Data);
		pLabel->Global = true;

		format.SetupForBitmap(platCharAddr, 8, 8);
		FormatData(state, format);

		platCharAddr += 8;
	}

	state.SetCodeAnalysisDirty(platCharAddr);
}

void FormatBigPlatformMemory(FSpectrumEmu* pEmu, int platformNo)
{
	FCodeAnalysisState& state = pEmu->CodeAnalysis;
	const uint16_t kBigPlatformData = kBigPlatformDataAddr + (platformNo * 4);
	char labelName[32];

	// Label
	sprintf(labelName, "BigPlatform_%d", platformNo);
	FLabelInfo* pLabel = AddLabel(state, kBigPlatformData, labelName, ELabelType::Data);
	pLabel->Global = true;

	// Format Charmap 2x2
	FDataFormattingOptions format;
	format.SetupForCharmap(kBigPlatformData, 2, 2);
	FormatData(state, format);
	state.SetCodeAnalysisDirty(kBigPlatformData);
}

void FormatScreenMemory(FSpectrumEmu* pEmu, int screenNo)
{
	FCodeAnalysisState& state = pEmu->CodeAnalysis;
	const uint16_t kScreenData = kScreenDataAddr + (screenNo * 12);
	char labelName[32];

	// Label
	sprintf(labelName, "Screen_%d", screenNo);
	FLabelInfo* pLabel = AddLabel(state, kScreenData, labelName, ELabelType::Data);
	pLabel->Global = true;

	// Format Charmap 4x3
	FDataFormattingOptions format;
	format.SetupForCharmap(kScreenData, 4, 3);
	FormatData(state, format);
	state.SetCodeAnalysisDirty(kScreenData);
}

void FormatPlatformMemory(FSpectrumEmu* pEmu, int platformNo)
{
	FCodeAnalysisState& state = pEmu->CodeAnalysis;
	const uint16_t platformAddr = kPlatformSpritesAddr + (platformNo * 16);
	char labelName[32];

	// Label
	sprintf(labelName, "Platform_%d", platformNo);
	FLabelInfo* pLabel = AddLabel(state, platformAddr, labelName, ELabelType::Data);
	pLabel->Global = true;

	// Format Bitmap 16x8
	FDataFormattingOptions format;
	format.SetupForBitmap(platformAddr, 16, 8);
	FormatData(state, format);
	state.SetCodeAnalysisDirty(platformAddr);
}

void DrawStarquakeViewer(FSpectrumEmu*pEmu, FGame *pGame)
{
	FStarquakeViewerData* pStarquakeViewer = (FStarquakeViewerData*)pGame->pViewerData;
	FCodeAnalysisState& state = pEmu->CodeAnalysis;

	UpdateSQGameState( pEmu, pStarquakeViewer->State );

	ImGui::BeginTabBar("StarquakeTabBar");

	if (ImGui::BeginTabItem("Small Platforms"))
	{
		static int platformNo = 0;
		FZXGraphicsView *pGraphicsView = pStarquakeViewer->pSpriteGraphicsView;
		ImGui::InputInt("Platform No", &platformNo);
		ImGui::Text( "%xh, Address:", platformNo );
		const uint16_t kPlatformPtr = kSmallPlatformGfxAddr + (platformNo * 2);
		const uint16_t kPlatformAddr = state.CPUInterface->ReadWord(kPlatformPtr);
		DrawAddressLabel(state, state.GetFocussedViewState(), kPlatformAddr);
		ImGui::SameLine();
		if (ImGui::Button("Format Memory"))
		{
			FormatSmallPlatformMemory(pEmu, platformNo);
		}
		const uint8_t SmallPlatformInfo = pEmu->ReadByte(kSmallPlatformTypeInfoAddr + platformNo );
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

		pGraphicsView->Clear(0xff000000);
		DrawSmallPlatform(platformNo, 0,0, pStarquakeViewer, pGraphicsView);
		pGraphicsView->Draw();

		ImGui::EndTabItem();
	}

	if (ImGui::BeginTabItem("Big Platforms"))
	{
		static int platformNo = 0;
		FZXGraphicsView *pGraphicsView = pStarquakeViewer->pScreenGraphicsView;
		ImGui::InputInt("Platform No", &platformNo);
		ImGui::Text( "%xh", platformNo );
		// show address label
		const uint16_t kBigPlatformData = kBigPlatformDataAddr + (platformNo * 4);
		DrawAddressLabel(state, state.GetFocussedViewState(), kBigPlatformData);

		// TODO: memory formatter - 2x2 char map
		ImGui::SameLine();
		if (ImGui::Button("Format"))
		{
			FormatBigPlatformMemory(pEmu, platformNo);
		}
		pGraphicsView->Clear(0xff000000);
		DrawBigPlatform(platformNo, 0, 0, pStarquakeViewer);
		pGraphicsView->Draw();
		ImGui::EndTabItem();
	}

	if ( ImGui::BeginTabItem( "Items" ) )
	{
		static int itemNo = 0;
		FZXGraphicsView* pGraphicsView = pStarquakeViewer->pScreenGraphicsView;
		ImGui::InputInt( "Item No", &itemNo );
		ImGui::Text( "%xh", itemNo );
		pGraphicsView->Clear(0xff000000);
		DrawItem( itemNo, 0, 0, 0x7, pStarquakeViewer, pGraphicsView );
		pGraphicsView->Draw();
		ImGui::EndTabItem();
	}
	if ( ImGui::BeginTabItem( "Platforms" ) )
	{
		static int platformNo = 0;
		FZXGraphicsView* pGraphicsView = pStarquakeViewer->pScreenGraphicsView;
		ImGui::InputInt( "Platform No", &platformNo );
		ImGui::Text( "%xh", platformNo );

		const uint16_t platformAddr = kPlatformSpritesAddr + (platformNo * 16);
		DrawAddressLabel(state, state.GetFocussedViewState(), platformAddr);
		ImGui::SameLine();
		if (ImGui::Button("Format"))
		{
			FormatPlatformMemory(pEmu, platformNo);
		}

		pGraphicsView->Clear(0xff000000);
		DrawPlatform( platformNo, 0, 0, 0x7, pStarquakeViewer, pGraphicsView);
		pGraphicsView->Draw();
		ImGui::EndTabItem();
	}
	if (ImGui::BeginTabItem("Screens"))
	{
		static int screenNo = 0;
		static bool getScreenFromGame = false;
		FGraphicsView *pGraphicsView = pStarquakeViewer->pScreenGraphicsView;
		ImGui::InputInt("Screen No", &screenNo);
		const uint16_t kScreenData = kScreenDataAddr + (screenNo * 12);
		DrawAddressLabel(state, state.GetFocussedViewState(), kScreenData);
		ImGui::SameLine();
		if (ImGui::Button("Format"))
		{
			FormatScreenMemory(pEmu, screenNo);
		}

		ImGui::Checkbox("Get from game", &getScreenFromGame);
		if (getScreenFromGame)
			screenNo = pStarquakeViewer->State.CurrentScreen;
		
		pGraphicsView->Clear(0xff000000);
		DrawScreen(screenNo, 0, 0, pStarquakeViewer);
		pGraphicsView->Draw();
		ImGui::EndTabItem();
	}

	if (ImGui::BeginTabItem("Sprites"))
	{
		DrawSpriteListGUI(pEmu->GraphicsViewer, pStarquakeViewer->pSpriteGraphicsView);
		ImGui::EndTabItem();
	}

	if ( ImGui::BeginTabItem( "Game View" ) )
	{
		FZXGraphicsView *pGraphicsView = pStarquakeViewer->pScreenGraphicsView;
		ImDrawList* pDrawList = ImGui::GetWindowDrawList();
		ImVec2 pos = ImGui::GetCursorScreenPos();
		pGraphicsView->Clear(0xff000000);

		// TODO: draw UI panel
		// 
		// Draw Game screen
		DrawScreen( pStarquakeViewer->State.CurrentScreen, 0, 48, pStarquakeViewer );

		// Draw Moving Sprites
		for ( int sprNo = 0; sprNo < 6; sprNo++ )
		{
			const FStarquakeSprite* pSprite = (FStarquakeSprite*)pEmu->GetMemPtr(kSpriteList + ( sprNo * (uint16_t)sizeof( FStarquakeSprite ) ) );

			const uint8_t* pImage = pEmu->GetMemPtr(pSprite->SpriteImagePtr );

			if (pSprite->XPixelPos > 16 || pSprite->YPixelPos > 16)
				pGraphicsView->DrawBitImage(pImage, pSprite->XCharacterPos * 8, 192 - pSprite->YPixelPos, 3, 2, pSprite->InkCol);
		}

		// draw platforms
		for ( int platNo = 0; platNo < 6; platNo++ )
		{
			const FPlatformState* pPlatform = (FPlatformState*)pEmu->GetMemPtr(kPlatformStates + ( platNo * (uint16_t)sizeof( FPlatformState ) ) );

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
			const FElectricHazardState* pElecHazardState = (FElectricHazardState *)pEmu->GetMemPtr( kElectricHazardStates + ( elecHaz * (uint16_t)sizeof( FElectricHazardState) ) );
		
			if ( pElecHazardState->XPos != 0 && pElecHazardState->Active != 0 )
			{
				DrawPlatform( 0x5, pElecHazardState->XPos * 8, pElecHazardState->YPos * 8, 0x7, pStarquakeViewer, pGraphicsView );
			}
		}
		
		// draw items
		for ( int collectableNo = 0; collectableNo < 12; collectableNo++ )
		{
			FItemState* pCollectable = (FItemState*)pEmu->GetMemPtr(kCollectableItemStates + ( collectableNo * 4 ) );
		
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
		uint8_t itemX = pEmu->ReadByte(0xd2c0 );
		uint8_t itemY = pEmu->ReadByte(0xd2c1 );
		uint8_t itemType = pEmu->ReadByte(0xd2c2 );
		uint8_t itemCol = 0x7;	// TODO:

		if(itemX!=0)
		{
			DrawItem( itemType, itemX * 8, itemY * 8, itemCol, pStarquakeViewer, pGraphicsView );
		}

		pGraphicsView->Draw();

		static bool bDrawDebug = false;

		ImGui::Checkbox( "Draw Debug", &bDrawDebug );

		if ( bDrawDebug )
		{
			for ( int sprNo = 0; sprNo < 6; sprNo++ )
			{
				FStarquakeSprite* pSprite = (FStarquakeSprite*)pEmu->GetMemPtr(kSpriteList + ( sprNo * 32 ) );

				const ImVec2 windowsPos( pos.x + pSprite->XPixelPos, pos.y + 192 - pSprite->YPixelPos );
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