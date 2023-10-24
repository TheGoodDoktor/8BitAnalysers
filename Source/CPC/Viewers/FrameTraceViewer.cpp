#include "FrameTraceViewer.h"
#include "CPCGraphicsView.h"
#include "../CPCEmu.h"

#include <imgui.h>
#include <CodeAnalyser/UI/CodeAnalyserUI.h>
#include <ImGuiSupport/ImGuiTexture.h>

#include <Util/Misc.h>


void FFrameTraceViewer::Init(FCpcEmu* pEmu)
{
	pCpcEmu = pEmu;
	chips_display_info_t dispInfo = cpc_display_info(&pEmu->CpcEmuState);

	// Init Frame Trace
	for (int i = 0; i < kNoFramesInTrace; i++)
	{
		FrameTrace[i].Texture = ImGui_CreateTextureRGBA(pEmu->CpcViewer.GetFrameBuffer(), dispInfo.frame.dim.width, dispInfo.frame.dim.height);
		FrameTrace[i].CPUState = malloc(sizeof(z80_t));
	}

	ShowWritesView = new FCpcGraphicsView(320, 256);
}

void FFrameTraceViewer::Reset()
{
	for (int i = 0; i < kNoFramesInTrace; i++)
	{
		auto& frame = FrameTrace[i];
		frame.InstructionTrace.clear();
		frame.FrameEvents.clear();
		frame.FrameOverview.clear();
		frame.MemoryDiffs.clear();
	}
}

void	FFrameTraceViewer::Shutdown()
{
	for (int i = 0; i < kNoFramesInTrace; i++)
	{
		ImGui_FreeTexture(FrameTrace[i].Texture);
		FrameTrace[i].Texture = nullptr;
		free(FrameTrace[i].CPUState);
	}

	delete ShowWritesView;
	ShowWritesView = nullptr;
}


void FFrameTraceViewer::CaptureFrame()
{
	// temp
	return;

	// set up new trace frame
	FCpcFrameTrace& frame = FrameTrace[CurrentTraceFrame];
	ImGui_UpdateTextureRGBA(frame.Texture, pCpcEmu->CpcViewer.GetFrameBuffer());
	frame.InstructionTrace = pCpcEmu->CodeAnalysis.Debugger.GetFrameTrace();	// copy frame trace - use method?
	frame.FrameEvents = pCpcEmu->CodeAnalysis.Debugger.GetEventTrace();
	frame.FrameOverview.clear();

	// copy memory
	const int noBanks = pCpcEmu->CpcEmuState.type == CPC_TYPE_464 ? 3:8;
	for (int i = 0; i < noBanks; i++)
		memcpy(frame.MemoryBanks[i], pCpcEmu->CpcEmuState.ram[i], 16 * 1024);

#if SPECCY
	frame.MemoryBankRegister = pCpcEmu->CpcEmuState.last_mem_config;
#endif
	// get CPU state
	memcpy(frame.CPUState, &pCpcEmu->CpcEmuState.cpu, sizeof(z80_t));

	// Generate diffs
	const int prevFrameIndex = CurrentTraceFrame == 0 ? kNoFramesInTrace - 1 : CurrentTraceFrame - 1;
	const FCpcFrameTrace& prevFrame = FrameTrace[prevFrameIndex];

	// Not used atm
	//GenerateMemoryDiff(frame, prevFrame, frame.MemoryDiffs);

	if (++CurrentTraceFrame == kNoFramesInTrace)
		CurrentTraceFrame = 0;
}


void FFrameTraceViewer::RestoreFrame(const FCpcFrameTrace& frame)
{
#if SPECCY
	// restore CPU regs
	memcpy(&pCpcEmu->CpcEmuState.cpu, frame.CPUState, sizeof(z80_t));

	// restore memory
	const int noBanks = pCpcEmu->CpcEmuState.type == CPC_TYPE_464 ? 3 : 8;
	for (int i = 0; i < noBanks; i++)
		memcpy(FCpcEmu->CpcEmuState.ram[i],frame.MemoryBanks[i],  16 * 1024);

	// restore bank setup
	if (pCpcEmu->CpcEmuState.type == CPC_TYPE_6128)
	{
		pCpcEmu->CpcEmuState.last_mem_config = frame.MemoryBankRegister;

		// bit 3 defines the video scanout memory bank (5 or 7) 
		pCpcEmu->CpcEmuState.display_ram_bank = (frame.MemoryBankRegister & (1 << 3)) ? 7 : 5;

		// map last bank
		mem_map_ram(&pCpcEmu->CpcEmuState.mem, 0, 0xC000, 0x4000, pCpcEmu->CpcEmuState.ram[frame.MemoryBankRegister & 0x7]);

		// map ROM
		if (frame.MemoryBankRegister & (1 << 4)) // bit 4 set: ROM1 
			mem_map_rom(&pCpcEmu->CpcEmuState.mem, 0, 0x0000, 0x4000, pCpcEmu->CpcEmuState.rom[1]);
		else // bit 4 clear: ROM0 
			mem_map_rom(&pCpcEmu->CpcEmuState.mem, 0, 0x0000, 0x4000, pCpcEmu->CpcEmuState.rom[0]);

		// Set code analysis banks
		pCpcEmu->SetROMBank(frame.MemoryBankRegister & (1 << 4) ? 1 : 0);
		pCpcEmu->SetRAMBank(3, frame.MemoryBankRegister & 0x7);
	}
#endif
}

void FFrameTraceViewer::Draw()
{
	// sam. todo
	return;

	if (ImGui::ArrowButton("##left", ImGuiDir_Left))
		ShowFrame = std::max(--ShowFrame, 0);

	ImGui::SameLine();

	if (ImGui::ArrowButton("##right", ImGuiDir_Right))
		ShowFrame = std::min(++ShowFrame, kNoFramesInTrace - 1);

	ImGui::SameLine();
	int frameNo = 0;

	if (ImGui::SliderInt("Backwards Offset", &ShowFrame, 0, kNoFramesInTrace - 1))
	{
		if (ShowFrame == 0)
			pCpcEmu->CodeAnalysis.Debugger.Continue();
		else
			pCpcEmu->CodeAnalysis.Debugger.Break();

		PixelWriteline = -1;
		SelectedTraceLine = -1;
		frameNo = CurrentTraceFrame - ShowFrame - 1;
		if (frameNo < 0)
			frameNo += kNoFramesInTrace;
		DrawFrameScreenWritePixels(FrameTrace[frameNo]);

		if (RestoreOnScrub)
			RestoreFrame(FrameTrace[frameNo]);
	}
	else
	{
		frameNo = CurrentTraceFrame - ShowFrame - 1;
		if (frameNo < 0)
			frameNo += kNoFramesInTrace;
	}
	const FCpcFrameTrace& frame = FrameTrace[frameNo];
	

	if (ImGui::Button("Restore"))
	{
		RestoreFrame(frame);

		// continue running
		pCpcEmu->CodeAnalysis.Debugger.Continue();

		CurrentTraceFrame = frameNo;
		ShowFrame = 0;
	}
#if SPECCY
	ImGui::SameLine();
	ImGui::Checkbox("Restore On Scrub", &RestoreOnScrub);
#endif
	ImVec2 uv0(0, 0);
	ImVec2 uv1(320.0f / 512.0f, 1.0f);
	ImGui::Image(frame.Texture, ImVec2(320, 256), uv0, uv1);
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

void	FFrameTraceViewer::DrawInstructionTrace(const FCpcFrameTrace& frame)
{
	FCodeAnalysisState& state = pCpcEmu->CodeAnalysis;
	FCodeAnalysisViewState& viewState = state.GetFocussedViewState();
	const float line_height = ImGui::GetTextLineHeight();
	ImGuiListClipper clipper((int)frame.InstructionTrace.size(), line_height);

	while (clipper.Step())
	{
		for (int i = clipper.DisplayStart; i < clipper.DisplayEnd; i++)
		{
			const FAddressRef instAddr = frame.InstructionTrace[i];

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
				ImGui::Text("%s %s", NumStr(instAddr.Address), pCodeInfo->Text.c_str());
				ImGui::SameLine();
				DrawAddressLabel(state, viewState, instAddr);
			}

			ImGui::PopID();
		}
	}
}

void	FFrameTraceViewer::GenerateTraceOverview(FCpcFrameTrace& frame)
{
	FCodeAnalysisState& state = pCpcEmu->CodeAnalysis;
	frame.FrameOverview.clear();
	for (int i = 0; i < frame.InstructionTrace.size(); i++)
	{
		const FAddressRef instAddr = frame.InstructionTrace[i];

		// TODO: find closest global label
		int labelOffset = 0;
		uint16_t labelAddress = 0;
		uint16_t functionAddress = 0;
		const char* pLabelString = nullptr;
		bool bFound = false;

		//for (int traceIndex = i; traceIndex >= 0; traceIndex--)
		for (int addrVal = instAddr.Address; addrVal >= 0; addrVal--)
		{
			//uint16_t addrVal = frame.InstructionTrace[traceIndex];
			const FLabelInfo* pLabel = state.GetLabelForPhysicalAddress(addrVal);
			if (pLabel != nullptr && (pLabel->LabelType == ELabelType::Code || pLabel->LabelType == ELabelType::Function))
			{
				if (pLabel->LabelType == ELabelType::Function)
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
				if (pLabel->LabelType == ELabelType::Code)
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

void FFrameTraceViewer::GenerateMemoryDiff(const FCpcFrameTrace& frame, const FCpcFrameTrace& otherFrame, std::vector<FMemoryDiff>& outDiff)
{
	outDiff.clear();

	// diff RAM with previous frame
	// skip ROM & screen memory
	// might want to exclude stack (once we determine where it is)
	const int noBanks = pCpcEmu->CpcEmuState.type == CPC_TYPE_464 ? 3 : 8;
	for (int bankNo = 0; bankNo < noBanks; bankNo++)
	{
		for (int addr = 0; addr < 16 * 1024; addr++)
		{
			if (frame.MemoryBanks[bankNo][addr] != otherFrame.MemoryBanks[bankNo][addr])
			{
				FMemoryDiff diff;
				diff.Address = addr;
				diff.NewVal = frame.MemoryBanks[bankNo][addr];
				diff.OldVal = otherFrame.MemoryBanks[bankNo][addr];
				outDiff.push_back(diff);
			}
		}
	}
}

void	FFrameTraceViewer::DrawTraceOverview(const FCpcFrameTrace& frame)
{
	FCodeAnalysisState& state = pCpcEmu->CodeAnalysis;
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

void FFrameTraceViewer::DrawFrameScreenWritePixels(const FCpcFrameTrace& frame, int lastIndex)
{
#if SPECCY
	if (lastIndex == -1 || lastIndex >= frame.ScreenPixWrites.size())
		lastIndex = (int)frame.ScreenPixWrites.size() - 1;
	ShowWritesView->Clear(0);
	for (int i = 0; i < lastIndex; i++)
	{
		const FMemoryAccess& access = frame.ScreenPixWrites[i];
		int xp, yp;
		GetScreenAddressCoords(access.Address.Address, xp, yp);
		const uint16_t attrAddress = GetScreenAttrMemoryAddress(xp, yp);
		const uint8_t attr = pCpcEmu->ReadByte(attrAddress);
		ShowWritesView->DrawCharLine(access.Value, xp, yp, attr);
	}
#endif
}

void	FFrameTraceViewer::DrawScreenWrites(const FCpcFrameTrace& frame)
{
	FCodeAnalysisState& state = pCpcEmu->CodeAnalysis;
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

				ImGui::Text("%s (%s) : ", NumStr(access.Address.Address), NumStr(access.Value));
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

void FFrameTraceViewer::DrawMemoryDiffs(const FCpcFrameTrace& frame)
{
	FCodeAnalysisState& state = pCpcEmu->CodeAnalysis;
	FCodeAnalysisViewState& viewState = state.GetFocussedViewState();

	for (const auto& diff : frame.MemoryDiffs)
	{
		ImGui::Text("%s : ", NumStr(diff.Address));
		DrawAddressLabel(state, viewState, diff.Address);
		ImGui::SameLine();
		ImGui::Text("%d(%s) -> %d(%s)", diff.OldVal, NumStr(diff.OldVal), diff.NewVal, NumStr(diff.NewVal));
	}
}
