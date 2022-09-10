#include "GraphicsView.h"
#include <imgui.h>
#include <ImGuiSupport/imgui_impl_lucidextra.h>
#include <cstdint>

FGraphicsView* CreateGraphicsView(int width, int height)
{
	FGraphicsView* pNewView = new FGraphicsView;

	pNewView->Width = width;
	pNewView->Height = height;
	pNewView->PixelBuffer = new uint32_t[width * height];
	pNewView->Texture = ImGui_ImplDX11_CreateTextureRGBA((uint8_t*)pNewView->PixelBuffer, width, height);

	return pNewView;
}

void FreeGraphicsView(FGraphicsView* pView)
{
	delete pView->PixelBuffer;
	ImGui_ImplDX11_FreeTexture(pView->Texture);
	delete pView;
}

void DisplayTextureInspector(const ImTextureID texture, float width, float height, bool bScale = false, bool bMagnifier = true)
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
}
