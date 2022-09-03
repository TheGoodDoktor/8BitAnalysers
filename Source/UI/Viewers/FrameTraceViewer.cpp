#include "FrameTraceViewer.h"
#include "../SpeccyUI.h"

#include <imgui.h>
#include <Shared/CodeAnalyser/CodeAnalyserUI.h>
#include <Shared/ImGuiSupport/imgui_impl_lucidextra.h>


void FFrameTraceViewer::Init(FSpeccyUI* pEmu)
{
	pSpectrumEmu = pEmu;

	// Init Frame Trace
	for (int i = 0; i < kNoFramesInTrace; i++)
	{
		FrameTrace[i].Texture = ImGui_ImplDX11_CreateTextureRGBA(static_cast<unsigned char*>(pEmu->pSpeccy->FrameBuffer), 320, 256);
	}
}

void FFrameTraceViewer::CaptureFrame()
{
	ImGui_ImplDX11_UpdateTextureRGBA(FrameTrace[CurrentTraceFrame].Texture, pSpectrumEmu->pSpeccy->FrameBuffer);
	FrameTrace[CurrentTraceFrame].InstructionTrace = pSpectrumEmu->CodeAnalysis.FrameTrace;
	if (++CurrentTraceFrame == kNoFramesInTrace)
		CurrentTraceFrame = 0;
}


void FFrameTraceViewer::Draw()
{
	if (ImGui::SliderInt("Backwards Offset", &ShowFrame, 0, kNoFramesInTrace - 1))
	{
		if (ShowFrame == 0)
			pSpectrumEmu->CodeAnalysis.CPUInterface->Continue();
		else
			pSpectrumEmu->CodeAnalysis.CPUInterface->Break();
	}

	int frameNo = CurrentTraceFrame - ShowFrame - 1;
	if (frameNo < 0)
		frameNo += kNoFramesInTrace;
	const FSpeccyFrameTrace& frame = FrameTrace[frameNo];
	ImGui::Image(frame.Texture, ImVec2(320, 256));

	// draw clipped list
	const float line_height = ImGui::GetTextLineHeight();
	ImGuiListClipper clipper((int)frame.InstructionTrace.size(), line_height);

	while (clipper.Step())
	{
		for (int i = clipper.DisplayStart; i < clipper.DisplayEnd; i++)
		{
			const uint16_t instAddr = frame.InstructionTrace[i];

			/*const int index = GetItemIndexForAddress(pUI->CodeAnalysis, instAddr);
			if (index != -1)
			{
				DrawCodeAnalysisItemAtIndex(pUI->CodeAnalysis, index);
			}*/
			//DrawCodeAddress(pUI->CodeAnalysis, instAddr);

			FCodeInfo* pCodeInfo = pSpectrumEmu->CodeAnalysis.GetCodeInfoForAddress(instAddr);
			if (pCodeInfo)
			{
				ImGui::Text("%04Xh %s", instAddr, pCodeInfo->Text.c_str());
				ImGui::SameLine();
				DrawAddressLabel(pSpectrumEmu->CodeAnalysis, instAddr);
			}
		}
	}
}
