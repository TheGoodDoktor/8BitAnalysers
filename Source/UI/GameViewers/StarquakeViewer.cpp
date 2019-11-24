#include "StarquakeViewer.h"
#include "UI/SpeccyUI.h"
#include "imgui_impl_lucidextra.h"

// Starquake addresses
static const uint16_t kPlatformGfxAddr = 0xeb23;
static const uint16_t kPlatformTypeInfoAddr = 0x9740;
static const uint16_t kBigPlatformDataAddr = 0x9840;
static const uint16_t kScreenDataAddr = 0x7530;

static const uint16_t kBlobSpritesAddr = 0xe074;

// Firelord
namespace Firelord
{
	static const uint16_t kFontAddr = 0xb240;
	static const uint16_t kBlobSpritesAddr = 0xb3b0;
}

// Graphics View Code - TODO: Move
struct FGraphicsView
{
	int				Width = 0;
	int				Height = 0;
	unsigned char*	PixelBuffer = nullptr;
	ImTextureID		Texture = nullptr;
};

FGraphicsView *CreateGraphicsView(int width, int height)
{
	FGraphicsView *pNewView = new FGraphicsView;

	pNewView->Width = width;
	pNewView->Height = height;
	const size_t pixelBufferSize = width * height * 4;
	pNewView->PixelBuffer = new unsigned char[pixelBufferSize];
	pNewView->Texture = ImGui_ImplDX11_CreateTextureRGBA(pNewView->PixelBuffer, width, height);

	return pNewView;
}

void DrawGraphicsView(const FGraphicsView &graphicsView,const ImVec2 &size)
{
	ImGui_ImplDX11_UpdateTextureRGBA(graphicsView.Texture, graphicsView.PixelBuffer);
	ImGui::Image(graphicsView.Texture, size);
}

void DrawGraphicsView(const FGraphicsView &graphicsView)
{
	DrawGraphicsView(graphicsView, ImVec2((float)graphicsView.Width, (float)graphicsView.Height));
}

// Sprite Stuff

// sprite definition
struct FSpriteDef
{
	uint16_t	Address;
	int			Width;	// width in chars
	int			Height;	// height in chars
};

// Sprite instance
struct FSprite
{
	FSpriteDef*	pDef;
	int			XPos;
	int			YPos;
};

void DrawSpriteOnGraphicsView(const FSpriteDef &spriteDef,int x,int y,FGraphicsView *pGraphicsView, const FSpeccy* pSpeccy)
{
	const uint8_t *pImage = GetSpeccyMemPtr(*pSpeccy, spriteDef.Address);
	PlotImageAt(pImage, 0, 0, spriteDef.Width, spriteDef.Height, (uint32_t*)pGraphicsView->PixelBuffer, pGraphicsView->Width);
}

// StarQuake Stuuf

struct FStarquakeViewerData
{
	FGraphicsView*				pGraphicsView = nullptr;
	std::vector< FSpriteDef>	BlobSprites;
};

void InitStarquakeViewer(FStarquakeViewerData *pStarquakeViewer)
{
	pStarquakeViewer->pGraphicsView = CreateGraphicsView(256, 256);

	const int kNoBlobSprites = 52;
	uint16_t spriteAddress = kBlobSpritesAddr;
	for(int i=0;i< kNoBlobSprites;i++)
	{
		FSpriteDef blobDef;

		blobDef.Address = spriteAddress;
		blobDef.Width = 3;
		blobDef.Height = 2;

		spriteAddress += 3 * 2 * 8;	// 8 bytes per char
		pStarquakeViewer->BlobSprites.push_back(blobDef);
	}
	
}

void DrawStarquakeViewer(FSpeccyUI *pUI, FGameViewer &viewer)
{
	FStarquakeViewerData* pData = (FStarquakeViewerData*)viewer.pUserData;

	ImGui::BeginTabBar("StarquakeTabBar");

	if (ImGui::BeginTabItem("Small Platforms"))
	{
		ImGui::EndTabItem();
	}

	if (ImGui::BeginTabItem("Blob Sprites"))
	{
		static int blobSpriteNo = 0;
		ImGui::InputInt("SpriteNo", &blobSpriteNo, 1, 1);

		DrawSpriteOnGraphicsView(pData->BlobSprites[blobSpriteNo], 0, 0, pData->pGraphicsView, pUI->pSpeccy);
		/*uint16_t speccyAddr = kBlobSpritesAddr + (blobSpriteNo * 3 * 2 * 8);
		const uint8_t *pImage = GetSpeccyMemPtr(*pUI->pSpeccy, speccyAddr);
		PlotImageAt(pImage, 0,0, 3, 2, (uint32_t*)pData->pGraphicsView->PixelBuffer, 256);
		*/
		DrawGraphicsView(*pData->pGraphicsView);

		ImGui::EndTabItem();
	}

	ImGui::EndTabBar();
}


void RegisterStarquakeViewer(FSpeccyUI *pUI)
{
	FGameViewer &viewer = AddGameViewer(pUI, "Starquake");
	viewer.pDrawFunction = DrawStarquakeViewer;

	FStarquakeViewerData* pData = new FStarquakeViewerData;

	InitStarquakeViewer(pData);
	viewer.pUserData = pData;
}