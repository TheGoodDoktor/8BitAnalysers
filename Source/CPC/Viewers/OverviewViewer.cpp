#include "OverviewViewer.h"
#include "../CPCEmu.h"

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
}

void FOverviewViewer::CalculateStats()
{
    int UnCommentedCodeCount = 0;
    int CommentedCodeCount = 0;
    int ReadOnlyDataCount = 0;
    int WriteOnlyDataCount = 0;
    int ReadWriteDataCount = 0;
    int UnknownCount = 0;

    for (int i = 0; i < (1 << 16); i++)
    {
        const bool bInRom = i < 0x4000;  // sam todo. this wont work for cpc
        const FCodeInfo* pCodeInfo = pCpcEmu->CodeAnalysis.GetCodeInfoForAddress(i);
        if (pCodeInfo != nullptr)
        {
            if (pCodeInfo->Comment.empty())
                UnCommentedCodeCount++;
            else
                CommentedCodeCount++;
        }
        else
        {
            const FDataInfo *pDataInfo = pCpcEmu->CodeAnalysis.GetReadDataInfoForAddress(i);
            const bool bRead = pDataInfo->LastFrameRead != -1;
            const bool bWrite = pDataInfo->LastFrameWritten != -1;

            if (bInRom)
            {
                ReadOnlyDataCount++;
            }
            else
            {
                if (bRead && !bWrite)
                    ReadOnlyDataCount++;
                else if (!bRead && bWrite)
                    WriteOnlyDataCount++;
                else if (bRead && bWrite)
                    ReadWriteDataCount++;
                else
                    UnknownCount++;
            }
        }
    }

    // Calculate percentages
    Stats.PercentCommentedCode = CommentedCodeCount * (1.0f / 65536.0f) * 100.0f;
    Stats.PercentUncommentedCode = UnCommentedCodeCount * (1.0f / 65536.0f) * 100.0f;
    Stats.PercentReadOnlyData = ReadOnlyDataCount * (1.0f / 65536.0f) * 100.0f;
    Stats.PercentWriteOnlyData = WriteOnlyDataCount * (1.0f / 65536.0f) * 100.0f;
    Stats.PercentReadWriteData = ReadWriteDataCount * (1.0f / 65536.0f) * 100.0f;
    Stats.PercentUnknown = UnknownCount * (1.0f / 65536.0f) * 100.0f;
}
