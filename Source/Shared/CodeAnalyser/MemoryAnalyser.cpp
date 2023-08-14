#include "MemoryAnalyser.h"

#include "CodeAnalyser.h"

#include <imgui.h>
#include "UI/CodeAnalyserUI.h"

void FMemoryAnalyser::Init(FCodeAnalysisState* ptrCodeAnalysis)
{
	pCodeAnalysis = ptrCodeAnalysis;
}

void FMemoryAnalyser::FrameTick(void)
{

}

void FMemoryAnalyser::DrawUI(void)
{
	if (ImGui::BeginTabBar("MemoryAnalyserTabBar"))
	{
		if (ImGui::BeginTabItem("Diff"))
		{
			DrawPhysicalMemoryDiffUI();
		}
		ImGui::EndTabItem();
	}
	ImGui::EndTabBar();
}


void FMemoryAnalyser::DrawPhysicalMemoryDiffUI(void)
{
	FCodeAnalysisViewState& viewState = pCodeAnalysis->GetFocussedViewState();
	
	if (ImGui::Button("SnapShot"))
	{
		for (int addr = 0; addr < (1 << 16); addr++)	// might as well snapshot the whole thing
		{
			DiffSnapShotMemory[addr] = pCodeAnalysis->ReadByte(addr);
		}
		bSnapshotAvailable = true;
		DiffChangedLocations.clear();
	}

	if (bSnapshotAvailable)
	{
		ImGui::SameLine();

		if (ImGui::Button("Diff"))
		{
			DiffChangedLocations.clear();
			for (int addr = 0; addr < (1 << 16); addr++)
			{
				if (ROMArea.InRange(addr) == false && (ScreenMemory.InRange(addr) == false || bDiffVideoMem))
				{
					if (pCodeAnalysis->ReadByte(addr) != DiffSnapShotMemory[addr])
						DiffChangedLocations.push_back(pCodeAnalysis->AddressRefFromPhysicalAddress(addr));
				}
			}
		}
	}

	ImGui::SameLine();
	ImGui::Checkbox("Include video memory", &bDiffVideoMem);
	ImGuiWindowFlags window_flags = ImGuiWindowFlags_HorizontalScrollbar;
	if (ImGui::BeginChild("DiffedMemory", ImVec2(0, 0), true, window_flags))
	{
		static ImGuiTableFlags tableFLags = ImGuiTableFlags_SizingFixedFit | ImGuiTableFlags_RowBg | ImGuiTableFlags_Borders | ImGuiTableFlags_Resizable | ImGuiTableFlags_Reorderable | ImGuiTableFlags_Hideable;
		if (ImGui::BeginTable("diffresults", 4, tableFLags))
		{
			ImGui::TableSetupColumn("Address");
			ImGui::TableSetupColumn("Old Value");
			ImGui::TableSetupColumn("New Value");
			ImGui::TableSetupColumn("Writer");
			ImGui::TableHeadersRow();

			// TODO: use clipper?
			for (const auto changedAddr : DiffChangedLocations)
			{
				const FDataInfo* pDataInfo = pCodeAnalysis->GetWriteDataInfoForAddress(changedAddr);
				ImGui::TableNextRow();
				ImGui::PushID(changedAddr.Val);
				
				ImGui::TableSetColumnIndex(0);
				ImGui::Text("%s", NumStr(changedAddr.Address));
				DrawAddressLabel(*pCodeAnalysis, viewState, changedAddr);

				ImGui::TableSetColumnIndex(1);
				ImGui::Text("%s", NumStr(DiffSnapShotMemory[changedAddr.Address]));

				ImGui::TableSetColumnIndex(2);
				ImGui::Text("%s", NumStr(pCodeAnalysis->ReadByte(changedAddr)));

				ImGui::TableSetColumnIndex(3);
				ImGui::Text("");
				DrawAddressLabel(*pCodeAnalysis, viewState, pDataInfo->LastWriter);

				ImGui::PopID();
			}
		}

		ImGui::EndTable();

	}
	ImGui::EndChild();
}

