#include "GraphicsView.h"
#include "imgui_impl_lucidextra.h"


FGraphicsView *CreateGraphicsView(int width, int height)
{
	FGraphicsView *pNewView = new FGraphicsView;

	pNewView->Width = width;
	pNewView->Height = height;
	pNewView->PixelBuffer = new uint32_t[width * height];
	pNewView->Texture = ImGui_ImplDX11_CreateTextureRGBA((uint8_t*)pNewView->PixelBuffer, width, height);

	return pNewView;
}


void DisplayTextureInspector(const float imgScale, const ImTextureID texture, float width, float height)
{
	const float size = ImGui::GetWindowContentRegionWidth() * imgScale;
	ImVec2 pos = ImGui::GetCursorScreenPos();

	ImGui::Image(texture, ImVec2(size, size));
	if (ImGui::IsItemHovered())
	{
		ImGuiIO& io = ImGui::GetIO();
		const float my_tex_w = size;
		const float my_tex_h = size;

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
void ClearGraphicsView(const FGraphicsView &graphicsView, const uint32_t col)
{
	for (int i = 0; i < graphicsView.Width * graphicsView.Height; i++)
		graphicsView.PixelBuffer[i] = col;
}
void DrawGraphicsView(const FGraphicsView &graphicsView, const ImVec2 &size)
{
	ImGui_ImplDX11_UpdateTextureRGBA(graphicsView.Texture, (uint8_t*)graphicsView.PixelBuffer);
	//ImGui::Image(graphicsView.Texture, size);
	DisplayTextureInspector(1.0f, graphicsView.Texture, size.x, size.y);
}

void DrawGraphicsView(const FGraphicsView &graphicsView)
{
	DrawGraphicsView(graphicsView, ImVec2((float)graphicsView.Width, (float)graphicsView.Height));
}
