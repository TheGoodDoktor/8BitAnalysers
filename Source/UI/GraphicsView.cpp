#include "GraphicsView.h"
#include <ImGuiSupport/imgui_impl_lucidextra.h>
#include "Speccy/Speccy.h"
#include "SpeccyUI.h"
#include "GameConfig.h"
#include <algorithm>
#include "CodeAnalyser/CodeAnalyserUI.h"

#include "misc/cpp/imgui_stdlib.h"

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

void DrawGraphicsView(const FGraphicsView &graphicsView,bool bMagnifier)
{
	DrawGraphicsView(graphicsView, ImVec2((float)graphicsView.Width, (float)graphicsView.Height), false,bMagnifier);
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
	0xFF000000,     // 0 - black
	0xFFFF0000,     // 1 - blue
	0xFF0000FF,     // 2 - red
	0xFFFF00FF,     // 3 - magenta
	0xFF00FF00,     // 4 - green
	0xFFFFFF00,     // 5 - cyan
	0xFF00FFFF,     // 6 - yellow
	0xFFFFFFFF,     // 7 - white
};




// coords are in pixel units
// w & h in characters
// 00000111 - ink
// 00111000 - paper
// 01000000 - bright
// 10000000 - flash

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
	for (int y = 0; y < h; y++)
	{
		for (int x = 0; x < w; x++)
		{
			const uint8_t charLine = *pSrc++;

			for (int xpix = 0; xpix < 8; xpix++)
			{
				const bool bSet = (charLine & (1 << (7 - xpix))) != 0;
				const uint32_t col = bSet ? inkCol : paperCol;
				if(col != 0xFF000000 )
					*(pBase + xpix + (x * 8)) = col;
			}
		}

		pBase += destWidth;
	}
}

uint16_t GetAddressFromPositionInView(FGraphicsViewerState &state, int x,int y)
{
	const int kHorizontalDispCharCount = kGraphicsViewerWidth / 8;

	const int addrInput = state.Address;
	const int xCount = kHorizontalDispCharCount / state.XSize;
	const int xSize = xCount * state.XSize;
	const int xp = std::max(std::min(xSize, x / 8), 0);
	const int yp = std::max(std::min(kGraphicsViewerHeight, y), 0);
	const int column = xp / state.XSize;
	const int columnSize = kGraphicsViewerHeight * state.XSize;

	ImGui::Text("xp: %d, yp: %d, column: %d", xp, yp, column);
	return (addrInput + xp) + (column * columnSize) + (y * state.XSize);
}

uint8_t GetHeatmapColourForMemoryAddress(FCodeAnalysisState &state, uint16_t addr, int frameThreshold)
{
	FDataInfo *pDataInfo = state.DataInfo[addr];
	FCodeInfo *pCodeInfo = state.CodeInfo[addr];
	uint8_t col = 7;	// white
	if (pCodeInfo)
	{
		const int framesSinceAccessed = state.CurrentFrameNo - pCodeInfo->FrameLastAccessed;
		if (pCodeInfo->FrameLastAccessed != -1 && (framesSinceAccessed < frameThreshold))
			col = 6;	// yellow code
	}
	else if (pDataInfo)
	{
		const int framesSinceWritten = state.CurrentFrameNo - pDataInfo->LastFrameWritten;
		const int framesSinceRead = state.CurrentFrameNo - pDataInfo->LastFrameRead;
		if (pDataInfo->LastFrameWritten != -1 && (framesSinceWritten < frameThreshold))
			col = 2;
		if (pDataInfo->LastFrameRead != -1 && (framesSinceRead < frameThreshold))
			col = 4;
	}

	return col;
}

void DrawMemoryAsGraphicsColumn(FGraphicsViewerState &state,uint16_t startAddr, int xPos, int columnWidth)
{
	uint16_t memAddr = startAddr;
	FGraphicsView *pGraphicsView = state.pGraphicsView;
	
	for (int y = 0; y < kGraphicsViewerHeight; y++)
	{
		for(int xChar =0;xChar<columnWidth;xChar++)
		{
			const uint8_t *pImage = GetSpeccyMemPtr(state.pSpeccy, memAddr);
			const uint8_t col = GetHeatmapColourForMemoryAddress(state.pUI->CodeAnalysis, memAddr, state.HeatmapThreshold);
			/*
			FDataInfo *pDataInfo = state.pUI->CodeAnalysis.DataInfo[memAddr];
			FCodeInfo *pCodeInfo = state.pUI->CodeAnalysis.CodeInfo[memAddr];
			uint8_t col = 7;	// white
			if (pCodeInfo)
			{
				const int framesSinceAccessed = state.pUI->CodeAnalysis.CurrentFrameNo - pCodeInfo->FrameLastAccessed;
				if (pCodeInfo->FrameLastAccessed != -1 && (framesSinceAccessed < state.HeatmapThreshold))
					col = 6;	// yellow code
			}
			else if (pDataInfo)
			{
				const int framesSinceWritten = state.pUI->CodeAnalysis.CurrentFrameNo - pDataInfo->LastFrameWritten;
				const int framesSinceRead = state.pUI->CodeAnalysis.CurrentFrameNo - pDataInfo->LastFrameRead;
				if (pDataInfo->LastFrameWritten != -1 && (framesSinceWritten < state.HeatmapThreshold))
					col = 2;
				if (pDataInfo->LastFrameRead != -1 && (framesSinceRead < state.HeatmapThreshold))
					col = 4;
			}*/

			PlotImageAt(pImage, xPos + (xChar * 8), y, 1, 1, pGraphicsView->PixelBuffer, kGraphicsViewerWidth,col);

			memAddr++;
		}
	}
}

// Viewer to view spectrum graphics
void DrawGraphicsViewer(FGraphicsViewerState &state)
{
	FGraphicsView *pGraphicsView = state.pGraphicsView;	

	int byteOff = 0;
	//const int offsetMax = 0xffff - ((kGraphicsViewerWidth / 8) * kGraphicsViewerHeight);
	const int kHorizontalDispCharCount = kGraphicsViewerWidth / 8;
	const int kVerticalDispPixCount = kGraphicsViewerHeight;

	if (ImGui::Begin("Graphics View") == false)
	{
		ImGui::End();
		return;
	}
	// Address input
	int addrInput = state.Address;
	ImGui::Text("Memory Map Address: %04Xh", addrInput);
	ImGuiIO& io = ImGui::GetIO();
	ImVec2 pos = ImGui::GetCursorScreenPos();
	DrawGraphicsView(*pGraphicsView);
	int ptrAddress = 0;
	if (ImGui::IsItemHovered())
	{
		const int xp = (int)(io.MousePos.x - pos.x);
		const int yp = (int)(io.MousePos.y - pos.y);

		ImDrawList* dl = ImGui::GetWindowDrawList();
		const int xPix = state.XSize * 8;
		const int rx = static_cast<int>(pos.x) + ((xp / xPix) * xPix);
		const int ry = static_cast<int>(pos.y) + ((yp / state.YSize) * state.YSize);
		dl->AddRect(ImVec2(rx, ry), ImVec2(rx + xPix, ry + state.YSize), 0xff00ffff);
		//const int addressOffset = (xp / 8) + (yp * (256 / 8));
		ImGui::BeginTooltip();
		ptrAddress = GetAddressFromPositionInView(state,xp, yp);
		if (ImGui::IsMouseDoubleClicked(0))
			CodeAnalyserGoToAddress(ptrAddress);
		if (ImGui::IsMouseClicked(0))
			state.ClickedAddress = ptrAddress;

		ImGui::Text("%04Xh", ptrAddress);
		ImGui::SameLine();
		DrawAddressLabel(GetSpeccyUI()->CodeAnalysis, ptrAddress);
		ImGui::EndTooltip();
	}
	
	ImGui::SameLine();

	static int kRowSize = kHorizontalDispCharCount * 8;
	int addrLine = addrInput / kRowSize;
	int addrOffset = addrInput % kRowSize;

	if(ImGui::VSliderInt("##int", ImVec2(64.0f, (float)kGraphicsViewerHeight), &addrLine,0, 0xffff / kRowSize))
	{
		addrInput = (addrLine * kRowSize) + addrOffset;
	}
	if (ImGui::SliderInt("##offset", &addrOffset, 0, kRowSize -1))
	{
		addrInput = (addrLine * kRowSize) + addrOffset;
	}
	
	ImGui::InputInt("Address", &addrInput, 1, 8, ImGuiInputTextFlags_CharsHexadecimal);
	

	int viewMode = (int)state.ViewMode;
	if(ImGui::Combo("ViewMode", &viewMode, "Character\0CharacterWinding\0Screen", (int)GraphicsViewMode::Count))
		state.ViewMode = (GraphicsViewMode)viewMode;
	ImGui::SliderInt("Heatmap frame threshold", &state.HeatmapThreshold, 0, 60);
	ClearGraphicsView(*pGraphicsView, 0xff000000);

	ImGui::Text("Clicked Address: %04Xh", state.ClickedAddress);
	ImGui::SameLine();
	DrawAddressLabel(state.pUI->CodeAnalysis, state.ClickedAddress);
	if(ImGui::CollapsingHeader("Details"))
	{
		DrawDataDetails(state.pUI->CodeAnalysis, state.pUI->CodeAnalysis.DataInfo[state.ClickedAddress]);
	}
	
	// view 1 - straight character
	if (state.ViewMode == GraphicsViewMode::Character || state.ViewMode == GraphicsViewMode::CharacterWinding)
	{
		const int graphicsUnitSize = state.XSize * state.YSize;

		//ImGui::Checkbox("Column Mode", &state.bColumnMode);
		if (ImGui::Button("<<"))
			addrInput -= graphicsUnitSize;
		ImGui::SameLine();
		if (ImGui::Button(">>"))
			addrInput += graphicsUnitSize;

		state.Address = (int)addrInput;
		// draw 64 * 8 bytes
		ImGui::InputInt("XSize", &state.XSize, 1, 4);
		ImGui::InputInt("YSize", &state.YSize, 8, 8);
		ImGui::InputInt("YSize Fine", &state.YSize, 1, 8);
		ImGui::InputInt("Count", &state.ImageCount, 1, 4);

		ImGui::Separator();
		ImGui::InputText("Config Name", &state.NewConfigName);
		ImGui::SameLine();
		if (ImGui::Button("Store"))
		{
			// Store this in the config map
			auto& spriteConfigs = state.pGame->pConfig->SpriteConfigs;
			if(spriteConfigs.find(state.NewConfigName) == spriteConfigs.end())	// not found - add
			{
				FSpriteDefConfig newConfig;
				newConfig.BaseAddress = state.Address;
				newConfig.Count = state.ImageCount;
				newConfig.Width = state.XSize;
				newConfig.Height = state.YSize / 8;	// sprite height in chars atm - TODO: move to line count
				spriteConfigs[state.NewConfigName] = newConfig;

				// TODO: tell sprite view to refresh
				GenerateSpriteListsFromConfig(state, state.pGame->pConfig);

				// TODO: Save Config?
			}
			
		}

		state.XSize = std::min(std::max(1, state.XSize), kHorizontalDispCharCount);
		state.YSize = std::min(std::max(1, state.YSize), kVerticalDispPixCount);

		const int xcount = kHorizontalDispCharCount / state.XSize;
		const int ycount = kVerticalDispPixCount / state.YSize;

		int y = 0;
		int address = state.Address;

		if (state.ViewMode == GraphicsViewMode::Character)
		{
			for (int x = 0; x < xcount; x++)
			{
				DrawMemoryAsGraphicsColumn(state, address, x * state.XSize * 8, state.XSize);
				address += state.XSize * kVerticalDispPixCount;
			}
		}
		else if (state.ViewMode == GraphicsViewMode::CharacterWinding)
		{
			int offsetX = 0;
			int offsetY = 0;
			for (int y = 0; y < ycount; y++)
			{
				for (int x = 0; x < xcount; x++)
				{
					// draw single item
					for (int yLine = 0; yLine < state.YSize; yLine++)	// loop down scan lines
					{
						for (int xChar = 0; xChar < state.XSize; xChar++)
						{
							const uint8_t *pImage = GetSpeccyMemPtr(state.pSpeccy, address);
							const int xp = ((yLine & 1) == 0) ? xChar : (state.XSize - 1) - xChar;
							if (address + graphicsUnitSize < 0xffff)
								PlotImageAt(pImage, offsetX + (xp * 8), offsetY + yLine, 1, 1, pGraphicsView->PixelBuffer, kGraphicsViewerWidth);
							address++;
						}
					}

					offsetX += state.XSize * 8;
				}
				offsetX = 0;
				offsetY += state.YSize;
			}
			address += graphicsUnitSize;
		}
	}
	else if (state.ViewMode == GraphicsViewMode::Screen)
	{
		// http://www.breakintoprogram.co.uk/computers/zx-spectrum/screen-memory-layout
		state.Address = (int)addrInput;

		int offset = 0;
		for (int y = 0; y < 192; y++)
		{
			if ((int)state.Address + offset > 0xffff)
				break;

			uint16_t addr = state.Address + offset;
			const uint8_t *pSrc = GetSpeccyMemPtr(state.pSpeccy, addr);
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
				const uint8_t col = GetHeatmapColourForMemoryAddress(state.pUI->CodeAnalysis, addr, state.HeatmapThreshold);
				
				for (int xpix = 0; xpix < 8; xpix++)
				{
					const bool bSet = (charLine & (1 << (7 - xpix))) != 0;
					const uint32_t colRGBA = bSet ? g_kColourLUT[col] : 0xff000000;
					*(pLineAddr + xpix + (x * 8)) = colRGBA;
				}

				addr++;
			}

			offset += 256 / 8;	// advance to next line
		}
	}

	
	ImGui::End();
}

void GraphicsViewerGoToAddress(uint16_t address)
{
	GetSpeccyUI()->GraphicsViewer.Address = address;
}