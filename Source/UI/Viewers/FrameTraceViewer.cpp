#include "FrameTraceViewer.h"
#include "ZXGraphicsView.h"
#include "../SpectrumEmu.h"

#include <imgui.h>
#include <Shared/CodeAnalyser/UI/CodeAnalyserUI.h>
#include <Shared/ImGuiSupport/imgui_impl_lucidextra.h>
#include <Shared/Util/Misc.h>


void FFrameTraceViewer::Init(FSpectrumEmu* pEmu)
{
	pSpectrumEmu = pEmu;

	// Init Frame Trace
	for (int i = 0; i < kNoFramesInTrace; i++)
	{
		FrameTrace[i].Texture = ImGui_ImplDX11_CreateTextureRGBA(static_cast<unsigned char*>(pEmu->FrameBuffer), 320, 256);
		FrameTrace[i].CPUState = malloc(sizeof(z80_t));
	}

	ShowWritesView = new FZXGraphicsView(320, 256);
}

void	FFrameTraceViewer::Shutdown()
{
	for (int i = 0; i < kNoFramesInTrace; i++)
	{
		ImGui_ImplDX11_FreeTexture(FrameTrace[i].Texture);
		FrameTrace[i].Texture = nullptr;
		free(FrameTrace[i].CPUState);
	}

	delete ShowWritesView;
	ShowWritesView = nullptr;
}


void FFrameTraceViewer::CaptureFrame()
{
	// set up new trace frame
	FSpeccyFrameTrace& frame = FrameTrace[CurrentTraceFrame];
	ImGui_ImplDX11_UpdateTextureRGBA(frame.Texture, pSpectrumEmu->FrameBuffer);
	frame.InstructionTrace = pSpectrumEmu->CodeAnalysis.FrameTrace;
	frame.ScreenPixWrites = pSpectrumEmu->FrameScreenPixWrites;
	frame.ScreenAttrWrites = pSpectrumEmu->FrameScreenAttrWrites;
	frame.FrameOverview.clear();

	// copy memory
	for (int i = 0; i < 1 << 16; i++)
		frame.MemoryDump[i] = pSpectrumEmu->ReadByte(i);

	// get CPU state
	z80_t* pCPUState = (z80_t*)frame.CPUState;
	pCPUState->bc_de_hl_fa = pSpectrumEmu->ZXEmuState.cpu.bc_de_hl_fa;
	pCPUState->bc_de_hl_fa_ = pSpectrumEmu->ZXEmuState.cpu.bc_de_hl_fa_;
	pCPUState->wz_ix_iy_sp = pSpectrumEmu->ZXEmuState.cpu.wz_ix_iy_sp;
	pCPUState->im_ir_pc_bits = pSpectrumEmu->ZXEmuState.cpu.im_ir_pc_bits;
	pCPUState->pins = pSpectrumEmu->ZXEmuState.cpu.pins;

	// Generate diffs
	const int prevFrameIndex = CurrentTraceFrame == 0 ? kNoFramesInTrace - 1 : CurrentTraceFrame - 1;
	const FSpeccyFrameTrace& prevFrame = FrameTrace[prevFrameIndex];

	GenerateMemoryDiff(frame, prevFrame, frame.MemoryDiffs);

	if (++CurrentTraceFrame == kNoFramesInTrace)
		CurrentTraceFrame = 0;
}


void FFrameTraceViewer::Draw()
{
	if (ImGui::ArrowButton("##left", ImGuiDir_Left))
		ShowFrame = std::max(--ShowFrame, 0);

	ImGui::SameLine();

	if (ImGui::ArrowButton("##right", ImGuiDir_Right))
		ShowFrame = std::min(++ShowFrame, kNoFramesInTrace - 1);

	ImGui::SameLine();

	if (ImGui::SliderInt("Backwards Offset", &ShowFrame, 0, kNoFramesInTrace - 1))
	{
		if (ShowFrame == 0)
			pSpectrumEmu->CodeAnalysis.CPUInterface->Continue();
		else
			pSpectrumEmu->CodeAnalysis.CPUInterface->Break();

		PixelWriteline = -1;
		SelectedTraceLine = -1;
		int frameNo = CurrentTraceFrame - ShowFrame - 1;
		if (frameNo < 0)
			frameNo += kNoFramesInTrace;
		DrawFrameScreenWritePixels(FrameTrace[frameNo]);
	}

	int frameNo = CurrentTraceFrame - ShowFrame - 1;
	if (frameNo < 0)
		frameNo += kNoFramesInTrace;
	const FSpeccyFrameTrace& frame = FrameTrace[frameNo];
	
	if (ImGui::Button("Restore"))
	{
		// restore CPU regs
		const z80_t* pCPUState = (const z80_t*)frame.CPUState;
		pSpectrumEmu->ZXEmuState.cpu.bc_de_hl_fa = pCPUState->bc_de_hl_fa;
		pSpectrumEmu->ZXEmuState.cpu.bc_de_hl_fa_ = pCPUState->bc_de_hl_fa_;
		pSpectrumEmu->ZXEmuState.cpu.wz_ix_iy_sp = pCPUState->wz_ix_iy_sp;
		pSpectrumEmu->ZXEmuState.cpu.im_ir_pc_bits = pCPUState->im_ir_pc_bits;
		pSpectrumEmu->ZXEmuState.cpu.pins = pCPUState->pins;

		// restore memory
		for (int i = 0; i < 1 << 16; i++)
			pSpectrumEmu->WriteByte(i,frame.MemoryDump[i]);

		// continue running
		pSpectrumEmu->CodeAnalysis.CPUInterface->Continue();

		CurrentTraceFrame = frameNo;
		ShowFrame = 0;
	}

	
	ImGui::Image(frame.Texture, ImVec2(320, 256));
	ImGui::SameLine();
	ShowWritesView->Draw();

	// draw clipped list
	if (ImGui::BeginTabBar("FrameTraceTabs"))
	{
		if (ImGui::BeginTabItem("Instruction Trace"))
		{
			DrawInstructionTrace(frame);
			ImGui::EndTabItem();
		}

		if (ImGui::BeginTabItem("Trace Overview"))
		{
			if (frame.FrameOverview.size() == 0)
				GenerateTraceOverview(FrameTrace[frameNo]);
			DrawTraceOverview(frame);
			ImGui::EndTabItem();
		}

		if (ImGui::BeginTabItem("Screen Writes"))
		{
			DrawScreenWrites(frame);
			ImGui::EndTabItem();
		}

		if (ImGui::BeginTabItem("Diff"))
		{
			DrawMemoryDiffs(frame);
			ImGui::EndTabItem();
		}

		ImGui::EndTabBar();
	}
	
}

void	FFrameTraceViewer::DrawInstructionTrace(const FSpeccyFrameTrace& frame)
{
	FCodeAnalysisState& state = pSpectrumEmu->CodeAnalysis;
	FCodeAnalysisViewState& viewState = state.GetFocussedViewState();
	const float line_height = ImGui::GetTextLineHeight();
	ImGuiListClipper clipper((int)frame.InstructionTrace.size(), line_height);

	while (clipper.Step())
	{
		for (int i = clipper.DisplayStart; i < clipper.DisplayEnd; i++)
		{
			const uint16_t instAddr = frame.InstructionTrace[i];

			ImGui::PushID(i);

			if (ImGui::Selectable("##traceline", i == SelectedTraceLine, 0))
			{
				SelectedTraceLine = i;
				DrawFrameScreenWritePixels(frame, i);
			}
			ImGui::SetItemAllowOverlap();	// allow buttons
			ImGui::SameLine();

			FCodeInfo* pCodeInfo = state.GetCodeInfoForAddress(instAddr);
			if (pCodeInfo)
			{
				ImGui::Text("%s %s", NumStr(instAddr), pCodeInfo->Text.c_str());
				ImGui::SameLine();
				DrawAddressLabel(state, viewState, instAddr);
			}

			ImGui::PopID();
		}
	}
}

void	FFrameTraceViewer::GenerateTraceOverview(FSpeccyFrameTrace& frame)
{
	FCodeAnalysisState& state = pSpectrumEmu->CodeAnalysis;
	frame.FrameOverview.clear();
	for (int i = 0; i < frame.InstructionTrace.size(); i++)
	{
		const uint16_t instAddr = frame.InstructionTrace[i];

		// TODO: find closest global label
		int labelOffset = 0;
		uint16_t labelAddress = 0;
		uint16_t functionAddress = 0;
		const char* pLabelString = nullptr;
		bool bFound = false;

		//for (int traceIndex = i; traceIndex >= 0; traceIndex--)
		for (int addrVal = instAddr; addrVal >= 0; addrVal--)
		{
			//uint16_t addrVal = frame.InstructionTrace[traceIndex];
			const FLabelInfo* pLabel = state.GetLabelForAddress(addrVal);
			if (pLabel != nullptr && (pLabel->LabelType == LabelType::Code || pLabel->LabelType == LabelType::Function))
			{
				if (pLabel->LabelType == LabelType::Function)
				{
					functionAddress = addrVal;
					pLabelString = pLabel->Name.c_str();
					bFound = true;
					if (labelAddress == 0)	// we found a function before a label
					{
						labelAddress = functionAddress;
					}
					break;
				}
				if (pLabel->LabelType == LabelType::Code)
				{
					labelAddress = addrVal;
				}
			}

			labelOffset++;
		}

		if (bFound)
		{
			if (frame.FrameOverview.empty() || frame.FrameOverview.back().LabelAddress != labelAddress)
			{
				FFrameOverviewItem newItem;
				newItem.Label = pLabelString;
				newItem.LabelAddress = labelAddress;
				newItem.FunctionAddress = functionAddress;
				frame.FrameOverview.push_back(newItem);
			}
		}
	}
}

void FFrameTraceViewer::GenerateMemoryDiff(const FSpeccyFrameTrace& frame, const FSpeccyFrameTrace& otherFrame, std::vector<FMemoryDiff>& outDiff)
{
	outDiff.clear();

	// diff RAM with previous frame
	// skip ROM & screen memory
	// might want to exclude stack (once we determine where it is)
	for (int i = 0x5C00; i < 1 << 16; i++)	
	{
		if (frame.MemoryDump[i] != otherFrame.MemoryDump[i])
		{
			FMemoryDiff diff;
			diff.Address = i;
			diff.NewVal = frame.MemoryDump[i];
			diff.OldVal = otherFrame.MemoryDump[i];
			outDiff.push_back(diff);
		}
	}
}

void	FFrameTraceViewer::DrawTraceOverview(const FSpeccyFrameTrace& frame)
{
	FCodeAnalysisState& state = pSpectrumEmu->CodeAnalysis;
	FCodeAnalysisViewState& viewState = state.GetFocussedViewState();
	const float line_height = ImGui::GetTextLineHeight();
	ImGuiListClipper clipper((int)frame.FrameOverview.size(), line_height);

	while (clipper.Step())
	{
		for (int i = clipper.DisplayStart; i < clipper.DisplayEnd; i++)
		{
			const FFrameOverviewItem& overview = frame.FrameOverview[i];

			ImGui::Text("%s", overview.Label.c_str());
			if (overview.FunctionAddress != 0)
			{
				DrawAddressLabel(state, viewState, overview.FunctionAddress);
			}
			if (overview.LabelAddress != overview.FunctionAddress)
			{
				ImGui::SameLine();
				DrawAddressLabel(state, viewState, overview.LabelAddress);
			}
		}
	}
}

void FFrameTraceViewer::DrawFrameScreenWritePixels(const FSpeccyFrameTrace& frame, int lastIndex)
{
	if (lastIndex == -1 || lastIndex >= frame.ScreenPixWrites.size())
		lastIndex = (int)frame.ScreenPixWrites.size() - 1;
	ShowWritesView->Clear(0);
	for (int i = 0; i < lastIndex; i++)
	{
		const FMemoryAccess& access = frame.ScreenPixWrites[i];
		int xp, yp;
		GetScreenAddressCoords(access.Address, xp, yp);
		const uint16_t attrAddress = GetScreenAttrMemoryAddress(xp, yp);
		const uint8_t attr = pSpectrumEmu->ReadByte(attrAddress);
		ShowWritesView->DrawCharLine(access.Value, xp, yp, attr);
	}
}

void	FFrameTraceViewer::DrawScreenWrites(const FSpeccyFrameTrace& frame)
{
	FCodeAnalysisState& state = pSpectrumEmu->CodeAnalysis;
	FCodeAnalysisViewState& viewState = state.GetFocussedViewState();

	if (ImGui::BeginChild("ScreenPxWrites", ImVec2(ImGui::GetWindowContentRegionWidth() * 0.5f, 0), true))
	{
		const float line_height = ImGui::GetTextLineHeight();
		ImGuiListClipper clipper((int)frame.ScreenPixWrites.size(), line_height);

		while (clipper.Step())
		{
			for (int i = clipper.DisplayStart; i < clipper.DisplayEnd; i++)
			{
				const FMemoryAccess& access = frame.ScreenPixWrites[i];
				ImGui::PushID(i);
				// selectable
				if (ImGui::Selectable("##screenwriteline", i == PixelWriteline, 0))
				{
					PixelWriteline = i;
					DrawFrameScreenWritePixels(frame, i);
				}
				ImGui::SetItemAllowOverlap();	// allow buttons
				ImGui::SameLine();

				ImGui::Text("%s (%s) : ", NumStr(access.Address), NumStr(access.Value));
				ImGui::SameLine();
				DrawCodeAddress(state, viewState, access.PC);
				ImGui::PopID();
			}
		}

	}
	ImGui::EndChild();
	ImGui::SameLine();
	if (ImGui::BeginChild("ScreenAttrWrites", ImVec2(0, 0), true))
	{
	}
	ImGui::EndChild();
}

void FFrameTraceViewer::DrawMemoryDiffs(const FSpeccyFrameTrace& frame)
{
	FCodeAnalysisState& state = pSpectrumEmu->CodeAnalysis;
	FCodeAnalysisViewState& viewState = state.GetFocussedViewState();

	for (const auto& diff : frame.MemoryDiffs)
	{
		ImGui::Text("%s : ", NumStr(diff.Address));
		DrawAddressLabel(state, viewState, diff.Address);
		ImGui::SameLine();
		ImGui::Text("%d(%s) -> %d(%s)", diff.OldVal, NumStr(diff.OldVal), diff.NewVal, NumStr(diff.NewVal));
	}
}
