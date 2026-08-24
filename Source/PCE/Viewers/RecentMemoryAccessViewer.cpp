#include "RecentMemoryAccessViewer.h"

#include <imgui.h>
#include "../PCEEmu.h"
#include "../RecentMemoryAccess.h"

#include "CodeAnalyser/UI/CodeAnalyserUI.h"

FRecentMemoryAccessViewer::FRecentMemoryAccessViewer(FEmuBase* pEmu)
	: FViewerBase(pEmu)
{
	Name    = "Recent Memory Access";
	pPCEEmu = static_cast<FPCEEmu*>(pEmu);
}

bool FRecentMemoryAccessViewer::Init()
{
	return true;
}

void FRecentMemoryAccessViewer::DrawUI()
{
	FCodeAnalysisState& state = pPCEEmu->GetCodeAnalysis();
	FRecentMemoryAccess* pRMA = pPCEEmu->GetRecentMemAccess();

	if (ImGui::Button("Reset"))
	{
		pRMA->Reset();
	}
	ImGui::SameLine();
	if (ImGui::Checkbox("Include Stack", &bTrackStack))
	{
		pRMA->SetStackTracking(bTrackStack);
	}

	ImGuiWindowFlags window_flags = ImGuiWindowFlags_HorizontalScrollbar;
	{
		ImGui::BeginChild("RMAChildL", ImVec2(ImGui::GetContentRegionAvail().x * 0.5f, 0), ImGuiChildFlags_None, window_flags);
		ImGui::SeparatorText("Reads");

		FMemoryAccessBuf& buf = pRMA->Reads;
		if (ImGui::BeginTable("RMAReadsTable", 2, ImGuiTableFlags_RowBg | ImGuiTableFlags_BordersInnerV | ImGuiTableFlags_ScrollX))
		{
			ImGui::TableSetupColumn("Bytes", ImGuiTableColumnFlags_WidthFixed);
			ImGui::TableSetupColumn("Address", ImGuiTableColumnFlags_WidthStretch);

			// Use a clipper because this table can be expensive to draw.
			// This is because DrawAddressLabel is expensive as it iterates through memory.
			ImGuiListClipper clipper;
			clipper.Begin(buf.Count);
			while (clipper.Step())
			{
				for (int i = clipper.DisplayStart; i < clipper.DisplayEnd; i++)
				{
					if (const FMemoryAccessItem* pAccess = buf.GetItem(i))
					{
						const FCodeAnalysisBank* pBank = state.GetBank(pAccess->Addr.GetBankId());
						ImGui::TableNextRow();
						ImGui::TableSetColumnIndex(0);
						ImGui::Text("%d", pAccess->NumBytes);

						ImGui::TableSetColumnIndex(1);
						DrawAddressLabel(state, state.GetFocussedViewState(), pAccess->Addr);
					}
				}
			}
			ImGui::EndTable();
		}
		ImGui::EndChild();
	}
	ImGui::SameLine();
	{
		ImGui::BeginChild("RMAChildR", ImVec2(0, 0), ImGuiChildFlags_None, window_flags);
		FMemoryAccessBuf& buf = pRMA->Writes;
		ImGui::SeparatorText("Writes");
		if (ImGui::BeginTable("RMAWritesTable", 2, ImGuiTableFlags_RowBg | ImGuiTableFlags_BordersInnerV))
		{
			ImGui::TableSetupColumn("Bytes", ImGuiTableColumnFlags_WidthFixed);
			ImGui::TableSetupColumn("Address", ImGuiTableColumnFlags_WidthStretch);

			// Use a clipper because this table can be expensive to draw.
			// This is because DrawAddressLabel is expensive as it iterates through memory.
			ImGuiListClipper clipper;
			clipper.Begin(buf.Count);
			while (clipper.Step())
			{
				for (int i = clipper.DisplayStart; i < clipper.DisplayEnd; i++)
				{
					if (const FMemoryAccessItem* pAccess = buf.GetItem(i))
					{
						const FCodeAnalysisBank* pBank = state.GetBank(pAccess->Addr.GetBankId());
						ImGui::TableNextRow();
						ImGui::TableSetColumnIndex(0);
						ImGui::Text("%d", pAccess->NumBytes);

						ImGui::TableSetColumnIndex(1);
						DrawAddressLabel(state, state.GetFocussedViewState(), pAccess->Addr);
					}
				}
			}
			ImGui::EndTable();
		}
		ImGui::EndChild();
	}
}
