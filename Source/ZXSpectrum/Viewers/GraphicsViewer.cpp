#include "GraphicsViewer.h"
#include <ImGuiSupport/ImGuiTexture.h>

#include "ZXGraphicsView.h"
#include "../SpectrumEmu.h"
#include "../GameConfig.h"
#include <algorithm>
#include "CodeAnalyser/UI/CodeAnalyserUI.h"

#include "misc/cpp/imgui_stdlib.h"
#include <Util/Misc.h>


// Graphics Viewer
static int kGraphicsViewerWidth = 256;
static int kGraphicsViewerHeight = 512;

bool InitGraphicsViewer(FGraphicsViewerState &state)
{
	state.pGraphicsView = new FZXGraphicsView(kGraphicsViewerWidth, kGraphicsViewerHeight);

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
	const FCodeInfo *pCodeInfo = state.GetCodeInfoForAddress(addr);
	uint8_t col = 7;	// white

	if (pCodeInfo)
	{
		const int framesSinceExecuted = state.CurrentFrameNo - pCodeInfo->FrameLastExecuted;
		if (pCodeInfo->FrameLastExecuted != -1 && (framesSinceExecuted < frameThreshold))
			return 6;	// yellow code
	}
	
	const FDataInfo* pReadDataInfo = state.GetReadDataInfoForAddress(addr);
	if (pReadDataInfo && pReadDataInfo->LastFrameRead != -1)
	{
		const int framesSinceRead = state.CurrentFrameNo - pReadDataInfo->LastFrameRead;
		
		if (pReadDataInfo->LastFrameRead != -1 && (framesSinceRead < frameThreshold))
			return 4;	// green
	}

	const FDataInfo* pWriteDataInfo = state.GetWriteDataInfoForAddress(addr);
	if (pWriteDataInfo && pWriteDataInfo->LastFrameWritten != -1)
	{
		const int framesSinceWritten = state.CurrentFrameNo - pWriteDataInfo->LastFrameWritten;
		if (pWriteDataInfo->LastFrameWritten != -1 && (framesSinceWritten < frameThreshold))
			return 2; // red
	}

	return col;
}

void DrawMemoryAsGraphicsColumn(FGraphicsViewerState &state,uint16_t startAddr, int xPos, int columnWidth)
{
	uint16_t memAddr = startAddr;
	FZXGraphicsView *pGraphicsView = state.pGraphicsView;
	
	for (int y = 0; y < kGraphicsViewerHeight; y++)
	{
		for(int xChar =0;xChar<columnWidth;xChar++)
		{
			const uint8_t *pImage = state.pEmu->GetMemPtr(memAddr);
			const uint8_t col = GetHeatmapColourForMemoryAddress(state.pEmu->CodeAnalysis, memAddr, state.HeatmapThreshold);
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

			pGraphicsView->DrawCharLine(*pImage, xPos + (xChar * 8), y,col);

			memAddr++;
		}
	}
}

// Viewer to view spectrum graphics
void DrawGraphicsViewer(FGraphicsViewerState &state)
{
	FZXGraphicsView *pGraphicsView = state.pGraphicsView;	

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
	ImGui::Text("Memory Map Address: %s", NumStr((uint16_t)addrInput));
	ImGuiIO& io = ImGui::GetIO();
	ImVec2 pos = ImGui::GetCursorScreenPos();
	pGraphicsView->Draw();
	uint16_t ptrAddress = 0;
	if (ImGui::IsItemHovered())
	{
		const int xp = (int)(io.MousePos.x - pos.x);
		const int yp = (int)(io.MousePos.y - pos.y);

		ImDrawList* dl = ImGui::GetWindowDrawList();
		const int xPix = state.XSize * 8;
		const int rx = static_cast<int>(pos.x) + ((xp / xPix) * xPix);
		const int ry = static_cast<int>(pos.y) + ((yp / state.YSize) * state.YSize);
		dl->AddRect(ImVec2((float)rx, (float)ry), ImVec2((float)(rx + xPix), (float)(ry + state.YSize)), 0xff00ffff);
		//const int addressOffset = (xp / 8) + (yp * (256 / 8));
		ImGui::BeginTooltip();
		ptrAddress = GetAddressFromPositionInView(state,xp, yp);
		if (ImGui::IsMouseDoubleClicked(0))
			CodeAnalyserGoToAddress(state.pEmu->CodeAnalysis.GetFocussedViewState(), ptrAddress);
		if (ImGui::IsMouseClicked(0))
			state.ClickedAddress = ptrAddress;

		ImGui::Text("%s", NumStr(ptrAddress));
		ImGui::SameLine();
		DrawAddressLabel(state.pEmu->CodeAnalysis, state.pEmu->CodeAnalysis.GetFocussedViewState(), ptrAddress);
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
	
	if (GetNumberDisplayMode() == ENumberDisplayMode::Decimal)
		ImGui::InputInt("Address", &addrInput, 1, 8, ImGuiInputTextFlags_CharsDecimal);
	else
		ImGui::InputInt("Address", &addrInput, 1, 8, ImGuiInputTextFlags_CharsHexadecimal);
	

	int viewMode = (int)state.ViewMode;
	if(ImGui::Combo("ViewMode", &viewMode, "Character\0CharacterWinding\0Screen", (int)GraphicsViewMode::Count))
		state.ViewMode = (GraphicsViewMode)viewMode;
	ImGui::SliderInt("Heatmap frame threshold", &state.HeatmapThreshold, 0, 60);
	pGraphicsView->Clear(0xff000000);

	ImGui::Text("Clicked Address: %s", NumStr(state.ClickedAddress));
	ImGui::SameLine();
	DrawAddressLabel(state.pEmu->CodeAnalysis, state.pEmu->CodeAnalysis.GetFocussedViewState(), state.ClickedAddress);
	if(ImGui::CollapsingHeader("Details"))
	{
		const FCodeAnalysisItem item(state.pEmu->CodeAnalysis.GetReadDataInfoForAddress(state.ClickedAddress), state.ClickedAddress);
		DrawDataDetails(state.pEmu->CodeAnalysis, state.pEmu->CodeAnalysis.GetFocussedViewState(), item);
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
							const uint8_t *pImage = state.pEmu->GetMemPtr( address);
							const int xp = ((yLine & 1) == 0) ? xChar : (state.XSize - 1) - xChar;
							if (address + graphicsUnitSize < 0xffff)
								pGraphicsView->DrawCharLine(*pImage, offsetX + (xp * 8), offsetY + yLine );
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
		state.Address = 0x4000;// (int)addrInput;

		int offset = 0;
		for (int y = 0; y < 192; y++)
		{
			if ((int)state.Address + offset > 0xffff)
				break;

			uint16_t addr = state.Address + offset;
			const uint8_t *pSrc = state.pEmu->GetMemPtr(addr);
			const int y0to2 = ((offset >> 8) & 7);
			const int y3to5 = ((offset >> 5) & 7) << 3;
			const int y6to7 = ((offset >> 11) & 3) << 6;
			const int yDestPos = y0to2 | y3to5 | y6to7;	// or offsets together

			// determine dest pointer for scanline
			uint32_t* pLineAddr = pGraphicsView->GetPixelBuffer() + (yDestPos * kGraphicsViewerWidth);

			// pixel line
			for (int x = 0; x < 256 / 8; x++)
			{
				const uint8_t charLine = *pSrc++;
				const uint8_t col = GetHeatmapColourForMemoryAddress(state.pEmu->CodeAnalysis, addr, state.HeatmapThreshold);
				
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

