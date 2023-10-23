#include "CodeAnalyserUI.h"
#include "CharacterMapViewer.h"
#include "../CodeAnalyser.h"
#include "Misc/GlobalConfig.h"	


#include "Util/Misc.h"
#include "ImageViewer.h"

#include "imgui.h"
#include "imgui_internal.h"
#include "misc/cpp/imgui_stdlib.h"
#include <algorithm>
#include <sstream>
#include <cctype>
#include "chips/z80.h"
#include "CodeToolTips.h"

// UI
void DrawCodeAnalysisItem(FCodeAnalysisState& state, FCodeAnalysisViewState& viewState, const FCodeAnalysisItem& item);
void DrawFormatTab(FCodeAnalysisState& state, FCodeAnalysisViewState& viewState);
void DrawCaptureTab(FCodeAnalysisState& state, FCodeAnalysisViewState& viewState);

void DrawCodeInfo(FCodeAnalysisState& state, FCodeAnalysisViewState& viewState, const FCodeAnalysisItem& item);
void DrawCodeDetails(FCodeAnalysisState& state, FCodeAnalysisViewState& viewState, const FCodeAnalysisItem& item);

void FCodeAnalysisViewState::GoToAddress(FAddressRef newAddress, bool bLabel)
{
	if(GetCursorItem().IsValid())
		AddressStack.push_back(GetCursorItem().AddressRef);
	GoToAddressRef = newAddress;
	GoToLabel = bLabel;
}

bool FCodeAnalysisViewState::GoToPreviousAddress()
{
	if (AddressStack.empty())
		return false;

	GoToAddressRef = AddressStack.back();
	GoToLabel = false;
	AddressStack.pop_back();
	return true;
}

int GetItemIndexForAddress(const FCodeAnalysisState &state, FAddressRef addr)
{
	const FCodeAnalysisBank* pBank = state.GetBank(addr.BankId);

	int index = -1;

	assert(pBank != nullptr);
	
	for (int i = 0; i < (int)pBank->ItemList.size(); i++)
	{
		if (pBank->ItemList[i].IsValid() && pBank->ItemList[i].AddressRef.Address > addr.Address)
			return index;
		index = i;
	}
	return -1;
}



std::vector<FMemoryRegionDescGenerator*>	g_RegionDescHandlers;

const char* GetRegionDesc(uint16_t addr)
{
	for (FMemoryRegionDescGenerator* pDescGen : g_RegionDescHandlers)
	{
		if (pDescGen)
		{
			if (pDescGen->InRegion(addr))
				return pDescGen->GenerateAddressString(addr);
		}
	}
	return nullptr;
}

bool AddMemoryRegionDescGenerator(FMemoryRegionDescGenerator* pGen)
{
	g_RegionDescHandlers.push_back(pGen);
	return true;
}

void DrawSnippetToolTip(FCodeAnalysisState& state, FCodeAnalysisViewState& viewState, const FAddressRef addr)
{
	// Bring up snippet in tool tip
	const FCodeAnalysisBank* pBank = state.GetBank(addr.BankId);
	if (pBank != nullptr)
	{
		const int index = GetItemIndexForAddress(state, addr);
		if (index != -1)
		{
			const int kToolTipNoLines = 10;
			ImGui::BeginTooltip();
			const int startIndex = std::max(index - (kToolTipNoLines / 2), 0);
			for (int line = 0; line < kToolTipNoLines; line++)
			{
				if (startIndex + line < (int)pBank->ItemList.size())
					DrawCodeAnalysisItem(state, viewState, pBank->ItemList[startIndex + line]);
			}
			ImGui::EndTooltip();
		}
	}
}

// TODO: phase this out
void DrawAddressLabel(FCodeAnalysisState& state, FCodeAnalysisViewState& viewState, uint16_t addr, bool bFunctionRel)
{
	DrawAddressLabel(state, viewState, { state.GetBankFromAddress(addr),addr }, bFunctionRel);
}

void DrawAddressLabel(FCodeAnalysisState &state, FCodeAnalysisViewState& viewState, FAddressRef addr, bool bFunctionRel)
{
	int labelOffset = 0;
	const char *pLabelString = GetRegionDesc(addr.Address);
	FCodeAnalysisBank* pBank = state.GetBank(addr.BankId);
	assert(pBank != nullptr);

	if (pLabelString == nullptr)	// get a label
	{
		// find a label for this address
		for (int addrVal = addr.Address; addrVal >= 0; addrVal--)
		{
			if (pBank->AddressValid(addrVal) == false)
			{
				pBank = state.GetBank(state.GetBankFromAddress(addrVal));
				assert(pBank != nullptr);
			}

			const FLabelInfo* pLabel = state.GetLabelForAddress(FAddressRef(pBank->Id,addrVal));
			if (pLabel != nullptr)
			{
				if (bFunctionRel == false || pLabel->LabelType == ELabelType::Function)
				{
					pLabelString = pLabel->Name.c_str();
					break;
				}
			}

			labelOffset++;

			if (pLabelString == nullptr && addrVal == 0)
				pLabelString = "0000";
		}
	}
	
	if (pLabelString != nullptr)
	{
		ImGui::SameLine();
		ImGui::PushStyleColor(ImGuiCol_Text, 0xff808080);
		const FCodeAnalysisBank* pBank = state.GetBank(addr.BankId);

		if (state.Config.bShowBanks)
		{
			ImGui::Text("[%s]", pBank->Name.c_str());
			ImGui::SameLine(0,0);
		}

		if(labelOffset == 0)
			ImGui::Text("[%s]", pLabelString);
		else
			ImGui::Text("[%s + %d]", pLabelString, labelOffset);

		if (ImGui::IsItemHovered())
		{
			// Bring up snippet in tool tip
			const int indentBkp = viewState.JumpLineIndent;
			viewState.JumpLineIndent = 0;
			DrawSnippetToolTip(state, viewState, addr);
			viewState.JumpLineIndent = indentBkp;

			ImGuiIO& io = ImGui::GetIO();
			if (io.KeyShift && ImGui::IsMouseDoubleClicked(0))
				state.GetAltViewState().GoToAddress( addr, false);
			else if (ImGui::IsMouseDoubleClicked(0))
				viewState.GoToAddress( addr, false);
		
			viewState.HoverAddress = addr;
		}

		ImGui::PopStyleColor();
	}
}

void DrawCodeAddress(FCodeAnalysisState &state, FCodeAnalysisViewState& viewState, FAddressRef addr, bool bFunctionRel)
{
	//ImGui::PushStyleColor(ImGuiCol_Text, 0xff00ffff);
	ImGui::Text("%s", NumStr(addr.Address));
	//ImGui::PopStyleColor();
	ImGui::SameLine();
	DrawAddressLabel(state, viewState, addr, bFunctionRel);
}

// TODO: Phase this out
void DrawCodeAddress(FCodeAnalysisState& state, FCodeAnalysisViewState& viewState, uint16_t addr, bool bFunctionRel)
{
	DrawCodeAddress(state, viewState, {state.GetBankFromAddress(addr), addr}, bFunctionRel);
}

void DrawComment(const FItem *pItem, float offset)
{
	if(pItem != nullptr && pItem->Comment.empty() == false)
	{
		ImGui::SameLine(offset);
		ImGui::PushStyleColor(ImGuiCol_Text, 0xff008000);
		ImGui::Text("\t; %s", pItem->Comment.c_str());
		ImGui::PopStyleColor();
	}
}

void DrawLabelInfo(FCodeAnalysisState &state, FCodeAnalysisViewState& viewState, const FCodeAnalysisItem& item)
{
	const FLabelInfo* pLabelInfo = static_cast<const FLabelInfo*>(item.Item);
	const FDataInfo* pDataInfo = state.GetDataInfoForAddress(item.AddressRef);	// for self-modifying code
	const FCodeInfo* pCodeInfo = state.GetCodeInfoForAddress(item.AddressRef);
	ImVec4 labelColour = ImVec4(0.5f, 0.5f, 0.5f, 1.0f);
	if (viewState.HighlightAddress == item.AddressRef)
		labelColour = ImVec4(0.0f, 1.0f, 0.0f, 1.0f);
	else if (pLabelInfo->Global || pLabelInfo->LabelType == ELabelType::Function)
		labelColour = ImVec4(1.0f, 1.0f, 1.0f, 1.0f);
	
	// draw SMC fixups differently
	if (pCodeInfo == nullptr && pDataInfo->DataType == EDataType::InstructionOperand)
	{
		ImGui::TextColored(labelColour, "\t\tOperand Fixup(%s) :",NumStr(item.AddressRef.Address));
		ImGui::SameLine();
		ImGui::TextColored(labelColour, "%s", pLabelInfo->Name.c_str());
	}
	else
	{
		ImGui::SameLine(state.Config.LabelPos);
		ImGui::TextColored(labelColour, "%s: ", pLabelInfo->Name.c_str());
	}

	// hover tool tip
	if (ImGui::IsItemHovered() && pLabelInfo->References.IsEmpty() == false)
	{
		ImGui::BeginTooltip();
		ImGui::Text("References:");
		for (const auto & caller : pLabelInfo->References.GetReferences())
		{
			ShowCodeAccessorActivity(state, caller);

			ImGui::Text("   ");
			ImGui::SameLine();
			DrawCodeAddress(state, viewState, caller);
		}
		ImGui::EndTooltip();
	}

	DrawComment(pLabelInfo);	
}

void DrawLabelDetails(FCodeAnalysisState &state, FCodeAnalysisViewState& viewState,const FCodeAnalysisItem& item )
{
	FLabelInfo* pLabelInfo = static_cast<FLabelInfo*>(item.Item);
	std::string LabelText = pLabelInfo->Name;
	if (ImGui::InputText("Name", &LabelText))
	{
		if (LabelText.empty())
			LabelText = pLabelInfo->Name;

		SetLabelName(state, pLabelInfo, LabelText.c_str());
	}

	if(ImGui::Checkbox("Global", &pLabelInfo->Global))
	{
		if (pLabelInfo->LabelType == ELabelType::Code && pLabelInfo->Global == true)
			pLabelInfo->LabelType = ELabelType::Function;
		if (pLabelInfo->LabelType == ELabelType::Function && pLabelInfo->Global == false)
			pLabelInfo->LabelType = ELabelType::Code;
		GenerateGlobalInfo(state);
	}

	ImGui::Text("References:");
	for (const auto & caller : pLabelInfo->References.GetReferences())
	{
		ShowCodeAccessorActivity(state, caller);

		ImGui::Text("   ");
		ImGui::SameLine();
		DrawCodeAddress(state, viewState, caller);
	}
}


void DrawCommentLine(FCodeAnalysisState& state, const FCommentLine* pCommentLine)
{
	ImGui::SameLine();
	ImGui::PushStyleColor(ImGuiCol_Text, 0xff008000);
	ImGui::SameLine(state.Config.CommentLinePos);
	ImGui::Text("; %s", pCommentLine->Comment.c_str());
	ImGui::PopStyleColor();
}

void DrawCommentBlockDetails(FCodeAnalysisState& state, const FCodeAnalysisItem& item)
{
	//const uint16_t physAddress = item.AddressRef.Address;
	FCommentBlock* pCommentBlock = state.GetCommentBlockForAddress(item.AddressRef);
	if (pCommentBlock == nullptr)
		return;

	if (ImGui::InputTextMultiline("Comment Text", &pCommentBlock->Comment))
	{
		if (pCommentBlock->Comment.empty() == true)
			state.SetCommentBlockForAddress(item.AddressRef, nullptr);
		state.SetCodeAnalysisDirty(item.AddressRef);
	}

}

int CommentInputCallback(ImGuiInputTextCallbackData *pData)
{
	return 1;
}

void AddLabelAtAddressUI(FCodeAnalysisState& state,FAddressRef address)
{
	FLabelInfo* pLabel = AddLabelAtAddress(state, address);
	if (pLabel != nullptr)
	{
		state.GetFocussedViewState().SetCursorItem(FCodeAnalysisItem(pLabel,address));
		ImGui::OpenPopup("Enter Label Text");
		ImGui::SetWindowFocus("Enter Label Text");
	}
}

void ProcessKeyCommands(FCodeAnalysisState& state, FCodeAnalysisViewState& viewState)
{
	ImGuiIO& io = ImGui::GetIO();
	if (io.WantTextInput)
		return;

	const FCodeAnalysisItem& cursorItem = viewState.GetCursorItem();

	if (ImGui::IsWindowFocused() && cursorItem.IsValid())
	{
		if (ImGui::IsKeyPressed(state.KeyConfig[(int)EKey::SetItemCode]))
		{
			SetItemCode(state, cursorItem.AddressRef);
		}
		else if (ImGui::IsKeyPressed(state.KeyConfig[(int)EKey::SetItemData]))
		{
			SetItemData(state, cursorItem);
		}
		else if (ImGui::IsKeyPressed(state.KeyConfig[(int)EKey::SetItemText]))
		{
			SetItemText(state, cursorItem);
		}
#ifdef ENABLE_IMAGE_TYPE
		else if (ImGui::IsKeyPressed(state.KeyConfig[(int)EKey::SetItemImage]))
		{
			SetItemImage(state, cursorItem);
		}
#endif
		else if (ImGui::IsKeyPressed(state.KeyConfig[(int)EKey::ToggleItemBinary]))
		{
			if (cursorItem.Item->Type == EItemType::Data)
			{
				FDataInfo* pDataItem = static_cast<FDataInfo*>(cursorItem.Item);
				if (pDataItem->DataType != EDataType::Bitmap)
					pDataItem->DataType = EDataType::Bitmap;
				else
					pDataItem->DataType = EDataType::Byte;
				//pDataItem->bShowBinary = !pDataItem->bShowBinary;
			}
		}
		else if (ImGui::IsKeyPressed(state.KeyConfig[(int)EKey::AddLabel]))
		{
			if (cursorItem.Item->Type != EItemType::Label)
			{
				AddLabelAtAddressUI(state, cursorItem.AddressRef);
			}
			else
			{
				ImGui::OpenPopup("Enter Label Text");
				ImGui::SetWindowFocus("Enter Label Text");
			}
		}
		else if (io.KeyShift && ImGui::IsKeyPressed(state.KeyConfig[(int)EKey::Comment]))
		{
			FCommentBlock* pCommentBlock = AddCommentBlock(state, cursorItem.AddressRef);
			viewState.SetCursorItem(FCodeAnalysisItem(pCommentBlock, cursorItem.AddressRef));
			ImGui::OpenPopup("Enter Comment Text Multi");
			ImGui::SetWindowFocus("Enter Comment Text Multi");
		}
		else if (ImGui::IsKeyPressed(state.KeyConfig[(int)EKey::Comment]) || ImGui::IsKeyPressed(state.KeyConfig[(int)EKey::CommentLegacy]))
		{
			ImGui::OpenPopup("Enter Comment Text");
			ImGui::SetWindowFocus("Enter Comment Text");
		}
		else if (cursorItem.Item->Type == EItemType::Label && ImGui::IsKeyPressed(state.KeyConfig[(int)EKey::Rename]))
		{
			//AddLabelAtAddress(state, state.pCursorItem->Address);
			ImGui::OpenPopup("Enter Label Text");
			ImGui::SetWindowFocus("Enter Label Text");
		}
		
		else if (ImGui::IsKeyPressed(state.KeyConfig[(int)EKey::Breakpoint]))
		{
			if (cursorItem.Item->Type == EItemType::Data)
				state.ToggleDataBreakpointAtAddress(cursorItem.AddressRef, cursorItem.Item->ByteSize);
			else if (cursorItem.Item->Type == EItemType::Code)
				state.ToggleExecBreakpointAtAddress(cursorItem.AddressRef);
		}
	}

	if (ImGui::IsKeyPressed(state.KeyConfig[(int)EKey::BreakContinue]))
	{
		if (state.Debugger.IsStopped())
		{
			state.Debugger.Continue();
			//viewState.TrackPCFrame = true;
		}
		else
		{
			state.Debugger.Break();
		}
	}
	else if (ImGui::IsKeyPressed(state.KeyConfig[(int)EKey::StepOver]))
	{
		state.Debugger.StepOver();
	}
	else if (ImGui::IsKeyPressed(state.KeyConfig[(int)EKey::StepInto]))
	{
		state.Debugger.StepInto();
	}
	else if (ImGui::IsKeyPressed(state.KeyConfig[(int)EKey::StepFrame]))
	{
		state.Debugger.StepFrame();
	}
	else if (ImGui::IsKeyPressed(state.KeyConfig[(int)EKey::StepScreenWrite]))
	{
		state.Debugger.StepScreenWrite();
	}
}

void UpdatePopups(FCodeAnalysisState& state, FCodeAnalysisViewState& viewState)
{
	const FCodeAnalysisItem& cursorItem = viewState.GetCursorItem();

	if (cursorItem.IsValid() == false)
		return;

	if (ImGui::BeginPopup("Enter Comment Text", ImGuiWindowFlags_AlwaysAutoResize))
	{
		ImGui::SetKeyboardFocusHere();
		if (ImGui::InputText("##comment", &cursorItem.Item->Comment, ImGuiInputTextFlags_EnterReturnsTrue))
		{
			ImGui::CloseCurrentPopup();
		}
		ImGui::SetItemDefaultFocus();
		ImGui::EndPopup();
	}

	if (ImGui::BeginPopup("Enter Comment Text Multi", ImGuiWindowFlags_AlwaysAutoResize))
	{
		ImGui::SetKeyboardFocusHere();
		if(ImGui::InputTextMultiline("##comment", &cursorItem.Item->Comment,ImVec2(), ImGuiInputTextFlags_EnterReturnsTrue | ImGuiInputTextFlags_CtrlEnterForNewLine))
		{
			state.SetCodeAnalysisDirty(cursorItem.AddressRef);
			ImGui::CloseCurrentPopup();
		}
		ImGui::SetItemDefaultFocus();
		ImGui::EndPopup();
	}

	if (ImGui::BeginPopup("Enter Label Text", ImGuiWindowFlags_AlwaysAutoResize))
	{
		FLabelInfo *pLabel = (FLabelInfo *)cursorItem.Item;
		
		ImGui::SetKeyboardFocusHere();
		std::string LabelText = pLabel->Name;
		if (ImGui::InputText("##comment", &LabelText, ImGuiInputTextFlags_EnterReturnsTrue))
		{
			if (LabelText.empty() == false)
				pLabel->Name = LabelText;
			ImGui::CloseCurrentPopup();
		}
		ImGui::SetItemDefaultFocus();
		ImGui::EndPopup();
	}
}

struct FItemListBuilder
{
	FItemListBuilder(std::vector<FCodeAnalysisItem>& itemList) :ItemList(itemList) {}

	std::vector<FCodeAnalysisItem>&	ItemList;
	int16_t				BankId = -1;
	int					CurrAddr = 0;
	FCommentBlock*		ViewStateCommentBlocks[FCodeAnalysisState::kNoViewStates] = { nullptr };

};

void ExpandCommentBlock(FCodeAnalysisState& state, FItemListBuilder& builder, FCommentBlock* pCommentBlock)
{
	// split comment into lines
	std::stringstream stringStream(pCommentBlock->Comment);
	std::string line;
	FCommentLine* pFirstLine = nullptr;

	while (std::getline(stringStream, line, '\n'))
	{
		if (line.empty() || line[0] == '@')	// skip lines starting with @ - we might want to create items from them in future
			continue;

		FCommentLine* pLine = FCommentLine::Allocate();
		pLine->Comment = line;
		//pLine->Address = addr;
		builder.ItemList.emplace_back(pLine, builder.BankId, builder.CurrAddr);
		if (pFirstLine == nullptr)
			pFirstLine = pLine;
	}

	// fix up having comment blocks as cursor items
	for (int i = 0; i < FCodeAnalysisState::kNoViewStates; i++)
	{
		if (builder.ViewStateCommentBlocks[i] == pCommentBlock)
			state.ViewState[i].SetCursorItem(FCodeAnalysisItem(pFirstLine, builder.BankId, builder.CurrAddr));
	}
}

void UpdateItemListForBank(FCodeAnalysisState& state, FCodeAnalysisBank& bank)
{
	bank.ItemList.clear();
	FItemListBuilder listBuilder(bank.ItemList);
	listBuilder.BankId = bank.Id;

	const int16_t page = bank.PrimaryMappedPage;
	const uint16_t bankPhysAddr = page * FCodeAnalysisPage::kPageSize;
	int nextItemAddress = 0;

	for (int bankAddr = 0; bankAddr < bank.NoPages * FCodeAnalysisPage::kPageSize; bankAddr++)
	{
		FCodeAnalysisPage& page = bank.Pages[bankAddr >> FCodeAnalysisPage::kPageShift];
		const uint16_t pageAddr = bankAddr & FCodeAnalysisPage::kPageMask;
		listBuilder.CurrAddr = bankPhysAddr + bankAddr;

		FCommentBlock* pCommentBlock = page.CommentBlocks[pageAddr];
		if (pCommentBlock != nullptr)
			ExpandCommentBlock(state, listBuilder, pCommentBlock);

		FLabelInfo* pLabelInfo = page.Labels[pageAddr];
		if (pLabelInfo != nullptr)
			listBuilder.ItemList.emplace_back(pLabelInfo, listBuilder.BankId, listBuilder.CurrAddr);

		// check if we have gone past this item
		if (bankAddr >= nextItemAddress)
		{
			//FCodeInfo* pCodeInfo = state.GetCodeInfoForAddress(listBuilder.CurrAddr);
			FCodeInfo* pCodeInfo = page.CodeInfo[pageAddr];
			if (pCodeInfo != nullptr && pCodeInfo->bDisabled == false)
			{
				nextItemAddress = bankAddr + pCodeInfo->ByteSize;
				listBuilder.ItemList.emplace_back(pCodeInfo, listBuilder.BankId, listBuilder.CurrAddr);
			}
			else // code and data are mutually exclusive
			{
				//FDataInfo* pDataInfo = state.GetReadDataInfoForAddress(listBuilder.CurrAddr);
				FDataInfo* pDataInfo = &page.DataInfo[pageAddr]; 
				if (pDataInfo != nullptr)
				{
					if (pDataInfo->DataType != EDataType::Blob && pDataInfo->DataType != EDataType::ScreenPixels)	// not sure why we want this
						nextItemAddress = bankAddr + pDataInfo->ByteSize;
					else
						nextItemAddress = bankAddr + 1;

					listBuilder.ItemList.emplace_back(pDataInfo, listBuilder.BankId, listBuilder.CurrAddr);
				}
			}
		}
	}
}

void UpdateItemList(FCodeAnalysisState &state)
{
	// build item list - not every frame please!
	if (state.IsCodeAnalysisDataDirty() )
	{
		const float line_height = ImGui::GetTextLineHeight();
		
		state.ItemList.clear();
		FCommentLine::FreeAll();	// recycle comment lines

		//int nextItemAddress = 0;

		auto& banks = state.GetBanks();
		for (auto& bank : banks)
		{
			if (bank.bIsDirty || bank.ItemList.empty())
			{
				UpdateItemListForBank(state, bank);
				bank.bIsDirty = false;
			}
		}
		int pageNo = 0;

		while (pageNo < FCodeAnalysisState::kNoPagesInAddressSpace)
		{
			int16_t bankId = state.GetBankFromAddress(pageNo * FCodeAnalysisPage::kPageSize);
			FCodeAnalysisBank* pBank = state.GetBank(bankId);
			if (pBank != nullptr)
			{
				state.ItemList.insert(state.ItemList.end(), pBank->ItemList.begin(), pBank->ItemList.end());
				/*const uint16_t bankAddrStart = pageNo * FCodeAnalysisPage::kPageSize;
				for (const auto& bankItem : pBank->ItemList)
				{
					FCodeAnalysisItem& item = state.ItemList.emplace_back(bankItem);
					item.AddressRef.Address += bankAddrStart;
				}*/
				pageNo += pBank->NoPages;
			}
			else
			{
				pageNo++;
			}
		}

		// Maybe this needs to follow the same algorithm as the main view?
		//ImGui::SetScrollY(state.GetFocussedViewState().CursorItemIndex * line_height);
		state.ClearDirtyStatus();

		if (state.HasMemoryBeenRemapped())
		{
			GenerateGlobalInfo(state);
			state.ClearRemappings();
		}
	}

}

void DoItemContextMenu(FCodeAnalysisState& state, const FCodeAnalysisItem &item)
{
	if (item.IsValid() == false)
		return;

	if (ImGui::BeginPopupContextItem("code item context menu"))
	{		
		if (ImGui::Selectable("Copy Address"))
		{
			state.CopiedAddress = item.AddressRef;
		}

		if (item.Item->Type == EItemType::Data)
		{
			if (ImGui::Selectable("Toggle data type (D)"))
			{
				SetItemData(state, item);
			}
			if (ImGui::Selectable("Set as text (T)"))
			{
				SetItemText(state, item);
			}
			if (ImGui::Selectable("Set as Code (C)"))
			{
				SetItemCode(state, item.AddressRef);
			}
#ifdef ENABLE_IMAGE_TYPE
			if (ImGui::Selectable("Set as Image (I)"))
			{
				SetItemImage(state, pItem);
			}
#endif
			if (ImGui::Selectable("Toggle Data Breakpoint"))
				state.ToggleDataBreakpointAtAddress(item.AddressRef, item.Item->ByteSize);
			if (ImGui::Selectable("Add Watch"))
				state.Debugger.AddWatch(item.AddressRef);

		}

		if (item.Item->Type == EItemType::Label)
		{
			if (ImGui::Selectable("Remove label"))
			{
				RemoveLabelAtAddress(state, item.AddressRef);
			}
		}
		else
		{
			if (ImGui::Selectable("Add label (L)"))
			{
				AddLabelAtAddressUI(state, item.AddressRef);
			}
		}

		// breakpoints
		if (item.Item->Type == EItemType::Code)
		{
			if (ImGui::Selectable("Toggle Exec Breakpoint"))
				state.ToggleExecBreakpointAtAddress(item.AddressRef);
		}
				
		if (ImGui::Selectable("View in graphics viewer"))
		{
			state.CPUInterface->GraphicsViewerSetView(item.AddressRef);
		}

		ImGui::EndPopup();
	}
}

void DrawCodeAnalysisItem(FCodeAnalysisState& state, FCodeAnalysisViewState& viewState, const FCodeAnalysisItem& item)
{
	//assert(i < (int)state.ItemList.size());
	//const FCodeAnalysisItem& item = state.ItemList[i];
	const uint16_t physAddr = item.AddressRef.Address;

	if (item.IsValid() == false)
		return;

	// TODO: item below might need bank check
	bool bHighlight = (viewState.HighlightAddress.IsValid() && viewState.HighlightAddress.Address >= physAddr && viewState.HighlightAddress.Address < physAddr + item.Item->ByteSize);
	uint32_t kHighlightColour = 0xff00ff00;
	ImGui::PushID(item.Item);

	// selectable
	const uint16_t endAddress = viewState.DataFormattingOptions.CalcEndAddress();
	const bool bSelected = (item.Item == viewState.GetCursorItem().Item) || 
		(viewState.DataFormattingTabOpen && 
			item.AddressRef.BankId == viewState.DataFormattingOptions.StartAddress.BankId && 
			item.AddressRef.Address >= viewState.DataFormattingOptions.StartAddress.Address && 
			item.AddressRef.Address <= endAddress);

	if (ImGui::Selectable("##codeanalysisline", bSelected, ImGuiSelectableFlags_SelectOnNav))
	{
		if (bSelected == false)
		{
			viewState.SetCursorItem(item);
			//viewState.CursorItemIndex = i;

			// Select Data Formatting Range
			if (viewState.DataFormattingTabOpen && item.Item->Type == EItemType::Data)
			{
				ImGuiIO& io = ImGui::GetIO();
				if (io.KeyShift)
				{
					if (viewState.DataFormattingOptions.ItemSize > 0)
						viewState.DataFormattingOptions.NoItems = (viewState.DataFormattingOptions.StartAddress.Address - viewState.GetCursorItem().AddressRef.Address) / viewState.DataFormattingOptions.ItemSize;
				}
				else
				{
					viewState.DataFormattingOptions.StartAddress = viewState.GetCursorItem().AddressRef;
				}
			}
		}
	}
	DoItemContextMenu(state, item);

	// double click to toggle breakpoints
	if (ImGui::IsItemHovered() && viewState.HighlightAddress.IsValid() == false &&  ImGui::IsMouseDoubleClicked(0))
	{
		if (item.Item->Type == EItemType::Code)
			state.ToggleExecBreakpointAtAddress(item.AddressRef);
		else if (item.Item->Type == EItemType::Data)
			state.ToggleDataBreakpointAtAddress(item.AddressRef, item.Item->ByteSize);
	}

	ImGui::SetItemAllowOverlap();	// allow buttons
	ImGui::SameLine();

	switch (item.Item->Type)
	{
	case EItemType::Label:
		DrawLabelInfo(state, viewState,item);
		break;
	case EItemType::Code:
		if (bHighlight)
			ImGui::PushStyleColor(ImGuiCol_Text, kHighlightColour);
		DrawCodeInfo(state, viewState, item);
		if (bHighlight)
			ImGui::PopStyleColor();
		break;
	case EItemType::Data:
		if (bHighlight)
			ImGui::PushStyleColor(ImGuiCol_Text, kHighlightColour);
		DrawDataInfo(state, viewState, item,false,state.bAllowEditing);
		if (bHighlight)
			ImGui::PopStyleColor();
		break;
	case EItemType::CommentLine:
		DrawCommentLine(state, static_cast<const FCommentLine*>(item.Item));
		break;
	}

	ImGui::PopID();
}

bool DrawNumberTypeCombo(const char *pLabel, ENumberDisplayMode& numberMode)
{
	const int index = (int)numberMode + 1;
	const char* numberTypes[] = { "None", "Decimal", "$ Hex", "Hex h" };
	bool bChanged = false;

	if (ImGui::BeginCombo(pLabel, numberTypes[index]))
	{
		for (int n = 0; n < IM_ARRAYSIZE(numberTypes); n++)
		{
			const bool isSelected = (index == n);
			if (ImGui::Selectable(numberTypes[n], isSelected))
			{
				numberMode = (ENumberDisplayMode)(n - 1);
				bChanged = true;
			}
		}
		ImGui::EndCombo();
	}

	return bChanged;
}

bool DrawOperandTypeCombo(const char* pLabel, EOperandType& operandType)
{
	const int index = (int)operandType;
	const char* operandTypes[] = { "Unknown", "Pointer", "JumpAddress", "Decimal", "Hex", "Binary" };
	bool bChanged = false;

	if (ImGui::BeginCombo(pLabel, operandTypes[index]))
	{
		for (int n = 0; n < IM_ARRAYSIZE(operandTypes); n++)
		{
			const bool isSelected = (index == n);
			if (ImGui::Selectable(operandTypes[n], isSelected))
			{
				operandType = (EOperandType)n;
				bChanged = true;
			}
		}
		ImGui::EndCombo();
	}

	return bChanged;
}

bool IsDisplayTypeSupported(EDataItemDisplayType displayType, const FCodeAnalysisState& state)
{
	switch (displayType)
	{
		case EDataItemDisplayType::ColMap2Bpp_CPC:
			return state.Config.bSupportedBitmapTypes[(int)EBitmapFormat::ColMap2Bpp_CPC];
		case EDataItemDisplayType::ColMap4Bpp_CPC:
			return state.Config.bSupportedBitmapTypes[(int)EBitmapFormat::ColMap4Bpp_CPC];
		default:
			return true;
	}
}

bool DrawDataDisplayTypeCombo(const char* pLabel, EDataItemDisplayType& displayType, const FCodeAnalysisState& state)
{
	const int index = (int)displayType;
	const char* operandTypes[] = { "Unknown", "Pointer", "JumpAddress", "Decimal", "Hex", "Binary",
									"Bitmap", "ColMap2Bpp CPC", "ColMap4Bpp CPC" };
	bool bChanged = false;

	if (ImGui::BeginCombo(pLabel, operandTypes[index]))
	{
		for (int n = 0; n < IM_ARRAYSIZE(operandTypes); n++)
		{
			if(IsDisplayTypeSupported((EDataItemDisplayType)n,state))
			{
				const bool isSelected = (index == n);
				if (ImGui::Selectable(operandTypes[n], isSelected))
				{
					displayType = (EDataItemDisplayType)n;
					bChanged = true;
				}
			}
		}
		ImGui::EndCombo();
	}

	return bChanged;
}

bool DrawDataTypeCombo(int& dataType)
{
	const int index = (int)dataType;
	const char* dataTypes[] = { "Byte", "Word", "Bitmap", "Char Map", "Col Attr", "Text" };
	
	bool bChanged = false;

	if (ImGui::BeginCombo("Data Type", dataTypes[index]))
	{
		for (int n = 0; n < IM_ARRAYSIZE(dataTypes); n++)
		{
			const bool isSelected = (index == n);
			if (ImGui::Selectable(dataTypes[n], isSelected))
			{
				dataType = n;
				bChanged = true;
			}
		}
		ImGui::EndCombo();
	}
	return bChanged;
}

bool DrawBitmapFormatCombo(EBitmapFormat& bitmapFormat, const FCodeAnalysisState& state)
{
	assert(bitmapFormat < EBitmapFormat::Max);
	
	int numFormatsSupported = 0;
	for (int n = 0; n < (int)EBitmapFormat::Max; n++)
		numFormatsSupported += state.Config.bSupportedBitmapTypes[n] ? 1 : 0;

	if (numFormatsSupported == 1)
	{
		// don't draw the combo box if there is only 1 format supported
		return false;
	}

	const int index = (int)bitmapFormat;
	const char* bitmapFormats[] = { "1bpp", "2bpp (CPC Mode 1)", "4bpp (CPC Mode 0)", "2bpp (C64 Multicolour)" };

	bool bChanged = false;

	if (ImGui::BeginCombo("Bitmap Format", bitmapFormats[index]))
	{
		for (int n = 0; n < IM_ARRAYSIZE(bitmapFormats); n++)
		{
			if (state.Config.bSupportedBitmapTypes[n])
			{
				const bool isSelected = (index == n);
				if (ImGui::Selectable(bitmapFormats[n], isSelected))
				{
					bitmapFormat = (EBitmapFormat)n;
					bChanged = true;
				}
			}
		}
		ImGui::EndCombo();
	}
	return bChanged;
}

void DrawDetailsPanel(FCodeAnalysisState &state, FCodeAnalysisViewState& viewState)
{
	const FCodeAnalysisItem& item = viewState.GetCursorItem();

	if (item.IsValid())
	{
		switch (item.Item->Type)
		{
		case EItemType::Label:
			DrawLabelDetails(state, viewState, item);
			break;
		case EItemType::Code:
			DrawCodeDetails(state, viewState, item);
			break;
		case EItemType::Data:
			DrawDataDetails(state, viewState, item);
			break;
		case EItemType::CommentLine:
			{
				FCommentBlock* pCommentBlock = state.GetCommentBlockForAddress(item.AddressRef);
				if (pCommentBlock != nullptr)
					DrawCommentBlockDetails(state, item);
			}
			break;
		}

		if(item.Item->Type != EItemType::CommentLine)
		{
			static std::string commentString;
			static FItem *pCurrItem = nullptr;
			if (pCurrItem != item.Item)
			{
				commentString = item.Item->Comment;
				pCurrItem = item.Item;
			}

			//ImGui::Text("Comments");
			if (ImGui::InputTextWithHint("Comments", "Comments", &commentString))
			{
				SetItemCommentText(state, item, commentString.c_str());
			}
		}

	}
}

// Move to Debugger?
void DrawDebuggerButtons(FCodeAnalysisState &state, FCodeAnalysisViewState& viewState)
{
	if (state.Debugger.IsStopped())
	{
		if (ImGui::Button("Continue (F5)"))
		{
			state.Debugger.Continue();
		}
	}
	else
	{
		if (ImGui::Button("Break (F5)"))
		{
			state.Debugger.Break();
			//viewState.TrackPCFrame = true;
		}
	}
	ImGui::SameLine();
	if (ImGui::Button("Step Over (F10)"))
	{
		state.Debugger.StepOver();
		viewState.TrackPCFrame = true;
	}
	ImGui::SameLine();
	if (ImGui::Button("Step Into (F11)"))
	{
		state.Debugger.StepInto();
		viewState.TrackPCFrame = true;
	}
	ImGui::SameLine();
	if (ImGui::Button("Step Frame (F6)"))
	{
		state.Debugger.StepFrame();
	}
	ImGui::SameLine();
	if (ImGui::Button("Step Screen Write (F7)"))
	{
		state.Debugger.StepScreenWrite();
	}
	ImGui::SameLine();
	if (ImGui::Button("<<< Trace"))
	{
		state.Debugger.TraceBack(viewState);
	}
	ImGui::SameLine();
	if (ImGui::Button("Trace >>>"))
	{
		state.Debugger.TraceForward(viewState);
	}
	//ImGui::SameLine();
	//ImGui::Checkbox("Jump to PC on break", &bJumpToPCOnBreak);
}

void DrawItemList(FCodeAnalysisState& state, FCodeAnalysisViewState& viewState, const std::vector<FCodeAnalysisItem>&	itemList)
{
	const float lineHeight = ImGui::GetTextLineHeight();
	FAddressRef& gotoAddress = viewState.GetGotoAddress();

	// jump to address
	if (gotoAddress.IsValid())
	{
		const float currScrollY = ImGui::GetScrollY();
		const float currWindowHeight = ImGui::GetWindowHeight();
		const int kJumpViewOffset = 5;
		for (int item = 0; item < (int)itemList.size(); item++)
		{
			if ((itemList[item].AddressRef.Address >= gotoAddress.Address) && (viewState.GoToLabel || itemList[item].Item->Type != EItemType::Label))
			{
				// set cursor
				viewState.SetCursorItem(itemList[item]);

				const float itemY = item * lineHeight;
				const float margin = kJumpViewOffset * lineHeight;

				const float moveDist = itemY - currScrollY;

				if (moveDist > currWindowHeight)
				{
					const int gotoItem = std::max(item - kJumpViewOffset, 0);
					ImGui::SetScrollY(gotoItem * lineHeight);
				}
				else
				{
					if (itemY < currScrollY + margin)
						ImGui::SetScrollY(itemY - margin);
					if (itemY > currScrollY + currWindowHeight - margin * 2)
						ImGui::SetScrollY((itemY - currWindowHeight) + margin * 2);
				}
				break;	// exit loop as we've found the address
			}
		}

		gotoAddress.SetInvalid();
		viewState.GoToLabel = false;
	}

	// draw clipped list
	ImGuiListClipper clipper((int)itemList.size(), lineHeight);
	std::vector<FAddressCoord> newList;

	while (clipper.Step())
	{
		viewState.JumpLineIndent = 0;

		for (int i = clipper.DisplayStart; i < clipper.DisplayEnd; i++)
		{
			const ImVec2 coord = ImGui::GetCursorScreenPos();
			if(itemList[i].Item->Type == EItemType::Code || itemList[i].Item->Type == EItemType::Data)
				newList.push_back({ itemList[i].AddressRef,coord.y });
			DrawCodeAnalysisItem(state, viewState, itemList[i]);
		}

	}
	viewState.AddressCoords = newList;
}

void DrawCodeAnalysisData(FCodeAnalysisState &state, int windowId)
{
	FCodeAnalysisViewState& viewState = state.ViewState[windowId];
	const float lineHeight = ImGui::GetTextLineHeight();
	const float glyph_width = ImGui::CalcTextSize("F").x;
	const float cell_width = 3 * glyph_width;

	if (ImGui::IsWindowFocused(ImGuiHoveredFlags_ChildWindows))
		state.FocussedWindowId = windowId;

	viewState.HighlightAddress = viewState.HoverAddress;
	viewState.HoverAddress.SetInvalid();

	if (state.Debugger.IsStopped() == false)
		state.CurrentFrameNo++;

	UpdateItemList(state);

	if (ImGui::ArrowButton("##btn", ImGuiDir_Left))
		viewState.GoToPreviousAddress();
	ImGui::SameLine();
	if (ImGui::Button("Jump To PC"))
	{
		//const FAddressRef PCAddress(state.GetBankFromAddress(state.CPUInterface->GetPC()), state.CPUInterface->GetPC());
		viewState.GoToAddress(state.CPUInterface->GetPC());
	}
	ImGui::SameLine();
	static int addrInput = 0;
	const ImGuiInputTextFlags inputFlags = (GetNumberDisplayMode() == ENumberDisplayMode::Decimal) ? ImGuiInputTextFlags_CharsDecimal : ImGuiInputTextFlags_CharsHexadecimal;
	if (ImGui::InputInt("Jump To", &addrInput, 1, 100, inputFlags))
	{
		const FAddressRef address(state.GetBankFromAddress(addrInput), addrInput);	// TODO: if we're in a bank view
		viewState.GoToAddress(address);
	}

	if (viewState.TrackPCFrame == true)
	{
		//const FAddressRef PCAddress(state.GetBankFromAddress(state.CPUInterface->GetPC()), state.CPUInterface->GetPC());
		viewState.GoToAddress(state.CPUInterface->GetPC());
		viewState.TrackPCFrame = false;
	}
	
	DrawDebuggerButtons(state, viewState);

	// Reset Reference Info
	if (ImGui::Button("Reset Reference Info"))
	{
		ResetReferenceInfo(state);
	}
	if (ImGui::IsItemHovered())
	{
		ImGui::BeginTooltip();
		ImGui::Text("This will reset all recorded references");
		ImGui::EndTooltip();
	}
	
	if(ImGui::BeginChild("##analysis", ImVec2(ImGui::GetWindowContentRegionWidth() * 0.75f, 0), true))
	{
		if (ImGui::BeginTabBar("itemlist_tabbar"))
		{
			// Determine if we want to switch tabs
			const FAddressRef& goToAddress = viewState.GetGotoAddress();
			int16_t showBank = -1;
			bool bSwitchTabs = false;
			if (goToAddress.IsValid())
			{
				// check if we can just jump in the address view
				//if (viewState.ViewingBankId != goToAddress.BankId && state.IsBankIdMapped(goToAddress.BankId))
				//	showBank = -1;
				//else
				if (state.Config.bShowBanks == false)
					showBank = -1;
				else
					showBank = goToAddress.BankId;

				bSwitchTabs = showBank != viewState.ViewingBankId;
			}

			ImGuiTabItemFlags tabFlags = (bSwitchTabs && showBank == -1) ? ImGuiTabItemFlags_SetSelected : 0;

			if (ImGui::BeginTabItem("Address Space",nullptr,tabFlags))
			{
				if (bSwitchTabs == false || showBank == -1)
				{
					viewState.ViewingBankId = -1;
					if (ImGui::BeginChild("##itemlist"))
						DrawItemList(state, viewState, state.ItemList);
					// only handle keypresses for focussed window
					if (state.FocussedWindowId == windowId)
						ProcessKeyCommands(state, viewState);
					UpdatePopups(state, viewState);

					ImGui::EndChild();
				}
				ImGui::EndTabItem();
			}

			if (state.Config.bShowBanks)
			{
				auto& banks = state.GetBanks();
				for (auto& bank : banks)
				{
					//if (bank.IsUsed() == false || bank.PrimaryMappedPage == -1)
					if (bank.PrimaryMappedPage == -1)
						continue;

					const uint16_t kBankStart = bank.PrimaryMappedPage * FCodeAnalysisPage::kPageSize;
					const uint16_t kBankEnd = kBankStart + (bank.NoPages * FCodeAnalysisPage::kPageSize) - 1;

					tabFlags = (bSwitchTabs && showBank == bank.Id) ? ImGuiTabItemFlags_SetSelected : 0;

					const bool bMapped = bank.MappedPages.empty() == false;
					if (!bMapped)
						ImGui::PushStyleColor(ImGuiCol_Text, IM_COL32(255, 144, 144, 144));
					const bool bTabOpen = ImGui::BeginTabItem(bank.Name.c_str(), nullptr, tabFlags);
					if (!bMapped)
						ImGui::PopStyleColor();

					if (ImGui::IsItemHovered())
					{
						ImGui::BeginTooltip();
						ImGui::Text("[%d]0x%04X - 0x%X %s", bank.Id, kBankStart, kBankEnd, bMapped ? "Mapped" : "");
						ImGui::EndTooltip();
					}

					if (bTabOpen)
					{
						if (bSwitchTabs == false || showBank == bank.Id)
						{
							// map bank in
							viewState.ViewingBankId = bank.Id;

							state.MapBankForAnalysis(bank);

							// Bank header
							ImGui::Text("%s[%d]: 0x%04X - 0x%X %s", bank.Name.c_str(), bank.Id, kBankStart, kBankEnd, bMapped ? "Mapped" : "");
							ImGui::InputText("Description", &bank.Description);

							if (ImGui::BeginChild("##itemlist"))
								DrawItemList(state, viewState, bank.ItemList);
							// only handle keypresses for focussed window
							if (state.FocussedWindowId == windowId)
								ProcessKeyCommands(state, viewState);
							UpdatePopups(state, viewState);

							ImGui::EndChild();

							// map bank out
							state.UnMapAnalysisBanks();
						}
						ImGui::EndTabItem();
						
					}


				}
			}

			ImGui::EndTabBar();
		}
		

	}
	ImGui::EndChild();
	ImGui::SameLine();
	if(ImGui::BeginChild("##rightpanel", ImVec2(0, 0), true))
	{
		//float height = ImGui::GetWindowContentRegionMax().y - ImGui::GetWindowContentRegionMin().y;
		//if (ImGui::BeginChild("##cadetails", ImVec2(0, height / 2), true))
		{
			if (ImGui::BeginTabBar("details_tab_bar"))
			{
				if (ImGui::BeginTabItem("Details"))
				{
					DrawDetailsPanel(state, viewState);
					ImGui::EndTabItem();
				}
				if (ImGui::BeginTabItem("Capture"))
				{
					DrawCaptureTab(state, viewState);
					ImGui::EndTabItem();
				}
				if (ImGui::BeginTabItem("Format"))
				{
					DrawFormatTab(state, viewState);
					ImGui::EndTabItem();
				}
				else
				{
					viewState.DataFormattingTabOpen = false;
				}
				if (ImGui::BeginTabItem("Globals"))
				{
					DrawGlobals(state, viewState);
					ImGui::EndTabItem();
				}

				ImGui::EndTabBar();
			}
		}
		//ImGui::EndChild();
		//if (ImGui::BeginChild("##caglobals", ImVec2(0, 0), true))
		//	DrawGlobals(state, viewState);
		//ImGui::EndChild();
	}
	ImGui::EndChild(); // right panel
}

void DrawLabelList(FCodeAnalysisState &state, FCodeAnalysisViewState& viewState, const std::vector<FCodeAnalysisItem>& labelList)
{
	if (ImGui::BeginChild("GlobalLabelList", ImVec2(0, 0), false))
	{		
		const float lineHeight = ImGui::GetTextLineHeight();
		ImGuiListClipper clipper((int)labelList.size(), lineHeight);

		while (clipper.Step())
		{
			for (int i = clipper.DisplayStart; i < clipper.DisplayEnd; i++)
			{
				const FCodeAnalysisItem& item = labelList[i];
				const FLabelInfo* pLabelInfo = static_cast<const FLabelInfo*>(item.Item);

				const FCodeInfo* pCodeInfo = state.GetCodeInfoForAddress(item.AddressRef);

				ImGui::PushID(item.AddressRef.Val);
				if (pCodeInfo && pCodeInfo->bDisabled == false)
					ShowCodeAccessorActivity(state, item.AddressRef);
				else
					ShowDataItemActivity(state, item.AddressRef);
								
				if (ImGui::Selectable("##labellistitem", viewState.GetCursorItem().Item == pLabelInfo))
				{
					viewState.GoToAddress(item.AddressRef, true);
				}
				ImGui::SameLine(30);
				//if(state.Config.bShowBanks)
				//	ImGui::Text("[%s]%s", item.AddressRef.BankId,pLabelInfo->Name.c_str());
				//else
					ImGui::Text("%s", pLabelInfo->Name.c_str());
				ImGui::PopID();

				if (ImGui::IsItemHovered())
				{
					DrawSnippetToolTip(state, viewState, item.AddressRef);
				}
			}
		}
	}
	ImGui::EndChild();
}

void DrawFormatTab(FCodeAnalysisState& state, FCodeAnalysisViewState& viewState)
{
	FDataFormattingOptions& formattingOptions = viewState.DataFormattingOptions;
	const ImGuiInputTextFlags inputFlags = (GetNumberDisplayMode() == ENumberDisplayMode::Decimal) ? ImGuiInputTextFlags_CharsDecimal : ImGuiInputTextFlags_CharsHexadecimal;

	if (viewState.DataFormattingTabOpen == false)
	{
		if (viewState.GetCursorItem().IsValid())
		{
			formattingOptions.StartAddress = viewState.GetCursorItem().AddressRef;
			//formattingOptions.EndAddress = viewState.pCursorItem->Address;
		}

		viewState.DataFormattingTabOpen = true;
	}

	// Set Start address of region to format
	ImGui::PushID("Start");
	ImGui::Text("Start Address: %s", NumStr(formattingOptions.StartAddress.Address));
	//ImGui::InputInt("Start Address", &formattingOptions.StartAddress.Address, 1, 100, inputFlags);
	ImGui::PopID();

	// Set End address of region to format
	ImGui::PushID("End");
	ImGui::Text("End Address: %s", NumStr(formattingOptions.CalcEndAddress()));
	//ImGui::InputInt("End Address", &formattingOptions.EndAddress, 1, 100, inputFlags);
	ImGui::PopID();

	static int dataTypeIndex = 0; 
	bool bDataTypeChanged = DrawDataTypeCombo(dataTypeIndex);

	switch (dataTypeIndex)
	{
	case 0:
		formattingOptions.DataType = EDataType::Byte;
		formattingOptions.ItemSize = 1;
		ImGui::InputInt("Item Count", &formattingOptions.NoItems);
		break;
	case 1:
		formattingOptions.DataType = EDataType::Word;
		formattingOptions.ItemSize = 2;
		ImGui::InputInt("Item Count", &formattingOptions.NoItems);
		break;
	case 2:
	{
		formattingOptions.DataType = EDataType::Bitmap;
		
		bool bBitmapOptionsChanged = DrawBitmapFormatCombo(viewState.CurBitmapFormat, state);

		static int size[2];
		bBitmapOptionsChanged |= ImGui::InputInt2("Bitmap Size(X,Y)", size);
			
		if (bBitmapOptionsChanged || bDataTypeChanged)
		{
			switch (viewState.CurBitmapFormat)
			{
			case EBitmapFormat::Bitmap_1Bpp:
				formattingOptions.DisplayType = EDataItemDisplayType::Bitmap;
				break;
			case EBitmapFormat::ColMap2Bpp_CPC:
				formattingOptions.DisplayType = EDataItemDisplayType::ColMap2Bpp_CPC;
				break;
			case EBitmapFormat::ColMap4Bpp_CPC:
				formattingOptions.DisplayType = EDataItemDisplayType::ColMap4Bpp_CPC;
				break;
			}

			int pixelsPerByte = 0;
			switch (formattingOptions.DisplayType)
			{
			case EDataItemDisplayType::Bitmap: // 1 bpp
				pixelsPerByte = 8;
				break;
			case EDataItemDisplayType::ColMap2Bpp_CPC: // 2 bpp
				pixelsPerByte = 4;
				break;
			case EDataItemDisplayType::ColMap4Bpp_CPC: // 4 bpp
				pixelsPerByte = 2;
				break;
			}
			formattingOptions.ItemSize = std::max(1, size[0] / pixelsPerByte);
			formattingOptions.NoItems = size[1];
		}
		break;
	}
	case 3:
		formattingOptions.DataType = EDataType::CharacterMap;
		{
			static int size[2];
			if (ImGui::InputInt2("CharMap Size(X,Y)", size))
			{
				formattingOptions.ItemSize = std::max(1, size[0]);
				formattingOptions.NoItems = size[1];
			}

			DrawCharacterSetComboBox(state, formattingOptions.CharacterSet);
			const char* format = "%02X";
			int flags = ImGuiInputTextFlags_EnterReturnsTrue | ImGuiInputTextFlags_CharsHexadecimal;
			ImGui::InputScalar("Null Character", ImGuiDataType_U8, &formattingOptions.EmptyCharNo, 0, 0, format, flags);
			//ImGui::InputInt("Item Size", &formattingOptions.ItemSize);
		}
		//ImGui::InputInt("Item Size", &formattingOptions.ItemSize);
		break;
	case 4:
		formattingOptions.DataType = EDataType::ColAttr;
		ImGui::InputInt("Item Size", &formattingOptions.ItemSize);
		ImGui::InputInt("Item Count", &formattingOptions.NoItems);
		break;
	case 5:
		formattingOptions.DataType = EDataType::Text;
		ImGui::InputInt("Item Size", &formattingOptions.ItemSize);
		formattingOptions.NoItems = 1;
		break;
	}

	//ImGui::SameLine();
	//if (ImGui::Button("Set"))
	//	formattingOptions.EndAddress = formattingOptions.StartAddress + (itemCount * formattingOptions.ItemSize);

	//ImGui::Checkbox("Binary Visualisation", &formattingOptions.BinaryVisualisation);
	//ImGui::Checkbox("Char Map Visualisation", &formattingOptions.CharMapVisualisation);
	ImGui::Checkbox("Clear Code Info", &formattingOptions.ClearCodeInfo);
	ImGui::SameLine();
	ImGui::Checkbox("Clear Labels", &formattingOptions.ClearLabels);
	ImGui::Checkbox("Add Label at Start", &formattingOptions.AddLabelAtStart);

	if (formattingOptions.IsValid())
	{
		if (ImGui::Button("Format"))
		{
			FormatData(state, formattingOptions);
			state.SetCodeAnalysisDirty(formattingOptions.StartAddress);
		}
		ImGui::SameLine();
		if (ImGui::Button("Format & Advance"))
		{
			FormatData(state, formattingOptions);
			state.AdvanceAddressRef(formattingOptions.StartAddress, formattingOptions.ItemSize * formattingOptions.NoItems);
			state.SetCodeAnalysisDirty(formattingOptions.StartAddress);
			viewState.GoToAddress(formattingOptions.StartAddress);
		}
	}

	if (ImGui::Button("Clear Selection"))
	{
		formattingOptions = FDataFormattingOptions();
	}
}

void GenerateFilteredLabelList(FCodeAnalysisState& state, const FLabelListFilter&filter,const std::vector<FCodeAnalysisItem>& sourceLabelList, std::vector<FCodeAnalysisItem>& filteredList)
{
	filteredList.clear();

	std::string filterTextLower = filter.FilterText;
	std::transform(filterTextLower.begin(), filterTextLower.end(), filterTextLower.begin(), [](unsigned char c){ return std::tolower(c); });

	for (const FCodeAnalysisItem& labelItem : sourceLabelList)
	{
		if (labelItem.AddressRef.Address < filter.MinAddress || labelItem.AddressRef.Address > filter.MaxAddress)	// skip min address
			continue;

		const FCodeAnalysisBank* pBank = state.GetBank(labelItem.AddressRef.BankId);
		if (pBank)
		{
			if (filter.bRAMOnly && pBank->bReadOnly)
				continue;
		}
		
		const FLabelInfo* pLabelInfo = static_cast<const FLabelInfo*>(labelItem.Item);
		std::string labelTextLower = pLabelInfo->Name;
		std::transform(labelTextLower.begin(), labelTextLower.end(), labelTextLower.begin(), [](unsigned char c){ return std::tolower(c); });

		if (filter.FilterText.empty() || labelTextLower.find(filterTextLower) != std::string::npos)
			filteredList.push_back(labelItem);
	}
}

void DrawGlobals(FCodeAnalysisState &state, FCodeAnalysisViewState& viewState)
{
	if (ImGui::InputText("Filter", &viewState.FilterText))
	{
		viewState.GlobalFunctionsFilter.FilterText = viewState.FilterText;
		viewState.GlobalDataItemsFilter.FilterText = viewState.FilterText;
		state.bRebuildFilteredGlobalFunctions = true;
		state.bRebuildFilteredGlobalDataItems = true;
	}
	ImGui::SameLine();
	if (ImGui::Checkbox("ROM", &viewState.ShowROMLabels))
	{
		viewState.GlobalFunctionsFilter.bRAMOnly = !viewState.ShowROMLabels;
		viewState.GlobalDataItemsFilter.bRAMOnly = !viewState.ShowROMLabels;
		state.bRebuildFilteredGlobalFunctions = true;
		state.bRebuildFilteredGlobalDataItems = true;
	}

	if(ImGui::BeginTabBar("GlobalsTabBar"))
	{
		if(ImGui::BeginTabItem("Functions"))
		{	
			// only constantly sort call frequency
			bool bSort = viewState.FunctionSortMode == EFunctionSortMode::CallFrequency;	
			if (ImGui::Combo("Sort Mode", (int*)&viewState.FunctionSortMode, "Location\0Alphabetical\0Call Frequency"))
				bSort = true;

			if (state.bRebuildFilteredGlobalFunctions)
			{
				GenerateFilteredLabelList(state, viewState.GlobalFunctionsFilter, state.GlobalFunctions, viewState.FilteredGlobalFunctions);
				bSort = true;
				state.bRebuildFilteredGlobalFunctions = false;
			}

			// sort by execution count
			if (bSort)
			{
				switch (viewState.FunctionSortMode)
				{
				case EFunctionSortMode::Location:	
					std::sort(viewState.FilteredGlobalFunctions.begin(), viewState.FilteredGlobalFunctions.end(), [&state](const FCodeAnalysisItem& a, const FCodeAnalysisItem& b)
						{
							return a.AddressRef.Address < b.AddressRef.Address;
						});
					break;
				case EFunctionSortMode::Alphabetical:	
					std::sort(viewState.FilteredGlobalFunctions.begin(), viewState.FilteredGlobalFunctions.end(), [&state](const FCodeAnalysisItem& a, const FCodeAnalysisItem& b)
						{
							const FLabelInfo* pLabelA = state.GetLabelForAddress(a.AddressRef);
							const FLabelInfo* pLabelB = state.GetLabelForAddress(b.AddressRef);
							return pLabelA->Name < pLabelB->Name;
						});
					break;
				case EFunctionSortMode::CallFrequency:	
					std::sort(viewState.FilteredGlobalFunctions.begin(), viewState.FilteredGlobalFunctions.end(), [&state](const FCodeAnalysisItem& a, const FCodeAnalysisItem& b)
						{
							const FCodeInfo* pCodeInfoA = state.GetCodeInfoForAddress(a.AddressRef);
							const FCodeInfo* pCodeInfoB = state.GetCodeInfoForAddress(b.AddressRef);

							const int countA = pCodeInfoA != nullptr ? pCodeInfoA->ExecutionCount : 0;
							const int countB = pCodeInfoB != nullptr ? pCodeInfoB->ExecutionCount : 0;

							return countA > countB;
						});
					break;
				default:
					break;
				}
			}

			DrawLabelList(state, viewState, viewState.FilteredGlobalFunctions);
			ImGui::EndTabItem();
		}

		if (ImGui::BeginTabItem("Data"))
		{
			if (state.bRebuildFilteredGlobalDataItems)
			{
				GenerateFilteredLabelList(state, viewState.GlobalDataItemsFilter, state.GlobalDataItems, viewState.FilteredGlobalDataItems);
				state.bRebuildFilteredGlobalDataItems = false;
			}

			DrawLabelList(state, viewState, viewState.FilteredGlobalDataItems);
			ImGui::EndTabItem();
		}

		

		ImGui::EndTabBar();
	}
}


void DrawMachineStateZ80(const FMachineState* pMachineState, FCodeAnalysisState& state, FCodeAnalysisViewState& viewState);

void DrawCaptureTab(FCodeAnalysisState& state, FCodeAnalysisViewState& viewState)
{
	const FCodeAnalysisItem& item = viewState.GetCursorItem();
	if (item.IsValid() == false)
		return;

	const uint16_t physAddress = item.AddressRef.Address;
	const FMachineState* pMachineState = state.GetMachineState(physAddress);
	if (pMachineState == nullptr)
		return;

	// TODO: display machine state
	if (state.CPUInterface->CPUType == ECPUType::Z80)
		DrawMachineStateZ80(pMachineState, state,viewState);

}

// Util functions - move?
bool DrawU8Input(const char* label, uint8_t* value)
{
	const char* format = "%02X";
	int flags = ImGuiInputTextFlags_CharsHexadecimal;
	return ImGui::InputScalar(label, ImGuiDataType_U8, value, 0, 0, format, flags);
}

bool DrawAddressInput(const char* label, uint16_t* value)
{
	const ImGuiInputTextFlags inputFlags = (GetNumberDisplayMode() == ENumberDisplayMode::Decimal) ? ImGuiInputTextFlags_CharsDecimal : ImGuiInputTextFlags_CharsHexadecimal;
	const char* format = (GetNumberDisplayMode() == ENumberDisplayMode::Decimal) ? "%d" : "%04X";
	return ImGui::InputScalar(label, ImGuiDataType_U16, value, 0, 0, format, inputFlags);
}

bool DrawAddressInput(FCodeAnalysisState& state, const char* label, FAddressRef& address)
{
	bool bValueInput = false;
	/*
	if (state.Config.bShowBanks)
	{
		ImGui::SetNextItemWidth(60.0f);
		DrawBankInput(state, "Bank", address.BankId);
		ImGui::SameLine();
	}*/

	ImGui::PushID(label);

	const ImGuiInputTextFlags inputFlags = (GetNumberDisplayMode() == ENumberDisplayMode::Decimal) ? ImGuiInputTextFlags_CharsDecimal : ImGuiInputTextFlags_CharsHexadecimal;
	const char* format = (GetNumberDisplayMode() == ENumberDisplayMode::Decimal) ? "%d" : "%04X";
	ImGui::Text("%s", label);
	ImGui::SameLine();
	ImGui::SetNextItemWidth(40.0f);
	if (ImGui::InputScalar("##addronput", ImGuiDataType_U16, &address.Address, 0, 0, format, inputFlags))
	{
		address = state.AddressRefFromPhysicalAddress(address.Address);
		bValueInput = true;
	}

	if (ImGui::BeginPopupContextItem("address input context menu"))
	{
		if (ImGui::Selectable("Paste Address"))
		{
			address = state.CopiedAddress;
			bValueInput = true;
		}
		ImGui::EndPopup();
	}
	ImGui::PopID();

	//if (state.Config.bShowBanks)
	{
		const FCodeAnalysisBank* pBank = state.GetBank(address.BankId);
		ImGui::SameLine();
		if (pBank != nullptr)
			ImGui::Text("(%s)", pBank->Name.c_str());
		else
			ImGui::Text("(None)");
	}

	return bValueInput;
}

const char* GetBankText(FCodeAnalysisState& state, int16_t bankId)
{
	const FCodeAnalysisBank* pBank = state.GetBank(bankId);

	if (pBank == nullptr)
		return "None";

	return pBank->Name.c_str();
}

bool DrawBankInput(FCodeAnalysisState& state, const char* label, int16_t& bankId, bool bAllowNone)
{
	bool bBankChanged = false;
	if (ImGui::BeginCombo("Bank", GetBankText(state, bankId)))
	{
		if (bAllowNone)
		{
			if (ImGui::Selectable(GetBankText(state, -1), bankId == -1))
			{
				bankId = -1;
				bBankChanged = true;
			}
		}

		const auto& banks = state.GetBanks();
		for (const auto& bank : banks)
		{
			if (ImGui::Selectable(GetBankText(state, bank.Id), bankId == bank.Id))
			{
				FCodeAnalysisBank* pNewBank = state.GetBank(bank.Id);
				bankId = bank.Id;
				bBankChanged = true;
			}
		}

		ImGui::EndCombo();
	}

	return bBankChanged;
}

// Config Window - Debug?

void DrawCodeAnalysisConfigWindow(FCodeAnalysisState& state)
{
	FCodeAnalysisConfig& config = state.Config;

	ImGui::SliderFloat("Label Pos", &config.LabelPos, 0, 200.0f);
	ImGui::SliderFloat("Comment Pos", &config.CommentLinePos, 0, 200.0f);
	ImGui::SliderFloat("Address Pos", &config.AddressPos, 0, 200.0f);
	ImGui::SliderFloat("Address Space", &config.AddressSpace, 0, 200.0f);

	ImGui::SliderFloat("Branch Line Start", &config.BranchLineIndentStart, 0, 200.0f);
	ImGui::SliderFloat("Branch Line Spacing", &config.BranchSpacing, 0, 20.0f);
	ImGui::SliderInt("Branch Line No Indents", &config.BranchMaxIndent,1,10);
}