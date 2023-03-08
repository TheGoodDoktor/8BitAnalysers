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

#if 0
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
#endif

uint8_t GetHeatmapColourForMemoryAddress(FCodeAnalysisPage& page, uint16_t addr, int currentFrameNo, int frameThreshold)
{
	const uint16_t pageAddress = addr & 1023;
	const FCodeInfo* pCodeInfo = page.CodeInfo[pageAddress];

	if (pCodeInfo)
	{
		const int framesSinceExecuted = currentFrameNo - pCodeInfo->FrameLastExecuted;
		if (pCodeInfo->FrameLastExecuted != -1 && (framesSinceExecuted < frameThreshold))
			return 6;	// yellow code
	}

	FDataInfo& dataInfo = page.DataInfo[pageAddress];
	
	if (dataInfo.LastFrameWritten != -1)
	{
		const int framesSinceWritten = currentFrameNo - dataInfo.LastFrameWritten;
		if (framesSinceWritten < frameThreshold)
			return 2; // red
	}

	if (dataInfo.LastFrameRead != -1)
	{
		const int framesSinceRead = currentFrameNo - dataInfo.LastFrameRead;
		if (framesSinceRead < frameThreshold)
			return 4;	// green
	}	

	return 7;
}


#if 0
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

			pGraphicsView->DrawCharLine(*pImage, xPos + (xChar * 8), y,col);

			memAddr++;
		}
	}
}
#endif

void DrawMemoryBankAsGraphicsColumn(FGraphicsViewerState& viewerState, int16_t bankId, uint16_t memAddr, int xPos, int columnWidth)
{
	FZXGraphicsView* pGraphicsView = viewerState.pGraphicsView;
	FCodeAnalysisState& state = viewerState.pEmu->CodeAnalysis;
	FCodeAnalysisBank* pBank = state.GetBank(bankId);

	for (int y = 0; y < kGraphicsViewerHeight; y++)
	{
		for (int xChar = 0; xChar < columnWidth; xChar++)
		{
			uint16_t bankAddr = memAddr & 0x3fff;
			const uint8_t charLine = pBank->Memory[bankAddr];
			FCodeAnalysisPage& page = pBank->Pages[bankAddr >> 10];
			const uint8_t col = GetHeatmapColourForMemoryAddress(page, memAddr, state.CurrentFrameNo,viewerState.HeatmapThreshold);
			pGraphicsView->DrawCharLine(charLine, xPos + (xChar * 8), y, col);

			memAddr++;
		}
	}
}

const char* GetBankText(FCodeAnalysisState& state, int16_t bankId)
{
	const FCodeAnalysisBank* pBank = state.GetBank(bankId);

	if (pBank == nullptr)
		return "None";

	return pBank->Name.c_str();
}

// Viewer to view spectrum graphics
void DrawGraphicsViewer(FGraphicsViewerState &viewerState)
{
	FZXGraphicsView *pGraphicsView = viewerState.pGraphicsView;
	FCodeAnalysisState& state = viewerState.pEmu->CodeAnalysis;

	int byteOff = 0;
	//const int offsetMax = 0xffff - ((kGraphicsViewerWidth / 8) * kGraphicsViewerHeight);
	const int kHorizontalDispCharCount = kGraphicsViewerWidth / 8;
	const int kVerticalDispPixCount = kGraphicsViewerHeight;

	if (ImGui::Begin("Graphics View") == false)
	{
		ImGui::End();
		return;
	}

	if (ImGui::BeginCombo("Bank", GetBankText(state, viewerState.Bank)))
	{
		/*for (int i = 1; i < 4; i++)
		{
			const int16_t bankId = viewerState.pEmu->CurRAMBank[i];
			const int bankSize = 16 * 1024;
			const int bankStartAddr = i * bankSize;
			char mappedBankTxt[32];
			sprintf(mappedBankTxt, "0x%04X-0x%04X : %s", bankStartAddr, bankStartAddr + bankSize - 1, GetBankText(state, bankId));
			if (ImGui::Selectable(mappedBankTxt, viewerState.Bank == bankId))
				viewerState.Bank = bankId;
		}*/
		
		if (ImGui::Selectable(GetBankText(state, -1), viewerState.Bank == -1))
			viewerState.Bank = -1;

		const auto& banks = state.GetBanks();
		for (const auto& bank : banks)
		{
			if (ImGui::Selectable(GetBankText(state, bank.Id), viewerState.Bank == bank.Id))
				viewerState.Bank = bank.Id;
		}	

		ImGui::EndCombo();
	}

	// Address input
	int addrInput = viewerState.Address;
	ImGui::Text("viewerState Map Address: %s", NumStr((uint16_t)addrInput));
	ImGuiIO& io = ImGui::GetIO();
	ImVec2 pos = ImGui::GetCursorScreenPos();
	pGraphicsView->Draw();
	uint16_t ptrAddress = 0;
	if (ImGui::IsItemHovered())
	{
		const int xp = (int)(io.MousePos.x - pos.x);
		const int yp = (int)(io.MousePos.y - pos.y);

		ImDrawList* dl = ImGui::GetWindowDrawList();
		const int xPix = viewerState.XSize * 8;
		const int rx = static_cast<int>(pos.x) + ((xp / xPix) * xPix);
		const int ry = static_cast<int>(pos.y) + ((yp / viewerState.YSize) * viewerState.YSize);
		dl->AddRect(ImVec2((float)rx, (float)ry), ImVec2((float)(rx + xPix), (float)(ry + viewerState.YSize)), 0xff00ffff);
		//const int addressOffset = (xp / 8) + (yp * (256 / 8));
		ImGui::BeginTooltip();
		ptrAddress = GetAddressFromPositionInView(viewerState,xp, yp);
		if (ImGui::IsMouseDoubleClicked(0))
			CodeAnalyserGoToAddress(state.GetFocussedViewState(), ptrAddress);
		if (ImGui::IsMouseClicked(0))
			viewerState.ClickedAddress = ptrAddress;

		ImGui::Text("%s", NumStr(ptrAddress));
		ImGui::SameLine();
		DrawAddressLabel(state, state.GetFocussedViewState(), ptrAddress);
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
	

	int viewMode = (int)viewerState.ViewMode;
	if(ImGui::Combo("ViewMode", &viewMode, "Character\0CharacterWinding\0Screen", (int)GraphicsViewMode::Count))
		viewerState.ViewMode = (GraphicsViewMode)viewMode;
	ImGui::SliderInt("Heatmap frame threshold", &viewerState.HeatmapThreshold, 0, 60);
	pGraphicsView->Clear(0xff000000);

	ImGui::Text("Clicked Address: %s", NumStr(viewerState.ClickedAddress));
	ImGui::SameLine();
	DrawAddressLabel(state, state.GetFocussedViewState(), viewerState.ClickedAddress);
	if(ImGui::CollapsingHeader("Details"))
	{
		const int16_t bankId = viewerState.Bank != -1 ? viewerState.Bank : state.GetBankFromAddress(viewerState.ClickedAddress);
		const FCodeAnalysisItem item(state.GetReadDataInfoForAddress(viewerState.ClickedAddress),bankId, viewerState.ClickedAddress);
		DrawDataDetails(state, state.GetFocussedViewState(), item);
	}
	
	// view 1 - straight character
	if (viewerState.ViewMode == GraphicsViewMode::Character || viewerState.ViewMode == GraphicsViewMode::CharacterWinding)
	{
		const int graphicsUnitSize = viewerState.XSize * viewerState.YSize;

		//ImGui::Checkbox("Column Mode", &state.bColumnMode);
		if (ImGui::Button("<<"))
			addrInput -= graphicsUnitSize;
		ImGui::SameLine();
		if (ImGui::Button(">>"))
			addrInput += graphicsUnitSize;

		viewerState.Address = (int)addrInput;
		// draw 64 * 8 bytes
		ImGui::InputInt("XSize", &viewerState.XSize, 1, 4);
		ImGui::InputInt("YSize", &viewerState.YSize, 8, 8);
		ImGui::InputInt("YSize Fine", &viewerState.YSize, 1, 8);
		ImGui::InputInt("Count", &viewerState.ImageCount, 1, 4);

		ImGui::Separator();
		ImGui::InputText("Config Name", &viewerState.NewConfigName);
		ImGui::SameLine();
		if (ImGui::Button("Store"))
		{
			// Store this in the config map
			auto& spriteConfigs = viewerState.pGame->pConfig->SpriteConfigs;
			if(spriteConfigs.find(viewerState.NewConfigName) == spriteConfigs.end())	// not found - add
			{
				FSpriteDefConfig newConfig;
				newConfig.BaseAddress = viewerState.Address;
				newConfig.Count = viewerState.ImageCount;
				newConfig.Width = viewerState.XSize;
				newConfig.Height = viewerState.YSize / 8;	// sprite height in chars atm - TODO: move to line count
				spriteConfigs[viewerState.NewConfigName] = newConfig;

				// TODO: tell sprite view to refresh
				GenerateSpriteListsFromConfig(viewerState, viewerState.pGame->pConfig);

				// TODO: Save Config?
			}
			
		}

		viewerState.XSize = std::min(std::max(1, viewerState.XSize), kHorizontalDispCharCount);
		viewerState.YSize = std::min(std::max(1, viewerState.YSize), kVerticalDispPixCount);

		const int xcount = kHorizontalDispCharCount / viewerState.XSize;
		const int ycount = kVerticalDispPixCount / viewerState.YSize;

		int y = 0;
		int address = viewerState.Address;

		if (viewerState.ViewMode == GraphicsViewMode::Character)
		{
			for (int x = 0; x < xcount; x++)
			{
				int16_t bankId = viewerState.Bank;
				uint16_t bankAddress = address;
				if (bankId == -1)
				{
					bankId = state.GetBankFromAddress(address);
					if (bankId != -1)
						bankAddress = address - state.GetBank(bankId)->MappedPage * FCodeAnalysisPage::kPageSize;
				}
					
				DrawMemoryBankAsGraphicsColumn(viewerState, bankId, bankAddress, x* viewerState.XSize * 8, viewerState.XSize);
				//	DrawMemoryAsGraphicsColumn(viewerState, address, x * viewerState.XSize * 8, viewerState.XSize);

				address += viewerState.XSize * kVerticalDispPixCount;
			}
		}
		else if (viewerState.ViewMode == GraphicsViewMode::CharacterWinding)
		{
			int offsetX = 0;
			int offsetY = 0;
			for (int y = 0; y < ycount; y++)
			{
				for (int x = 0; x < xcount; x++)
				{
					// draw single item
					for (int yLine = 0; yLine < viewerState.YSize; yLine++)	// loop down scan lines
					{
						for (int xChar = 0; xChar < viewerState.XSize; xChar++)
						{
							const uint8_t *pImage = viewerState.pEmu->GetMemPtr( address);
							const int xp = ((yLine & 1) == 0) ? xChar : (viewerState.XSize - 1) - xChar;
							if (address + graphicsUnitSize < 0xffff)
								pGraphicsView->DrawCharLine(*pImage, offsetX + (xp * 8), offsetY + yLine );
							address++;
						}
					}

					offsetX += viewerState.XSize * 8;
				}
				offsetX = 0;
				offsetY += viewerState.YSize;
			}
			address += graphicsUnitSize;
		}
	}
	else if (viewerState.ViewMode == GraphicsViewMode::Screen)
	{
		// http://www.breakintoprogram.co.uk/computers/zx-spectrum/screen-memory-layout
		//viewerState.Address = 0x4000;// (int)addrInput;
		const int16_t bankId = state.GetBankFromAddress(0x4000);
		FCodeAnalysisBank* pBank = state.GetBank(bankId);

		uint16_t bankAddr = 0;
		for (int y = 0; y < 192; y++)
		{
			//if ((int)viewerState.Address + offset > 0xffff)
			//	break;

			//uint16_t addr = viewerState.Address + offset;
			//const uint8_t pixelLine = pBank->Memory[bankAddr];
			const int y0to2 = ((bankAddr >> 8) & 7);
			const int y3to5 = ((bankAddr >> 5) & 7) << 3;
			const int y6to7 = ((bankAddr >> 11) & 3) << 6;
			const int yDestPos = y0to2 | y3to5 | y6to7;	// or offsets together

			// determine dest pointer for scanline
			uint32_t* pLineAddr = pGraphicsView->GetPixelBuffer() + (yDestPos * kGraphicsViewerWidth);

			// pixel line
			for (int x = 0; x < 256 / 8; x++)
			{
				const uint8_t charLine = pBank->Memory[bankAddr];
				FCodeAnalysisPage& page = pBank->Pages[bankAddr >> 10];
				const uint8_t col = GetHeatmapColourForMemoryAddress(page, bankAddr, state.CurrentFrameNo, viewerState.HeatmapThreshold);

				//const uint8_t col = GetHeatmapColourForMemoryAddress(viewerState.pEmu->CodeAnalysis, addr, viewerState.HeatmapThreshold);
				
				for (int xpix = 0; xpix < 8; xpix++)
				{
					const bool bSet = (charLine & (1 << (7 - xpix))) != 0;
					const uint32_t colRGBA = bSet ? g_kColourLUT[col] : 0xff000000;
					*(pLineAddr + xpix + (x * 8)) = colRGBA;
				}

				bankAddr++;
			}

			//offset += 256 / 8;	// advance to next line
		}
	}

	
	ImGui::End();
}

