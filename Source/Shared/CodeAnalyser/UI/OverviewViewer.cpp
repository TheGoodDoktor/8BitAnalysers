#include "OverviewViewer.h"

#include <imgui.h>
#include <implot.h>
#include "CodeAnalyserUI.h"
#include "Util/GraphicsView.h"
#include "ImGuiSupport/ImGuiScaling.h"
#include "UIColours.h"

static const int kMemoryViewImageWidth = 128;
static const int kMemoryViewImageHeight = 512;

void DrawHighlightBar(float x, float y, float width, float height);

bool FOverviewViewer::Init(void)
{
	MemoryViewImage = new FGraphicsView(kMemoryViewImageWidth, kMemoryViewImageHeight);
	MemoryViewImage->Clear(0xff000000);	

	return true;
}

void FOverviewViewer::DrawUI(void)
{
    //DrawStats();
	//DrawBankOverview();
	DrawPhysicalMemoryOverview();
}

void FOverviewViewer::DrawStats()
{
    static const char* labels1[] = { "Read Only Data","Write Only Data","Read/Write Data","Commented Code","Uncommented Code","Unknown" };
    float data1[] = {Stats.PercentReadOnlyData,Stats.PercentWriteOnlyData,Stats.PercentReadWriteData, Stats.PercentCommentedCode, Stats.PercentUncommentedCode, Stats.PercentUnknown };
    static ImPlotPieChartFlags flags = 0;// ImPlotPieChartFlags_Normalize;

    //if (pSpectrumEmu->CodeAnalysis.bCodeAnalysisDataDirty)
        CalculateStats();

    if (ImPlot::BeginPlot("##Pie1", ImVec2(400, 400), ImPlotFlags_Equal | ImPlotFlags_NoMouseText)) 
    {
        ImPlot::SetupAxes(NULL, NULL, ImPlotAxisFlags_NoDecorations, ImPlotAxisFlags_NoDecorations);
        ImPlot::SetupAxesLimits(0, 1, 0, 1);
        ImPlot::PlotPieChart(labels1, data1, 6, 0.5, 0.5, 0.4, "%.2f%%", 90, flags);
        ImPlot::EndPlot();
    }

    //ImPlot::ShowDemoWindow();
}

void FOverviewViewer::CalculateStats()
{
	FCodeAnalysisState& codeAnalysis = pEmulator->GetCodeAnalysis();
	const std::vector<FCodeAnalysisBank>& banks =codeAnalysis.GetBanks();

	Stats = FOverviewStats();	// reset


	for (const FCodeAnalysisBank& bank : banks)
	{
		// Skip unused banks
		if(bank.bEverBeenMapped == false)
			continue;

		uint16_t bankAddress = 0;
		const uint16_t bankSizeBytes = bank.GetSizeBytes();
		while (bankAddress < bankSizeBytes)
		{
			const FCodeInfo* pCodeInfo = bank.Pages[bankAddress >> FCodeAnalysisPage::kPageShift].CodeInfo[bankAddress & FCodeAnalysisPage::kPageMask];
			if (pCodeInfo)
			{
				if (pCodeInfo->Comment.empty())
					Stats.UnCommentedCodeCount++;
				else
					Stats.CommentedCodeCount++;

				bankAddress += pCodeInfo->ByteSize;
				Stats.TotalItems++;
			}
			else
			{
				const FDataInfo& dataInfo = bank.Pages[bankAddress >> FCodeAnalysisPage::kPageShift].DataInfo[bankAddress & FCodeAnalysisPage::kPageMask];
				const bool bRead = dataInfo.LastFrameRead != -1;
				const bool bWrite = dataInfo.LastFrameWritten != -1;
				if(bank.bMachineROM)	// TODO: a 'read only' bool would service this better
				{
					Stats.ReadOnlyDataCount++;
				}
				else
				{
					if (bRead && !bWrite)
						Stats.ReadOnlyDataCount++;
					else if (!bRead && bWrite)
						Stats.WriteOnlyDataCount++;
					else if (bRead && bWrite)
						Stats.ReadWriteDataCount++;
					else
						Stats.UnknownCount++;
				}

				bankAddress += dataInfo.ByteSize;
				Stats.TotalItems++;
			}
		}
	}

	// Calculate percentages
	Stats.PercentCommentedCode = Stats.CommentedCodeCount * (1.0f / Stats.TotalItems) * 100.0f;
	Stats.PercentUncommentedCode = Stats.UnCommentedCodeCount * (1.0f / Stats.TotalItems) * 100.0f;
	Stats.PercentReadOnlyData = Stats.ReadOnlyDataCount * (1.0f / Stats.TotalItems) * 100.0f;
	Stats.PercentWriteOnlyData = Stats.WriteOnlyDataCount * (1.0f / Stats.TotalItems) * 100.0f;
	Stats.PercentReadWriteData = Stats.ReadWriteDataCount * (1.0f / Stats.TotalItems) * 100.0f;
	Stats.PercentUnknown = Stats.UnknownCount * (1.0f / Stats.TotalItems) * 100.0f;
}

#if 0
void	FOverviewViewer::DrawBankOverview()
{
	const FCodeAnalysisState& state = pEmulator->GetCodeAnalysis();
	DrawBankInput(state,"Bank",OverviewBankId);

	const FCodeAnalysisBank* pBank = state.GetBank(OverviewBankId);

	MemoryViewImage->Clear(0xff808080);

	uint32_t* pViewImagePixels = MemoryViewImage->GetPixelBuffer();
	uint32_t* pPix = pViewImagePixels;

	const int frameThreshold = 16;
	const int currentFrameNo = state.CurrentFrameNo;

	if (pBank)
	{
		uint16_t bankAddress = 0;
		const uint16_t bankSizeBytes = pBank->GetSizeBytes();
		while (bankAddress < bankSizeBytes)
		{
			const FCodeInfo* pCodeInfo = pBank->Pages[bankAddress >> FCodeAnalysisPage::kPageShift].CodeInfo[bankAddress & FCodeAnalysisPage::kPageMask];
			if (pCodeInfo)
			{
				const int framesSinceExecuted = currentFrameNo - pCodeInfo->FrameLastExecuted;
				const uint32_t codeCol = (pCodeInfo->FrameLastExecuted != -1 && framesSinceExecuted < frameThreshold) ? 0xFF00FFFF : 0xFF008080;
				for (int i = 0; i < pCodeInfo->ByteSize; i++)
				{
					bankAddress++;
					*pPix++ = codeCol;
				}
			}
			else
			{
				const FDataInfo& dataInfo = pBank->Pages[bankAddress >> FCodeAnalysisPage::kPageShift].DataInfo[bankAddress & FCodeAnalysisPage::kPageMask];
				const bool bRead = dataInfo.LastFrameRead != -1;
				const bool bWrite = dataInfo.LastFrameWritten != -1;

				uint32_t dataCol = 0xff000000;	// black for unknown areas

				if (bWrite)
				{
					const int framesSinceWritten = currentFrameNo - dataInfo.LastFrameWritten;
					dataCol = (framesSinceWritten <  frameThreshold) ? 0xFF0000FF : 0xFF000080;
				}
				else if(bRead)
				{
					const int framesSinceRead = currentFrameNo - dataInfo.LastFrameRead;
					dataCol = (framesSinceRead < frameThreshold) ? 0xFF00FF00 - (framesSinceRead << 16) : 0xFF008000;
				}

				for (int i = 0; i < dataInfo.ByteSize; i++)
				{
					bankAddress++;
					*pPix++ = dataCol;
				}
			}
		}
	}

	MemoryViewImage->Draw();
}
#endif

void	FOverviewViewer::DrawLegend()
{
	ImGui::BeginTooltip();

	ImGui::ColorButton("Code", ImGui::ColorConvertU32ToFloat4(kCodeCol), ImGuiColorEditFlags_NoTooltip);
	ImGui::SameLine();
	ImGui::Text("Code");

	ImGui::ColorButton("Active Code", ImGui::ColorConvertU32ToFloat4(kCodeColActive), ImGuiColorEditFlags_NoTooltip);
	ImGui::SameLine();
	ImGui::Text("Active Code");

	ImGui::ColorButton("Data Read", ImGui::ColorConvertU32ToFloat4(kDataReadCol), ImGuiColorEditFlags_NoTooltip);
	ImGui::SameLine();
	ImGui::Text("Data Read");

	ImGui::ColorButton("Data Read Active", ImGui::ColorConvertU32ToFloat4(kDataReadActiveCol), ImGuiColorEditFlags_NoTooltip);
	ImGui::SameLine();
	ImGui::Text("Data Read Active");

	ImGui::ColorButton("Data Write", ImGui::ColorConvertU32ToFloat4(kDataWriteCol), ImGuiColorEditFlags_NoTooltip);
	ImGui::SameLine();
	ImGui::Text("Data Write");

	ImGui::ColorButton("Data Write Active", ImGui::ColorConvertU32ToFloat4(kDataWriteActiveCol), ImGuiColorEditFlags_NoTooltip);
	ImGui::SameLine();
	ImGui::Text("Data Write Active");

	ImGui::ColorButton("Bitmap Data", ImGui::ColorConvertU32ToFloat4(kBitmapDataCol), ImGuiColorEditFlags_NoTooltip);
	ImGui::SameLine();
	ImGui::Text("Bitmap Data");

	ImGui::ColorButton("Char Map Data", ImGui::ColorConvertU32ToFloat4(kCharMapDataCol), ImGuiColorEditFlags_NoTooltip);
	ImGui::SameLine();
	ImGui::Text("Char Map Data");

	ImGui::ColorButton("Screen Pixels", ImGui::ColorConvertU32ToFloat4(kScreenPixelsDataCol), ImGuiColorEditFlags_NoTooltip);
	ImGui::SameLine();
	ImGui::Text("Screen Pixels");

	ImGui::ColorButton("Colour Attributes", ImGui::ColorConvertU32ToFloat4(kColAttribDataCol), ImGuiColorEditFlags_NoTooltip);
	ImGui::SameLine();
	ImGui::Text("Colour Attributes");

	ImGui::ColorButton("Unknown Data", ImGui::ColorConvertU32ToFloat4(kUnknownDataCol), ImGuiColorEditFlags_NoTooltip);
	ImGui::SameLine();
	ImGui::Text("Unknown Data");

	ImGui::EndTooltip();
}



void	FOverviewViewer::DrawPhysicalMemoryOverview()
{
	FCodeAnalysisState& state = pEmulator->GetCodeAnalysis();

	MemoryViewImage->Clear(0xff808080);

	uint32_t* pViewImagePixels = MemoryViewImage->GetPixelBuffer();
	uint32_t* pPix = pViewImagePixels;

	DrawUtilisationMap(state,pPix);

	FCodeAnalysisViewState& viewState = state.GetFocussedViewState();

	FGlobalConfig* pConfig = state.pGlobalConfig;

	ImGui::InputInt("Scale", &pConfig->OverviewScale, 1, 1);
	pConfig->OverviewScale = std::max(1, pConfig->OverviewScale);	// clamp
	ImGui::SameLine();
	ImGui::Checkbox("Include ROM", &bShowROM);

	const float scale = ImGui_GetScaling() * (float)pConfig->OverviewScale;

	MemoryViewImage->UpdateTexture();

	ImGuiIO& io = ImGui::GetIO();
	ImVec2 pos = ImGui::GetCursorScreenPos();

	// Draw Image
	const int height = bShowROM ? kMemoryViewImageHeight : kMemoryViewImageHeight - (kMemoryViewImageHeight / 4);
	const ImVec2 size((float)kMemoryViewImageWidth * scale, height * scale);
	const ImVec2 uv0(0, 0);
	const ImVec2 uv1(1.0f, bShowROM ? 1.0f : 0.75f);
	ImGui::Image((void*)MemoryViewImage->GetTexture(), size,uv0,uv1);

	const bool bMapIsHovered = ImGui::IsItemHovered();

	ImGui::SameLine();
	ImGui::Button("?");

	if (ImGui::IsItemHovered())
	{
		DrawLegend();
	}

	ImDrawList* dl = ImGui::GetWindowDrawList();

	if (bMapIsHovered)
	{
		const int xp = (int)((io.MousePos.x - pos.x) / scale);
		const int yp = (int)((io.MousePos.y - pos.y) / scale);

		const uint16_t addr = (xp + yp * kMemoryViewImageWidth) + (bShowROM ? 0 : 0x4000);	// add offset if we're not showing ROM
		const FAddressRef addrRef = state.AddressRefFromPhysicalAddress(addr);
		ImGui::Text("Location:");
		DrawAddressLabel(state, viewState, addrRef);
		DrawSnippetToolTip(state, viewState, addrRef);

		if (ImGui::IsMouseDoubleClicked(0))
		{
			viewState.GoToAddress(addrRef, false);
		}
	}
	else
	{
		ImGui::NewLine();
	}

	ImGui::Checkbox("Highlight Current Location", &bShowCurrentLocation);

	if (bShowCurrentLocation)
	{
		// Highlight current Code Analysis view position
		if (viewState.GetCursorItem().IsValid())
		{
			const FAddressRef cursorAddr = viewState.GetCursorItem().AddressRef;
			const int address = cursorAddr.Address - (bShowROM ? 0 : 0x4000);
			if (address >= 0)
			{
				float barMargin = std::max(scale, 8.0f);
				float barHeight = scale + barMargin * 2; 
		
				const int bytesPerLine = 0x10000 / kMemoryViewImageHeight; // how many bytes in a single horizontal line?
				const float offsetY = (address / bytesPerLine) * scale;

				const float maxClampY = pos.y + size.y - barHeight;
				const float pixelTopY = (float)((int)(pos.y + offsetY)); // Position of the top of the pixel on the pixel buffer texture.
				const float barPosY = std::min(std::max((float)((int)(pixelTopY - barMargin)), pos.y), maxClampY);

				DrawHighlightBar(pos.x, barPosY, size.x, barHeight);
			}
		}
	}
}

#if 0
void FOverviewViewer::DrawAccessMap(FCodeAnalysisState& state, uint32_t* pPix)
{
	const int frameThreshold = 8;
	const int currentFrameNo = state.CurrentFrameNo;

	uint32_t addr = 0;

	while (addr < (1 << 16))
	{
		const FCodeInfo* pCodeInfo = state.GetCodeInfoForPhysicalAddress(addr);

		if (pCodeInfo)
		{
			const int framesSinceExecuted = currentFrameNo - pCodeInfo->FrameLastExecuted;
			const uint32_t codeCol = (pCodeInfo->FrameLastExecuted != -1 && framesSinceExecuted < frameThreshold) ? 0xFF00FFFF : 0xFF008080;
			for (int i = 0; i < pCodeInfo->ByteSize; i++)
			{
				addr++;
				*pPix++ = codeCol;
			}
		}
		else
		{
			const FDataInfo* pReadDataInfo = state.GetReadDataInfoForAddress(addr);
			const FDataInfo* pWriteDataInfo = state.GetWriteDataInfoForAddress(addr);
			const bool bRead = pReadDataInfo->Reads.IsEmpty() == false;
			const bool bWrite = pWriteDataInfo->Writes.IsEmpty() == false;
			const bool bUsed = (bRead || bWrite);

			uint32_t dataCol = bUsed ? 0xffff0000 : 0xff000000;


			if (pReadDataInfo != nullptr && pReadDataInfo->LastFrameRead != -1)
			{
				const int framesSinceRead = currentFrameNo - pReadDataInfo->LastFrameRead;
				if (framesSinceRead < frameThreshold)
					dataCol = 0xFF00FF00;
			}

			if (pWriteDataInfo != nullptr && pWriteDataInfo->LastFrameWritten != -1)
			{
				const int framesSinceWritten = currentFrameNo - pWriteDataInfo->LastFrameWritten;
				if (framesSinceWritten < frameThreshold)
					dataCol = 0xFF0000FF;
			}


			addr++;
			*pPix++ = dataCol;
		}
	}	
}
#endif


void FOverviewViewer::DrawUtilisationMap(FCodeAnalysisState& state, uint32_t* pPix)
{
	FCodeAnalysisViewState& viewState = state.GetFocussedViewState();

	const int frameThreshold = 4;
	const int currentFrameNo = state.CurrentFrameNo;

	uint32_t physicalAddress = bShowROM ? 0 : 0x4000;

	FCodeInfo* pSelectedItemCodeInfo = nullptr;
	int selectedItemAddr = -1;
	if (bShowCurrentLocation)
	{
		if (viewState.GetCursorItem().IsValid())
		{
			pSelectedItemCodeInfo = state.GetCodeInfoForAddress(viewState.GetCursorItem().AddressRef);
			selectedItemAddr = viewState.GetCursorItem().AddressRef.Address;
		}
	}

	while (physicalAddress < (1 << 16))
	{
		FAddressRef readAddrRef = state.AddressRefFromPhysicalReadAddress(physicalAddress);
		FAddressRef writeAddrRef = state.AddressRefFromPhysicalWriteAddress(physicalAddress);

		const FCodeInfo* pCodeInfo = state.GetCodeInfoForAddress(readAddrRef);

		if (pCodeInfo)
		{
			uint32_t codeCol = kCodeCol;
			
			if (bShowActivity)
			{
				const int framesSinceExecuted = currentFrameNo - pCodeInfo->FrameLastExecuted;
				if(pCodeInfo->FrameLastExecuted != -1 && framesSinceExecuted < frameThreshold) 
					codeCol = kCodeColActive;
			}

			const bool bIsSelectedItem = pSelectedItemCodeInfo == pCodeInfo;

			for (int i = 0; i < pCodeInfo->ByteSize; i++)
			{
				physicalAddress++;
				*pPix++ = bIsSelectedItem ? Colours::GetFlashColour() : codeCol;
			}
		}
		else
		{
			const FDataInfo* pDataInfo = state.GetDataInfoForAddress(readAddrRef);

			const bool bIsSelectedItem = selectedItemAddr >= (int)physicalAddress && selectedItemAddr < (int)(physicalAddress + pDataInfo->ByteSize);

			uint32_t dataCol = kDefaultDataCol;

			switch (pDataInfo->DataType)
			{
				case EDataType::Bitmap:
					dataCol = kBitmapDataCol;
				break;

				case EDataType::CharacterMap:
					dataCol = kCharMapDataCol;
					break;

				case EDataType::Text:
					dataCol = kTextDataCol;
					break;
				case EDataType::ScreenPixels:
					dataCol = kScreenPixelsDataCol;
					break;
				case EDataType::ColAttr:
					dataCol = kColAttribDataCol;
					break;
				default:
					if(pDataInfo->DisplayType == EDataItemDisplayType::Unknown)
						dataCol = kUnknownDataCol;
			}

			for (int i = 0; i < pDataInfo->ByteSize; i++)
			{
				if (bShowActivity)
				{
					const FDataInfo* pReadDataInfo = state.GetDataInfoForAddress(readAddrRef);
					const FDataInfo* pWriteDataInfo = state.GetDataInfoForAddress(writeAddrRef);
					uint32_t drawCol = dataCol;

					// show unknowns that have been read
					if(dataCol == kUnknownDataCol && pWriteDataInfo->Reads.IsEmpty() == false)
						drawCol = kDataReadCol;

					// show unknowns that have been written to
					if (dataCol == kUnknownDataCol && pWriteDataInfo->Writes.IsEmpty() == false)
						drawCol = kDataWriteCol;

					if (pWriteDataInfo != nullptr && pWriteDataInfo->LastFrameWritten != -1)	// Show write
					{
						const int framesSinceWritten = currentFrameNo - pWriteDataInfo->LastFrameWritten;
						if (framesSinceWritten < frameThreshold)
							drawCol = kDataWriteActiveCol;
					}
					else if (pReadDataInfo != nullptr && pReadDataInfo->LastFrameRead != -1)	// Show read
					{
						const int framesSinceRead = currentFrameNo - pReadDataInfo->LastFrameRead;
						if (framesSinceRead < frameThreshold)
							drawCol = kDataReadActiveCol;
					}

					*pPix++ = bIsSelectedItem ? Colours::GetFlashColour() : drawCol;
				}
				else
				{
					*pPix++ = bIsSelectedItem ? Colours::GetFlashColour() : dataCol;
				}

				state.AdvanceAddressRef(readAddrRef);
				state.AdvanceAddressRef(writeAddrRef);

				physicalAddress++;
			}
		}
	}
}

void DrawHighlightBar(float x, float y, float width, float height)
{
	ImDrawList* dl = ImGui::GetWindowDrawList();

	const float t1 = 2.f; // Thickness of frame's main part
	const float t2 = 1.f; // Thickness of top and bottom "shadow"
	const float frameHeight = t1 + t2 + t2;
	const float minX = x - t1;
	const float maxX = x + width + t1;
	float posY = y - frameHeight; // t1 needs to be even for this to work

	const int alpha = 0xf0 << 24;

	// Horizontal top section
	dl->AddRectFilled(ImVec2(minX, posY), ImVec2(maxX, posY + t2), alpha | 0x323232);
	posY += t2;
	dl->AddRectFilled(ImVec2(minX, posY), ImVec2(maxX, posY + t1), alpha | 0xf0f0f0);
	posY += t1;
	dl->AddRectFilled(ImVec2(minX, posY), ImVec2(maxX, posY + t2), alpha | 0x404040);

	// Horizontal bottom section
	posY += height + 1;
	dl->AddRectFilled(ImVec2(minX, posY), ImVec2(maxX, posY + t2), alpha | 0x323232);
	posY += t2;
	dl->AddRectFilled(ImVec2(minX, posY), ImVec2(maxX, posY + t1), alpha | 0xf0f0f0);
	posY += t1;
	dl->AddRectFilled(ImVec2(minX, posY), ImVec2(maxX, posY + t2), alpha | 0x404040);

	// Vertical side sections
	dl->AddRectFilled(ImVec2(minX, y - frameHeight + t2), ImVec2(minX + t1, posY), alpha | 0xf0f0f0);
	dl->AddRectFilled(ImVec2(x + width, y - frameHeight + t2), ImVec2(x + width + t1, posY), alpha | 0xf0f0f0);
}