#include "OverviewViewer.h"

#include <imgui.h>
#include <implot.h>
#include "CodeAnalyserUI.h"
#include "Util/GraphicsView.h"
#include "ImGuiSupport/ImGuiScaling.h"

static const int kMemoryViewImageWidth = 128;
static const int kMemoryViewImageHeight = 512;

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

// TODO: this needs to cover banks
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
				if(bank.bReadOnly)
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

void	FOverviewViewer::DrawPhysicalMemoryOverview()
{
	FCodeAnalysisState& state = pEmulator->GetCodeAnalysis();

	MemoryViewImage->Clear(0xff808080);

	uint32_t* pViewImagePixels = MemoryViewImage->GetPixelBuffer();
	uint32_t* pPix = pViewImagePixels;

	DrawUtilisationMap(state,pPix);

	FCodeAnalysisViewState& viewState = state.GetFocussedViewState();

	ImGui::InputInt("Scale", &ViewScale, 1, 1);
	ViewScale = std::max(1, ViewScale);	// clamp
	ImGui::SameLine();
	ImGui::Checkbox("Include ROM", &bShowROM);

	const float scale = ImGui_GetScaling() * (float)ViewScale;

	MemoryViewImage->UpdateTexture();

	ImGuiIO& io = ImGui::GetIO();
	ImVec2 pos = ImGui::GetCursorScreenPos();

	// Draw Image
	const int height = bShowROM ? kMemoryViewImageHeight : kMemoryViewImageHeight - (kMemoryViewImageHeight / 4);
	const ImVec2 size((float)kMemoryViewImageWidth * scale, height * scale);
	const ImVec2 uv0(0, 0);
	const ImVec2 uv1(1.0f, bShowROM ? 1.0f : 0.75f);
	ImGui::Image((void*)MemoryViewImage->GetTexture(), size,uv0,uv1);

	if (ImGui::IsItemHovered())
	{
		const int xp = (int)((io.MousePos.x - pos.x) / scale);
		const int yp = (int)((io.MousePos.y - pos.y) / scale);

		const uint16_t addr = (xp + yp * kMemoryViewImageWidth) + (bShowROM ? 0 : 0x4000);	// add offset if we're not showing ROM
		const FAddressRef addrRef = state.AddressRefFromPhysicalAddress(addr);
		ImGui::Text("Location:");
		DrawAddressLabel(state,viewState,addrRef);
		DrawSnippetToolTip(state, viewState, addrRef);

		if (ImGui::IsMouseDoubleClicked(0))
		{
			viewState.GoToAddress(addrRef, false);
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
	const int frameThreshold = 4;
	const int currentFrameNo = state.CurrentFrameNo;

	const uint32_t kCodeCol = 0xff008080;
	const uint32_t kCodeColActive = 0xff00ffff;
	const uint32_t kDataReadCol = 0xff00ff00;
	const uint32_t kDataWriteCol = 0xff0000ff;
	const uint32_t kDefaultDataCol = 0xffff0000;
	const uint32_t kBitmapDataCol = 0xffffffff;
	const uint32_t kCharMapDataCol = 0xff00ff00;
	const uint32_t kTextDataCol = 0xffff00ff;
	const uint32_t kScreenPixelsDataCol = 0xffff80ff;
	const uint32_t kColAttribDataCol = 0xff0080ff;
	const uint32_t kUnknownDataCol = 0xff808080;

	uint32_t physicalAddress = bShowROM ? 0 : 0x4000;

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

			for (int i = 0; i < pCodeInfo->ByteSize; i++)
			{
				physicalAddress++;
				*pPix++ = codeCol;
			}
		}
		else
		{
			const FDataInfo* pDataInfo = state.GetDataInfoForAddress(readAddrRef);
			
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

					if (pWriteDataInfo != nullptr && pWriteDataInfo->LastFrameWritten != -1)	// Show write
					{
						const int framesSinceWritten = currentFrameNo - pWriteDataInfo->LastFrameWritten;
						if (framesSinceWritten < frameThreshold)
							drawCol = kDataWriteCol;
					}
					else if (pReadDataInfo != nullptr && pReadDataInfo->LastFrameRead != -1)	// Show read
					{
						const int framesSinceRead = currentFrameNo - pReadDataInfo->LastFrameRead;
						if (framesSinceRead < frameThreshold)
							drawCol = kDataReadCol;
					}

					*pPix++ = drawCol;
				}
				else
				{
					*pPix++ = dataCol;
				}

				state.AdvanceAddressRef(readAddrRef);
				state.AdvanceAddressRef(writeAddrRef);
				physicalAddress++;
			}
		}
	}
}


// TODO: Insights
// Search for common coding patterns such as:
// Multiplications through adds
// Conditional jumps