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

struct FStarquakeViewerData
{
	unsigned char*	GraphicsViewPixelBuffer;
	ImTextureID		GraphicsViewTexture;

};

void InitStarquakeViewer(FStarquakeViewerData *pStarquakeViewer)
{
	// setup pixel buffer
	const int graphicsViewSize = 256;
	const size_t pixelBufferSize = graphicsViewSize * graphicsViewSize * 4;
	pStarquakeViewer->GraphicsViewPixelBuffer = new unsigned char[pixelBufferSize];

	pStarquakeViewer->GraphicsViewTexture = ImGui_ImplDX11_CreateTextureRGBA(pStarquakeViewer->GraphicsViewPixelBuffer, graphicsViewSize, graphicsViewSize);

	
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
		uint16_t speccyAddr = kBlobSpritesAddr + (blobSpriteNo * 3 * 2 * 8);
		const uint8_t *pImage = GetSpeccyMemPtr(*pUI->pSpeccy, speccyAddr);
		PlotImageAt(pImage, 0,0, 3, 2, (uint32_t*)pData->GraphicsViewPixelBuffer, 256);

		ImGui::Image(pData->GraphicsViewTexture, ImVec2(256, 256));

		ImGui::EndTabItem();
	}

	ImGui::EndTabBar();

	ImGui_ImplDX11_UpdateTextureRGBA(pData->GraphicsViewTexture, pData->GraphicsViewPixelBuffer);

}


void RegisterStarquakeViewer(FSpeccyUI *pUI)
{
	FGameViewer &viewer = AddGameViewer(pUI, "Starquake");
	viewer.pDrawFunction = DrawStarquakeViewer;

	FStarquakeViewerData* pData = new FStarquakeViewerData;

	InitStarquakeViewer(pData);
	viewer.pUserData = pData;
}