#include "GraphicsViewer.h"

#include <ImGuiSupport/ImGuiTexture.h>

#include "Util/GraphicsView.h"
#include <algorithm>
#include "CodeAnalyser/CodeAnalyser.h"
#include "CodeAnalyser/UI/CodeAnalyserUI.h"

#include <Util/Misc.h>
#include <imgui.h>
#include <misc/cpp/imgui_stdlib.h>

// Graphics Viewer
static int kMaxImageSize = 256;
static int kGraphicsViewerWidth = 256;
static int kGraphicsViewerHeight = 512;

bool FGraphicsViewer::Init(FCodeAnalysisState* pCodeAnalysisState)
{
	pCodeAnalysis = pCodeAnalysisState;

	assert(pGraphicsView == nullptr);
	assert(pScreenView == nullptr);
	assert(pItemView == nullptr);
	pGraphicsView = new FGraphicsView(kGraphicsViewerWidth, kGraphicsViewerHeight);
	pScreenView = new FGraphicsView(ScreenWidth, ScreenHeight);
	pItemView = new FGraphicsView(kMaxImageSize, kMaxImageSize);
	return true;
}

void FGraphicsViewer::Shutdown(void)
{
	delete pGraphicsView;
	pGraphicsView = nullptr;
	delete pScreenView;
	pScreenView = nullptr;
	delete pItemView;
	pItemView = nullptr;
}

void FGraphicsViewer::Reset(void)
{
	Bank = -1;
	AddressOffset = 0;
	bShowPhysicalMemory = true;

	XSizePixels = 8;
	YSizePixels = 8;
	ImageCount = 0;

	ClickedAddress = FAddressRef();
	ViewMode = GraphicsViewMode::CharacterBitmap;
	ViewScale = 1;
	YSizePixelsFineCtrl = false;

	ImageSetName = "";

	GraphicsSets.clear();
	SelectedGraphicSet = FAddressRef();

	ItemNo = 0;
	ImageGraphicSet = FAddressRef();
}


void FGraphicsViewer::GoToAddress(FAddressRef address)
{
	FCodeAnalysisState& state = GetCodeAnalysis();

	const FDataInfo* pDataInfo = state.GetReadDataInfoForAddress(address);
	if (pDataInfo->DataType == EDataType::Bitmap)
	{
		// see if we can find a graphics set
		const auto& graphicsSetIt = GraphicsSets.find(pDataInfo->GraphicsSetRef);
		if (graphicsSetIt != GraphicsSets.end())
		{
			const FGraphicsSet& graphicsSet = graphicsSetIt->second;
			XSizePixels = graphicsSet.XSizePixels;
			YSizePixels = graphicsSet.YSizePixels;
			ImageCount = graphicsSet.Count;
			address = graphicsSet.Address;
			SelectedGraphicSet = address;
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

uint16_t FGraphicsViewer::GetAddressOffsetFromPositionInView(int x, int y) const
{
	const int scaledViewWidth = kGraphicsViewerWidth / ViewScale;
	const int scaledViewHeight = kGraphicsViewerHeight / ViewScale;
	const int scaledX = x / ViewScale;
	const int scaledY = y / ViewScale;

	const int xSizeChars = XSizePixels >> 3;
	const FCodeAnalysisState& state = GetCodeAnalysis();
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

uint32_t GetHeatmapColourForMemoryAddress(const FCodeAnalysisPage& page, uint16_t addr, int currentFrameNo, int frameThreshold)
{
	const uint16_t pageAddress = addr & FCodeAnalysisPage::kPageMask;
	const FCodeInfo* pCodeInfo = page.CodeInfo[pageAddress];

	if (pCodeInfo)
	{
		const int framesSinceExecuted = currentFrameNo - pCodeInfo->FrameLastExecuted;
		if (pCodeInfo->FrameLastExecuted != -1 && (framesSinceExecuted < frameThreshold))
			return 0xFF00FFFF;	// yellow code
	}

	const FDataInfo& dataInfo = page.DataInfo[pageAddress];

	if (dataInfo.LastFrameWritten != -1)
	{
		const int framesSinceWritten = currentFrameNo - dataInfo.LastFrameWritten;
		if (framesSinceWritten < frameThreshold)
			return 0xFF0000FF; // red
	}

	if (dataInfo.LastFrameRead != -1)
	{
		const int framesSinceRead = currentFrameNo - dataInfo.LastFrameRead;
		if (framesSinceRead < frameThreshold)
			return 0xFF00FF00;	// green
	}

	return 0xFFFFFFFF;
}

void FGraphicsViewer::DrawMemoryBankAsGraphicsColumn(int16_t bankId, uint16_t memAddr, int xPos, int columnWidth)
{
	const FCodeAnalysisState& state = GetCodeAnalysis();
	const FCodeAnalysisBank* pBank = state.GetBank(bankId);
	const uint16_t bankSizeMask = pBank->SizeMask;

	const int kVerticalDispPixCount = kGraphicsViewerHeight;
	const int scaledVDispPixCount = kVerticalDispPixCount / ViewScale;
	const int ycount = scaledVDispPixCount / YSizePixels;

	for (int y = 0; y < ycount * YSizePixels; y++)
	{
		for (int xChar = 0; xChar < columnWidth; xChar++)
		{
			const uint16_t bankAddr = memAddr & bankSizeMask;
			const uint8_t charLine = pBank->Memory[bankAddr];
			FCodeAnalysisPage& page = pBank->Pages[bankAddr >> FCodeAnalysisPage::kPageShift];
			const uint32_t col = GetHeatmapColourForMemoryAddress(page, memAddr, state.CurrentFrameNo, HeatmapThreshold);
			pGraphicsView->DrawCharLine(charLine, xPos + (xChar * 8), y, col, 0);

			memAddr++;
		}
	}
}

// WIP
// the idea is to store graphic sets that run sequentially in memory as these structures and have a viewer for them


// draw a graphic set to a graphics view
void DrawGraphicsSetToView(FGraphicsView* pGraphicsView, const FCodeAnalysisState& state, const FGraphicsSet& graphic)
{
}

// Viewer to view spectrum graphics
void FGraphicsViewer::Draw()
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

void FGraphicsViewer::UpdateCharacterGraphicsViewerImage(void)
{
	const FCodeAnalysisState& state = GetCodeAnalysis();

	const int kHorizontalDispCharCount = kGraphicsViewerWidth / 8;
	const int kVerticalDispPixCount = kGraphicsViewerHeight;

	const int scaledHDispCharCount = kHorizontalDispCharCount / ViewScale;
	const int scaledVDispPixCount = kVerticalDispPixCount / ViewScale;
	const int xcount = scaledHDispCharCount / (XSizePixels >> 3);
	const int ycount = scaledVDispPixCount / YSizePixels;

	int address = AddressOffset;
	const int xSizeChars = XSizePixels >> 3;

	if (ViewMode == GraphicsViewMode::CharacterBitmap)
	{
		for (int x = 0; x < xcount; x++)
		{
			const int16_t bankId = bShowPhysicalMemory ? state.GetBankFromAddress(address) : Bank;
			assert(bankId != -1);
			DrawMemoryBankAsGraphicsColumn(bankId, address & 0x3fff, x * XSizePixels, xSizeChars);

			address += xSizeChars * ycount * YSizePixels;
		}
	}
	else if (ViewMode == GraphicsViewMode::CharacterBitmapWinding)
	{
		const int graphicsUnitSize = (XSizePixels >> 3) * YSizePixels;
		int offsetX = 0;
		int offsetY = 0;

		const int16_t bankId = bShowPhysicalMemory ? state.GetBankFromAddress(address) : Bank;
		const FCodeAnalysisBank* pBank = state.GetBank(bankId);
		const uint16_t bankSizeMask = pBank->SizeMask;

		for (int y = 0; y < ycount; y++)
		{
			for (int x = 0; x < xcount; x++)
			{
				// draw single item
				for (int yLine = 0; yLine < YSizePixels; yLine++)	// loop down scan lines
				{
					for (int xChar = 0; xChar < xSizeChars; xChar++)
					{
						const int xp = ((yLine & 1) == 0) ? xChar : (xSizeChars - 1) - xChar;

						const uint16_t bankAddr = address & bankSizeMask;
						const uint8_t charLine = pBank->Memory[bankAddr];
						FCodeAnalysisPage& page = pBank->Pages[bankAddr >> FCodeAnalysisPage::kPageShift];
						const uint32_t col = GetHeatmapColourForMemoryAddress(page, address, state.CurrentFrameNo, HeatmapThreshold);

						if (address + graphicsUnitSize < 0xffff)
							pGraphicsView->DrawCharLine(charLine, offsetX + (xp * 8), offsetY + yLine, col, 0);
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

void FGraphicsViewer::DrawCharacterGraphicsViewer(void)
{
	FCodeAnalysisState& state = GetCodeAnalysis();
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
			if (Bank == -1)
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

	ImGui::Combo("ViewMode", (int*)&ViewMode, "CharacterBitmap\0CharacterBitmapWinding", (int)GraphicsViewMode::Count);

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
	if (pClickedBank != nullptr && state.Config.bShowBanks)
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
	const float kNumSize = 80.0f;	// size for number GUI widget
	ImGui::SetNextItemWidth(kNumSize);
	ImGui::InputInt("XSize", &XSizePixels, 8, 8);
	//ImGui::SameLine();
	ImGui::SetNextItemWidth(kNumSize);
	ImGui::InputInt("YSize", &YSizePixels, YSizePixelsFineCtrl ? 1 : 8, 8);
	ImGui::SameLine();
	ImGui::Checkbox("Fine", &YSizePixelsFineCtrl);

	// clamp sizes
	XSizePixels = std::min(std::max(8, XSizePixels), kMaxImageSize);
	YSizePixels = std::min(std::max(1, YSizePixels), kMaxImageSize);
	
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

		// TODO: put in function?
		const FAddressRef baseAddrRef = bShowPhysicalMemory ? state.AddressRefFromPhysicalAddress(addrInput) : FAddressRef(pBank->Id, addrInput);

		if (ImGui::Button("Format Memory"))
		{

			FAddressRef imageAddressRef = baseAddrRef;	// updated per image

			for (int i = 0; i < ImageCount; i++)
			{
				FDataFormattingOptions format;
				format.AddLabelAtStart = true;
				format.ClearLabels = true;

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

				format.SetupForBitmap(imageAddressRef, XSizePixels, YSizePixels);
				format.GraphicsSetRef = imageAddressRef;
				FormatData(state, format);
				state.SetCodeAnalysisDirty(imageAddressRef);

				state.AdvanceAddressRef(imageAddressRef, graphicsUnitSize);
				//address += graphicsUnitSize;
			}

			// Add graphic set
			FGraphicsSet graphicsSet;
			graphicsSet.Name = ImageSetName;
			graphicsSet.Address = baseAddrRef;
			graphicsSet.XSizePixels = XSizePixels;
			graphicsSet.YSizePixels = YSizePixels;
			graphicsSet.Count = ImageCount;
			GraphicsSets[baseAddrRef] = graphicsSet;
			SelectedGraphicSet = baseAddrRef;
		}

		// TODO: We could have something to store the image set and reference it in address space somehow
		StepInt("Step Image Set", addrInput, graphicsUnitSize * ImageCount);

		if(SelectedGraphicSet.IsValid())
		{
			ImGui::Separator();
		
			bool bUpdate = ImageGraphicSet != baseAddrRef;
			ItemNo = std::min(ItemNo, ImageCount - 1);	// clamp
			bUpdate |= ImGui::SliderInt("Item Number", &ItemNo, 0, ImageCount - 1);

			// TODO: update image view - only if needed
			//if (bUpdate)
			{
				FAddressRef itemAddress = baseAddrRef;
				state.AdvanceAddressRef(itemAddress, ItemNo * graphicsUnitSize);
				for (int yp = 0; yp < YSizePixels; yp++)
				{
					for (int xp = 0; xp < xChars; xp++)
					{
						const uint8_t charLine = state.ReadByte(itemAddress);
						pItemView->DrawCharLine(charLine, xp * 8, yp, 0xffffffff, 0);
						state.AdvanceAddressRef(itemAddress, 1);
					}
				}
				pItemView->UpdateTexture();

				ImageGraphicSet = baseAddrRef;				
			}

			float scaleFactor = 4.0f;
			const ImVec2 uv0(0, 0);
			const ImVec2 uv1((float)XSizePixels / (float)kMaxImageSize, (float)YSizePixels / (float)kMaxImageSize);
			const ImVec2 size(std::min((float)XSizePixels * scaleFactor, (float)kMaxImageSize), std::min((float)YSizePixels * scaleFactor, (float)kMaxImageSize));
			ImGui::Image((void*)pItemView->GetTexture(), size, uv0, uv1);
		}
	}

	AddressOffset = pBank != nullptr ? (int)addrInput - pBank->GetMappedAddress() : addrInput;

	ImGui::Separator();

	// List graphic sets
	ImGui::Text("Graphic Sets");
	if (ImGui::BeginChild("GraphicSetListChild",ImVec2(0,0),true))
	{
		for (const auto& graphicsSetIt : GraphicsSets)
		{
			const FGraphicsSet& set = graphicsSetIt.second;
			bool bSelected = set.Address == SelectedGraphicSet;
			if (ImGui::Selectable(set.Name.c_str(), &bSelected))
			{
				ImageSetName = set.Name;
				GoToAddress(set.Address);
				state.GetFocussedViewState().GoToAddress(set.Address);
			}
		}
	}
	ImGui::EndChild();
	if (ImGui::Button("Delete"))
	{

	}

}



// Save/Load Graphics sets to json
#include <iomanip>
#include <fstream>
#include <sstream>
#include <json.hpp>
using json = nlohmann::json;

bool FGraphicsViewer::SaveGraphicsSets(const char* pJsonFileName)
{
	json jsonGraphicsSets;

	// Fill up document
	for (const auto& graphicsSetIt : GraphicsSets)
	{
		const FGraphicsSet& set = graphicsSetIt.second;
		json graphicsSetJson;
		graphicsSetJson["Name"] = set.Name;
		graphicsSetJson["AddressRef"] = set.Address.Val;
		graphicsSetJson["XSizePixels"] = set.XSizePixels;
		graphicsSetJson["YSizePixels"] = set.YSizePixels;
		graphicsSetJson["ImageCount"] = set.Count;

		jsonGraphicsSets["GraphicsSets"].push_back(graphicsSetJson);
	}

	// Write file out
	std::ofstream outFileStream(pJsonFileName);
	if (outFileStream.is_open())
	{
		outFileStream << std::setw(4) << jsonGraphicsSets << std::endl;
		return true;
	}

	// TODO: in the future we might want to save out the graphic sets as PNGs - wouldn't that be nice!

	return false;
}

bool FGraphicsViewer::LoadGraphicsSets(const char* pJsonFileName)
{
	std::ifstream inFileStream(pJsonFileName);
	if (inFileStream.is_open() == false)
		return false;

	json jsonGraphicsSets;

	inFileStream >> jsonGraphicsSets;
	inFileStream.close();

	if (jsonGraphicsSets.contains("GraphicsSets"))
	{
		GraphicsSets.clear();

		for (const auto& graphicsSetJson : jsonGraphicsSets["GraphicsSets"])
		{
			FGraphicsSet set;
			set.Name = graphicsSetJson["Name"];
			set.Address.Val = graphicsSetJson["AddressRef"];
			set.XSizePixels = graphicsSetJson["XSizePixels"];
			set.YSizePixels = graphicsSetJson["YSizePixels"];
			set.Count = graphicsSetJson["ImageCount"];
			GraphicsSets[set.Address] = set;
		}
	}

	return true;
}
