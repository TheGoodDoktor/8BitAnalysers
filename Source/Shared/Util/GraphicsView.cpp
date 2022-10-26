#include "GraphicsView.h"
#include <imgui.h>
#include <ImGuiSupport/imgui_impl_lucidextra.h>
#include <cstdint>

void DisplayTextureInspector(const ImTextureID texture, float width, float height, bool bScale = false, bool bMagnifier = true);

FGraphicsView::FGraphicsView(int width, int height)
	: Width(width)
	, Height(height)
{
	Width = width;
	Height = height;
	PixelBuffer = new uint32_t[width * height];
	Texture = ImGui_ImplDX11_CreateTextureRGBA((uint8_t*)PixelBuffer, width, height);
}

FGraphicsView::~FGraphicsView()
{
	delete PixelBuffer;
	ImGui_ImplDX11_FreeTexture(Texture);
}

void FGraphicsView::Clear(const uint32_t col)
{
	for (int i = 0; i < Width * Height; i++)
		PixelBuffer[i] = col;
}

void FGraphicsView::Draw(float xSize, float ySize, bool bScale, bool bMagnifier)
{
	ImGui_ImplDX11_UpdateTextureRGBA(Texture, (uint8_t*)PixelBuffer);
	DisplayTextureInspector(Texture, xSize, ySize, bScale, bMagnifier);
}

void FGraphicsView::Draw(bool bMagnifier)
{
	Draw((float)Width, (float)Height, false, bMagnifier);
}

void FGraphicsView::DrawCharLine(uint8_t charLine, int xp, int yp, uint32_t inkCol, uint32_t paperCol)
{
	uint32_t* pBase = PixelBuffer + (xp + (yp * Width));

	for (int xpix = 0; xpix < 8; xpix++)
	{
		const bool bSet = (charLine & (1 << (7 - xpix))) != 0;
		const uint32_t col = bSet ? inkCol : paperCol;
		if (col != 0xFF000000)
			*(pBase + xpix) = col;
	}
}

void FGraphicsView::DrawBitImage(const uint8_t* pSrc, int xp, int yp, int widthChars, int heightChars,  uint32_t inkCol, uint32_t paperCol)
{
	uint32_t* pBase = PixelBuffer + (xp + (yp * Width));

	for (int y = 0; y < heightChars * 8; y++)
	{
		for (int x = 0; x < widthChars; x++)
		{
			const uint8_t charLine = *pSrc++;

			for (int xpix = 0; xpix < 8; xpix++)
			{
				const bool bSet = (charLine & (1 << (7 - xpix))) != 0;
				const uint32_t col = bSet ? inkCol : paperCol;
				if (col != 0xFF000000)
					*(pBase + xpix + (x * 8)) = col;
			}
		}

		pBase += Width;
	}
}

void FGraphicsView::DrawBitImageChars(const uint8_t* pSrc, int xp, int yp, int widthChars, int heightChars, uint32_t inkCol, uint32_t paperCol)
{
	for (int y = 0; y < heightChars; y++)
	{
		for (int x = 0; x < widthChars; x++)
		{
			DrawBitImage(pSrc, xp + (x * 8), yp + (y * 8), 1, 1, inkCol, paperCol);
			pSrc+=8;
		}
	}
}


void DisplayTextureInspector(const ImTextureID texture, float width, float height, bool bScale, bool bMagnifier)
{
	const float imgScale = 1.0f;
	ImVec2 pos = ImGui::GetCursorScreenPos();
	ImVec2 size(width, height);

	if (bScale)
	{
		const float scaledSize = ImGui::GetWindowContentRegionWidth() * imgScale;
		size = ImVec2(scaledSize, scaledSize);
	}

	ImGui::Image(texture, size);

	if (bMagnifier && ImGui::IsItemHovered())
	{
		ImGuiIO& io = ImGui::GetIO();
		const float my_tex_w = size.x;
		const float my_tex_h = size.y;

		ImGui::BeginTooltip();
		const float region_sz = 64.0f;
		float region_x = io.MousePos.x - pos.x - region_sz * 0.5f;
		if (region_x < 0.0f)
			region_x = 0.0f;
		else if (region_x > my_tex_w - region_sz)
			region_x = my_tex_w - region_sz;

		float region_y = io.MousePos.y - pos.y - region_sz * 0.5f;
		if (region_y < 0.0f)
			region_y = 0.0f;
		else if (region_y > my_tex_h - region_sz)
			region_y = my_tex_h - region_sz;

		const float zoom = 4.0f;

		//ImGui::Text("Min: (%.2f, %.2f)", region_x, region_y);
		//ImGui::Text("Max: (%.2f, %.2f)", region_x + region_sz, region_y + region_sz);
		ImVec2 uv0 = ImVec2((region_x) / my_tex_w, (region_y) / my_tex_h);
		ImVec2 uv1 = ImVec2((region_x + region_sz) / my_tex_w, (region_y + region_sz) / my_tex_h);
		ImGui::Image(texture, ImVec2(region_sz * zoom, region_sz * zoom), uv0, uv1, ImVec4(1.0f, 1.0f, 1.0f, 1.0f), ImVec4(1.0f, 1.0f, 1.0f, 0.5f));


		ImGui::EndTooltip();
	}
}
/*
void ClearGraphicsView(FGraphicsView& graphicsView, const uint32_t col)
{
	for (int i = 0; i < graphicsView.Width * graphicsView.Height; i++)
		graphicsView.PixelBuffer[i] = col;
}

void DrawGraphicsView(const FGraphicsView& graphicsView, float xSize, float ySize, bool bScale, bool bMagnifier)
{
	ImGui_ImplDX11_UpdateTextureRGBA(graphicsView.Texture, (uint8_t*)graphicsView.PixelBuffer);
	DisplayTextureInspector(graphicsView.Texture, xSize, ySize, bScale, bMagnifier);
}

void DrawGraphicsView(const FGraphicsView& graphicsView, bool bMagnifier)
{
	DrawGraphicsView(graphicsView, (float)graphicsView.Width, (float)graphicsView.Height, false, bMagnifier);
}*/
