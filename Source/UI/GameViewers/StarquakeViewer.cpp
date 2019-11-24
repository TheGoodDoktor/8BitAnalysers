#include "StarquakeViewer.h"
#include "UI/SpeccyUI.h"
#include "imgui_impl_lucidextra.h"
#include <algorithm>

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

struct FSpriteDefList
{
	uint16_t	BaseAddress;
	int			Width;
	int			Height;

	std::vector< FSpriteDef>	Sprites;
};

void GenerateSpriteList(FSpriteDefList &spriteList, uint16_t startAddress, int count, int width, int height)
{
	spriteList.Sprites.clear();
	spriteList.BaseAddress = startAddress;
	spriteList.Width = width;
	spriteList.Height = height;

	uint16_t spriteAddress = startAddress;
	for (int i = 0; i < count; i++)
	{
		FSpriteDef blobDef;

		blobDef.Address = spriteAddress;
		blobDef.Width = width;
		blobDef.Height = height;

		spriteAddress += width * height * 8;	// 8 bytes per char
		spriteList.Sprites.push_back(blobDef);
	}
}

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

void DrawSpriteList(const FSpriteDefList &spriteList, int &selection, FGraphicsView *pGraphicsView, FSpeccy *pSpeccy)
{
	ImGui::InputInt("SpriteNo", &selection, 1, 1);
	selection = std::min(std::max(selection, 0), (int)spriteList.Sprites.size() - 1);

	DrawSpriteOnGraphicsView(spriteList.Sprites[selection], 0, 0, pGraphicsView, pSpeccy);
}


// StarQuake Stuuf

struct FStarquakeViewerData
{
	FGraphicsView*		pGraphicsView = nullptr;
	FSpriteDefList		BlobSprites;
};



void InitStarquakeViewer(FStarquakeViewerData *pStarquakeViewer)
{
	pStarquakeViewer->pGraphicsView = CreateGraphicsView(256, 256);
	GenerateSpriteList(pStarquakeViewer->BlobSprites, kBlobSpritesAddr, kNoBlobSprites, 3, 2);	
}

void DrawStarquakeViewer(FSpeccyUI *pUI, FGameViewer &viewer)
{
	FStarquakeViewerData* pStarquakeViewer = (FStarquakeViewerData*)viewer.pUserData;

	ImGui::BeginTabBar("StarquakeTabBar");

	if (ImGui::BeginTabItem("Small Platforms"))
	{
		ImGui::EndTabItem();
	}

	if (ImGui::BeginTabItem("Sprites"))
	{
		static int blobSpriteNo = 0;
		static int baseAddress = pStarquakeViewer->BlobSprites.BaseAddress;
		static int w = pStarquakeViewer->BlobSprites.Width;
		static int h = pStarquakeViewer->BlobSprites.Height;
		static int count = (int)pStarquakeViewer->BlobSprites.Sprites.size();
		ImGui::InputInt("BaseAddress", &baseAddress,1, 8, ImGuiInputTextFlags_CharsHexadecimal);
		ImGui::InputInt("Width", &w);
		ImGui::InputInt("Height", &h);
		ImGui::InputInt("Count", &count);
		if (ImGui::Button("Regenerate"))
		{
			GenerateSpriteList(pStarquakeViewer->BlobSprites, baseAddress, count, w, h);
		}
		
		DrawSpriteList(pStarquakeViewer->BlobSprites, blobSpriteNo, pStarquakeViewer->pGraphicsView, pUI->pSpeccy);
		DrawGraphicsView(*pStarquakeViewer->pGraphicsView);

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