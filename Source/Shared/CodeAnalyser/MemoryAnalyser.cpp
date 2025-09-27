#include "MemoryAnalyser.h"

#include "CodeAnalyser.h"
#include "Misc/EmuBase.h"

#include <imgui.h>
#include "UI/CodeAnalyserUI.h"

bool FMemoryAnalyser::Init(void)
{
	FCodeAnalysisState& state = pEmulator->GetCodeAnalysis();

	for (int b = 0; b < FCodeAnalysisState::BankCount; b++)
	{
		FCodeAnalysisBank& bank = state.GetBanks()[b];
	
		FBankMemory& bankMem = DiffSnapshotMemoryBanks[bank.Id];
		bankMem.BankId = bank.Id;
		bankMem.SizeBytes = bank.GetSizeBytes();
		bankMem.pMemory = new uint8_t[bankMem.SizeBytes];
	}

	FindTool.Init(&state);
	return true;
}

void FMemoryAnalyser::ResetForGame()
{

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
			DrawMemoryDiffUI();
			ImGui::EndTabItem();
		}

		if (ImGui::BeginTabItem("Find"))
		{
			FindTool.DrawUI();
			ImGui::EndTabItem();
		}

		if (ImGui::BeginTabItem("String Search"))
		{
			DrawStringSearchUI();
			ImGui::EndTabItem();
		}
	}
	ImGui::EndTabBar();
}


void FMemoryAnalyser::DrawMemoryDiffUI(void)
{
	FCodeAnalysisState& state = pEmulator->GetCodeAnalysis();
	FCodeAnalysisViewState& viewState = state.GetFocussedViewState();

	if (ImGui::Button("SnapShot"))
	{
		// Capture banks
		for (auto& memBankIt : DiffSnapshotMemoryBanks)
		{
			FBankMemory& memBank = memBankIt.second;
			const FCodeAnalysisBank* pBank = state.GetBank(memBank.BankId);

			if (pBank->bMachineROM)	// skip machine ROM banks
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
				const FCodeAnalysisBank* pBank = state.GetBank(memBank.BankId);

				if (bDiffPhysicalMemory == false || pBank->IsMapped())
				{
					assert(pBank != nullptr);
					assert(pBank->GetSizeBytes() == memBank.SizeBytes);

					if (pBank->bMachineROM)	// skip machine ROM banks
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
	//if (ImGui::BeginChild("DiffedMemory", ImVec2(0, 0), true, window_flags))
	{
		static ImGuiTableFlags tableFLags = ImGuiTableFlags_SizingFixedFit | ImGuiTableFlags_RowBg 
			| ImGuiTableFlags_Borders | ImGuiTableFlags_Resizable | ImGuiTableFlags_Reorderable | ImGuiTableFlags_Hideable
			| ImGuiTableFlags_ScrollY;

		if (ImGui::BeginTable("diffresults", 4, tableFLags))
		{
			ImGui::TableSetupScrollFreeze(0, 1); // Make top row always visible
			ImGui::TableSetupColumn("Address");
			ImGui::TableSetupColumn("Old Value");
			ImGui::TableSetupColumn("New Value");
			ImGui::TableSetupColumn("Writer");
			ImGui::TableHeadersRow();

			ImGuiListClipper clipper;
			clipper.Begin((int)DiffChangedLocations.size());
			while (clipper.Step())
			{
				for (int rowNum = clipper.DisplayStart; rowNum < clipper.DisplayEnd; rowNum++)
				{
					FAddressRef changedAddr = DiffChangedLocations[rowNum];
					const FDataInfo* pDataInfo = state.GetDataInfoForAddress(changedAddr);
					ImGui::TableNextRow();
					ImGui::PushID(changedAddr.GetVal());

					// Address
					ImGui::TableSetColumnIndex(0);
					ImGui::Text("%s", NumStr(changedAddr.GetAddress()));
					DrawAddressLabel(state, viewState, changedAddr);

					// Snapshot value
					ImGui::TableSetColumnIndex(1);
					const uint8_t oldValue = DiffSnapshotMemoryBanks[changedAddr.GetBankId()].pMemory[changedAddr.GetAddress()];
					ImGui::Text("%s", NumStr(oldValue));

					// Current value
					ImGui::TableSetColumnIndex(2);
					ImGui::Text("%s", NumStr(state.ReadByte(changedAddr)));

					// Code address that last wrote to value
					ImGui::TableSetColumnIndex(3);
					ImGui::Text("");
					DrawAddressLabel(state, viewState, pDataInfo->LastWriter);

					ImGui::PopID();
				}
			}
			ImGui::EndTable();
		}
	}
	//ImGui::EndChild();
}

void FMemoryAnalyser::DrawStringSearchUI()
{
	FCodeAnalysisState& state = pEmulator->GetCodeAnalysis();

	if (ImGui::Button("Search"))
	{
		FoundStrings = state.FindAllStrings(bSearchStringsInROM, bSearchStringsPhysicalMemOnly);
	}
	ImGui::SameLine();
	ImGui::Checkbox("Search ROM", &bSearchStringsInROM);
	ImGui::SameLine();
	ImGui::Checkbox("Physical Memory", &bSearchStringsPhysicalMemOnly);

	// list strings
	static ImGuiTableFlags tableFLags = ImGuiTableFlags_SizingFixedFit 
				| ImGuiTableFlags_RowBg | ImGuiTableFlags_Borders | ImGuiTableFlags_Resizable 
				| ImGuiTableFlags_Reorderable | ImGuiTableFlags_Hideable | ImGuiTableFlags_ScrollY;
	const float TEXT_BASE_HEIGHT = ImGui::GetTextLineHeightWithSpacing();
	ImVec2 outer_size = ImVec2(0.0f, TEXT_BASE_HEIGHT * 9);
	if (ImGui::BeginTable("foundstring", 3, tableFLags))
	{
		ImGui::TableSetupScrollFreeze(0, 1); // Make top row always visible
		ImGui::TableSetupColumn("String");
		ImGui::TableSetupColumn("Location");
		ImGui::TableSetupColumn("Action");
		ImGui::TableHeadersRow();

		ImGuiListClipper clipper;
		clipper.Begin((int)FoundStrings.size());
		while (clipper.Step())
		{
			for (int rowNum = clipper.DisplayStart; rowNum < clipper.DisplayEnd; rowNum++)
			{
				const FFoundString& foundString = FoundStrings[rowNum];
				ImGui::TableNextRow();
				ImGui::PushID(foundString.Address.GetVal());

				ImGui::TableSetColumnIndex(0);
				ImGui::Text("%s", foundString.String.c_str());

				ImGui::TableSetColumnIndex(1);
				ImGui::Text("%s", NumStr(foundString.Address.GetAddress()));
				DrawAddressLabel(state, state.GetFocussedViewState(), foundString.Address);

				ImGui::TableSetColumnIndex(2);
				if (ImGui::Button("Format"))
				{
					// Set string at address
					FAddressRef addr = foundString.Address;
					const FCodeAnalysisBank* pBank = state.GetBank(addr.GetBankId());
					int itemIndex = GetItemIndexForAddress(state, addr);
					if (itemIndex != -1)
					{
						SetItemText(state, pBank->ItemList[itemIndex]);
					}
				}

				ImGui::PopID();
			}
		}
	
		ImGui::EndTable();
	}

}

void FMemoryAnalyser::FixupAddressRefs()
{
	FCodeAnalysisState& state = pEmulator->GetCodeAnalysis();
	FindTool.FixupAddressRefs();

	for (FFoundString& foundStr : FoundStrings)
	{
		FixupAddressRef(state, foundStr.Address);
	}

	FixupAddressRefList(state, DiffChangedLocations);
}