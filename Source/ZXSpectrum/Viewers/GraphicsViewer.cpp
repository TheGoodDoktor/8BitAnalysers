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
static int kScreenViewerWidth = 256;
static int kScreenViewerHeight = 192;


bool FGraphicsViewerState::Init(FSpectrumEmu* emuPtr)
{
	pEmu = emuPtr;
	const FCodeAnalysisState& state = pEmu->CodeAnalysis;

	assert(pGraphicsView == nullptr);
	assert(pScreenView == nullptr);
	pGraphicsView = new FZXGraphicsView(kGraphicsViewerWidth, kGraphicsViewerHeight);
	pScreenView = new FZXGraphicsView(kScreenViewerWidth, kScreenViewerHeight);

	return true;
}

void FGraphicsViewerState::Shutdown(void)
{
	delete pGraphicsView;
	pGraphicsView = nullptr;
	delete pScreenView;
	pScreenView = nullptr;
}

void FGraphicsViewerState::GoToAddress(FAddressRef address)
{
	FCodeAnalysisState& state = pEmu->CodeAnalysis;

	const FDataInfo* pDataInfo = state.GetReadDataInfoForAddress(address);
	if (pDataInfo->DataType == EDataType::Bitmap)
	{
		// see if we can find a graphics set
		const auto& graphicsSetIt = GraphicSets.find(pDataInfo->GraphicsSetRef);
		if (graphicsSetIt != GraphicSets.end())
		{
			const FGraphicsSet& graphicsSet = graphicsSetIt->second;
			XSizePixels = graphicsSet.XSizePixels;
			XSizePixels = graphicsSet.XSizePixels;
			ImageCount = graphicsSet.Count;
			address = graphicsSet.Address;
		}
		else
		{
			XSizePixels = pDataInfo->ByteSize * 8;
		}
	}

	const FCodeAnalysisBank* pBank = state.GetBank(address.BankId);

	if (pBank == nullptr || pBank->IsMapped())	// default to physical memory view
	{
		AddressOffset = address.Address;
		bShowPhysicalMemory = true;
	}
	else
	{
		AddressOffset = address.Address - pBank->GetMappedAddress();
		Bank = address.BankId;
		bShowPhysicalMemory = false;
	}

	
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

uint16_t FGraphicsViewerState::GetAddressOffsetFromPositionInView(int x,int y) const
{
	const int scaledViewWidth = kGraphicsViewerWidth / ViewScale;
	const int scaledViewHeight = kGraphicsViewerHeight / ViewScale;
	const int scaledX = x / ViewScale;
	const int scaledY = y / ViewScale;

	const int xSizeChars = XSizePixels >> 3;
	const FCodeAnalysisState& state = pEmu->CodeAnalysis;
	const int kHorizontalDispCharCount = (scaledViewWidth / 8);
	const FCodeAnalysisBank* pBank = state.GetBank(Bank);
	const uint16_t addrInput = AddressOffset;
	const int xCount = kHorizontalDispCharCount / xSizeChars;
	const int xSize = xCount * xSizeChars;
	const int xp = std::max(std::min(xSize, (scaledX / 8)), 0);
	const int yp = std::max(std::min(scaledViewHeight, scaledY), 0);
	const int column = xp / xSizeChars;
	const int columnSize = scaledViewHeight * xSizeChars;

	//ImGui::Text("ScaledX: %d, Scaled Y: %d", scaledX, scaledY);
	//ImGui::Text("xp: %d, yp: %d, column: %d", xp, yp, column);
	return (addrInput + (column * columnSize) + (scaledY * xSizeChars)) % MemorySize;
}

uint8_t GetHeatmapColourForMemoryAddress(const FCodeAnalysisPage& page, uint16_t addr, int currentFrameNo, int frameThreshold)
{
	const uint16_t pageAddress = addr & FCodeAnalysisPage::kPageMask;
	const FCodeInfo* pCodeInfo = page.CodeInfo[pageAddress];

	if (pCodeInfo)
	{
		const int framesSinceExecuted = currentFrameNo - pCodeInfo->FrameLastExecuted;
		if (pCodeInfo->FrameLastExecuted != -1 && (framesSinceExecuted < frameThreshold))
			return 6;	// yellow code
	}

	const FDataInfo& dataInfo = page.DataInfo[pageAddress];
	
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

void FGraphicsViewerState::DrawMemoryBankAsGraphicsColumn(int16_t bankId, uint16_t memAddr, int xPos, int columnWidth)
{
	FCodeAnalysisState& state = pEmu->CodeAnalysis;
	FCodeAnalysisBank* pBank = state.GetBank(bankId);
	const uint16_t bankSizeMask = pBank->SizeMask;

	for (int y = 0; y < kGraphicsViewerHeight; y++)
	{
		for (int xChar = 0; xChar < columnWidth; xChar++)
		{
			const uint16_t bankAddr = memAddr & bankSizeMask;
			const uint8_t charLine = pBank->Memory[bankAddr];
			FCodeAnalysisPage& page = pBank->Pages[bankAddr >> FCodeAnalysisPage::kPageShift];
			const uint8_t col = GetHeatmapColourForMemoryAddress(page, memAddr, state.CurrentFrameNo,HeatmapThreshold);
			pGraphicsView->DrawCharLine(charLine, xPos + (xChar * 8), y, col);

			memAddr++;
		}
	}
}

// WIP
// the idea is to store graphic sets that run sequentially in memory as these structures and have a viewer for them


// draw a graphic set to a graphics view
void DrawGraphicsSetToView(FZXGraphicsView* pGraphicsView, const FCodeAnalysisState& state, const FGraphicsSet& graphic)
{
}

// Viewer to view spectrum graphics
void FGraphicsViewerState::Draw()
{
	if (ImGui::Begin("Graphics View"))
	{
		if (ImGui::BeginTabBar("GraphicsViewTabBar"))
		{
			if (ImGui::BeginTabItem("GFX"))
			{
				DrawCharacterGraphicsViewer();
				ImGui::EndTabItem();
			}
			if (ImGui::BeginTabItem("Screen"))
			{
				DrawScreenViewer();
				ImGui::EndTabItem();
			}
		}
		ImGui::EndTabBar();
	}

	ImGui::End();
}

// UI widget - move?
bool StepInt(const char* title, int& val, int stepAmount)
{
	const int oldVal = val;
	ImGui::PushID(title);
	ImGui::Text(title);
	ImGui::SameLine();
	if (ImGui::Button("<<"))
		val -= stepAmount;
	ImGui::SameLine();
	if (ImGui::Button(">>"))
		val += stepAmount;
	ImGui::PopID();

	return val != oldVal;
}

void FGraphicsViewerState::UpdateCharacterGraphicsViewerImage(void)
{
	//FZXGraphicsView* pGraphicsView = viewerState.pGraphicsView;
	FCodeAnalysisState& state = pEmu->CodeAnalysis;

	const int kHorizontalDispCharCount = kGraphicsViewerWidth / 8;
	const int kVerticalDispPixCount = kGraphicsViewerHeight;

	XSizePixels = std::min(std::max(8, XSizePixels), kHorizontalDispCharCount * 8);
	YSizePixels = std::min(std::max(1, YSizePixels), kVerticalDispPixCount);

	const int scaledHDispCharCount = kHorizontalDispCharCount / ViewScale;
	const int scaledVDispPixCount = kVerticalDispPixCount / ViewScale;
	const int xcount = scaledHDispCharCount / (XSizePixels >> 3);
	const int ycount = scaledVDispPixCount / YSizePixels;

	int y = 0;
	int address = AddressOffset;
	const int xSizeChars = XSizePixels >> 3;

	if (ViewMode == GraphicsViewMode::Character)
	{
		for (int x = 0; x < xcount; x++)
		{
			const int16_t bankId = bShowPhysicalMemory ? state.GetBankFromAddress(address) : Bank;
			assert(bankId != -1);
			DrawMemoryBankAsGraphicsColumn(bankId, address & 0x3fff, x * XSizePixels, xSizeChars);

			address += xSizeChars * scaledVDispPixCount;
		}
	}
	else if (ViewMode == GraphicsViewMode::CharacterWinding)
	{
		const int graphicsUnitSize = (XSizePixels >> 3) * YSizePixels;
		int offsetX = 0;
		int offsetY = 0;
		for (int y = 0; y < ycount; y++)
		{
			for (int x = 0; x < xcount; x++)
			{
				// draw single item
				for (int yLine = 0; yLine < YSizePixels; yLine++)	// loop down scan lines
				{
					for (int xChar = 0; xChar < xSizeChars; xChar++)
					{
						const uint8_t* pImage = pEmu->GetMemPtr(address);
						const int xp = ((yLine & 1) == 0) ? xChar : (xSizeChars - 1) - xChar;
						if (address + graphicsUnitSize < 0xffff)
							pGraphicsView->DrawCharLine(*pImage, offsetX + (xp * 8), offsetY + yLine);
						address++;
					}
				}

				offsetX += XSizePixels;
			}
			offsetX = 0;
			offsetY += YSizePixels;
		}
		address += graphicsUnitSize;
	}
}

void FGraphicsViewerState::DrawCharacterGraphicsViewer(void)
{
	FCodeAnalysisState& state = pEmu->CodeAnalysis;
	FCodeAnalysisBank* pBank = state.GetBank(Bank);

	const int kHorizontalDispCharCount = kGraphicsViewerWidth / 8;
	const int kVerticalDispPixCount = kGraphicsViewerHeight;

	if (ImGui::Checkbox("Physical Memory", &bShowPhysicalMemory))
	{
		if (bShowPhysicalMemory == true)
		{
			AddressOffset = 0;
			MemorySize = 65536;
		}
		else
		{
			// get the first bank - better way?
			if(Bank == -1)
				Bank = state.GetBanks()[0].Id;

			FCodeAnalysisBank* pNewBank = state.GetBank(Bank);
			AddressOffset = 0;
			MemorySize = pNewBank->GetSizeBytes();
		}
	}

	// show combo for banked mode
	if (bShowPhysicalMemory == false)
	{
		if (ImGui::BeginCombo("Bank", GetBankText(state, Bank)))
		{
			const auto& banks = state.GetBanks();
			for (const auto& bank : banks)
			{
				if (ImGui::Selectable(GetBankText(state, bank.Id), Bank == bank.Id))
				{
					FCodeAnalysisBank* pNewBank = state.GetBank(bank.Id);
					Bank = bank.Id;
					AddressOffset = 0;
					MemorySize = pNewBank->GetSizeBytes();
				}
			}

			ImGui::EndCombo();
		}
	}

	ImGui::Combo("ViewMode", (int*)&ViewMode, "Character\0CharacterWinding", (int)GraphicsViewMode::Count);

	// View Scale
	ImGui::InputInt("Scale", &ViewScale, 1, 1);
	ViewScale = std::max(1, ViewScale);	// clamp
	const int viewSizeX = XSizePixels * ViewScale;
	const int viewSizeY = YSizePixels * ViewScale;
	const int xChars = XSizePixels >> 3;

	// Address input
	int addrInput = pBank ? pBank->GetMappedAddress() + AddressOffset : AddressOffset;
	ImGuiIO& io = ImGui::GetIO();
	ImVec2 pos = ImGui::GetCursorScreenPos();

	// Zoomed graphics view - put into class?
	ImVec2 uv0(0, 0);
	ImVec2 uv1(1.0f / (float)ViewScale, 1.0f / (float)ViewScale);
	ImVec2 size((float)kGraphicsViewerWidth, (float)kGraphicsViewerHeight);
	pGraphicsView->UpdateTexture();
	ImGui::Image((void*)pGraphicsView->GetTexture(), size, uv0, uv1);

	if (ImGui::IsItemHovered())
	{
		const int xp = (int)(io.MousePos.x - pos.x);
		const int yp = std::max((int)(io.MousePos.y - pos.y - (YSizePixels / 2)), 0);

		ImDrawList* dl = ImGui::GetWindowDrawList();
		const int rx = (xp / viewSizeX) * viewSizeX;
		const int ry = (yp / ViewScale) * ViewScale;
		const float rxp = pos.x + (float)rx;
		const float ryp = pos.y + (float)ry;
		dl->AddRect(ImVec2(rxp, ryp), ImVec2(rxp + (float)viewSizeX, ryp + (float)viewSizeY), 0xff00ffff);
		ImGui::BeginTooltip();
		const uint16_t gfxAddressOffset = GetAddressOffsetFromPositionInView(rx, ry);
		FAddressRef ptrAddress;
		if (pBank != nullptr)
			ptrAddress = FAddressRef(pBank->Id, gfxAddressOffset + pBank->GetMappedAddress());
		else
			ptrAddress = state.AddressRefFromPhysicalAddress(gfxAddressOffset);

		if (ImGui::IsMouseDoubleClicked(0))
		{
			state.GetFocussedViewState().GoToAddress(ptrAddress);
			addrInput = pBank ? pBank->GetMappedAddress() + gfxAddressOffset : gfxAddressOffset;
		}
		if (ImGui::IsMouseClicked(0))
			ClickedAddress = ptrAddress;

		ImGui::Text("%s", NumStr(ptrAddress.Address));
		ImGui::SameLine();
		DrawAddressLabel(state, state.GetFocussedViewState(), ptrAddress);
		ImGui::EndTooltip();
	}
		
	ImGui::SameLine();

	// simpler slider
	ImGui::VSliderInt("##int", ImVec2(64.0f, (float)kGraphicsViewerHeight), &addrInput, 0, 0xffff);
	
	ImGui::SetNextItemWidth(120.0f);
	if (GetNumberDisplayMode() == ENumberDisplayMode::Decimal)
		ImGui::InputInt("##Address", &addrInput, 1, 8, ImGuiInputTextFlags_CharsDecimal);
	else
		ImGui::InputInt("##Address", &addrInput, 1, 8, ImGuiInputTextFlags_CharsHexadecimal);
	DrawAddressLabel(state, state.GetFocussedViewState(), state.AddressRefFromPhysicalAddress(addrInput));

	// put in config? 
	//ImGui::SliderInt("Heatmap frame threshold", &viewerState.HeatmapThreshold, 0, 60);
	pGraphicsView->Clear(0xff000000);

	FCodeAnalysisBank* pClickedBank = state.GetBank(ClickedAddress.BankId);
	if(pClickedBank !=nullptr && state.Config.bShowBanks)
		ImGui::Text("Clicked Address: [%s]%s", pClickedBank->Name.c_str(), NumStr(ClickedAddress.Address));
	else
		ImGui::Text("Clicked Address: %s", NumStr(ClickedAddress.Address));
	//ImGui::SameLine();
	if (ClickedAddress.IsValid())
	{
		DrawAddressLabel(state, state.GetFocussedViewState(), ClickedAddress);
		if (ImGui::CollapsingHeader("Details"))
		{
			const int16_t bankId = bShowPhysicalMemory ? state.GetBankFromAddress(ClickedAddress.Address) : Bank;
			const FCodeAnalysisItem item(state.GetReadDataInfoForAddress(ClickedAddress), ClickedAddress);
			DrawDataDetails(state, state.GetFocussedViewState(), item);
		}
	}
	
	const int graphicsUnitSize = (XSizePixels >> 3) * YSizePixels;

	// step address based on image attributes
	StepInt("Step Line", addrInput, xChars);
	StepInt("Step Image", addrInput, graphicsUnitSize);

	// draw 64 * 8 bytes
	ImGui::InputInt("XSize", &XSizePixels, 8, 8);
	ImGui::InputInt("YSize", &YSizePixels, YSizePixelsFineCtrl? 1:8, 8);
	ImGui::SameLine();
	ImGui::Checkbox("Fine", &YSizePixelsFineCtrl);
	ImGui::InputInt("Count", &ImageCount, 1, 1);

	UpdateCharacterGraphicsViewerImage();	// update texture data

	// Image Set UI - move to function
	if (ImageCount > 0)
	{
		ImDrawList* dl = ImGui::GetWindowDrawList();
		const int scaledVDispPixCount = kVerticalDispPixCount / ViewScale;
		const int ycount = scaledVDispPixCount / YSizePixels;

		for (int i = 0; i < ImageCount; i++)
		{
			ImVec2 rectPos;
			rectPos.x = pos.x + static_cast<float>(i / ycount) * (float)viewSizeX;
			rectPos.y = pos.y + static_cast<float>(i % ycount) * (float)viewSizeY;
			dl->AddRect(rectPos, ImVec2(rectPos.x + (float)viewSizeX, rectPos.y + (float)viewSizeY), 0xff00ff00);
		}

		ImGui::InputText("Image Set Name", &ImageSetName);

		// currently this only works for physical memory
		if (ImGui::Button("Format Memory"))
		{
			FAddressRef addrRef = state.AddressRefFromPhysicalAddress(addrInput);
			uint16_t address = addrInput;

			for (int i = 0; i < ImageCount; i++)
			{
				FDataFormattingOptions format;
				format.AddLabelAtStart = true;

				// Generate label
				if (ImageSetName.empty() == false)
				{
					if (ImageCount > 1)
					{
						char numStr[8];
						sprintf(numStr, "_%d", i);
						format.LabelName = ImageSetName + numStr;
					}
					else
					{
						format.LabelName = ImageSetName;
					}
				}

				format.SetupForBitmap(address, XSizePixels, YSizePixels);
				format.GraphicsSetRef = addrRef;
				FormatData(state, format);
				state.SetCodeAnalysisDirty(address);

				address += graphicsUnitSize;
			}

			// Add graphic set
			FGraphicsSet graphicsSet;
			graphicsSet.Address = addrRef;
			graphicsSet.XSizePixels = XSizePixels;
			graphicsSet.YSizePixels = YSizePixels;
			graphicsSet.Count = ImageCount;
			GraphicSets[addrRef] = graphicsSet;
		}

		// TODO: We could have something to store the image set and reference it in address space somehow
		StepInt("Step Image Set", addrInput, graphicsUnitSize * ImageCount);

		for (const auto& graphicsSetIt : GraphicSets)
		{

		}
	}

	AddressOffset = pBank != nullptr ? (int)addrInput - pBank->GetMappedAddress() : addrInput;
}

// http://www.breakintoprogram.co.uk/computers/zx-spectrum/screen-memory-layout
void FGraphicsViewerState::DrawScreenViewer()
{
	FZXGraphicsView* pGraphicsView = pScreenView;
	const FCodeAnalysisState& state = pEmu->CodeAnalysis;	
	const int16_t bankId = Bank == -1 ? state.GetBankFromAddress(0x4000) : Bank;
	const FCodeAnalysisBank* pBank = state.GetBank(bankId);

	uint16_t bankAddr = 0;
	for (int y = 0; y < 192; y++)
	{
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
			const FCodeAnalysisPage& page = pBank->Pages[bankAddr >> 10];
			const uint8_t col = GetHeatmapColourForMemoryAddress(page, bankAddr, state.CurrentFrameNo, HeatmapThreshold);

			for (int xpix = 0; xpix < 8; xpix++)
			{
				const bool bSet = (charLine & (1 << (7 - xpix))) != 0;
				const uint32_t colRGBA = bSet ? g_kColourLUT[col] : 0xff000000;
				*(pLineAddr + xpix + (x * 8)) = colRGBA;
			}

			bankAddr++;
		}
	}

	pGraphicsView->Draw();
}

// Save/Load Graphics sets to json
#include <iomanip>
#include <fstream>
#include <sstream>
#include <json.hpp>
using json = nlohmann::json;

bool FGraphicsViewerState::SaveGraphicsSets(const char* pJsonFileName)
{
	return false; // TODO: remove when saving is implemented
	json jsonGraphicsSets;

	// TODO: fill up document

	// Write file out
	std::ofstream outFileStream(pJsonFileName);
	if (outFileStream.is_open())
	{
		outFileStream << std::setw(4) << jsonGraphicsSets << std::endl;
		return true;
	}

	return false;
}

bool FGraphicsViewerState::LoadGraphicsSets(const char* pJsonFileName)
{
	std::ifstream inFileStream(pJsonFileName);
	if (inFileStream.is_open() == false)
		return false;

	json jsonGraphicsSets;

	inFileStream >> jsonGraphicsSets;
	inFileStream.close();

	return true;
}