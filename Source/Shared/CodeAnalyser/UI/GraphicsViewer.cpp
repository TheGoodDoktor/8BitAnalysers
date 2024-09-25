#include "GraphicsViewer.h"

#include <ImGuiSupport/ImGuiTexture.h>

#include "Util/GraphicsView.h"
#include <algorithm>
#include "CodeAnalyser/CodeAnalyser.h"
#include "CodeAnalyser/UI/CodeAnalyserUI.h"
#include "CodeAnalyser/UI/ComboBoxes.h"

#include <Util/Misc.h>

#define IMGUI_DEFINE_MATH_OPERATORS
#include <imgui_internal.h>
#include <imgui.h>

#include <misc/cpp/imgui_stdlib.h>
#include <ImGuiSupport/ImGuiScaling.h>

// Graphics Viewer
static int kMaxImageSize = 256;
static int kGraphicsViewerWidth = 256;
static int kGraphicsViewerHeight = 512;

bool FGraphicsViewer::Init()
{
	pCodeAnalysis = &pEmulator->GetCodeAnalysis();

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
	ViewMode = EGraphicsViewMode::Bitmap;
	//ViewScale = 1;
	bYSizePixelsFineCtrl = false;

	ImageSetName = "";

	GraphicsSets.clear();
	SelectedGraphicSet = FAddressRef();

	ItemNo = 0;
	ImageGraphicSet = FAddressRef();
}


void FGraphicsViewer::GoToAddress(FAddressRef address)
{
	FCodeAnalysisState& state = GetCodeAnalysis();

	const FDataInfo* pDataInfo = state.GetDataInfoForAddress(address);
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
			const EBitmapFormat bitmapFormat = GetBitmapFormatForDisplayType(pDataInfo->DisplayType);
			const int bpp = bitmapFormat == EBitmapFormat::None ? 1 : GetBppForBitmapFormat(bitmapFormat);
			XSizePixels = (pDataInfo->ByteSize / bpp) * 8;
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
	const FCodeAnalysisState& state = GetCodeAnalysis();
	FGlobalConfig* pConfig = state.pGlobalConfig;

	const int scaledViewWidth = kGraphicsViewerWidth / pConfig->GfxViewerScale;
	const int scaledViewHeight = kGraphicsViewerHeight / pConfig->GfxViewerScale;
	// Deal with odd ViewScale values, when there is a gap between the bottom of the graphic column and the edge of the view. 
	const int roundedViewHeight = (scaledViewHeight / 8) * 8;
	const int scaledX = x / pConfig->GfxViewerScale;
	const int scaledY = y / pConfig->GfxViewerScale;

	const int xSizeChars = XSizePixels >> 3;
	const int kHorizontalDispCharCount = (scaledViewWidth / 8);
	const FCodeAnalysisBank* pBank = state.GetBank(Bank);
	const uint16_t addrInput = AddressOffset;
	const int xCount = kHorizontalDispCharCount / xSizeChars;
	const int xSize = xCount * xSizeChars;
	const int xp = std::max(std::min(xSize, (scaledX / 8)), 0);
	const int yp = std::max(std::min(roundedViewHeight, scaledY), 0);
	const int column = xp / xSizeChars;
	const int bpp = GetBppForBitmapFormat(BitmapFormat);
	const int columnSize = roundedViewHeight * xSizeChars;
	const int widthFactor = IsBitmapFormatDoubleWidth(BitmapFormat) ? 2 : 1;

	//ImGui::Text("ScaledX: %d, Scaled Y: %d", scaledX, scaledY);
	//ImGui::Text("xp: %d, yp: %d, column: %d", xp, yp, column);
	
	return (addrInput + (column * columnSize * (bpp / widthFactor)) + (scaledY * xSizeChars * bpp)) % MemorySize;
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

void FGraphicsViewer::DrawPhysicalMemoryAsGraphicsColumn(uint16_t memAddr, int xPos, int columnWidth)
{
	const FCodeAnalysisState& state = GetCodeAnalysis();
	FGlobalConfig* pConfig = state.pGlobalConfig;

	const int kVerticalDispPixCount = kGraphicsViewerHeight;
	const int scaledVDispPixCount = kVerticalDispPixCount / pConfig->GfxViewerScale;
	const int ycount = scaledVDispPixCount / YSizePixels;
	const uint32_t* pPaletteColours = GetPaletteFromPaletteNo(PaletteNo);
	const ICPUInterface* pCPUInterface = state.GetCPUInterface();

	for (int y = 0; y < ycount * YSizePixels; y++)
	{
		for (int xChar = 0; xChar < columnWidth; xChar++)
		{
			switch (BitmapFormat)
			{
			case EBitmapFormat::Bitmap_1Bpp:
			{
				const FCodeAnalysisPage* pPage = state.GetReadPage(memAddr);
				const uint32_t col = GetHeatmapColourForMemoryAddress(*pPage, memAddr, state.CurrentFrameNo, HeatmapThreshold);

				// handle masked view modes - move to another function?
				if (ViewMode == EGraphicsViewMode::MaskedInterleaved || ViewMode == EGraphicsViewMode::MaskedInterleavedZigZag)
				{
					const uint8_t maskLine = pCPUInterface->ReadByte(memAddr++);
					const uint8_t charLine = pCPUInterface->ReadByte(memAddr++);
					if(ViewMode == EGraphicsViewMode::MaskedInterleaved || (y & 1) == 0)
						pGraphicsView->DrawMaskedCharLine(charLine,maskLine, xPos + (xChar * 8), y, col, 0);
					else
						pGraphicsView->DrawMaskedCharLine(charLine, maskLine, xPos + ((columnWidth - 1 - xChar) * 8), y, col, 0);
				}
				else
				{ 
					const uint8_t charLine = pCPUInterface->ReadByte(memAddr);

					pGraphicsView->DrawCharLine(charLine, xPos + (xChar * 8), y, col, 0);
					memAddr++;
				}
			}
			break;
			case EBitmapFormat::ColMap2Bpp_CPC:
			{
				const uint8_t* pPixels = pCPUInterface->GetMemPtr(memAddr);
				pGraphicsView->Draw2BppImageAt(pPixels, xPos + (xChar * 8), y, 8, 1, pPaletteColours ? pPaletteColours : GetCurrentPalette());
				memAddr += 2;
			}
			break;
			case EBitmapFormat::ColMap4Bpp_CPC:
			{
				const uint8_t* pPixels = pCPUInterface->GetMemPtr(memAddr);
				pGraphicsView->Draw4BppWideImageAt(pPixels, xPos + (xChar * 8), y, 4, 1, pPaletteColours ? pPaletteColours : GetCurrentPalette());
				memAddr += 2;
			}
			break;
			case EBitmapFormat::ColMapMulticolour_C64:
			{
				const uint8_t* pPixels = pCPUInterface->GetMemPtr(memAddr);
				pGraphicsView->Draw2BppWideImageAt(pPixels, xPos + (xChar * 8), y, 8, 1, pPaletteColours ? pPaletteColours : GetCurrentPalette());
				memAddr++;
			}
			break;
            default:
            break;
			}
		}
	}
}

void FGraphicsViewer::DrawPhysicalMemoryAsGraphicsColumnChars(uint16_t memAddr, int xPos, int columnWidth)
{
	const FCodeAnalysisState& state = GetCodeAnalysis();
	FGlobalConfig* pConfig = state.pGlobalConfig;

	const int kVerticalDispPixCount = kGraphicsViewerHeight;
	const int scaledVDispPixCount = kVerticalDispPixCount / pConfig->GfxViewerScale;
	const int ycount = scaledVDispPixCount / 8;
	const uint32_t* pPaletteColours = GetPaletteFromPaletteNo(PaletteNo);
	const ICPUInterface* pCPUInterface = state.GetCPUInterface();

	for (int y = 0; y < ycount; y++)
	{
		for (int xChar = 0; xChar < columnWidth; xChar++)
		{
			switch (BitmapFormat)
			{
				case EBitmapFormat::Bitmap_1Bpp:
				{
					const uint8_t* pPixels = pCPUInterface->GetMemPtr(memAddr);
					const FCodeAnalysisPage* pPage = state.GetReadPage(memAddr);
					const uint32_t col = GetHeatmapColourForMemoryAddress(*pPage, memAddr, state.CurrentFrameNo, HeatmapThreshold);
					const uint32_t cols[] = { 0, col };
					pGraphicsView->Draw1BppImageAt(pPixels, xPos + (xChar * 8), y * 8, 8, 8, cols);
					memAddr += 8;
				}
				break;
				case EBitmapFormat::ColMap2Bpp_CPC:
				{
					const uint8_t* pPixels = pCPUInterface->GetMemPtr(memAddr);
					pGraphicsView->Draw2BppImageAt(pPixels, xPos + (xChar * 8), y * 8, 8, 8, pPaletteColours ? pPaletteColours : GetCurrentPalette());

					memAddr += 2 * 8;
				}
				break;
				case EBitmapFormat::ColMap4Bpp_CPC:
				{
					const uint8_t* pPixels = pCPUInterface->GetMemPtr(memAddr);
					pGraphicsView->Draw4BppWideImageAt(pPixels, xPos + (xChar * 8), y * 8, 4, 8, pPaletteColours ? pPaletteColours : GetCurrentPalette());

					memAddr += 2 * 8;
				}
				break;
				case EBitmapFormat::ColMapMulticolour_C64:
				{
					const uint8_t* pPixels = pCPUInterface->GetMemPtr(memAddr);
					pGraphicsView->Draw2BppWideImageAt(pPixels, xPos + (xChar * 8), y * 8, 8, 8, pPaletteColours ? pPaletteColours : GetCurrentPalette());
					memAddr += 8;
				}
				break;
                default:
                break;
			}
		}
	}
}

void FGraphicsViewer::DrawMemoryBankAsGraphicsColumn(int16_t bankId, uint16_t memAddr, int xPos, int columnWidth)
{
	const FCodeAnalysisState& state = GetCodeAnalysis();
	FGlobalConfig* pConfig = state.pGlobalConfig;

	const FCodeAnalysisBank* pBank = state.GetBank(bankId);
	const uint16_t bankSizeMask = pBank->SizeMask;

	const int kVerticalDispPixCount = kGraphicsViewerHeight;
	const int scaledVDispPixCount = kVerticalDispPixCount / pConfig->GfxViewerScale;
	const int ycount = scaledVDispPixCount / YSizePixels;
	const uint32_t* pPaletteColours = GetPaletteFromPaletteNo(PaletteNo);

	for (int y = 0; y < ycount * YSizePixels; y++)
	{
		for (int xChar = 0; xChar < columnWidth; xChar++)
		{
			const uint16_t bankAddr = memAddr & bankSizeMask;
			switch (BitmapFormat)
			{
				case EBitmapFormat::Bitmap_1Bpp:
				{
					const uint8_t charLine = pBank->Memory[bankAddr];
					FCodeAnalysisPage& page = pBank->Pages[bankAddr >> FCodeAnalysisPage::kPageShift];
					const uint32_t col = GetHeatmapColourForMemoryAddress(page, memAddr, state.CurrentFrameNo, HeatmapThreshold);
					pGraphicsView->DrawCharLine(charLine, xPos + (xChar * 8), y, col, 0);

					memAddr++;
				}
				break;
				case EBitmapFormat::ColMap2Bpp_CPC:
				{
					const uint8_t* pPixels = &pBank->Memory[bankAddr];
					pGraphicsView->Draw2BppImageAt(pPixels, xPos + (xChar * 8), y, 8, 1, pPaletteColours ? pPaletteColours : GetCurrentPalette());

					memAddr += 2;
				}
				break;
				case EBitmapFormat::ColMap4Bpp_CPC:
				{
					const uint8_t* pPixels = &pBank->Memory[bankAddr];
					pGraphicsView->Draw4BppWideImageAt(pPixels, xPos + (xChar * 8), y, 4, 1, pPaletteColours ? pPaletteColours : GetCurrentPalette());

					memAddr += 2;
				}
				break;
				case EBitmapFormat::ColMapMulticolour_C64:
				{
					const uint8_t* pPixels = &pBank->Memory[bankAddr];
					pGraphicsView->Draw2BppWideImageAt(pPixels, xPos + (xChar * 8), y, 8, 1, pPaletteColours ? pPaletteColours : GetCurrentPalette());
					memAddr ++;
				}
				break;
                default:
                break;
			}
		}
	}
}

// draw columns using 8x8 chars
void FGraphicsViewer::DrawMemoryBankAsGraphicsColumnChars(int16_t bankId, uint16_t memAddr, int xPos, int columnWidth)
{
	const FCodeAnalysisState& state = GetCodeAnalysis();
	FGlobalConfig* pConfig = state.pGlobalConfig;

	const FCodeAnalysisBank* pBank = state.GetBank(bankId);
	const uint16_t bankSizeMask = pBank->SizeMask;

	const int kVerticalDispPixCount = kGraphicsViewerHeight;
	const int scaledVDispPixCount = kVerticalDispPixCount / pConfig->GfxViewerScale;
	const int ycount = scaledVDispPixCount / 8;
	const uint32_t* pPaletteColours = GetPaletteFromPaletteNo(PaletteNo);

	for (int y = 0; y < ycount; y++)
	{
		for (int xChar = 0; xChar < columnWidth; xChar++)
		{
			const uint16_t bankAddr = memAddr & bankSizeMask;
			switch (BitmapFormat)
			{
			case EBitmapFormat::Bitmap_1Bpp:
			{
				const uint8_t* pPixels = &pBank->Memory[bankAddr];
				FCodeAnalysisPage& page = pBank->Pages[bankAddr >> FCodeAnalysisPage::kPageShift];
				const uint32_t col = GetHeatmapColourForMemoryAddress(page, memAddr, state.CurrentFrameNo, HeatmapThreshold);
				const uint32_t cols[] = { 0, col};
				pGraphicsView->Draw1BppImageAt(pPixels, xPos + (xChar * 8), y * 8, 8, 8, cols);
				memAddr+=8;
			}
			break;
			case EBitmapFormat::ColMap2Bpp_CPC:
			{
				const uint8_t* pPixels = &pBank->Memory[bankAddr];
				pGraphicsView->Draw2BppImageAt(pPixels, xPos + (xChar * 8), y * 8, 8, 8, pPaletteColours ? pPaletteColours : GetCurrentPalette());

				memAddr += 2 * 8;
			}
			break;
			case EBitmapFormat::ColMap4Bpp_CPC:
			{
				const uint8_t* pPixels = &pBank->Memory[bankAddr];
				pGraphicsView->Draw4BppWideImageAt(pPixels, xPos + (xChar * 8), y * 8, 8, 8, pPaletteColours ? pPaletteColours : GetCurrentPalette());

				memAddr += 2 * 8;
			}
			break;
			case EBitmapFormat::ColMapMulticolour_C64:
			{
				const uint8_t* pPixels = &pBank->Memory[bankAddr];
				pGraphicsView->Draw2BppWideImageAt(pPixels, xPos + (xChar * 8), y * 8, 8, 8, pPaletteColours ? pPaletteColours : GetCurrentPalette());
				memAddr += 8;
			}
			break;
            default:
            break;
			}
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
void FGraphicsViewer::DrawUI()
{
	//if (ImGui::Begin("Graphics View"))
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

	//ImGui::End();
}

// UI widget - move?
bool StepInt(const char* title, int& val, int stepAmount)
{
	const int oldVal = val;
	ImGui::PushID(title);
	ImGui::Text("%s",title);
	ImGui::SameLine();
	ImGui::PushButtonRepeat(true);
	if (ImGui::Button("<<"))
		val -= stepAmount;
	ImGui::SameLine();
	if (ImGui::Button(">>"))
		val += stepAmount;
	ImGui::PopButtonRepeat();
	ImGui::PopID();

	return val != oldVal;
}

void FGraphicsViewer::UpdateCharacterGraphicsViewerImage(void)
{
	const FCodeAnalysisState& state = GetCodeAnalysis();
	FGlobalConfig* pConfig = state.pGlobalConfig;

	const int widthFactor = IsBitmapFormatDoubleWidth(BitmapFormat) ? 2 : 1;
	const int kColumnWidthPixels = 8 * widthFactor;

	const int kHorizontalDispCharCount = kGraphicsViewerWidth / kColumnWidthPixels;
	const int kVerticalDispPixCount = kGraphicsViewerHeight;

	const int scaledHDispCharCount = kHorizontalDispCharCount / pConfig->GfxViewerScale;
	const int scaledVDispPixCount = kVerticalDispPixCount / pConfig->GfxViewerScale;
	
	const int xcount = scaledHDispCharCount / (XSizePixels / 8);
	const int ycount = scaledVDispPixCount / YSizePixels;

	int address = AddressOffset;
	const int xSizeChars = (XSizePixels >> 3) * widthFactor;
	const int bpp = GetBppForBitmapFormat(BitmapFormat);

	GraphicColumnSizeBytes = xSizeChars * ycount * YSizePixels * bpp;
	const int columnWidthPixels = XSizePixels * widthFactor;

	if (ViewMode == EGraphicsViewMode::Bitmap)
	{
		for (int x = 0; x < xcount; x++)
		{
			if (bShowPhysicalMemory)
				DrawPhysicalMemoryAsGraphicsColumn(address, x * columnWidthPixels, xSizeChars);
			else
				DrawMemoryBankAsGraphicsColumn(Bank, address & 0x3fff, x * columnWidthPixels, xSizeChars);
			
			address += GraphicColumnSizeBytes / widthFactor;
		}
	}
	else if (ViewMode == EGraphicsViewMode::MaskedInterleaved || ViewMode == EGraphicsViewMode::MaskedInterleavedZigZag)
	{
		for (int x = 0; x < xcount; x++)
		{
			if (bShowPhysicalMemory)
				DrawPhysicalMemoryAsGraphicsColumn(address, x * columnWidthPixels, xSizeChars);
			else
				DrawMemoryBankAsGraphicsColumn(Bank, address & 0x3fff, x * columnWidthPixels, xSizeChars);

			address += (GraphicColumnSizeBytes * 2) / widthFactor;
		}
	}
	else if (ViewMode == EGraphicsViewMode::BitmapChars)
	{
		for (int x = 0; x < xcount; x++)
		{
			if (bShowPhysicalMemory)
				DrawPhysicalMemoryAsGraphicsColumnChars(address, x * columnWidthPixels, xSizeChars);
			else
				DrawMemoryBankAsGraphicsColumnChars(Bank, address & 0x3fff, x * columnWidthPixels, xSizeChars);

			address += GraphicColumnSizeBytes / widthFactor;
		}
	}
	else if (ViewMode == EGraphicsViewMode::BitmapWinding)
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

				offsetX += columnWidthPixels;
			}
			offsetX = 0;
			offsetY += YSizePixels;
		}
		address += graphicsUnitSize;
	}
}

void FGraphicsViewer::DrawCharacterGraphicsViewer(void)
{
	const float scale = ImGui_GetScaling();

	FCodeAnalysisState& state = GetCodeAnalysis();
	FCodeAnalysisBank* pBank = state.GetBank(Bank);

	const int kHorizontalDispCharCount = kGraphicsViewerWidth / 8;
	const int kVerticalDispPixCount = kGraphicsViewerHeight;

	if (DrawBitmapFormatCombo(BitmapFormat, state))
		PaletteNo = -1;

	const int bpp = GetBppForBitmapFormat(BitmapFormat);
	if (BitmapFormatHasPalette(BitmapFormat))
	{
		const int numColours = GetNumColoursForBitmapFormat(BitmapFormat);
		DrawPaletteCombo("Palette", "Current", PaletteNo, GetNumColoursForBitmapFormat(BitmapFormat));

		if (PaletteNo == -1)
		{
			ImGui::SameLine();
			if (const uint32_t* pCurPalette = GetCurrentPalette())
				DrawPalette(pCurPalette, numColours);
		}
	}

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

			pBank = state.GetBank(Bank);
			AddressOffset = 0;
			MemorySize = pBank->GetSizeBytes();
		}
	}

	// show combo for banked mode
	if (bShowPhysicalMemory == false)
	{
		if (DrawBankInput(state, "Bank", Bank))
		{
			pBank = state.GetBank(Bank);
			AddressOffset = 0;
			MemorySize = pBank->GetSizeBytes();
		}
	}

	DrawGraphicsViewModeCombo("ViewMode", ViewMode);
	//ImGui::Combo("ViewMode", (int*)&ViewMode, "Bitmap\0BitmapChars\0BitmapWinding", (int)EGraphicsViewMode::Count);

	// View Scale
	int& viewScale = state.pGlobalConfig->GfxViewerScale;
	ImGui::InputInt("Scale", &viewScale, 1, 1);
	viewScale = std::max(1, viewScale);	// clamp

	const int widthFactor = IsBitmapFormatDoubleWidth(BitmapFormat) ? 2 : 1;
	const int viewSizeX = XSizePixels * viewScale * widthFactor;
	const int viewSizeY = YSizePixels * viewScale;
	const int xChars = XSizePixels >> 3;

	// Address input
	int addrInput = pBank ? pBank->GetMappedAddress() + AddressOffset : AddressOffset;
	ImGuiIO& io = ImGui::GetIO();
	ImVec2 pos = ImGui::GetCursorScreenPos();

	// Zoomed graphics view - put into class?
	const ImVec2 uv0(0, 0);
	const ImVec2 uv1(1.0f / (float)viewScale, 1.0f / (float)viewScale);
	const ImVec2 size((float)kGraphicsViewerWidth * scale, (float)kGraphicsViewerHeight * scale);
	pGraphicsView->UpdateTexture();
	ImGui::Image((void*)pGraphicsView->GetTexture(), size, uv0, uv1);

	if (ImGui::IsItemHovered())
	{
		const int xp = (int)((io.MousePos.x - pos.x) / scale);
		const int yp = std::max((int)((io.MousePos.y - pos.y - (YSizePixels / 2)) / scale), 0);

		ImDrawList* dl = ImGui::GetWindowDrawList();
		const int rx = (xp / viewSizeX) * viewSizeX;
		const int ry = (yp / viewScale) * viewScale;
		const float rxp = pos.x + (float)rx * scale;
		const float ryp = pos.y + (float)ry * scale;
		dl->AddRect(ImVec2(rxp, ryp), ImVec2(rxp + (float)viewSizeX * scale, ryp + (float)viewSizeY * scale), 0xff00ffff);
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
		
		// show magnifier
		const float magAmount = 8.0f;
		const int magXP = rx / viewScale;// ((int)(io.MousePos.x - pos.x) / viewSizeX)* viewSizeX;
		const int magYP = ry / viewScale;//std::max((int)(io.MousePos.y - pos.y - (viewSizeY / 2)), 0);
		const ImVec2 magSize(XSizePixels * magAmount * widthFactor, YSizePixels * magAmount);
		const ImVec2 magUV0(magXP * (1.0f/ kGraphicsViewerWidth), magYP * (1.0f / kGraphicsViewerHeight));
		const ImVec2 magUV1(magUV0.x + XSizePixels * widthFactor * (1.0f / kGraphicsViewerWidth), magUV0.y + YSizePixels * (1.0f / kGraphicsViewerHeight));
		ImGui::Image((void*)pGraphicsView->GetTexture(), magSize, magUV0, magUV1);

		ImGui::EndTooltip();
	}

	char format[6] = { 0 };
	if (GetNumberDisplayMode() == ENumberDisplayMode::HexAitch)
		strncpy(format, "%02Xh", 6);
	else if (GetNumberDisplayMode() == ENumberDisplayMode::HexDollar)
		strncpy(format, "$%02X", 6);
	else 
		strncpy(format, "%d", 6);

	ImGui::SameLine();

	// simpler slider
	if (ImGui::VSliderInt("##int", ImVec2(64.0f, (float)kGraphicsViewerHeight), &addrInput, 0, 0xffff, format))
	{
		if (!bVSliderFineControl)
		{ 
			// Round up to the number of bytes for a column
			const int c = GraphicColumnSizeBytes;
			addrInput = ((addrInput + c - 1) / c) * c;
		}
	}

	const int pixelsPerByte = 8 / bpp;
	const int graphicsUnitSize = (XSizePixels / pixelsPerByte) * YSizePixels;

	if (ImGui::IsItemHovered())
	{
		ImGui::SetKeyOwner(ImGuiKey_MouseWheelY, ImGui::GetItemID());
		const float wheel = ImGui::GetIO().MouseWheel;
		if (wheel != 0)
		{
			if(ImGui::GetIO().KeyShift || bVSliderFineControl)
				addrInput += (int)wheel;
			else
				addrInput += (int)wheel * GraphicColumnSizeBytes;
		}
	}
	addrInput = std::min(std::max(0, addrInput), 0xffff);

	ImGui::SameLine();
	ImGui::Checkbox("Fine", &bVSliderFineControl);

	ImGui::SetNextItemWidth(120.0f * scale);
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
			const FCodeAnalysisItem item(state.GetDataInfoForAddress(ClickedAddress), ClickedAddress);
			DrawDataDetails(state, state.GetFocussedViewState(), item);
		}
	}

	// step address based on image attributes
	StepInt("Step Line", addrInput, xChars * bpp);
	StepInt("Step Image", addrInput, graphicsUnitSize);
	StepInt("Step Column", addrInput, GraphicColumnSizeBytes);

	// draw 64 * 8 bytes
	const float kNumSize = 80.0f * scale;	// size for number GUI widget
	ImGui::SetNextItemWidth(kNumSize);
	ImGui::InputInt("XSize", &XSizePixels, 8, 8);
	//ImGui::SameLine();
	ImGui::SetNextItemWidth(kNumSize);
	ImGui::InputInt("YSize", &YSizePixels, bYSizePixelsFineCtrl ? 1 : 8, 8);
	ImGui::SameLine();
	ImGui::Checkbox("Pixel Adjust", &bYSizePixelsFineCtrl);

	// clamp sizes
	XSizePixels = std::min(std::max(8, XSizePixels), kMaxImageSize);
	YSizePixels = std::min(std::max(1, YSizePixels), kMaxImageSize);
	
	ImGui::InputInt("Count", &ImageCount, 1, 1);

	UpdateCharacterGraphicsViewerImage();	// update texture data

	// Image Set UI - move to function
	if (ImageCount > 0)
	{
		ImDrawList* dl = ImGui::GetWindowDrawList();
		const int scaledVDispPixCount = kVerticalDispPixCount / viewScale;
		const int ycount = scaledVDispPixCount / YSizePixels;

		for (int i = 0; i < ImageCount; i++)
		{
			ImVec2 rectPos;
			rectPos.x = pos.x + (static_cast<float>(i / ycount) * (float)viewSizeX) * scale;
			rectPos.y = pos.y + (static_cast<float>(i % ycount) * (float)viewSizeY) * scale;
			dl->AddRect(rectPos, ImVec2(rectPos.x + (float)viewSizeX * scale, rectPos.y + (float)viewSizeY * scale), 0xff00ff00);
		}

		ImGui::InputText("Image Set Name", &ImageSetName);


		// TODO: put in function?
		const FAddressRef baseAddrRef = bShowPhysicalMemory ? state.AddressRefFromPhysicalAddress(addrInput) : FAddressRef(pBank->Id, addrInput);

		if (ImGui::Button("Format Memory"))
		{
			if (ImageSetName.empty())
			{
				ImageSetName = "Unnamed";
			}

			FAddressRef imageAddressRef = baseAddrRef;	// updated per image

			for (int i = 0; i < ImageCount; i++)
			{
				FDataFormattingOptions format;
				format.AddLabelAtStart = true;
				format.ClearLabels = true;
				format.ClearCodeInfo = true;

				// Generate label
				if (ImageCount > 1)
				{
					char numStr[8];
					snprintf(numStr,8, "_%d", i);
					format.LabelName = ImageSetName + numStr;
				}
				else
				{
					format.LabelName = ImageSetName;
				}
				
				int paletteToFormat = PaletteNo;
				if (PaletteNo == -1)
				{
					// Store current palette
					if (const uint32_t* pCurrentPalette = GetCurrentPalette())
					{
						paletteToFormat = GetPaletteNo(pCurrentPalette, bpp * bpp);
					}
				}
				EDataItemDisplayType displayType = GetDisplayTypeForBitmapFormat(BitmapFormat);

				format.SetupForBitmap(imageAddressRef, XSizePixels, YSizePixels, bpp);
				format.DisplayType = displayType;
				format.GraphicsSetRef = imageAddressRef;
				format.PaletteNo = paletteToFormat;
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
						switch (BitmapFormat)
						{
							case EBitmapFormat::Bitmap_1Bpp:
							{
								const uint8_t charLine = state.ReadByte(itemAddress);
								pItemView->DrawCharLine(charLine, xp * 8, yp, 0xffffffff, 0);
								state.AdvanceAddressRef(itemAddress, 1);
								break;
							}
							case EBitmapFormat::ColMap2Bpp_CPC:
							{
								const uint16_t charLine = state.ReadWord(itemAddress);
								const uint32_t* pPaletteColours = GetPaletteFromPaletteNo(PaletteNo);
								pItemView->Draw2BppImageAt((uint8_t*)&charLine, xp * 8, yp, 8, 1, pPaletteColours ? pPaletteColours : GetCurrentPalette());
								state.AdvanceAddressRef(itemAddress, 2);
								break;
							}
							case EBitmapFormat::ColMap4Bpp_CPC:
							{
								const uint16_t charLine = state.ReadWord(itemAddress);
								const uint32_t* pPaletteColours = GetPaletteFromPaletteNo(PaletteNo);
								pItemView->Draw2BppImageAt((uint8_t*)&charLine, xp * 8, yp, 4, 1, pPaletteColours ? pPaletteColours : GetCurrentPalette());
								state.AdvanceAddressRef(itemAddress, 2);
								break;
							}
							case EBitmapFormat::ColMapMulticolour_C64:
							{
								// todo
								break;
							}
                            default:
                            break;
						}
					}
				}
				pItemView->UpdateTexture();

				ImageGraphicSet = baseAddrRef;				
			}

			float scaleFactor = 4.0f * scale;
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
	const float childHeight = ImGui::GetTextLineHeightWithSpacing() * (GraphicsSets.size() + 1);
	if (ImGui::BeginChild("GraphicSetListChild",ImVec2(0, childHeight),true))
	{
		for (const auto& graphicsSetIt : GraphicsSets)
		{
			const FGraphicsSet& set = graphicsSetIt.second;
			bool bSelected = set.Address == SelectedGraphicSet;
			ImGui::PushID(set.Address.Val);
			if (ImGui::Selectable(set.Name.c_str(), &bSelected))
			{
				ImageSetName = set.Name;
				SelectedGraphicSet = set.Address;
				GoToAddress(set.Address);
				state.GetFocussedViewState().GoToAddress(set.Address);
			}
			ImGui::PopID();
		}
	}
	ImGui::EndChild();
	if (ImGui::Button("Delete"))
	{
		auto delIt = GraphicsSets.find(SelectedGraphicSet);
		if(delIt != GraphicsSets.end())
			GraphicsSets.erase(delIt);
	}
	ImGui::SameLine();
	if (ImGui::Button("Export"))
	{
		ExportImages();
		//pGraphicsView->SavePNG("test.png");
	}

}



// Save/Load Graphics sets to json
#include <iomanip>
#include <fstream>
#include <sstream>
#include <json.hpp>
#include <Util/FileUtil.h>
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

void FGraphicsViewer::DrawGraphicToView(const FGraphicsSet& set, FGraphicsView* pView, int imageNo, int x, int y)
{
	FCodeAnalysisState& state = GetCodeAnalysis();
	const int xChars = (set.XSizePixels >> 3);
	const int graphicsUnitSize = xChars * set.YSizePixels;

	FAddressRef itemAddress = set.Address;

	state.AdvanceAddressRef(itemAddress, imageNo * graphicsUnitSize);
	for (int yp = 0; yp < set.YSizePixels; yp++)
	{
		for (int xp = 0; xp < xChars; xp++)
		{
			switch (BitmapFormat)
			{
				case EBitmapFormat::Bitmap_1Bpp:
				{
					const uint8_t charLine = state.ReadByte(itemAddress);
					pView->DrawCharLine(charLine, x + (xp * 8), y + yp, 0xffffffff, 0);
					state.AdvanceAddressRef(itemAddress, 1);
					break;
				}
				case EBitmapFormat::ColMap2Bpp_CPC:
				{
					const uint16_t charLine = state.ReadWord(itemAddress);
					const uint32_t* pPaletteColours = GetPaletteFromPaletteNo(PaletteNo);
					pGraphicsView->Draw2BppImageAt((uint8_t*)&charLine, x + (xp * 8), y + yp, 8, 1, pPaletteColours ? pPaletteColours : GetCurrentPalette());
					state.AdvanceAddressRef(itemAddress, 2);
					break;
				}
				case EBitmapFormat::ColMap4Bpp_CPC:
				{
					break;
				}
                default:
                break;
			}
		}
	}
}

// export all graphic sets as separate PNG files
bool FGraphicsViewer::ExportGraphicSet(const FGraphicsSet& set)
{
	const int maxImageXSize = 256;
	const int maxImagesX = maxImageXSize / set.XSizePixels;
	const int noImagesX = set.Count % maxImagesX;
	const int noImagesY = (set.Count / maxImagesX) + 1;

	FGraphicsView* pSaveImage = new FGraphicsView(set.XSizePixels * noImagesX, set.YSizePixels * noImagesY);

	// write out graphics to memory buffer
	FAddressRef addrRef = set.Address;

	for (int imageNo = 0; imageNo < set.Count; imageNo++)
	{
		const int x = (imageNo % noImagesX) * set.XSizePixels;
		const int y = (imageNo / noImagesX) / set.YSizePixels;

		DrawGraphicToView(set, pSaveImage, imageNo, x, y);
	}

	// export png
	std::string fName = ImagesRoot + set.Name + ".png";
	pSaveImage->SavePNG(fName.c_str());

	delete pSaveImage;
	return true;
}

bool FGraphicsViewer::ExportImages(void)
{
	EnsureDirectoryExists(ImagesRoot.c_str());
	for (const auto& graphicsSetIt : GraphicsSets)
	{
		const FGraphicsSet& set = graphicsSetIt.second;
		if (ExportGraphicSet(set) == false)
			return false;
	}

	return true;
}

