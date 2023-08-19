#include "FindTool.h"

#include "CodeAnalyser.h"
#include <imgui.h>
#include <misc/cpp/imgui_stdlib.h>

#include "UI/CodeAnalyserUI.h"
#include "Util/Misc.h"

FFindTool::FFindTool()
{
	pCurFinder = &ByteFinder;
}

void FFindTool::Init(FCodeAnalysisState* ptrCodeAnalysis)
{
	pCodeAnalysis = ptrCodeAnalysis;
	ByteFinder.Init(ptrCodeAnalysis);
	WordFinder.Init(ptrCodeAnalysis);
	TextFinder.Init(ptrCodeAnalysis);
}

void FFindTool::Reset()
{
	ByteFinder.Reset();
	WordFinder.Reset();
	TextFinder.Reset();
}

void FFindTool::DrawUI()
{
	FCodeAnalysisViewState& viewState = pCodeAnalysis->GetFocussedViewState();
	const ESearchType lastSearchType = SearchType;
	const ESearchDataType lastDataSize = DataSize;

	if (ImGui::RadioButton("Decimal Value", SearchType == ESearchType::SearchValue && bDecimal == true))
	{
		SearchType = ESearchType::SearchValue;
		bDecimal = true;
	}
	ImGui::SameLine();
	if (ImGui::RadioButton("Hexadecimal Value", SearchType == ESearchType::SearchValue && bDecimal == false))
	{
		SearchType = ESearchType::SearchValue;
		bDecimal = false;
	}
	ImGui::SameLine();
	if (ImGui::RadioButton("Text", SearchType == ESearchType::SearchText))
	{
		SearchType = ESearchType::SearchText;
	}

	if (SearchType == ESearchType::SearchValue)
	{
		if (ImGui::RadioButton("Byte", DataSize == ESearchDataType::SearchByte))
		{
			DataSize = ESearchDataType::SearchByte;
		}
		ImGui::SameLine();
		if (ImGui::RadioButton("Word", DataSize == ESearchDataType::SearchWord))
		{
			DataSize = ESearchDataType::SearchWord;
		}
	}

	if (lastSearchType != SearchType || lastDataSize != DataSize)
	{
		if (SearchType == ESearchType::SearchText)
			pCurFinder = &TextFinder;
		else
			pCurFinder = DataSize == ESearchDataType::SearchByte ? (FFinder*)&ByteFinder : &WordFinder;
	}

	ImGui::SetNextItemOpen(true, ImGuiCond_Once);
	if (ImGui::TreeNode("Search Options"))
	{
		if (SearchType == ESearchType::SearchValue)
		{
			if (ImGui::RadioButton("Data", Options.MemoryType == ESearchMemoryType::SearchData))
			{
				Options.MemoryType = ESearchMemoryType::SearchData;
			}
			ImGui::SameLine();

			if (ImGui::IsItemHovered(ImGuiHoveredFlags_DelayNormal))
			{
				ImGui::SetTooltip("Search only memory locations marked as data.");
			}

			if (ImGui::RadioButton("Code", Options.MemoryType == ESearchMemoryType::SearchCode))
			{
				Options.MemoryType = ESearchMemoryType::SearchCode;
			}
			ImGui::SameLine();

			if (ImGui::IsItemHovered(ImGuiHoveredFlags_DelayNormal))
			{
				ImGui::SetTooltip("Search only memory locations marked as code.");
			}

			if (ImGui::RadioButton("Code & Data", Options.MemoryType == ESearchMemoryType::SearchCodeAndData))
			{
				Options.MemoryType = ESearchMemoryType::SearchCodeAndData;
			}

			if (ImGui::IsItemHovered(ImGuiHoveredFlags_DelayNormal))
			{
				ImGui::SetTooltip("Search all memory locations - code and data.");
			}

			ImGui::Checkbox("Search Graphics Memory", &Options.bSearchGraphicsMem);
		}

		if (pCodeAnalysis->Config.bShowBanks)
		{
			ImGui::Checkbox("Search Address Space Only", &Options.bSearchPhysicalOnly);
		}
		ImGui::Checkbox("Search ROM", &Options.bSearchROM);

		ImGui::Checkbox("Search Unaccessed Memory", &Options.bSearchUnaccessed);
		if (ImGui::IsItemHovered(ImGuiHoveredFlags_DelayNormal))
		{
			ImGui::SetTooltip("Include search results from memory locations that have not been accessed.");
		}

		ImGui::Checkbox("Search Memory With No References", &Options.bSearchUnreferenced);
		if (ImGui::IsItemHovered(ImGuiHoveredFlags_DelayNormal))
		{
			ImGui::SetTooltip("Include search results from memory locations that have no code references.");
		}
		ImGui::TreePop();
	}

	bool bPressedEnter = false;
	if (SearchType == ESearchType::SearchValue)
	{
		const char* formatStr = bDecimal ? "%u" : "%x";
		ImGuiInputTextFlags flags = bDecimal ? ImGuiInputTextFlags_CharsDecimal : ImGuiInputTextFlags_CharsHexadecimal;

		ImGui::Text("Search Value");
		ImGui::SameLine();

		ImGui::SetNextItemWidth(ImGui::GetFontSize() * 8);
		if (DataSize == ESearchDataType::SearchByte)
		{
			ImGui::InputScalar("##bytevalue", ImGuiDataType_U8, &ByteFinder.SearchValue, NULL, NULL, formatStr, flags);
		}
		else
		{
			ImGui::InputScalar("##wordvalue", ImGuiDataType_U16, &WordFinder.SearchValue, NULL, NULL, formatStr, flags);
		}
	}
	else if (SearchType == ESearchType::SearchText)
	{
		ImGui::Text("Search Text");
		ImGui::SameLine();
		ImGui::SetNextItemWidth(ImGui::GetFontSize() * 16);
		ImGui::InputText("##searchtext", &TextFinder.SearchText);
	}

	// sam. I wanted to use ImGuiInputTextFlags_EnterReturnsTrue here to do the search when enter is pressed but
	// I ran into a bug where when clicking away from the input box would revert the value. 
	// https://github.com/ocornut/imgui/issues/6284
	// Doing this as a workaround:
	if (ImGui::IsItemFocused())
	{
		if (ImGui::IsKeyPressed(ImGuiKey_Enter, false) || ImGui::IsKeyPressed(ImGuiKey_KeypadEnter, false))
		{
			bPressedEnter = true;
		}
	}

	ImGui::SameLine();
	if (ImGui::Button("Find") || bPressedEnter)
	{
		pCurFinder->Find(Options);
	}

	ImGuiWindowFlags window_flags = ImGuiWindowFlags_HorizontalScrollbar;
	const ImVec2 childSize = ImVec2(0, -ImGui::GetFrameHeightWithSpacing() * (SearchType == ESearchType::SearchValue ? 2.0f : 1.0f)); // Leave room for 1 or 2 lines below us
	if (ImGui::BeginChild("SearchResults", childSize, true, window_flags))
	{
		if (pCurFinder->GetNumResults())
		{
			static const float TEXT_BASE_WIDTH = ImGui::CalcTextSize("A").x;

			static ImGuiTableFlags flags = ImGuiTableFlags_Borders | ImGuiTableFlags_RowBg | ImGuiTableFlags_SizingFixedFit | ImGuiTableFlags_Resizable | ImGuiTableFlags_ScrollY;
			int numColumns = SearchType == ESearchType::SearchText ? 2 : 3;
			if (pCodeAnalysis->Config.bShowBanks)
				numColumns++;

			if (ImGui::BeginTable("SearchResultsTable", numColumns, flags))
			{
				if (pCodeAnalysis->Config.bShowBanks)
					ImGui::TableSetupColumn("Bank", ImGuiTableColumnFlags_WidthFixed, TEXT_BASE_WIDTH * 8);
				ImGui::TableSetupColumn("Address", ImGuiTableColumnFlags_WidthFixed, TEXT_BASE_WIDTH * 40);
				if (SearchType == ESearchType::SearchValue)
					ImGui::TableSetupColumn("Value", ImGuiTableColumnFlags_WidthFixed, TEXT_BASE_WIDTH * 6);
				ImGui::TableSetupColumn("Comment", ImGuiTableColumnFlags_WidthStretch);
				ImGui::TableHeadersRow();

				//const float lineHeight = ImGui::GetTextLineHeight(); // this breaks the clipper and makes it impossible to see the last few rows.
				ImGuiListClipper clipper((int)pCurFinder->GetNumResults()/*, lineHeight*/);

				bool bUserPrefersHexAitch = GetNumberDisplayMode() == ENumberDisplayMode::HexAitch;
				const ENumberDisplayMode numberMode = bDecimal ? ENumberDisplayMode::Decimal : bUserPrefersHexAitch ? ENumberDisplayMode::HexAitch : ENumberDisplayMode::HexDollar;
				while (clipper.Step())
				{
					for (int i = clipper.DisplayStart; i < clipper.DisplayEnd; i++)
					{
						const FAddressRef resultAddr = pCurFinder->GetResult(i);
						ImGui::PushID(i);
						ImGui::TableNextRow();

						ImGui::TableNextColumn();

						bool bValueChanged = pCurFinder->HasValueChanged(resultAddr);
						if (bValueChanged)
						{
							ImGui::PushStyleColor(ImGuiCol_Text, IM_COL32(255, 255, 0, 255));
						}

						if (pCodeAnalysis->Config.bShowBanks)
						{
							// Bank
							std::string bankName = "Unknown";
							if (FCodeAnalysisBank* pBank = pCodeAnalysis->GetBank(resultAddr.BankId))
							{
								bankName = pBank->Name;
							}
							ImGui::Text("%s", bankName.c_str());
							ImGui::TableNextColumn();
						}

						// Address
						if (const FDataInfo* pWriteDataInfo = pCodeAnalysis->GetDataInfoForAddress(resultAddr))
						{
							ShowDataItemActivity(*pCodeAnalysis, resultAddr);
						}

						ImGui::Text("    %s", NumStr(resultAddr.Address));
						ImGui::SameLine();
						DrawAddressLabel(*pCodeAnalysis, viewState, resultAddr);

						// Value
						if (SearchType == ESearchType::SearchValue)
						{
							ImGui::TableNextColumn();
							if (SearchType == ESearchType::SearchValue)
							{
								ImGui::Text("%s", pCurFinder->GetValueString(resultAddr, numberMode));
							}
						}

						// Comment
						ImGui::TableNextColumn();
						if (const FDataInfo* pWriteDataInfo = pCodeAnalysis->GetDataInfoForAddress(resultAddr))
						{
							// what if this is code?
							DrawComment(pWriteDataInfo);
						}

						if (bValueChanged)
							ImGui::PopStyleColor();

						ImGui::PopID();
					}
				}
				ImGui::EndTable();
			}
		}
	}
	ImGui::EndChild();
	ImGui::Text("%d results found", pCurFinder->GetNumResults());

	if (SearchType == ESearchType::SearchValue)
	{
		if (ImGui::Button("Remove Unchanged Results"))
		{
			pCurFinder->RemoveUnchangedResults();
		}

		if (ImGui::IsItemHovered(ImGuiHoveredFlags_DelayNormal))
		{
			ImGui::SetTooltip("Remove unchanged results todo.");
		}
	}
}

void FFinder::Init(FCodeAnalysisState* ptrCodeAnalysis)
{
	pCodeAnalysis = ptrCodeAnalysis;
	Reset();
}

void FFinder::Reset()
{
	SearchResults.clear();
}

bool FFinder::HasValueChanged(FAddressRef addr) const
{
	return false;
}

void FFinder::Find(const FSearchOptions& opt)
{
	SearchResults.clear();

	std::vector<FAddressRef> allMatches = FindAllMatchesInBanks(opt);

	for (const FAddressRef& addr : allMatches)
	{
		ProcessMatch(addr, opt);
	}
}

void FFinder::ProcessMatch(FAddressRef addr, const FSearchOptions& opt)
{
	bool bAddResult = true;
	const FDataInfo* pDataInfo = pCodeAnalysis->GetDataInfoForAddress(addr);
	const FCodeInfo* pCodeInfo = pCodeAnalysis->GetCodeInfoForAddress(addr);
	const bool bIsCode = pCodeInfo || (pDataInfo && pDataInfo->DataType == EDataType::InstructionOperand);

	if (opt.MemoryType == ESearchMemoryType::SearchCode)
	{
		bAddResult = bIsCode;
	}
	else if (opt.MemoryType == ESearchMemoryType::SearchData)
	{
		bAddResult = !bIsCode;
	}

	if (bAddResult)
	{
		if (pDataInfo)	// MC: always true
		{
			if (!opt.bSearchUnaccessed)
			{
				if (pDataInfo->LastFrameRead == -1 && pDataInfo->LastFrameWritten == -1)
					bAddResult = false;
			}

			if (!opt.bSearchUnreferenced)
			{
				if (pDataInfo->Reads.IsEmpty() && pDataInfo->Writes.IsEmpty())
				{
					bAddResult = false;
				}
			}
		}
	}

	if (bAddResult)
	{
		if (!opt.bSearchGraphicsMem)
		{
			if (pCodeAnalysis->MemoryAnalyser.IsAddressInScreenMemory(addr.Address))
				bAddResult = false;
		}
	}

	if (bAddResult)
	{
		SearchResults.push_back(addr);
	}
}

void FFinder::RemoveUnchangedResults()
{
	for (std::vector<FAddressRef>::iterator it = SearchResults.begin(); it != SearchResults.end(); )
	{
		if (!HasValueChanged(*it))
			it = SearchResults.erase(it);
		else
			++it;
	}
}

std::vector<FAddressRef> FByteFinder::FindAllMatchesInBanks(const FSearchOptions& opt)
{
	return pCodeAnalysis->FindAllMemoryPatterns(&SearchValue, 1, opt.bSearchROM, opt.bSearchPhysicalOnly);
}

bool FByteFinder::HasValueChanged(FAddressRef addr) const
{
	const uint16_t curValue = pCodeAnalysis->ReadByte(addr);
	return curValue != LastValue;
}

const char* FByteFinder::GetValueString(FAddressRef addr, ENumberDisplayMode numberMode) const
{
	const uint8_t value = pCodeAnalysis->ReadByte(addr);
	return NumStr(value, numberMode);
}

void FWordFinder::Find(const FSearchOptions& opt)
{
	LastValue = SearchValue;
	SearchBytes[1] = static_cast<uint8_t>(SearchValue >> 8);
	SearchBytes[0] = static_cast<uint8_t>(SearchValue);
	FFinder::Find(opt);
}

std::vector<FAddressRef> FWordFinder::FindAllMatchesInBanks(const FSearchOptions& opt)
{
	return pCodeAnalysis->FindAllMemoryPatterns(SearchBytes, 2, opt.bSearchROM, opt.bSearchPhysicalOnly);
}

bool FWordFinder::HasValueChanged(FAddressRef addr) const
{
	const uint16_t curValue = pCodeAnalysis->ReadWord(addr);
	return false;
}

const char* FWordFinder::GetValueString(FAddressRef addr, ENumberDisplayMode numberMode) const
{
	const uint16_t value = pCodeAnalysis->ReadWord(addr);
	return NumStr(value, numberMode);
}

std::vector<FAddressRef> FTextFinder::FindAllMatchesInBanks(const FSearchOptions& opt)
{
	return pCodeAnalysis->FindAllMemoryPatterns((uint8_t*)SearchText.c_str(), SearchText.size(), opt.bSearchROM, opt.bSearchPhysicalOnly);
}
