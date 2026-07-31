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
		ImGui::BeginChild("ChildL", ImVec2(ImGui::GetContentRegionAvail().x * 0.5f, 0), ImGuiChildFlags_None, window_flags);
		ImGui::SeparatorText("Reads");

		FMemoryAccessBuf& buf = pRMA->Reads;
		for (int i = 0; i < buf.Count; i++)
		{
			if (const FMemoryAccessItem* pAccess = buf.GetItem(i))
			{
				const FCodeAnalysisBank* pBank = state.GetBank(pAccess->Addr.GetBankId());
				ImGui::Text("%d byte%s", pAccess->NumBytes, pAccess->NumBytes > 1 ? "s " : "  ");
				DrawAddressLabel(state, state.GetFocussedViewState(), pAccess->Addr);
			}
		}
		ImGui::EndChild();
	}
	ImGui::SameLine();
	{
		ImGui::BeginChild("ChildR", ImVec2(0, 0), ImGuiChildFlags_None, window_flags);
		FMemoryAccessBuf& buf = pRMA->Writes;
		ImGui::SeparatorText("Writes");
		for (int i = 0; i < buf.Count; i++)
		{
			if (const FMemoryAccessItem* pAccess = buf.GetItem(i))
			{
				const FCodeAnalysisBank* pBank = state.GetBank(pAccess->Addr.GetBankId());
				ImGui::Text("%d byte%s", pAccess->NumBytes, pAccess->NumBytes > 1 ? "s " : "  ");
				DrawAddressLabel(state, state.GetFocussedViewState(), pAccess->Addr);
			}
		}
		ImGui::EndChild();
	}
}
