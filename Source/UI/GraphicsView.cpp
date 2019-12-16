#include "GraphicsView.h"
#include "imgui_impl_lucidextra.h"
#include "Speccy/Speccy.h"
#include "SpeccyUI.h"
#include <algorithm>

FGraphicsView *CreateGraphicsView(int width, int height)
{
	FGraphicsView *pNewView = new FGraphicsView;

	pNewView->Width = width;
	pNewView->Height = height;
	pNewView->PixelBuffer = new uint32_t[width * height];
	pNewView->Texture = ImGui_ImplDX11_CreateTextureRGBA((uint8_t*)pNewView->PixelBuffer, width, height);

	return pNewView;
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

void ClearGraphicsView(FGraphicsView &graphicsView, const uint32_t col)
{
	for (int i = 0; i < graphicsView.Width * graphicsView.Height; i++)
		graphicsView.PixelBuffer[i] = col;
}

void DrawGraphicsView(const FGraphicsView &graphicsView, const ImVec2 &size, bool bScale, bool bMagnifier)
{
	ImGui_ImplDX11_UpdateTextureRGBA(graphicsView.Texture, (uint8_t*)graphicsView.PixelBuffer);
	DisplayTextureInspector(graphicsView.Texture, size.x, size.y, bScale,bMagnifier);
}

void DrawGraphicsView(const FGraphicsView &graphicsView)
{
	DrawGraphicsView(graphicsView, ImVec2((float)graphicsView.Width, (float)graphicsView.Height));
}

// Graphics Viewer
static int kGraphicsViewerWidth = 256;
static int kGraphicsViewerHeight = 512;

bool InitGraphicsViewer(FGraphicsViewerState &state)
{
	state.pGraphicsView = CreateGraphicsView(kGraphicsViewerWidth, kGraphicsViewerHeight);

	return true;
}

// speccy colour CLUT
static const uint32_t g_kColourLUT[8] =
{
	0xFF000000,     // black
	0xFFFF0000,     // blue
	0xFF0000FF,     // red
	0xFFFF00FF,     // magenta
	0xFF00FF00,     // green
	0xFFFFFF00,     // cyan
	0xFF00FFFF,     // yellow
	0xFFFFFFFF,     // white
};




// coords are in pixel units
// w & h in characters
void PlotImageAt(const uint8_t *pSrc, int xp, int yp, int w, int h, uint32_t *pDest, int destWidth, uint8_t colAttr)
{
	uint32_t* pBase = pDest + (xp + (yp * destWidth));
	uint32_t inkCol = g_kColourLUT[colAttr & 7];
	uint32_t paperCol = g_kColourLUT[(colAttr >> 3) & 7];

	if (0 == (colAttr & (1 << 6)))
	{
		// standard brightness
		inkCol &= 0xFFD7D7D7;
		paperCol &= 0xFFD7D7D7;
	}

	*pBase = 0;
	for (int y = 0; y < h * 8; y++)
	{
		for (int x = 0; x < w; x++)
		{
			const uint8_t charLine = *pSrc++;

			for (int xpix = 0; xpix < 8; xpix++)
			{
				const bool bSet = (charLine & (1 << (7 - xpix))) != 0;
				const uint32_t col = bSet ? inkCol : paperCol;
				*(pBase + xpix + (x * 8)) = col;
			}
		}

		pBase += destWidth;
	}
}

// Viewer to view spectrum graphics
void DrawGraphicsViewer(FGraphicsViewerState &state)
{
	FGraphicsView *pGraphicsView = state.pGraphicsView;	

	int byteOff = 0;
	const int offsetMax = 0xffff - ((kGraphicsViewerWidth / 8) * kGraphicsViewerHeight);
	const int kHorizontalDispCharCount = kGraphicsViewerWidth / 8;
	const int kVerticalDispCharCount = kGraphicsViewerHeight / 8;

	if (ImGui::Begin("Graphics View") == false)
		return;

	// Address input
	int addrInput = state.Address;
	ImGui::Text("Memory Map Address: %04Xh", addrInput);
	DrawGraphicsView(*pGraphicsView);
	ImGui::SameLine();
	
	int addrLine = addrInput / kHorizontalDispCharCount;
	int addrOffset = addrInput % kHorizontalDispCharCount;

	if(ImGui::VSliderInt("##int", ImVec2(64.0f, (float)kGraphicsViewerHeight), &addrLine, 0, offsetMax / kHorizontalDispCharCount))
	{
		addrInput = (addrLine * kHorizontalDispCharCount) + addrOffset;
	}
	if (ImGui::SliderInt("##offset", &addrOffset, 0, kHorizontalDispCharCount-1))
	{
		addrInput = (addrLine * kHorizontalDispCharCount) + addrOffset;
	}
	
	ImGui::InputInt("Address", &addrInput, 1, 8, ImGuiInputTextFlags_CharsHexadecimal);
		
	// view 1 - straight character
	if (state.ViewMode == GraphicsViewMode::Character)
	{
		const int graphicsUnitSize = state.XSize * state.YSize * 8;

		ClearGraphicsView(*pGraphicsView, 0xff000000);

		if (ImGui::Button("<<"))
			addrInput -= graphicsUnitSize;
		ImGui::SameLine();
		if (ImGui::Button(">>"))
			addrInput += graphicsUnitSize;

		state.Address = (int)addrInput;
		// draw 64 * 8 bytes
		ImGui::InputInt("XSize", &state.XSize, 1, 4);
		ImGui::InputInt("YSize", &state.YSize, 1, 4);

		static char configName[64];
		ImGui::Separator();
		ImGui::InputText("Config Name", configName, 64);
		ImGui::SameLine();
		if (ImGui::Button("Store"))
		{
			// TODO: store this in the config map
		}

		state.XSize = std::min(std::max(1, state.XSize), kHorizontalDispCharCount);
		state.YSize = std::min(std::max(1, state.YSize), kHorizontalDispCharCount);

		const int xcount = kHorizontalDispCharCount / state.XSize;
		const int ycount = kVerticalDispCharCount / state.YSize;

		int y = 0;
		uint16_t address = state.Address;
			
		for (int y = 0; y < ycount; y++)
		{
			for (int x = 0; x < xcount; x++)
			{
				const uint8_t *pImage = GetSpeccyMemPtr(state.pSpeccy, address);
				PlotImageAt(pImage, x * state.XSize * 8, y * state.YSize * 8, state.XSize, state.YSize, pGraphicsView->PixelBuffer, kGraphicsViewerWidth);
				address += graphicsUnitSize;
			}
		}
	}
	else if (state.ViewMode == GraphicsViewMode::Screen)
	{
		// http://www.breakintoprogram.co.uk/computers/zx-spectrum/screen-memory-layout
		state.Address = (int)addrInput;

		uint16_t offset = 0;
		for (int y = 0; y < 192; y++)
		{
			const uint8_t *pSrc = GetSpeccyMemPtr(state.pSpeccy, state.Address + offset);
			const int y0to2 = ((offset >> 8) & 7);
			const int y3to5 = ((offset >> 5) & 7) << 3;
			const int y6to7 = ((offset >> 11) & 3) << 6;
			const int yDestPos = y0to2 | y3to5 | y6to7;	// or offsets together

			// determine dest pointer for scanline
			uint32_t* pLineAddr = pGraphicsView->PixelBuffer + (yDestPos * kGraphicsViewerWidth);

			// pixel line
			for (int x = 0; x < 256 / 8; x++)
			{
				const uint8_t charLine = *pSrc++;

				for (int xpix = 0; xpix < 8; xpix++)
				{
					const bool bSet = (charLine & (1 << (7 - xpix))) != 0;
					const uint32_t col = bSet ? 0xffffffff : 0xff000000;
					*(pLineAddr + xpix + (x * 8)) = col;
				}
			}

			offset += 256 / 8;	// advance to next line
		}
	}

	
	ImGui::End();
}