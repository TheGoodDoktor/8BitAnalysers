#include "CodeAnalyserUI.h"
#include "CharacterMapViewer.h"
#include "../CodeAnalyser.h"
#include "CodeAnalyser/DataTypes.h"
#include "Misc/GlobalConfig.h"	
#include "ComboBoxes.h"

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
#include <functional>

#include "UIColours.h"
#include <ImGuiSupport/ImGuiScaling.h>

// UI
void DrawCodeAnalysisItem(FCodeAnalysisState& state, FCodeAnalysisViewState& viewState, const FCodeAnalysisItem& item);
void DrawFormatTab(FCodeAnalysisState& state, FCodeAnalysisViewState& viewState);
void DrawCaptureTab(FCodeAnalysisState& state, FCodeAnalysisViewState& viewState);
void DrawFindTab(FCodeAnalysisState& state, FCodeAnalysisViewState& viewState);

void DrawCodeInfo(FCodeAnalysisState& state, FCodeAnalysisViewState& viewState, const FCodeAnalysisItem& item);
void DrawCodeDetails(FCodeAnalysisState& state, FCodeAnalysisViewState& viewState, const FCodeAnalysisItem& item);

namespace ImGui
{
	bool BeginComboPreview();
	void EndComboPreview();
}

void FCodeAnalysisViewState::GoToAddress(FAddressRef newAddress, bool bLabel)
{
	if(GetCursorItem().IsValid())
		PreviousAddressStack.push_back(GetCursorItem().AddressRef);
	GoToAddressRef = newAddress;
	GoToLabel = bLabel;
}

bool FCodeAnalysisViewState::GoToPreviousAddress()
{
	if (PreviousAddressStack.empty())
		return false;

	GoToAddressRef = PreviousAddressStack.back();
	NextAddressStack.push_back(GetCursorItem().AddressRef);
	GoToLabel = false;
	PreviousAddressStack.pop_back();
	return true;
}

bool FCodeAnalysisViewState::GoToNextAddress()
{
	if (NextAddressStack.empty())
		return false;

	GoToAddressRef = NextAddressStack.back();
	PreviousAddressStack.push_back(GetCursorItem().AddressRef);
	GoToLabel = false;
	NextAddressStack.pop_back();
	return true;
}

void FCodeAnalysisViewState::FixupAddressRefs(const FCodeAnalysisState& state)
{
	FixupAddressRef(state, CursorItem.AddressRef);
	FixupAddressRef(state, HoverAddress);
	FixupAddressRef(state, HighlightAddress);
	FixupAddressRef(state, GoToAddressRef);
	FixupAddressRefList(state, PreviousAddressStack);
	FixupAddressRefList(state, NextAddressStack);

	for (FCodeAnalysisItem& item : FilteredGlobalDataItems)
		FixupAddressRef(state, item.AddressRef);
	for (FCodeAnalysisItem& item : FilteredGlobalFunctions)
		FixupAddressRef(state, item.AddressRef);

	for (int i = 0; i < kNoBookmarks; i++)
	{
		FixupAddressRef(state, Bookmarks[i]);
	}

	for (FAddressCoord& coord : AddressCoords)
	{
		FixupAddressRef(state, coord.Address);
	}
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

void GotoJumpAddress(FCodeAnalysisState& state, FCodeAnalysisViewState& viewState)
{
	if (viewState.ViewingBankId == -1)
		viewState.GoToAddress(state.AddressRefFromPhysicalAddress(viewState.JumpAddress));
	else
	{
		FAddressRef jumpAddr(viewState.ViewingBankId, viewState.JumpAddress);
		if (state.IsAddressValid(jumpAddr))
			viewState.GoToAddress(jumpAddr);
		else
			viewState.GoToAddress(state.AddressRefFromPhysicalAddress(viewState.JumpAddress));
	}
}

std::vector<FMemoryRegionDescGenerator*>	g_RegionDescHandlers;

void ResetRegionDescs(void)
{
    g_RegionDescHandlers.clear();
}

void UpdateRegionDescs(void)
{
	for (FMemoryRegionDescGenerator* pDescGen : g_RegionDescHandlers)
	{
		if (pDescGen)
			pDescGen->FrameTick();
	}
}

const char* GetRegionDesc(FAddressRef addr)
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

void DrawSnippetToolTip(FCodeAnalysisState& state, FCodeAnalysisViewState& viewState, const FAddressRef addr, int noLines /* = 10 */)
{
	// Bring up snippet in tool tip
	const FCodeAnalysisBank* pBank = state.GetBank(addr.BankId);
	if (pBank != nullptr)
	{
		const int index = GetItemIndexForAddress(state, addr);
		if (index != -1)
		{
			ImGui::BeginTooltip();
			const int startIndex = std::max(index - (noLines / 2), 0);
			for (int line = 0; line < noLines; line++)
			{
				if (startIndex + line < (int)pBank->ItemList.size())
					DrawCodeAnalysisItem(state, viewState, pBank->ItemList[startIndex + line]);
			}
			ImGui::EndTooltip();
		}
	}

	viewState.pLabelScope = nullptr;
}

// TODO: phase this out
bool DrawAddressLabel(FCodeAnalysisState& state, FCodeAnalysisViewState& viewState, uint16_t addr, uint32_t displayFlags)
{
	return DrawAddressLabel(state, viewState, { state.GetBankFromAddress(addr),addr },displayFlags);
}

std::string GenerateAddressLabelString(FCodeAnalysisState& state, FAddressRef addr)
{
	bool bFunctionRel = false;
	const char* pLabelString = nullptr;
	int labelOffset = 0;
	std::string labelOut;

	FCodeAnalysisBank* pBank = state.GetBank(addr.BankId);
	if (pBank == nullptr)
		return labelOut;

	// find a label for this address
	for (int addrVal = addr.Address; addrVal >= 0; addrVal--)
	{
		if (pBank->AddressValid(addrVal) == false)
		{
			pBank = state.GetBank(state.GetBankFromAddress(addrVal));
			assert(pBank != nullptr);
		}

		const FLabelInfo* pLabel = state.GetLabelForAddress(FAddressRef(pBank->Id, addrVal));
		if (pLabel != nullptr)
		{
			pLabelString = pLabel->GetName();
			break;
		}

		labelOffset++;

		if (pLabelString == nullptr && addrVal == 0)
			pLabelString = "0000";
	}

	if(pLabelString != nullptr)
	{
		labelOut = pLabelString;

		if (labelOffset > 0)	// append offset
		{
			char offsetString[32];
			snprintf(offsetString,32,"+%d",labelOffset);
			labelOut += offsetString;
		}
	}

	return labelOut;
}

bool DrawAddressLabel(FCodeAnalysisState &state, FCodeAnalysisViewState& viewState, FAddressRef addr, uint32_t displayFlags)
{
	char localLabelString[64];
	bool bFunctionRel = false;
	bool bToolTipShown = false;
	int labelOffset = 0;
	const char *pLabelString = GetRegionDesc(addr);
	const FLabelInfo* pAddressScope = state.GetScopeForAddress(addr);
	FCodeAnalysisBank* pBank = state.GetBank(addr.BankId);
	if(pBank == nullptr)
		return false;

	assert(pBank != nullptr);
	bool bGlobalHighlighting = pLabelString != nullptr;
	bool bFunctionHighlighting = false;

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
				bFunctionHighlighting = pLabel->LabelType == ELabelType::Function;
				bGlobalHighlighting = pLabel->Global;

				if (bFunctionRel == false || pLabel->LabelType == ELabelType::Function)
				{
					if (pLabel->Global == false && pAddressScope != nullptr && pAddressScope != viewState.pLabelScope)	// TODO: check if this is our scope
					{
						snprintf(localLabelString,64,"%s.%s", pAddressScope->GetName(), pLabel->GetName());
						pLabelString = localLabelString;
					}
					else
					{
						pLabelString = pLabel->GetName();
					}
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
		ImGui::SameLine(0,0);

		if (bFunctionHighlighting && displayFlags & kAddressLabelFlag_White)
			ImGui::PushStyleColor(ImGuiCol_Text, Colours::function);
		else if(bGlobalHighlighting && displayFlags & kAddressLabelFlag_White)
			ImGui::PushStyleColor(ImGuiCol_Text, Colours::globalLabel);
		else
			ImGui::PushStyleColor(ImGuiCol_Text, Colours::localLabel);

		const FCodeAnalysisBank* pBank = state.GetBank(addr.BankId);

		if (pBank->bFixed == false && state.Config.bShowBanks && (displayFlags & kAddressLabelFlag_NoBank) == 0)
		{
			ImGui::Text("[%s]", pBank->Name.c_str());
			ImGui::SameLine(0,0);
		}

		if (displayFlags & kAddressLabelFlag_NoBrackets)
		{
			if (labelOffset == 0)
				ImGui::Text("%s", pLabelString);
			else
				ImGui::Text("%s + %d", pLabelString, labelOffset);
		}
		else
		{
			if(labelOffset == 0)
				ImGui::Text("[%s]", pLabelString);
			else
				ImGui::Text("[%s + %d]", pLabelString, labelOffset);
		}
		if (ImGui::IsItemHovered())
		{
			ImGui::PopStyleColor();
			viewState.HoverAddress = addr;
			viewState.HighlightAddress = viewState.HoverAddress;

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
	
			bToolTipShown = true;
		}
		else
		{ 
			ImGui::PopStyleColor();
		}
	}

	return bToolTipShown;
}

void DrawCodeAddress(FCodeAnalysisState &state, FCodeAnalysisViewState& viewState, FAddressRef addr, uint32_t displayFlags)
{
	//ImGui::PushStyleColor(ImGuiCol_Text, 0xff00ffff);
	ImGui::Text("%s", NumStr(addr.Address));
	//ImGui::PopStyleColor();
	ImGui::SameLine();
	DrawAddressLabel(state, viewState, addr, displayFlags);
}

// TODO: Phase this out
void DrawCodeAddress(FCodeAnalysisState& state, FCodeAnalysisViewState& viewState, uint16_t addr, uint32_t displayFlags)
{
	DrawCodeAddress(state, viewState, {state.GetBankFromAddress(addr), addr});
}

void DrawComment(FCodeAnalysisState& state, FCodeAnalysisViewState& viewState, const FItem *pItem, float offset)
{
	if(pItem != nullptr && pItem->Comment.empty() == false)
	{
		ImGui::SameLine(offset);
		ImGui::PushStyleColor(ImGuiCol_Text, Colours::comment);
		//old ImGui::Text("\t; %s", pItem->Comment.c_str());
		ImGui::Text("\t; ");
		ImGui::SameLine();
		Markup::DrawText(state,viewState,pItem->Comment.c_str());
		ImGui::PopStyleColor();
	}
}

void DrawLabelInfo(FCodeAnalysisState &state, FCodeAnalysisViewState& viewState, const FCodeAnalysisItem& item)
{
	const FLabelInfo* pLabelInfo = static_cast<const FLabelInfo*>(item.Item);
	const FDataInfo* pDataInfo = state.GetDataInfoForAddress(item.AddressRef);	// for self-modifying code
	const FCodeInfo* pCodeInfo = state.GetCodeInfoForAddress(item.AddressRef);
	
	ImU32 labelColour = Colours::localLabel;
	if (viewState.HighlightAddress == item.AddressRef)
		labelColour = Colours::highlight;
	else if (pLabelInfo->LabelType == ELabelType::Function)
		labelColour = Colours::function;
	else if (pLabelInfo->Global)
		labelColour = Colours::globalLabel;

	ImGui::PushStyleColor(ImGuiCol_Text, labelColour);

	// draw SMC fixups differently
	if (pCodeInfo == nullptr && pDataInfo->DataType == EDataType::InstructionOperand)
	{
		ImGui::Text( "\t\tOperand Fixup(%s) :",NumStr(item.AddressRef.Address));
		ImGui::SameLine();
		ImGui::Text("%s", pLabelInfo->GetName());
	}
	else
	{
		ImGui::SameLine(state.Config.LabelPos);
		ImGui::Text("%s: ", pLabelInfo->GetName());
	}

	ImGui::PopStyleColor();

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

	DrawComment(state,viewState,pLabelInfo);	
}

void DrawLabelDetails(FCodeAnalysisState &state, FCodeAnalysisViewState& viewState,const FCodeAnalysisItem& item )
{
	FLabelInfo* pLabelInfo = static_cast<FLabelInfo*>(item.Item);
	std::string LabelText = pLabelInfo->GetName();
	if (ImGui::InputText("Name", &LabelText, ImGuiInputTextFlags_EnterReturnsTrue))
	{
		if (LabelText.empty())
			LabelText = pLabelInfo->GetName();

		pLabelInfo->ChangeName(LabelText.c_str(),item.AddressRef);
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
	FAddressRef removeRef;
	for (const auto & ref : pLabelInfo->References.GetReferences())
	{
		ImGui::PushID(ref.Val);
		ShowCodeAccessorActivity(state, ref);

		ImGui::Text("   ");
		ImGui::SameLine();
		DrawCodeAddress(state, viewState, ref);
		ImGui::SameLine();
		if(ImGui::Button("Remove"))
		{
			removeRef = ref;
		}
		ImGui::PopID();
	}
	if(removeRef.IsValid())
		pLabelInfo->References.RemoveReference(removeRef);

	if(ImGui::Button("Find References"))
	{
		std::vector<FAddressRef> results = state.FindAllMemoryPatterns((const uint8_t*)&item.AddressRef.Address,2,false,false);

		for(const auto& result : results)
		{
			FDataInfo* pDataInfo = state.GetDataInfoForAddress(result);

			if(pDataInfo->DataType == EDataType::InstructionOperand)	// handle instructions differently
				pLabelInfo->References.RegisterAccess(pDataInfo->InstructionAddress);
			else
				pLabelInfo->References.RegisterAccess(result);
		}
	}
	ImGui::SameLine();
	if (ImGui::Button("Clear References"))
	{
		pLabelInfo->References.Reset();
	}
}


void DrawCommentLine(FCodeAnalysisState& state, const FCommentLine* pCommentLine)
{
	ImGui::SameLine();
	ImGui::PushStyleColor(ImGuiCol_Text, Colours::comment);
	ImGui::SameLine(state.Config.CommentLinePos);
	//ImGui::Text("; %s", pCommentLine->Comment.c_str());
	ImGui::Text("\t; ");
	ImGui::SameLine();
	//Markup::DrawText(state, viewState, pItem->Comment.c_str());
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
		if (ImGui::IsKeyPressed((ImGuiKey)state.KeyConfig[(int)EKey::SetItemCode]))
		{
			SetItemCode(state, cursorItem.AddressRef);
		}
		else if (ImGui::IsKeyPressed((ImGuiKey)state.KeyConfig[(int)EKey::SetItemData]))
		{
			SetItemData(state, cursorItem);
		}
		else if (ImGui::IsKeyPressed((ImGuiKey)state.KeyConfig[(int)EKey::SetItemText]))
		{
			SetItemText(state, cursorItem);
		}
#ifdef ENABLE_IMAGE_TYPE
		else if (ImGui::IsKeyPressed((ImGuiKey)state.KeyConfig[(int)EKey::SetItemImage]))
		{
			SetItemImage(state, cursorItem);
		}
#endif
		else if (ImGui::IsKeyPressed((ImGuiKey)state.KeyConfig[(int)EKey::SetItemBinary]))
		{
			if (cursorItem.Item->Type == EItemType::Data)
			{
				FDataInfo* pDataItem = static_cast<FDataInfo*>(cursorItem.Item);
				pDataItem->DataType = EDataType::Byte;
				pDataItem->DisplayType = EDataItemDisplayType::Binary;
			}
			else if (cursorItem.Item->Type == EItemType::Code)
			{
				FCodeInfo* pCodeItem = static_cast<FCodeInfo*>(cursorItem.Item);
				pCodeItem->OperandType = EOperandType::Binary;
				pCodeItem->Text.clear();
			}
		}
		else if (ImGui::IsKeyPressed((ImGuiKey)state.KeyConfig[(int)EKey::SetItemPointer]))
		{
			if (cursorItem.Item->Type == EItemType::Data)
			{
				FDataInfo* pDataItem = static_cast<FDataInfo*>(cursorItem.Item);
				pDataItem->DataType = EDataType::Word;
				pDataItem->ByteSize = 2;
				pDataItem->DisplayType = EDataItemDisplayType::Pointer;
				state.SetCodeAnalysisDirty(cursorItem.AddressRef);
			}
			else if (cursorItem.Item->Type == EItemType::Code)
			{
				FCodeInfo* pCodeItem = static_cast<FCodeInfo*>(cursorItem.Item);
				pCodeItem->OperandType = EOperandType::Pointer;
				pCodeItem->Text.clear();
			}
		} 
		else if (ImGui::IsKeyPressed((ImGuiKey)state.KeyConfig[(int)EKey::SetItemJumpAddress]))
		{
			if (cursorItem.Item->Type == EItemType::Data)
			{
				FDataInfo* pDataItem = static_cast<FDataInfo*>(cursorItem.Item);
				pDataItem->DataType = EDataType::Word;
				pDataItem->ByteSize = 2;
				pDataItem->DisplayType = EDataItemDisplayType::JumpAddress;
				state.SetCodeAnalysisDirty(cursorItem.AddressRef);

				// Mark address as function & set as code
				const FAddressRef jumpAddress = state.AddressRefFromPhysicalAddress(state.ReadWord(cursorItem.AddressRef));
				GenerateLabelForAddress(state,jumpAddress,ELabelType::Function);
				// maybe we should only do this if it's 'unknown'?
				FDataInfo* pJumpAddressData = state.GetDataInfoForAddress(jumpAddress);
				if(pJumpAddressData->IsUninitialised())
				{
					SetItemCode(state, jumpAddress);
				}
			}
			else if (cursorItem.Item->Type == EItemType::Code)
			{
				FCodeInfo* pCodeItem = static_cast<FCodeInfo*>(cursorItem.Item);
				pCodeItem->OperandType = EOperandType::JumpAddress;
				pCodeItem->Text.clear();
			}
		}
		else if (ImGui::IsKeyPressed((ImGuiKey)state.KeyConfig[(int)EKey::SetItemAscii]))
		{
			if (cursorItem.Item->Type == EItemType::Data)
			{
				FDataInfo* pDataItem = static_cast<FDataInfo*>(cursorItem.Item);
				pDataItem->DisplayType = EDataItemDisplayType::Ascii;
				state.SetCodeAnalysisDirty(cursorItem.AddressRef);
			}
			else if (cursorItem.Item->Type == EItemType::Code)
			{
				FCodeInfo* pCodeItem = static_cast<FCodeInfo*>(cursorItem.Item);
				pCodeItem->OperandType = EOperandType::Ascii;
				pCodeItem->Text.clear();
			}
		}
		else if (ImGui::IsKeyPressed((ImGuiKey)state.KeyConfig[(int)EKey::SetItemNumber]))
		{
			if (cursorItem.Item->Type == EItemType::Data)
			{
				FDataInfo* pDataItem = static_cast<FDataInfo*>(cursorItem.Item);
				if(pDataItem->DisplayType == EDataItemDisplayType::SignedNumber)
					pDataItem->DisplayType = EDataItemDisplayType::Decimal;
				else
					pDataItem->DisplayType = EDataItemDisplayType::SignedNumber;
				state.SetCodeAnalysisDirty(cursorItem.AddressRef);
			}
			else if (cursorItem.Item->Type == EItemType::Code)
			{
				FCodeInfo* pCodeItem = static_cast<FCodeInfo*>(cursorItem.Item);
				pCodeItem->OperandType = EOperandType::SignedNumber;
				pCodeItem->Text.clear();
			}
		}
		else if (ImGui::IsKeyPressed((ImGuiKey)state.KeyConfig[(int)EKey::SetItemUnknown]))
		{
			if (cursorItem.Item->Type == EItemType::Data)
			{
				FDataInfo* pDataItem = static_cast<FDataInfo*>(cursorItem.Item);
				pDataItem->DisplayType = EDataItemDisplayType::Unknown;
				state.SetCodeAnalysisDirty(cursorItem.AddressRef);
			}
			else if (cursorItem.Item->Type == EItemType::Code)
			{
				FCodeInfo* pCodeItem = static_cast<FCodeInfo*>(cursorItem.Item);
				pCodeItem->OperandType = EOperandType::Unknown;
				pCodeItem->Text.clear();
			}
		}
		else if (ImGui::IsKeyPressed((ImGuiKey)state.KeyConfig[(int)EKey::AddLabel]))
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
		else if (io.KeyShift && ImGui::IsKeyPressed((ImGuiKey)state.KeyConfig[(int)EKey::Comment]))
		{
			FCommentBlock* pCommentBlock = AddCommentBlock(state, cursorItem.AddressRef);
			viewState.SetCursorItem(FCodeAnalysisItem(pCommentBlock, cursorItem.AddressRef));
			ImGui::OpenPopup("Enter Comment Text Multi");
			ImGui::SetWindowFocus("Enter Comment Text Multi");
		}
		else if (ImGui::IsKeyPressed((ImGuiKey)state.KeyConfig[(int)EKey::Comment]) || ImGui::IsKeyPressed((ImGuiKey)state.KeyConfig[(int)EKey::CommentLegacy]))
		{
			ImGui::OpenPopup("Enter Comment Text");
			ImGui::SetWindowFocus("Enter Comment Text");
		}
		else if (cursorItem.Item->Type == EItemType::Label && ImGui::IsKeyPressed((ImGuiKey)state.KeyConfig[(int)EKey::Rename]))
		{
			//AddLabelAtAddress(state, state.pCursorItem->Address);
			ImGui::OpenPopup("Enter Label Text");
			ImGui::SetWindowFocus("Enter Label Text");
		}
		
		else if (ImGui::IsKeyPressed((ImGuiKey)state.KeyConfig[(int)EKey::Breakpoint]))
		{
			if (cursorItem.Item->Type == EItemType::Data)
				state.ToggleDataBreakpointAtAddress(cursorItem.AddressRef, cursorItem.Item->ByteSize);
			else if (cursorItem.Item->Type == EItemType::Code)
				state.ToggleExecBreakpointAtAddress(cursorItem.AddressRef);
		}
	}

	if (ImGui::IsKeyPressed((ImGuiKey)state.KeyConfig[(int)EKey::BreakContinue]))
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
	else if (ImGui::IsKeyPressed((ImGuiKey)state.KeyConfig[(int)EKey::StepOver]))
	{
		state.Debugger.StepOver();
	}
	else if (ImGui::IsKeyPressed((ImGuiKey)state.KeyConfig[(int)EKey::StepInto]))
	{
		state.Debugger.StepInto();
	}
	else if (ImGui::IsKeyPressed((ImGuiKey)state.KeyConfig[(int)EKey::StepFrame]))
	{
		state.Debugger.StepFrame();
	}
	else if (ImGui::IsKeyPressed((ImGuiKey)state.KeyConfig[(int)EKey::StepScreenWrite]))
	{
		state.Debugger.StepScreenWrite();
	}

	// navigation controls
	if(io.KeyCtrl || io.KeyShift)
	{
		int bookmarkNo = -1;

		for(int keyNo =0;keyNo<5;keyNo++)
		if(ImGui::IsKeyPressed((ImGuiKey)(ImGuiKey_1 + keyNo)))
			bookmarkNo = keyNo;

		if(bookmarkNo != -1)
		{
			// store bookmark
			if (io.KeyCtrl)
				viewState.BookmarkAddress(bookmarkNo, cursorItem.AddressRef);
			// go to bookmark
			else if(io.KeyShift)
				viewState.GoToBookmarkAddress(bookmarkNo);
		}

		if (io.KeyCtrl && ImGui::IsKeyPressed((ImGuiKey)state.KeyConfig[(int)EKey::GoToAddress]))
		{
			ImGui::OpenPopup("Goto Address");
		}
	}
}

void MarkupHelpPopup()
{
	ImGui::SameLine();
	ImGui::Button("?");

	if (ImGui::IsItemHovered())
	{
		ImGui::BeginTooltip();
		ImGui::Text("Markup Syntax");
		ImGui::BulletText("#REG:N#");
		ImGui::BulletText("#ADDR:0xNNNN#");
		ImGui::Text("");
		ImGui::Text("Examples");
		ImGui::BulletText("#REG:B#");
		ImGui::BulletText("#REG:HL#");
		ImGui::BulletText("#ADDR:0x8000#");
		ImGui::EndTooltip();
	}
}

void UpdatePopups(FCodeAnalysisState& state, FCodeAnalysisViewState& viewState)
{
	const FCodeAnalysisItem& cursorItem = viewState.GetCursorItem();

	if (cursorItem.IsValid() == false)
		return;

	if (ImGui::BeginPopup("Enter Comment Text", ImGuiWindowFlags_AlwaysAutoResize))
	{
		ImGui::SetNextItemWidth(50 * ImGui::GetFontSize());

		ImGui::SetKeyboardFocusHere();
		if (ImGui::InputText("##comment", &cursorItem.Item->Comment, ImGuiInputTextFlags_EnterReturnsTrue))
		{
			ImGui::CloseCurrentPopup();
		}

		MarkupHelpPopup();

		ImGui::SetItemDefaultFocus();
		ImGui::EndPopup();
	}

	if (ImGui::BeginPopup("Enter Comment Text Multi", ImGuiWindowFlags_AlwaysAutoResize))
	{
		ImGui::SetNextItemWidth(50 * ImGui::GetFontSize());

		ImGui::SetKeyboardFocusHere();
		if(ImGui::InputTextMultiline("##comment", &cursorItem.Item->Comment,ImVec2(), ImGuiInputTextFlags_EnterReturnsTrue | ImGuiInputTextFlags_CtrlEnterForNewLine))
		{
			state.SetCodeAnalysisDirty(cursorItem.AddressRef);
			ImGui::CloseCurrentPopup();
		}

		MarkupHelpPopup();

		ImGui::SetItemDefaultFocus();
		ImGui::EndPopup();
	}

	if (ImGui::BeginPopup("Enter Label Text", ImGuiWindowFlags_AlwaysAutoResize))
	{
		FLabelInfo *pLabel = (FLabelInfo *)cursorItem.Item;
		
		ImGui::SetKeyboardFocusHere();
		std::string LabelText = pLabel->GetName();
		if (ImGui::InputText("##comment", &LabelText, ImGuiInputTextFlags_EnterReturnsTrue))
		{
			pLabel->ChangeName(LabelText.c_str(),cursorItem.AddressRef);
			ImGui::CloseCurrentPopup();
		}
		ImGui::SetItemDefaultFocus();
		ImGui::EndPopup();
	}

	if (ImGui::BeginPopupModal("Goto Address", NULL, ImGuiWindowFlags_AlwaysAutoResize))
	{
		ImGui::SetKeyboardFocusHere();
		const ImGuiInputTextFlags inputFlags = ((GetNumberDisplayMode() == ENumberDisplayMode::Decimal) ? ImGuiInputTextFlags_CharsDecimal : ImGuiInputTextFlags_CharsHexadecimal);
		const char* format = (GetNumberDisplayMode() == ENumberDisplayMode::Decimal) ? "%d" : "%04X";
		ImGui::InputScalar("##gotoaddresspopup", ImGuiDataType_U16, &viewState.JumpAddress, nullptr, nullptr, format, inputFlags);

		if (ImGui::IsKeyPressed(ImGuiKey_Enter))
		{
			GotoJumpAddress(state, viewState);
			ImGui::CloseCurrentPopup();
		}

		if (ImGui::IsKeyPressed(ImGuiKey_Escape))
			ImGui::CloseCurrentPopup();

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
	FCodeAnalysisBank* pBank = state.GetBank(builder.BankId);

	while (std::getline(stringStream, line, '\n'))
	{
		if (line.empty() || line[0] == '@')	// skip lines starting with @ - we might want to create items from them in future
			continue;

		FCommentLine* pLine = pBank->CommentLineAllocator.Allocate();
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

void UpdateItemListForBank(FCodeAnalysisState& state, FCodeAnalysisBank& bank, int startOffset)
{
	bank.ItemList.clear();
	bank.CommentLineAllocator.FreeAll();
	FItemListBuilder listBuilder(bank.ItemList);
	listBuilder.BankId = bank.Id;

	const int16_t page = bank.PrimaryMappedPage;
	const uint16_t bankPhysAddr = page * FCodeAnalysisPage::kPageSize;
	int nextItemAddress = 0;

	// This bank might start in the middle of an instruction from the previous bank
	int bankStart = startOffset;
	/* {
		FDataInfo* pDataInfo = &bank.Pages[0].DataInfo[bankStart];
		FCodeInfo* pCodeInfo = bank.Pages[0].CodeInfo[bankStart];
		while(pCodeInfo == nullptr && pDataInfo->DataType == EDataType::InstructionOperand)
		{
			bankStart++;
			pDataInfo = &bank.Pages[0].DataInfo[bankStart];
			pCodeInfo = bank.Pages[0].CodeInfo[bankStart];
		}
	}*/
	
	for (int bankAddr = bankStart; bankAddr < bank.NoPages * FCodeAnalysisPage::kPageSize; bankAddr++)
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
		//FCommentLine::FreeAll();	// recycle comment lines

		//int nextItemAddress = 0;

		auto& banks = state.GetBanks();
		int startOffset = 0;
		for (auto& bank : banks)
		{
			if (bank.bIsDirty || bank.ItemList.empty())
			{
				UpdateItemListForBank(state, bank, startOffset);
				bank.bIsDirty = false;
			}

			// calculate start offset for next bank
			const FCodeAnalysisItem& lastItem = bank.ItemList.back();
			const int itemEndAddr = lastItem.AddressRef.Address + lastItem.Item->ByteSize;
			const int bankEndAddr = (bank.PrimaryMappedPage + bank.NoPages) * FCodeAnalysisPage::kPageSize;
			startOffset = itemEndAddr - bankEndAddr;
		}
		int pageNo = 0;

		while (pageNo < FCodeAnalysisState::kNoPagesInAddressSpace)
		{
			int16_t bankId = state.GetBankFromAddress(pageNo * FCodeAnalysisPage::kPageSize);
			FCodeAnalysisBank* pBank = state.GetBank(bankId);
			if (pBank != nullptr)
			{
				state.ItemList.insert(state.ItemList.end(), pBank->ItemList.begin(), pBank->ItemList.end());
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
			const FLabelInfo* pLabel = state.GetLabelForAddress(item.AddressRef);

			if (ImGui::Selectable("Remove label"))
			{
				RemoveLabelAtAddress(state, item.AddressRef);
			}

			if(pLabel)
			{
				FEmuBase* pEmu = state.GetEmulator();

				if(pEmu->IsLabelStubbed(pLabel->GetName()) == false)
				{
					if (ImGui::Selectable("Stub from ASM export"))
						state.GetEmulator()->AddStubbedLabel(pLabel->GetName());
				}
				else
				{
					if (ImGui::Selectable("Remove stub from ASM export"))
						state.GetEmulator()->RemoveStubbedLabel(pLabel->GetName());
				}
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
			if (ImGui::Selectable("Run until here")) 
				state.Debugger.Continue(item.AddressRef);
		}
				
		if (ImGui::Selectable("View in graphics viewer"))
		{
			state.GetEmulator()->GraphicsViewerSetView(item.AddressRef);
		}

		if (ImGui::Selectable("View in character map viewer"))
		{
			state.GetEmulator()->CharacterMapViewerSetView(item.AddressRef);
		}


		ImGui::EndPopup();
	}
}

void DrawCodeAnalysisItem(FCodeAnalysisState& state, FCodeAnalysisViewState& viewState, const FCodeAnalysisItem& item)
{
	const uint16_t physAddr = item.AddressRef.Address;

	if (item.IsValid() == false)
		return;

	viewState.pLabelScope = state.GetScopeForAddress(item.AddressRef);

	// TODO: item below might need bank check
	bool bHighlight = (viewState.HighlightAddress.IsValid() && viewState.HighlightAddress.Address >= physAddr && viewState.HighlightAddress.Address < physAddr + item.Item->ByteSize);
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
		if (bSelected == false)	// item newly selected
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

			FFunctionInfo* pFunctionInfo = state.Functions.GetFunctionBeforeAddress(item.AddressRef);
			if (pFunctionInfo)
			{
				ImGui::Text("Function: %s", pFunctionInfo->Name.c_str()); 
			}
		}
	}
	DoItemContextMenu(state, item);

	const bool bDoubleClickOnItem = ImGui::IsItemHovered() && ImGui::IsMouseDoubleClicked(0);	// this will be used later for breakpoint toggling

	ImGui::SetItemAllowOverlap();	// allow buttons
	ImGui::SameLine();

	switch (item.Item->Type)
	{
	case EItemType::Label:
		DrawLabelInfo(state, viewState,item);
		break;
	case EItemType::Code:
		if (bHighlight)
			ImGui::PushStyleColor(ImGuiCol_Text, Colours::highlight);
		DrawCodeInfo(state, viewState, item);
		if (bHighlight)
			ImGui::PopStyleColor();
		break;
	case EItemType::Data:
		if (bHighlight)
			ImGui::PushStyleColor(ImGuiCol_Text, Colours::highlight);
		DrawDataInfo(state, viewState, item,false,state.bAllowEditing);
		if (bHighlight)
			ImGui::PopStyleColor();
		break;
	case EItemType::CommentLine:
		DrawComment(state,viewState,item.Item);
		//DrawCommentLine(state, static_cast<const FCommentLine*>(item.Item));
		break;
    default:
        break;
	}

	// double click to toggle breakpoints
	if (bDoubleClickOnItem && viewState.HighlightAddress.IsValid() == false)
	{
		if (item.Item->Type == EItemType::Code)
			state.ToggleExecBreakpointAtAddress(item.AddressRef);
		else if (item.Item->Type == EItemType::Data)
			state.ToggleDataBreakpointAtAddress(item.AddressRef, item.Item->ByteSize);
	}

	ImGui::PopID();
}

void DrawPalette(const uint32_t* palette, int numColours, float height)
{
	if (!height)
		height = ImGui::GetTextLineHeight();

	const ImVec2 size(height, height);

	for (int c = 0; c < numColours; c++)
	{
		ImGui::PushID(c);
		const uint32_t colour = *(palette + c);
		ImGui::ColorButton("##palette_color", ImColor(colour), ImGuiColorEditFlags_NoAlpha | ImGuiColorEditFlags_Uint8, size);
		ImGui::PopID();
		if (c < numColours - 1)
			ImGui::SameLine();
	}
}

EDataItemDisplayType GetDisplayTypeForBitmapFormat(EBitmapFormat bitmapFormat)
{
	switch (bitmapFormat)
	{
	case EBitmapFormat::Bitmap_1Bpp:
		return EDataItemDisplayType::Bitmap;
	case EBitmapFormat::ColMap2Bpp_CPC:
		return  EDataItemDisplayType::ColMap2Bpp_CPC;
	case EBitmapFormat::ColMap4Bpp_CPC:
		return  EDataItemDisplayType::ColMap4Bpp_CPC;
	case EBitmapFormat::ColMapMulticolour_C64:
		return  EDataItemDisplayType::ColMapMulticolour_C64;
	default:
		return EDataItemDisplayType::Unknown;
	}
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
        default:
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

			//if (ImGui::InputTextMultiline("##detailscomment", &pCommentBlock->Comment, ImGui::GetContentRegionAvail()))

			//ImGui::Text("Comments");
			if (ImGui::InputTextWithHint("Comments", "Comments", &commentString))
			{
				SetItemCommentText(state, item, commentString.c_str());
			}
			ImGui::Separator();
			ImGui::Text(commentString.c_str());
			ImGui::Separator();

			ImGui::Text("[");
			ImGui::SameLine();
			Markup::DrawText(state, viewState, commentString.c_str());
			ImGui::SameLine();
			ImGui::Text("]");

		}

	}
}

void DrawNavigationButtons(FCodeAnalysisState& state, FCodeAnalysisViewState& viewState)
{
	const float glyphWidth = ImGui_GetFontCharWidth();

	if (ImGui::ArrowButton("##btnprev", ImGuiDir_Left))
		viewState.GoToPreviousAddress();
	ImGui::SameLine();
	if (ImGui::ArrowButton("##btnnext", ImGuiDir_Right))
		viewState.GoToNextAddress();
	ImGui::SameLine();
	if (ImGui::Button("Jump To PC"))
	{
		//const FAddressRef PCAddress(state.GetBankFromAddress(state.CPUInterface->GetPC()), state.CPUInterface->GetPC());
		viewState.GoToAddress(state.CPUInterface->GetPC());
	}
	ImGui::SameLine();
	const ImGuiInputTextFlags inputFlags = (GetNumberDisplayMode() == ENumberDisplayMode::Decimal) ? ImGuiInputTextFlags_CharsDecimal : ImGuiInputTextFlags_CharsHexadecimal;
	const char* format = (GetNumberDisplayMode() == ENumberDisplayMode::Decimal) ? "%d" : "%04X";
	ImGui::Text("Jump To Address:");
	ImGui::SameLine();
	ImGui::SetNextItemWidth(glyphWidth * 10.0f);
	ImGui::InputScalar("##jumpaddressinput", ImGuiDataType_U16, &viewState.JumpAddress, nullptr, nullptr, format, inputFlags);
	const bool bEnteredJumpAddress = ImGui::IsItemFocused() && ImGui::IsKeyPressed(ImGuiKey_Enter, false);
	
	ImGui::SameLine();
	
	if (ImGui::Button("Go") || bEnteredJumpAddress)
	{
		GotoJumpAddress(state, viewState);
	}
}

void DrawHelpButton()
{
	ImGui::SameLine(ImGui::GetWindowWidth() - ImGui::GetFrameHeight());
	ImGui::Button("?");

	if (ImGui::IsItemHovered())
	{
		ImGui::BeginTooltip();
		ImGui::Text("Keyboard shortcuts");
		ImGui::SeparatorText("Item Type");
		ImGui::BulletText("c : Set as Code");
		ImGui::BulletText("d : Set as Data");
		ImGui::BulletText("t : Set as Text");
		ImGui::SeparatorText("Display Mode & Operand Type");
		ImGui::BulletText("a : Set as Ascii");
		ImGui::BulletText("b : Set as Binary");
		ImGui::BulletText("j : Set as Jump Address");
		ImGui::BulletText("n : Set as Number");
		ImGui::BulletText("p : Set as Pointer");
		ImGui::BulletText("u : Set as Unknown");
		ImGui::SeparatorText("Labels");
		ImGui::BulletText("l : Add label");
		ImGui::BulletText("r : Rename label");
		ImGui::SeparatorText("Comments");
		ImGui::BulletText("; : Add inline comment");
		ImGui::BulletText("Shift + ; : Add multi-line comment");
		ImGui::SeparatorText("Bookmarks");
		ImGui::BulletText("Ctrl + 1..5 : Store bookmark");
		ImGui::BulletText("Shift + 1..5 : Goto bookmark");
		ImGui::SeparatorText("Navigation");
		ImGui::BulletText("Ctrl + g : Jump to address");
		ImGui::EndTooltip();
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
	// note: this will not take effect until next frame due to the way ImGui works.
	if (gotoAddress.IsValid())
	{
		if (viewState.ViewingBankId == -1 || viewState.ViewingBankId == gotoAddress.BankId)
		{
			const float currScrollY = ImGui::GetScrollY();
			const float currWindowHeight = ImGui::GetWindowHeight();
			const int kJumpViewOffset = 5;
			for (int itemNo = 0; itemNo < (int)itemList.size(); itemNo++)
			{
				const FCodeAnalysisItem& item = itemList[itemNo];

				if ((item.AddressRef.Address >= gotoAddress.Address) && (viewState.GoToLabel || item.Item->Type != EItemType::Label) && item.Item->Type != EItemType::CommentLine)
				{
					// set cursor
					viewState.SetCursorItem(item);

					// we want the keyboard focus to be set on this item next frame.
					viewState.pKeyboardFocusItem = itemList[itemNo].Item;

					const float itemY = itemNo * lineHeight;
					const float margin = kJumpViewOffset * lineHeight;

					const float moveDist = itemY - currScrollY;

					if (moveDist > currWindowHeight)
					{
						const int gotoItem = std::max(itemNo - kJumpViewOffset, 0);
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
	}

	// draw clipped list
	ImGuiListClipper clipper;
	clipper.Begin((int)itemList.size(), lineHeight);
	std::vector<FAddressCoord> newList;

	while (clipper.Step())
	{
		viewState.JumpLineIndent = 0;

		for (int i = clipper.DisplayStart; i < clipper.DisplayEnd; i++)
		{
			const ImVec2 coord = ImGui::GetCursorScreenPos();
			if (itemList[i].Item->Type == EItemType::Code || itemList[i].Item->Type == EItemType::Data)
				newList.push_back({ itemList[i].AddressRef,coord.y });

			if (viewState.pKeyboardFocusItem == itemList[i].Item)
			{
				ImGui::SetKeyboardFocusHere();
				viewState.pKeyboardFocusItem = nullptr;
			}

			DrawCodeAnalysisItem(state, viewState, itemList[i]);
		}
	}

	viewState.AddressCoords = newList;
	viewState.pLabelScope = nullptr;
}

#define NEWBANKVIEW 1

void DrawBankAnalysis(FCodeAnalysisState& state, FCodeAnalysisViewState& viewState, int windowId)
{
	const float lineHeight = ImGui::GetTextLineHeight();
	const float lh2 = (float)(int)(lineHeight / 2);
	const float glyphWidth = ImGui_GetFontCharWidth();
	const float kListWidth = 24.0f * glyphWidth;
	const ImU32 brd_color = 0xFF000000;

	if (ImGui::BeginChild("##bankList", ImVec2(kListWidth, 0), true))
	{
		if (ImGui::Selectable("##addressspace", viewState.ViewingBankId == -1))
		{
			// Set selected bank
			viewState.ViewingBankId = -1;
		}
		const float lineStartX = ImGui::GetCursorPosX();
		ImGui::SameLine(lineStartX + 20);
		ImGui::Text(" Address Space");

		auto& banks = state.GetBanks();
		for (auto& bank : banks)
		{
			if (bank.PrimaryMappedPage == -1)
				continue;
			
			const bool bSelected = viewState.ViewingBankId == bank.Id;
			ImVec2 pos = ImGui::GetCursorScreenPos();
			ImDrawList* dl = ImGui::GetWindowDrawList();
			bool bRead = false;
			bool bWrite = false;

			const EBankAccess access = bank.GetBankMapping();
			ImU32 textCol = IM_COL32(144, 144, 144, 255);

			switch (access)
			{
			case EBankAccess::Read:
				textCol = 0xffffffff;
				bRead = true;
				break;
			case EBankAccess::Write:
				textCol = 0xffffffff;
				bWrite = true;
				break;
			case EBankAccess::ReadWrite:
				textCol = 0xffffffff;
				bRead = true;
				bWrite = true;
				break;
            default:
                break;
			}
			
			if (bWrite)
			{
				const ImVec2 a(pos.x + 2, pos.y);
				const ImVec2 b(pos.x + 12, pos.y + lh2);
				const ImVec2 c(pos.x + 2, pos.y + lineHeight);

				dl->AddTriangleFilled(a, b, c, 0xff0000ff);
				dl->AddTriangle(a, b, c, brd_color);
			}
			pos.x += 10;
			if (bRead)
			{
				const ImVec2 a(pos.x + 2, pos.y);
				const ImVec2 b(pos.x + 12, pos.y + lh2);
				const ImVec2 c(pos.x + 2, pos.y + lineHeight);

				dl->AddTriangleFilled(a, b, c, 0xff00ff00);
				dl->AddTriangle(a, b, c, brd_color);
			}
			ImGui::PushID(bank.Id);
			if (ImGui::Selectable("##bankselector", bSelected))
			{
				// Set selected bank
				viewState.ViewingBankId = bank.Id;
			}
			//ImGui::SameLine();
			const float lineStartX = ImGui::GetCursorPosX();
			ImGui::SameLine(lineStartX + 20);
			ImGui::PushStyleColor(ImGuiCol_Text, textCol);
			ImGui::Text(" %s", bank.Name.c_str());
			
			ImGui::PopStyleColor();
			ImGui::PopID();
		}
	}
	ImGui::EndChild();
	ImGui::SameLine();
	if (ImGui::BeginChild("##bankanalysis"))
	{
		FCodeAnalysisBank* pBank = state.GetBank(viewState.ViewingBankId);
		if (pBank != nullptr)
		{
			FCodeAnalysisBank&bank = *pBank;
			const uint16_t kBankStart = bank.PrimaryMappedPage * FCodeAnalysisPage::kPageSize;
			const uint16_t kBankEnd = kBankStart + (bank.NoPages * FCodeAnalysisPage::kPageSize) - 1;
			const bool bMapped = bank.IsMapped();

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
		else
		{
			//ImGui::Text("No bank selected");
			DrawItemList(state, viewState, state.ItemList);
			// only handle keypresses for focussed window
			if (state.FocussedWindowId == windowId)
				ProcessKeyCommands(state, viewState);
		}

	}
	ImGui::EndChild();

}

void DrawCodeAnalysisData(FCodeAnalysisState &state, int windowId)
{
	FCodeAnalysisViewState& viewState = state.ViewState[windowId];
	const float lineHeight = ImGui::GetTextLineHeight();
	//const float cellWidth = 3 * glyph_width;

	if (ImGui::IsWindowFocused(ImGuiHoveredFlags_ChildWindows))
		state.FocussedWindowId = windowId;

	viewState.HighlightAddress = viewState.HoverAddress;
	viewState.HoverAddress.SetInvalid();

	//if (state.Debugger.IsStopped() == false)
	//	state.CurrentFrameNo++;

	UpdateItemList(state);

	// TODO: separate function for navigation buttons
	//ImGui::Text("Navigation");
	//ImGui::SameLine();
	DrawNavigationButtons(state,viewState);

	DrawHelpButton();

	if (viewState.TrackPCFrame == true)
	{
		//const FAddressRef PCAddress(state.GetBankFromAddress(state.CPUInterface->GetPC()), state.CPUInterface->GetPC());
		viewState.GoToAddress(state.CPUInterface->GetPC());
		viewState.TrackPCFrame = false;
	}
	
	//ImGui::Text("Debugger");
	//ImGui::SameLine();
	DrawDebuggerButtons(state, viewState);
	

	ImGuiWindowFlags windowFlags = ImGuiWindowFlags_HorizontalScrollbar | ImGuiWindowFlags_AlwaysHorizontalScrollbar;
	if(ImGui::BeginChild("##analysis", ImVec2(ImGui::GetContentRegionAvail().x * 0.75f, 0), /*true*/ImGuiChildFlags_Border | ImGuiChildFlags_ResizeX, windowFlags))
	//if(ImGui::BeginChild("##analysis", ImVec2(ImGui::GetContentRegionAvail().x * 0.75f, 0), true))
	{
		// Determine if we want to switch tabs
		const FAddressRef& goToAddress = viewState.GetGotoAddress();
		int16_t showBank = -1;
		bool bSwitchViewBank = false;
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

			bSwitchViewBank = showBank != viewState.ViewingBankId;
		}

#if NEWBANKVIEW
		//ImGuiTabItemFlags tabFlags = (bSwitchTabs && showBank != -1) ? ImGuiTabItemFlags_SetSelected : 0;

		if (state.Config.bShowBanks)
		{
			if (bSwitchViewBank)
				viewState.ViewingBankId = showBank;

			DrawBankAnalysis(state, viewState, windowId);
		}
		else
		{
			if (ImGui::BeginChild("##itemlist"))
				DrawItemList(state, viewState, state.ItemList);
			// only handle keypresses for focussed window
			if (state.FocussedWindowId == windowId)
				ProcessKeyCommands(state, viewState);
			UpdatePopups(state, viewState);

			ImGui::EndChild();
		}
#else
		if (ImGui::BeginTabBar("itemlist_tabbar"))
		{
			ImGuiTabItemFlags tabFlags = (bSwitchViewBank && showBank == -1) ? ImGuiTabItemFlags_SetSelected : 0;

			if (ImGui::BeginTabItem("Address Space",nullptr,tabFlags))
			{
				if (bSwitchViewBank == false || showBank == -1)
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
					if (bank.PrimaryMappedPage == -1 || bank.bHidden == true)
						continue;

					const uint16_t kBankStart = bank.PrimaryMappedPage * FCodeAnalysisPage::kPageSize;
					const uint16_t kBankEnd = kBankStart + (bank.NoPages * FCodeAnalysisPage::kPageSize) - 1;

					tabFlags = (bSwitchViewBank && showBank == bank.Id) ? ImGuiTabItemFlags_SetSelected : 0;

					// TODO: Maybe we could colour code for read or write only access?
					const bool bMapped = bank.IsMapped();
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
						if (bSwitchViewBank == false || showBank == bank.Id)
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
#endif

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
				/*if (ImGui::BeginTabItem("Capture"))
				{
					DrawCaptureTab(state, viewState);
					ImGui::EndTabItem();
				}*/
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
				if (ImGui::BeginTabItem("Find"))
				{
					DrawFindTab(state, viewState);
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
		ImGuiListClipper clipper;
		clipper.Begin((int)labelList.size(), lineHeight);

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
					ImGui::Text("%s", pLabelInfo->GetName());
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
	FBatchDataFormattingOptions& batchFormattingOptions = viewState.BatchFormattingOptions;

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

	//static EDataType dataType = EDataType::Byte;
	DrawDataTypeCombo("Data Type", formattingOptions.DataType);   // TODSO: maybe pass in a list of supported types?

	switch (formattingOptions.DataType)
	{
	case EDataType::Byte:
		formattingOptions.ItemSize = 1;
		ImGui::InputInt("Item Count", &formattingOptions.NoItems);
		ImGui::Text("Display Mode:");
		ImGui::SameLine();
		ImGui::SetNextItemWidth(120.0f);
		DrawDataDisplayTypeCombo("##dataOperand", formattingOptions.DisplayType, state);
		break;
	case EDataType::ByteArray:
	{
		ImGui::InputInt("Array Size", &formattingOptions.ItemSize);
		ImGui::InputInt("Item Count", &formattingOptions.NoItems);
		ImGui::Text("Display Mode:");
		ImGui::SameLine();
		ImGui::SetNextItemWidth(120.0f);
		DrawDataDisplayTypeCombo("##dataOperand", formattingOptions.DisplayType, state);
		break;
	}
	case EDataType::Word:
		formattingOptions.ItemSize = 2;
		ImGui::InputInt("Item Count", &formattingOptions.NoItems);
		break;
	case EDataType::WordArray:
	{
		static int arraySize = 0;
		ImGui::InputInt("Array Size", &arraySize);
		ImGui::InputInt("Item Count", &formattingOptions.NoItems);
		formattingOptions.ItemSize = arraySize * 2;
		break;
	}
	case EDataType::Bitmap:
	{
		static int paletteNo = -1;
		if (DrawBitmapFormatCombo(viewState.CurBitmapFormat, state))
			paletteNo = -1;

		static int size[2];
		ImGui::InputInt2("Bitmap Size(X,Y)", size);
			
		formattingOptions.DisplayType = GetDisplayTypeForBitmapFormat(viewState.CurBitmapFormat);
		int pixelsPerByte = 8 / GetBppForBitmapFormat(viewState.CurBitmapFormat);
		formattingOptions.ItemSize = std::max(1, size[0] / pixelsPerByte);
		formattingOptions.NoItems = size[1];

		if (viewState.CurBitmapFormat != EBitmapFormat::Bitmap_1Bpp)
		{
			DrawPaletteCombo("Palette", "None", paletteNo, GetNumColoursForBitmapFormat(viewState.CurBitmapFormat));
			formattingOptions.PaletteNo = paletteNo;
		}
		break;
	}
	case EDataType::CharacterMap:
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
			ImGui::Checkbox("Register Char Map",&formattingOptions.RegisterItem);
		}
		break;
	case EDataType::ColAttr:
		ImGui::InputInt("Item Size", &formattingOptions.ItemSize);
		ImGui::InputInt("Item Count", &formattingOptions.NoItems);
		break;
	case EDataType::Text:
		ImGui::InputInt("Item Size", &formattingOptions.ItemSize);
		formattingOptions.NoItems = 1;
		break;
	case EDataType::Struct:
		state.GetDataTypes()->DrawStructComboBox("Struct", formattingOptions.StructId);
    default:
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
		ImGui::SameLine();
		if (ImGui::Button("Undo"))
		{
			UndoCommand(state);
		}

		if (ImGui::CollapsingHeader("Batch Format"))
		{
			//static int batchSize = 1;
			//static bool addLabel = false;
			//static bool addComment = false;
			//static char prefix[16];

			ImGui::InputInt("Count",&batchFormattingOptions.NoItems);
			ImGui::Checkbox("Add Label", &batchFormattingOptions.AddLabel);
			ImGui::SameLine();
			ImGui::Checkbox("Add Comment", &batchFormattingOptions.AddComment);
			ImGui::InputText("Prefix", &batchFormattingOptions.Prefix);
			if (ImGui::Button("Process Batch"))
			{
				batchFormattingOptions.FormatOptions = formattingOptions;	// copy formatting options
				BatchFormatData(state, batchFormattingOptions);
				/*
				for(int i=0;i<batchSize;i++)
				{
					char prefixTxt[32];
					snprintf(prefixTxt,32,"%s_%d",prefix,i);
					if(addLabel)
					{
						formattingOptions.AddLabelAtStart = true;
						formattingOptions.LabelName = prefixTxt;
					}
					if (addComment)
					{
						formattingOptions.AddCommentAtStart = true;
						formattingOptions.CommentText = prefixTxt;
					}

					FormatData(state, formattingOptions);
					state.AdvanceAddressRef(formattingOptions.StartAddress, formattingOptions.ItemSize* formattingOptions.NoItems);
					state.SetCodeAnalysisDirty(formattingOptions.StartAddress);
				}
				*/
			}

			//formattingOptions.AddCommentAtStart = false;
			//formattingOptions.CommentText = std::string();
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
			if (filter.bNoMachineRoms && pBank->bMachineROM)
				continue;
		}
		
		if (filter.DataType != EDataTypeFilter::All)
		{
			if (const FDataInfo* pDataInfo = state.GetDataInfoForAddress(labelItem.AddressRef))
			{
				switch (filter.DataType)
				{
				case EDataTypeFilter::Pointer:
					if (pDataInfo->DisplayType != EDataItemDisplayType::Pointer)
						continue;
						break;
				case EDataTypeFilter::Text:
					if (pDataInfo->DataType != EDataType::Text)
						continue;
					break;
				case EDataTypeFilter::Bitmap:
					if (pDataInfo->DataType != EDataType::Bitmap)
						continue;
					break;
				case EDataTypeFilter::CharacterMap:
					if (pDataInfo->DataType != EDataType::CharacterMap)
						continue;
					break;
				case EDataTypeFilter::ColAttr:
					if (pDataInfo->DataType != EDataType::ColAttr)
						continue;
					break;
                default:
                    break;
				}
			}
		}

		const FLabelInfo* pLabelInfo = static_cast<const FLabelInfo*>(labelItem.Item);
		std::string labelTextLower = pLabelInfo->GetName();
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
		viewState.GlobalFunctionsFilter.bNoMachineRoms = !viewState.ShowROMLabels;
		viewState.GlobalDataItemsFilter.bNoMachineRoms = !viewState.ShowROMLabels;
		state.bRebuildFilteredGlobalFunctions = true;
		state.bRebuildFilteredGlobalDataItems = true;
	}

	if(ImGui::BeginTabBar("GlobalsTabBar"))
	{
		if(ImGui::BeginTabItem("Functions"))
		{	
			// only constantly sort call frequency
			bool bSort = viewState.FunctionSortMode == EFunctionSortMode::CallFrequency;	
			if (ImGui::Combo("Sort Mode", (int*)&viewState.FunctionSortMode, "Location\0Alphabetical\0Call Frequency\0Num References"))
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
							return std::string(pLabelA->GetName()) < std::string(pLabelB->GetName());	// dodgy!
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
				case EFunctionSortMode::NoReferences:
					std::sort(viewState.FilteredGlobalFunctions.begin(), viewState.FilteredGlobalFunctions.end(), [&state](const FCodeAnalysisItem& a, const FCodeAnalysisItem& b)
						{
							const FLabelInfo* pLabelA = state.GetLabelForAddress(a.AddressRef);
							const FLabelInfo* pLabelB = state.GetLabelForAddress(b.AddressRef);
							return pLabelA->References.NumReferences() > pLabelB->References.NumReferences();
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
			if (DrawDataTypeFilterCombo("Data Type", viewState.DataTypeFilter))
			{
				viewState.GlobalDataItemsFilter.DataType = viewState.DataTypeFilter;
				state.bRebuildFilteredGlobalDataItems = true;
			}

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

void DrawFindTab(FCodeAnalysisState& state, FCodeAnalysisViewState& viewState)
{
	bool bActivateFind = ImGui::InputText("##findText", &viewState.FindText, ImGuiInputTextFlags_EnterReturnsTrue);
	ImGui::SameLine();
	bActivateFind |= ImGui::Button("Find");
	if(bActivateFind)
	{
		viewState.FindResults = state.FindInAnalysis(viewState.FindText.c_str(),viewState.SearchROM);
	}
	ImGui::SameLine();
	ImGui::Checkbox("Search ROM", &viewState.SearchROM);

	if(ImGui::BeginChild("FindResults"))
	{
		ImGuiListClipper clipper;
		clipper.Begin((int)viewState.FindResults.size());
		while (clipper.Step())
		{
			for (int i = clipper.DisplayStart; i < clipper.DisplayEnd; i++)
			{
				const auto& result = viewState.FindResults[i];
				ImGui::Text("%s ",NumStr(result.Address));
				DrawAddressLabel(state,viewState,result);
			}
		}
	}
	ImGui::EndChild();
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
	ImGui::SetNextItemWidth(ImGui::GetFontSize() * 3.f);
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

const char* GetBankText(const FCodeAnalysisState& state, int16_t bankId)
{
	const FCodeAnalysisBank* pBank = state.GetBank(bankId);

	if (pBank == nullptr)
		return "None";

	return pBank->Name.c_str();
}

bool DrawBankInput(const FCodeAnalysisState& state, const char* label, int16_t& bankId, bool bAllowNone)
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
				//const FCodeAnalysisBank* pNewBank = state.GetBank(bank.Id);
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

EBitmapFormat GetBitmapFormatForDisplayType(EDataItemDisplayType displayType)
{
	switch (displayType)
	{
	case EDataItemDisplayType::Bitmap:
		return EBitmapFormat::Bitmap_1Bpp;
	case EDataItemDisplayType::ColMap2Bpp_CPC:
		return EBitmapFormat::ColMap2Bpp_CPC;
	case EDataItemDisplayType::ColMap4Bpp_CPC:
		return EBitmapFormat::ColMap4Bpp_CPC;
	case  EDataItemDisplayType::ColMapMulticolour_C64:
		return EBitmapFormat::ColMapMulticolour_C64;
    default:
        return EBitmapFormat::None;
	}
}

int GetBppForBitmapFormat(EBitmapFormat bitmapFormat)
{
	switch (bitmapFormat)
	{
	case EBitmapFormat::Bitmap_1Bpp:
		return 1;
	case EBitmapFormat::ColMap2Bpp_CPC:
		return 2;
	case EBitmapFormat::ColMap4Bpp_CPC:
		return 4;
	case EBitmapFormat::ColMapMulticolour_C64:
		return 1;	// it's a bit of a bodge because they're wide
    default:
        return 1;
	}
	
}

bool BitmapFormatHasPalette(EBitmapFormat bitmapFormat)
{
	switch (bitmapFormat)
	{
	case EBitmapFormat::Bitmap_1Bpp:
		return false;
	case EBitmapFormat::ColMap2Bpp_CPC:
	case EBitmapFormat::ColMap4Bpp_CPC:
	case EBitmapFormat::ColMapMulticolour_C64:
		return true;	
    default:
        return false;
	}
}

int GetNumColoursForBitmapFormat(EBitmapFormat bitmapFormat)
{
	if(bitmapFormat == EBitmapFormat::ColMapMulticolour_C64)	// special case
		return 4;

	return 1 << GetBppForBitmapFormat(bitmapFormat);
}

bool IsBitmapFormatDoubleWidth(EBitmapFormat bitmapFormat)
{
	switch (bitmapFormat)
	{
	case EBitmapFormat::Bitmap_1Bpp:
	case EBitmapFormat::ColMap2Bpp_CPC:
	case EBitmapFormat::ColMapMulticolour_C64:
		return false;
	case EBitmapFormat::ColMap4Bpp_CPC:
		return true;
	default:
		return false;
	}
}


// Markup code
// -----------
// 
// tag format is <tagName>:<tagValue>
// E.g. ADDR:0x1234
namespace Markup
{
static const FCodeInfo* g_CodeInfo = nullptr;

void SetCodeInfo(const FCodeInfo* pCodeInfo)
{
	g_CodeInfo= pCodeInfo;
}

std::string ExpandTag(FCodeAnalysisState& state, const std::string& tag)
{
	const size_t tagNameEnd = tag.find(":");
	const std::string tagName = tag.substr(0, tagNameEnd);
	const std::string tagValue = tag.substr(tagNameEnd + 1);
	
	if (tagName == std::string("ADDR"))
	{
		int address = 0;
		if (sscanf(tagValue.c_str(), "0x%04x", &address) != 0)
		{
			return GenerateAddressLabelString(state,state.AddressRefFromPhysicalAddress((uint16_t)address));
		}
			//return std::string(NumStr((uint16_t)address));
	}
	else if (tagName == std::string("OPERAND_ADDR"))
	{
		const FCodeInfo* pCodeInfo = g_CodeInfo;
		if (pCodeInfo != nullptr)
		{
			if (g_CodeInfo->OperandAddress.IsValid())
			{
				return GenerateAddressLabelString(state, g_CodeInfo->OperandAddress);
			}
		}
	}
	else if (tagName == std::string("IM"))	// immediate
	{
		return tagValue;
	}
	else if (tagName == std::string("REG"))
	{
		return tagValue;
	}

	return std::string();
}

bool ProcessTag(FCodeAnalysisState& state, FCodeAnalysisViewState& viewState,const std::string& tag)
{
	const size_t tagNameEnd = tag.find(":");
	const std::string tagName = tag.substr(0, tagNameEnd);
	const std::string tagValue = tag.substr(tagNameEnd+1);
	bool bShownToolTip = false;

	if (tagName == std::string("ADDR"))
	{
		int address = 0;
		if(sscanf(tagValue.c_str(), "0x%04x",&address) != 0)
			bShownToolTip = DrawAddressLabel(state,viewState,state.AddressRefFromPhysicalAddress(address));
	}
	else if (tagName == std::string("OPERAND_ADDR"))
	{
		const FCodeInfo* pCodeInfo = g_CodeInfo;
		if(pCodeInfo != nullptr)
		{
			uint32_t labelFlags = kAddressLabelFlag_NoBank | kAddressLabelFlag_NoBrackets;
			//if(pCodeInfo->OperandType == EOperandType::Pointer)
				labelFlags |= kAddressLabelFlag_White;
			if(g_CodeInfo->OperandAddress.IsValid())
				bShownToolTip = DrawAddressLabel(state, viewState, g_CodeInfo->OperandAddress, labelFlags);
		}
	}
	else if (tagName == std::string("IM"))	// immediate
	{
		ImGui::SameLine(0, 0);
		ImGui::PushStyleColor(ImGuiCol_Text, Colours::immediate);
		ImGui::Text("%s", tagValue.c_str());
		ImGui::PopStyleColor();
	}
	else if (tagName == std::string("REG"))
	{
		char regName[4];
		if (sscanf(tagValue.c_str(), "%s",regName))
		{
			ImGui::SameLine(0,0);
			ImGui::PushStyleColor(ImGuiCol_Text, Colours::reg);
			ImGui::Text( "%s", regName);
			ImGui::PopStyleColor();
			// tooltip of register value
			if (ImGui::IsItemHovered())
			{
				uint8_t byteVal = 0;
				uint16_t wordVal = 0;
				ImGui::BeginTooltip();
				if (state.Debugger.GetRegisterByteValue(regName, byteVal))
				{
					ImGui::Text("%s: %s (%d,'%c')", regName, NumStr(byteVal, GetHexNumberDisplayMode()),byteVal,byteVal);
				}
				else if (state.Debugger.GetRegisterWordValue(regName, wordVal))
				{
					FAddressRef addr = state.AddressRefFromPhysicalAddress(wordVal);	// I think this should be OK
					ImGui::Text("%s: %s", regName, NumStr(wordVal));
					DrawAddressLabel(state, viewState, addr);
					// Bring up snippet in tool tip
					const int indentBkp = viewState.JumpLineIndent;
					viewState.JumpLineIndent = 0;
					DrawSnippetToolTip(state, viewState, addr);
					viewState.JumpLineIndent = indentBkp;
					viewState.HoverAddress = addr;
					if (ImGui::IsMouseDoubleClicked(0))
					{
						viewState.GoToAddress(addr);
					}
					
				}

				ImGui::EndTooltip();
				bShownToolTip = true;
			}
		}
	}

	return bShownToolTip;
}

bool DrawText(FCodeAnalysisState& state, FCodeAnalysisViewState& viewState,const char* pText)
{
	ImGui::BeginGroup();
	//std::string inString("This is at #ADDR:0x3456#");

	const char* pTxtPtr = pText;
	bool bInTag = false;
	bool bToolTipshown = false;
	bool bCarriageReturn = false;

	// temp string on stack
	const int kMaxStringSize = 64;
	char str[kMaxStringSize + 1];
	int strPos = 0;

	std::string tag;
		
	while (*pTxtPtr != 0)
	{
		const char ch = *pTxtPtr++;

		if (ch == '\\')	// escape char
		{
			str[strPos++] = *pTxtPtr++;
			if (strPos == kMaxStringSize)
			{
				str[strPos] = 0;
				strPos = 0;
			}
			continue;
		}

		if (bInTag == false)
		{
			if (ch == '#')	// start tag
			{
				bInTag = true;
				tag.clear();
			}
			else if (ch == '\n')
			{
				bCarriageReturn = true;
			}
			else
			{
				str[strPos++] = ch;	// add to string
			}
		}
		else
		{
			if (ch == '#')	// finish tag
			{
				bToolTipshown |= ProcessTag(state,viewState,tag);
				bInTag = false;
			}
			else
			{
				tag += ch;	// add to tag
			}
		}

		if (strPos == kMaxStringSize || (bInTag && strPos != 0) || *pTxtPtr == 0 || bCarriageReturn)
		{
			str[strPos] = 0;
			strPos = 0;
			ImGui::SameLine(0,0);
			ImGui::Text("%s", str);
			if (bCarriageReturn)
			{
				ImGui::NewLine();
				bCarriageReturn = false;
			}
		}
	}

	ImGui::EndGroup();

	return bToolTipshown;
}

std::string ExpandString(FCodeAnalysisState& state, const char* pText)
{
	const char* pTxtPtr = pText;
	bool bInTag = false;
	std::string outString;

	// temp string on stack
	const int kMaxStringSize = 64;
	char str[kMaxStringSize + 1];
	int strPos = 0;

	std::string tag;

	while (*pTxtPtr != 0)
	{
		const char ch = *pTxtPtr++;

		// dont write escape character
		/*if (ch == '\\')	
		{
			*pTxtPtr++;
			if (strPos == kMaxStringSize)
			{
				str[strPos] = 0;
				strPos = 0;
			}
			continue;
		}*/

		if (bInTag == false)
		{
			if (ch == '#')	// start tag
			{
				bInTag = true;
				tag.clear();
			}
			else
			{
				str[strPos++] = ch;	// add to string
			}
		}
		else
		{
			if (ch == '#')	// finish tag
			{
				outString += ExpandTag(state,tag);
				bInTag = false;
			}
			else
			{
				tag += ch;	// add to tag
			}
		}

		if (strPos == kMaxStringSize || (bInTag && strPos != 0) || *pTxtPtr == 0)
		{
			str[strPos] = 0;
			strPos = 0;
			outString += str;
			//ImGui::SameLine(0, 0);
			//ImGui::Text("%s", str);
		}
	}

	return outString;
}

}// namespace Markup
