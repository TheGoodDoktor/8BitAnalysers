#include "MemoryAnalyser.h"

#include "CodeAnalyser.h"

#include <imgui.h>
#include "UI/CodeAnalyserUI.h"



void FMemoryAnalyser::Init(FCodeAnalysisState* ptrCodeAnalysis)
{
	pCodeAnalysis = ptrCodeAnalysis;

	const auto& banks = pCodeAnalysis->GetBanks();
	for(const FCodeAnalysisBank& bank : banks)
	{
		FBankMemory& bankMem = DiffSnapshotMemoryBanks[bank.Id];
		bankMem.BankId = bank.Id;
		bankMem.SizeBytes = bank.GetSizeBytes();
		bankMem.pMemory = new uint8_t[bankMem.SizeBytes];
	}

	FindTool.Init(ptrCodeAnalysis);
}

void FMemoryAnalyser::Shutdown()
{
	for (auto& bankMemIt : DiffSnapshotMemoryBanks)
	{
		delete[] bankMemIt.second.pMemory;
	}
	DiffSnapshotMemoryBanks.clear();
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
			ImGui::EndTabItem();
		}

		if (ImGui::BeginTabItem("Find"))
		{
			FindTool.DrawUI();
			ImGui::EndTabItem();
		}
	}
	ImGui::EndTabBar();
}


void FMemoryAnalyser::DrawPhysicalMemoryDiffUI(void)
{
	FCodeAnalysisViewState& viewState = pCodeAnalysis->GetFocussedViewState();
	
	if (ImGui::Button("SnapShot"))
	{
		// Capture banks
		for (auto& memBankIt : DiffSnapshotMemoryBanks)
		{
			FBankMemory& memBank = memBankIt.second;
			const FCodeAnalysisBank* pBank = pCodeAnalysis->GetBank(memBank.BankId);

			if (pBank->bReadOnly)	// skip ROM banks
				continue;

			if (bDiffPhysicalMemory == false || pBank->IsMapped())
			{
				assert(pBank != nullptr);
				assert(pBank->GetSizeBytes() == memBank.SizeBytes);

				memcpy(memBank.pMemory, pBank->Memory, memBank.SizeBytes);
			}
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

			for (auto& memBankIt : DiffSnapshotMemoryBanks)
			{
				FBankMemory& memBank = memBankIt.second;
				const FCodeAnalysisBank* pBank = pCodeAnalysis->GetBank(memBank.BankId);

				if (bDiffPhysicalMemory == false || pBank->IsMapped())
				{
					assert(pBank != nullptr);
					assert(pBank->GetSizeBytes() == memBank.SizeBytes);

					if (pBank->bReadOnly)	// skip ROM banks
						continue;

					for (uint16_t addrOffset = 0; addrOffset < memBank.SizeBytes; addrOffset++)
					{
						uint16_t address = pBank->GetMappedAddress() + addrOffset;
						if (ScreenMemory.InRange(address) == false || bDiffVideoMem)
						{
							if (pBank->Memory[addrOffset] != memBank.pMemory[addrOffset])
								DiffChangedLocations.push_back(FAddressRef(pBank->Id, address));
						}
					}
				}
			}
		}
	}

	ImGui::SameLine();
	ImGui::Checkbox("Include video memory", &bDiffVideoMem);
	ImGui::SameLine();
	ImGui::Checkbox("Physical memory", &bDiffPhysicalMemory);
	
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
				
				// Address
				ImGui::TableSetColumnIndex(0);
				ImGui::Text("%s", NumStr(changedAddr.Address));
				DrawAddressLabel(*pCodeAnalysis, viewState, changedAddr);

				// Snapshot value
				ImGui::TableSetColumnIndex(1);
				const uint8_t oldValue = DiffSnapshotMemoryBanks[changedAddr.BankId].pMemory[changedAddr.Address];
				ImGui::Text("%s", NumStr(oldValue));

				// Current value
				ImGui::TableSetColumnIndex(2);
				ImGui::Text("%s", NumStr(pCodeAnalysis->ReadByte(changedAddr)));

				// Code address that last wrote to value
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

