#include "OverviewViewer.h"

#include <imgui.h>
#include <implot.h>

void FOverviewViewer::DrawUI(void)
{
    DrawStats();
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
