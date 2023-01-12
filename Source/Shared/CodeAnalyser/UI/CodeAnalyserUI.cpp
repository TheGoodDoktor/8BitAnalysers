#include "CodeAnalyserUI.h"
#include "CharacterMapViewer.h"
#include "../CodeAnalyser.h"

#include "Util/Misc.h"
#include "Util/GraphicsView.h"
#include "ImageViewer.h"


#include "imgui.h"
#include "Vendor/imgui-docking/imgui_internal.h"
#include "misc/cpp/imgui_stdlib.h"
#include <algorithm>
#include <sstream>
#include "chips/z80.h"
#include "CodeToolTips.h"

// UI
void DrawCodeAnalysisItemAtIndex(FCodeAnalysisState& state, FCodeAnalysisViewState& viewState, int i);

void GoToAddress(FCodeAnalysisViewState&state, uint16_t newAddress, bool bLabel = false)
{
	if(state.pCursorItem != nullptr)
		state.AddressStack.push_back(state.pCursorItem->Address);
	state.GoToAddress = newAddress;
	state.GoToLabel = bLabel;
}

void CodeAnalyserGoToAddress(FCodeAnalysisViewState& state, uint16_t newAddress, bool bLabel)
{
	GoToAddress(state, newAddress, bLabel);
}

bool GoToPreviousAddress(FCodeAnalysisViewState&state)
{
	if (state.AddressStack.empty())
		return false;

	state.GoToAddress = state.AddressStack.back();
	state.GoToLabel = false;
	state.AddressStack.pop_back();
	return true;
}

int GetItemIndexForAddress(const FCodeAnalysisState &state, uint16_t addr)
{
	int index = -1;
	for(int i=0;i<(int)state.ItemList.size();i++)
	{
		if (state.ItemList[i] != nullptr && state.ItemList[i]->Address > addr)
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

void DrawAddressLabel(FCodeAnalysisState &state, FCodeAnalysisViewState& viewState, uint16_t addr, bool bFunctionRel)
{
	int labelOffset = 0;
	const char *pLabelString = GetRegionDesc(addr);

	if (pLabelString == nullptr)	// get a label
	{
		// find a label for this address
		for (int addrVal = addr; addrVal >= 0; addrVal--)
		{
			const FLabelInfo* pLabel = state.GetLabelForAddress(addrVal);
			if (pLabel != nullptr)
			{
				if (bFunctionRel == false || pLabel->LabelType == LabelType::Function)
				{
					pLabelString = pLabel->Name.c_str();
					break;
				}
			}

			labelOffset++;
		}
	}
	
	if (pLabelString != nullptr)
	{
		ImGui::SameLine();
		ImGui::PushStyleColor(ImGuiCol_Text, 0xff808080);
		if(labelOffset == 0)
			ImGui::Text("[%s]", pLabelString);
		else
			ImGui::Text("[%s + %d]", pLabelString, labelOffset);

		if (ImGui::IsItemHovered())
		{
			// Bring up snippet in tool tip
			const int index = GetItemIndexForAddress(state, addr);
			if(index !=-1)
			{
				const int kToolTipNoLines = 10;
				ImGui::BeginTooltip();
				const int startIndex = std::max(index - (kToolTipNoLines / 2), 0);
				for(int line=0;line < kToolTipNoLines;line++)
				{
					if(startIndex + line < (int)state.ItemList.size())
						DrawCodeAnalysisItemAtIndex(state,viewState,startIndex + line);
				}
				ImGui::EndTooltip();
			}
			

			ImGuiIO& io = ImGui::GetIO();
			if (io.KeyShift && ImGui::IsMouseDoubleClicked(0))
				GoToAddress(state.GetAltViewState(), addr, false);
			else if (ImGui::IsMouseDoubleClicked(0))
				GoToAddress(viewState, addr, false);
		
			viewState.HoverAddress = addr;
		}

		ImGui::PopStyleColor();
	}
}

void DrawCodeAddress(FCodeAnalysisState &state, FCodeAnalysisViewState& viewState, uint16_t addr, bool bFunctionRel)
{
	//ImGui::PushStyleColor(ImGuiCol_Text, 0xff00ffff);
	ImGui::Text("%s", NumStr(addr));
	//ImGui::PopStyleColor();
	ImGui::SameLine();
	DrawAddressLabel(state, viewState, addr, bFunctionRel);
}

void DrawCallStack(FCodeAnalysisState& state)
{
	FCodeAnalysisViewState& viewState = state.GetFocussedViewState();
	if (state.CallStack.empty() == false)
	{
		const FLabelInfo* pLabel = state.GetLabelForAddress(state.CallStack.back().FunctionAddr);
		if (pLabel != nullptr)
		{
			ImGui::Text("%s :", pLabel->Name.c_str());
			ImGui::SameLine();
		}
	}
	DrawCodeAddress(state, viewState, state.CPUInterface->GetPC(), false);	// draw current PC

	for (int i = (int)state.CallStack.size() - 1; i >= 0; i--)
	{
		if (i > 0)
		{
			const FLabelInfo* pLabel = state.GetLabelForAddress(state.CallStack[i-1].FunctionAddr);
			if (pLabel != nullptr)
			{
				ImGui::Text("%s :", pLabel->Name.c_str());
				ImGui::SameLine();
			}
		}
		DrawCodeAddress(state, viewState, state.CallStack[i].CallAddr, false);
	}
}

void DrawTrace(FCodeAnalysisState& state)
{
	FCodeAnalysisViewState& viewState = state.GetFocussedViewState();
	const float line_height = ImGui::GetTextLineHeight();
	ImGuiListClipper clipper((int)state.FrameTrace.size(), line_height);

	while (clipper.Step())
	{
		for (int i = clipper.DisplayStart; i < clipper.DisplayEnd; i++)
		{
			DrawCodeAddress(state, viewState, state.FrameTrace[state.FrameTrace.size() - i - 1], false);	// draw current PC
		}
	}
	
}


void DrawRegisters_Z80(FCodeAnalysisState& state);

void DrawRegisters(FCodeAnalysisState& state)
{
	if (state.CPUInterface->CPUType == ECPUType::Z80)
		DrawRegisters_Z80(state);
}

void DrawWatchWindow(FCodeAnalysisState& state)
{
	FCodeAnalysisViewState& viewState = state.GetFocussedViewState();
	static int selectedWatch = -1;
	bool bDeleteSelectedWatch = false;

	for (const auto& watch : state.GetWatches())
	{
		const FDataInfo* pDataInfo = state.GetReadDataInfoForAddress(watch);
		ImGui::PushID(watch);
		if (ImGui::Selectable("##watchselect", watch == selectedWatch, 0))
		{
			selectedWatch = watch;
		}
		if (selectedWatch != -1 && ImGui::BeginPopupContextItem("watch context menu"))
		{
			if (ImGui::Selectable("Delete Watch"))
			{
				bDeleteSelectedWatch = true;
			}
			if (ImGui::Selectable("Toggle Breakpoint"))
			{
				FDataInfo* pInfo = state.GetWriteDataInfoForAddress(selectedWatch);
				state.CPUInterface->ToggleDataBreakpointAtAddress(pInfo->Address, pInfo->ByteSize);
			}

			ImGui::EndPopup();
		}
		ImGui::SetItemAllowOverlap();	// allow buttons
		ImGui::SameLine();
		DrawDataInfo(state, viewState, pDataInfo, true,true);

		// TODO: Edit Watch
		ImGui::PopID();		
	}

	if(bDeleteSelectedWatch)
		state.RemoveWatch(selectedWatch);
}

void DrawComment(const FItem *pItem, float offset)
{
	if(pItem != nullptr && pItem->Comment.empty() == false)
	{
		ImGui::SameLine(offset);
		ImGui::PushStyleColor(ImGuiCol_Text, 0xff008000);
		ImGui::Text("\t// %s", pItem->Comment.c_str());
		ImGui::PopStyleColor();
	}
}

void DrawLabelInfo(FCodeAnalysisState &state, FCodeAnalysisViewState& viewState, const FLabelInfo *pLabelInfo)
{
	const FCodeInfo* pCodeInfo = state.GetCodeInfoForAddress(pLabelInfo->Address);	// for self-modifying code

	ImVec4 labelColour = ImVec4(0.5f, 0.5f, 0.5f, 1.0f);
	if (viewState.HighlightAddress == pLabelInfo->Address)
		labelColour = ImVec4(0.0f, 1.0f, 0.0f, 1.0f);
	else if (pLabelInfo->Global || pLabelInfo->LabelType == LabelType::Function)
		labelColour = ImVec4(1.0f, 1.0f, 1.0f, 1.0f);
	

	// draw SMC fixups differently
	if (pCodeInfo!=nullptr && pCodeInfo->Address != pLabelInfo->Address)	// label doesn't point to first byte of instruction
	{
		ImGui::TextColored(labelColour, "\t\tOperand Fixup(% s) :",NumStr(pLabelInfo->Address));
		ImGui::SameLine();
		ImGui::TextColored(labelColour, "%s", pLabelInfo->Name.c_str());
	}
	else
	{
		ImGui::TextColored(labelColour, "%s: ", pLabelInfo->Name.c_str());
	}

	// hover tool tip
	if (ImGui::IsItemHovered() && pLabelInfo->References.empty() == false)
	{
		ImGui::BeginTooltip();
		ImGui::Text("References:");
		for (const auto & caller : pLabelInfo->References)
		{
			const uint16_t accessorCodeAddr = caller.first;
			ShowCodeAccessorActivity(state, accessorCodeAddr);

			ImGui::Text("   ");
			ImGui::SameLine();
			DrawCodeAddress(state, viewState, accessorCodeAddr);
		}
		ImGui::EndTooltip();
	}

	DrawComment(pLabelInfo);	
}

void DrawLabelDetails(FCodeAnalysisState &state, FCodeAnalysisViewState& viewState, FLabelInfo *pLabelInfo)
{
	static std::string oldLabelString;
	static std::string labelString;
	static FItem *pCurrItem = nullptr;
	if (pCurrItem != pLabelInfo)
	{
		oldLabelString = labelString = pLabelInfo->Name;
		pCurrItem = pLabelInfo;
	}

	//ImGui::Text("Comments");
	if (ImGui::InputText("Name", &labelString))
	{
		if (labelString.empty())
			labelString = oldLabelString;

		SetLabelName(state, pLabelInfo, labelString.c_str());
	}

	if(ImGui::Checkbox("Global", &pLabelInfo->Global))
	{
		if (pLabelInfo->LabelType == LabelType::Code && pLabelInfo->Global == true)
			pLabelInfo->LabelType = LabelType::Function;
		if (pLabelInfo->LabelType == LabelType::Function && pLabelInfo->Global == false)
			pLabelInfo->LabelType = LabelType::Code;
		GenerateGlobalInfo(state);
	}

	ImGui::Text("References:");
	for (const auto & caller : pLabelInfo->References)
	{
		const uint16_t accessorCodeAddr = caller.first;
		ShowCodeAccessorActivity(state, accessorCodeAddr);

		ImGui::Text("   ");
		ImGui::SameLine();
		DrawCodeAddress(state, viewState, accessorCodeAddr);
	}
}

void ShowCodeAccessorActivity(FCodeAnalysisState& state, const uint16_t accessorCodeAddr)
{
	const FCodeInfo* pCodeInfo = state.GetCodeInfoForAddress(accessorCodeAddr);
	if (pCodeInfo != nullptr)
	{

		const int framesSinceAccessed = state.CurrentFrameNo - pCodeInfo->FrameLastAccessed;
		const int brightVal = (255 - std::min(framesSinceAccessed << 2, 255)) & 0xff;

		if (brightVal > 0)
		{
			const float line_height = ImGui::GetTextLineHeight();
			const ImU32 col = 0xff000000 | (brightVal << 16) | (brightVal << 8) | (brightVal << 0);

			const ImU32 pc_color = 0xFF00FFFF;
			const ImU32 brd_color = 0xFF000000;

			ImVec2 pos = ImGui::GetCursorScreenPos();
			ImDrawList* dl = ImGui::GetWindowDrawList();
			const float lh2 = (float)(int)(line_height / 2);

			pos.x += 10;
			const ImVec2 a(pos.x + 2, pos.y);
			const ImVec2 b(pos.x + 12, pos.y + lh2);
			const ImVec2 c(pos.x + 2, pos.y + line_height);

			dl->AddTriangleFilled(a, b, c, col);
			dl->AddTriangle(a, b, c, brd_color);
		}
	}
}

void DrawCodeInfo(FCodeAnalysisState &state, FCodeAnalysisViewState& viewState, const FCodeInfo *pCodeInfo)
{
	const float line_height = ImGui::GetTextLineHeight();
	const float glyph_width = ImGui::CalcTextSize("F").x;
	const float cell_width = 3 * glyph_width;

	const int framesSinceAccessed = state.CurrentFrameNo - pCodeInfo->FrameLastAccessed;
	const int brightVal = (255 - std::min(framesSinceAccessed << 2, 255)) & 0xff;

	const ImU32 col = 0xff000000 | (brightVal << 16) | (brightVal << 8) | (brightVal << 0);

	const bool bPCLine = pCodeInfo->Address == state.CPUInterface->GetPC();// z80_pc(&state.pSpeccy->CurrentState.cpu);

	if (bPCLine || brightVal > 0)
	{
		const ImU32 pc_color = 0xFF00FFFF;
		const ImU32 brd_color = 0xFF000000;

		ImVec2 pos = ImGui::GetCursorScreenPos();
		ImDrawList* dl = ImGui::GetWindowDrawList();
		const float lh2 = (float)(int)(line_height / 2);

		pos.x += 10;
		const ImVec2 a(pos.x + 2, pos.y);
		const ImVec2 b(pos.x + 12, pos.y + lh2);
		const ImVec2 c(pos.x + 2, pos.y + line_height);

		if (bPCLine)
		{
			dl->AddTriangleFilled(a, b, c, pc_color);
			dl->AddTriangle(a, b, c, brd_color);
		}
		else
		{
			dl->AddTriangleFilled(a, b, c, col);
			dl->AddTriangle(a, b, c, brd_color);
		}
	}

	// show if breakpointed
	if (state.CPUInterface->IsAddressBreakpointed(pCodeInfo->Address))
	{
		const ImU32 bp_enabled_color = 0xFF0000FF;
		const ImU32 bp_disabled_color = 0xFF000088;
		const ImU32 brd_color = 0xFF000000;
		ImDrawList* dl = ImGui::GetWindowDrawList();
		const ImVec2 pos = ImGui::GetCursorScreenPos();
		const float lh2 = (float)(int)(line_height / 2);
		const ImVec2 mid(pos.x + 7, pos.y + lh2);
		
		dl->AddCircleFilled(mid, 7, bp_enabled_color);
		dl->AddCircle(mid, 7, brd_color);
	}

	//ImGui::PushStyleColor(ImGuiCol_Text, col);
	//ImGui::Text(">> ");
	//ImGui::SameLine();
	//ImGui::PopStyleColor();
	if(pCodeInfo->bSelfModifyingCode == true || pCodeInfo->Text.empty())
	{
		//UpdateCodeInfoForAddress(state, pCodeInfo->Address);
		WriteCodeInfoForAddress(state, pCodeInfo->Address);
	}

	ImGui::Text("\t%s", NumStr(pCodeInfo->Address));
	const float line_start_x = ImGui::GetCursorPosX();
	ImGui::SameLine(line_start_x + cell_width * 4 + glyph_width * 2);

	// grey out NOPed code
	if(pCodeInfo->bNOPped)
		ImGui::PushStyleColor(ImGuiCol_Text, 0xff808080);

	ImGui::Text("%s", pCodeInfo->Text.c_str());

	if (pCodeInfo->bNOPped)
		ImGui::PopStyleColor();

	if(ImGui::IsItemHovered())
	{
		ShowCodeToolTip(state, pCodeInfo);
	}

	// draw jump address label name
	if (pCodeInfo->OperandType == EOperandType::JumpAddress)
	{
		DrawAddressLabel(state, viewState, pCodeInfo->JumpAddress);
	}
	else if (pCodeInfo->OperandType == EOperandType::Pointer)
	{
		DrawAddressLabel(state, viewState, pCodeInfo->PointerAddress);
	}

	DrawComment(pCodeInfo);

}

void DrawCodeDetails(FCodeAnalysisState &state, FCodeAnalysisViewState& viewState, FCodeInfo *pCodeInfo)
{
	if (DrawOperandTypeCombo("Operand Type", pCodeInfo->OperandType))
		pCodeInfo->Text.clear();	// clear for a rewrite

	if (ImGui::Checkbox("NOP out instruction", &pCodeInfo->bNOPped))
	{
		if (pCodeInfo->bNOPped == true)
		{
			assert(pCodeInfo->ByteSize <= sizeof(pCodeInfo->OpcodeBkp));
			// backup code
			for (int i = 0; i < pCodeInfo->ByteSize; i++)
			{
				pCodeInfo->OpcodeBkp[i] = state.CPUInterface->ReadByte(pCodeInfo->Address + i);

				// NOP it out
				if(state.CPUInterface->CPUType == ECPUType::Z80)
					state.CPUInterface->WriteByte(pCodeInfo->Address + i,0);
				else if(state.CPUInterface->CPUType == ECPUType::M6502)
					state.CPUInterface->WriteByte(pCodeInfo->Address + i, 0xEA);
			}
		}
		else
		{
			// Restore
			for (int i = 0; i < pCodeInfo->ByteSize; i++)
				state.CPUInterface->WriteByte(pCodeInfo->Address + i, pCodeInfo->OpcodeBkp[i]);

		}
	}

	if (pCodeInfo->bSelfModifyingCode == true)
	{
		ImGui::Text("Self modifying code");

		for (int i = 1; i < pCodeInfo->ByteSize; i++)
		{
			FDataInfo* pOperandData = state.GetWriteDataInfoForAddress(pCodeInfo->Address + i);
			if (pOperandData->Writes.empty() == false)
			{
				ImGui::Text("Operand Writes:");
				for (const auto& caller : pOperandData->Writes)
				{
					DrawCodeAddress(state, viewState, caller.first);
				}
				break;
			}
		}
	}
}


void DrawCommentLine(FCodeAnalysisState& state, const FCommentLine* pCommentLine)
{
	ImGui::SameLine();
	ImGui::PushStyleColor(ImGuiCol_Text, 0xff008000);
	ImGui::Text("; %s", pCommentLine->Comment.c_str());
	ImGui::PopStyleColor();
}

void DrawCommentBlockDetails(FCodeAnalysisState& state, FCommentBlock* pCommentBlock)
{
	if (ImGui::InputTextMultiline("Comment Text", &pCommentBlock->Comment))
	{
		if (pCommentBlock->Comment.empty() == true)
			state.SetCommentBlockForAddress(pCommentBlock->Address, nullptr);
		state.bCodeAnalysisDataDirty = true;
	}

}

int CommentInputCallback(ImGuiInputTextCallbackData *pData)
{
	return 1;
}


void ProcessKeyCommands(FCodeAnalysisState& state, FCodeAnalysisViewState& viewState)
{
	ImGuiIO& io = ImGui::GetIO();
	if (io.WantTextInput)
		return;

	if (ImGui::IsWindowFocused() && viewState.pCursorItem != nullptr)
	{

		if (ImGui::IsKeyPressed(state.KeyConfig[(int)Key::SetItemCode]))
		{
			SetItemCode(state, viewState.pCursorItem);
		}
		else if (ImGui::IsKeyPressed(state.KeyConfig[(int)Key::SetItemData]))
		{
			SetItemData(state, viewState.pCursorItem);
		}
		else if (ImGui::IsKeyPressed(state.KeyConfig[(int)Key::SetItemText]))
		{
			SetItemText(state, viewState.pCursorItem);
		}
#ifdef ENABLE_IMAGE_TYPE
		else if (ImGui::IsKeyPressed(state.KeyConfig[(int)Key::SetItemImage]))
		{
			SetItemImage(state, state.pCursorItem);
		}
#endif
		else if (ImGui::IsKeyPressed(state.KeyConfig[(int)Key::ToggleItemBinary]))
		{
			if (viewState.pCursorItem->Type == ItemType::Data)
			{
				FDataInfo* pDataItem = static_cast<FDataInfo*>(viewState.pCursorItem);
				if (pDataItem->DataType != DataType::Bitmap)
					pDataItem->DataType = DataType::Bitmap;
				else
					pDataItem->DataType = DataType::Byte;
				//pDataItem->bShowBinary = !pDataItem->bShowBinary;
			}
		}
		else if (ImGui::IsKeyPressed(state.KeyConfig[(int)Key::AddLabel]))
		{
			AddLabelAtAddress(state, viewState.pCursorItem->Address);
		}
		else if (ImGui::IsKeyPressed(state.KeyConfig[(int)Key::Comment]))
		{
			ImGui::OpenPopup("Enter Comment Text");
			ImGui::SetWindowFocus("Enter Comment Text");
		}
		else if (viewState.pCursorItem->Type == ItemType::Label && ImGui::IsKeyPressed(state.KeyConfig[(int)Key::Rename]))
		{
			//AddLabelAtAddress(state, state.pCursorItem->Address);
			ImGui::OpenPopup("Enter Label Text");
			ImGui::SetWindowFocus("Enter Label Text");
		}
		else if (ImGui::IsKeyPressed(state.KeyConfig[(int)Key::AddCommentBlock]))
		{
			FCommentBlock* pCommentBlock = AddCommentBlock(state, viewState.pCursorItem->Address);
			viewState.pCursorItem = pCommentBlock;
			ImGui::OpenPopup("Enter Comment Text Multi");
			ImGui::SetWindowFocus("Enter Comment Text Multi");
		}
		else if (ImGui::IsKeyPressed(state.KeyConfig[(int)Key::Breakpoint]))
		{
			if (viewState.pCursorItem->Type == ItemType::Data)
				state.CPUInterface->ToggleDataBreakpointAtAddress(viewState.pCursorItem->Address, viewState.pCursorItem->ByteSize);
			else if (viewState.pCursorItem->Type == ItemType::Code)
				state.CPUInterface->ToggleExecBreakpointAtAddress(viewState.pCursorItem->Address);
		}
	}

	if (ImGui::IsKeyPressed(state.KeyConfig[(int)Key::BreakContinue]))
	{
		if (state.CPUInterface->ShouldExecThisFrame())
		{
			state.CPUInterface->Break();
			//viewState.TrackPCFrame = true;
		}
		else
		{
			state.CPUInterface->Continue();
		}
	}
	else if (ImGui::IsKeyPressed(state.KeyConfig[(int)Key::StepOver]))
	{
		state.CPUInterface->StepOver();
	}
	else if (ImGui::IsKeyPressed(state.KeyConfig[(int)Key::StepInto]))
	{
		state.CPUInterface->StepInto();
	}
	else if (ImGui::IsKeyPressed(state.KeyConfig[(int)Key::StepFrame]))
	{
		state.CPUInterface->StepFrame();
	}
	else if (ImGui::IsKeyPressed(state.KeyConfig[(int)Key::StepScreenWrite]))
	{
		state.CPUInterface->StepScreenWrite();
	}
}

void UpdatePopups(FCodeAnalysisState& state, FCodeAnalysisViewState& viewState)
{
	if (ImGui::BeginPopup("Enter Comment Text", ImGuiWindowFlags_AlwaysAutoResize))
	{
		ImGui::SetKeyboardFocusHere();
		if (ImGui::InputText("##comment", &viewState.pCursorItem->Comment, ImGuiInputTextFlags_EnterReturnsTrue))
		{
			ImGui::CloseCurrentPopup();
		}
		ImGui::SetItemDefaultFocus();
		ImGui::EndPopup();
	}

	if (ImGui::BeginPopup("Enter Comment Text Multi", ImGuiWindowFlags_AlwaysAutoResize))
	{
		ImGui::SetKeyboardFocusHere();
		if(ImGui::InputTextMultiline("##comment", &viewState.pCursorItem->Comment,ImVec2(), ImGuiInputTextFlags_EnterReturnsTrue | ImGuiInputTextFlags_CtrlEnterForNewLine))
		{
			state.bCodeAnalysisDataDirty = true;
			ImGui::CloseCurrentPopup();
		}
		ImGui::SetItemDefaultFocus();
		ImGui::EndPopup();
	}

	if (ImGui::BeginPopup("Enter Label Text", ImGuiWindowFlags_AlwaysAutoResize))
	{
		FLabelInfo *pLabel = (FLabelInfo *)viewState.pCursorItem;
		
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

void UpdateItemList(FCodeAnalysisState &state)
{
	// build item list - not every frame please!
	if (state.bCodeAnalysisDataDirty)
	{
		const float line_height = ImGui::GetTextLineHeight();
		
		state.ItemList.clear();
		FCommentLine::FreeAll();	// recycle comment lines

		int nextItemAddress = 0;

		// special case for expanded lines
		// TODO: there should be a more general case
		FCommentBlock* viewStateCommentBlocks[FCodeAnalysisState::kNoViewStates] = { nullptr };
		for (int i = 0; i < FCodeAnalysisState::kNoViewStates; i++)
		{
			if (state.ViewState[i].pCursorItem != nullptr && state.ViewState[i].pCursorItem->Type == ItemType::CommentLine)
			{
				FCommentBlock* pBlock = state.GetCommentBlockForAddress(state.ViewState[i].pCursorItem->Address);
				viewStateCommentBlocks[i] = pBlock;
			}
		}

		// loop across address range
		for (int addr = 0; addr < (1 << 16); addr++)
		{
			// convert comment block into multiple comment lines
			FCommentBlock* pCommentBlock = state.GetCommentBlockForAddress(addr);
			if (pCommentBlock != nullptr)
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
					pLine->Address = addr;
					state.ItemList.push_back(pLine);	
					if (pFirstLine == nullptr)
						pFirstLine = pLine;
				}

				// fix up having comment blocks as cursor items
				for (int i = 0; i < FCodeAnalysisState::kNoViewStates; i++)
				{
					if (viewStateCommentBlocks[i] == pCommentBlock)
						state.ViewState[i].pCursorItem = pFirstLine;
				}
			}
			
			FLabelInfo* pLabelInfo = state.GetLabelForAddress(addr);
			if (pLabelInfo != nullptr)
			{
				state.ItemList.push_back(pLabelInfo);
			}

			// check if we have gone past this item
			if (addr >= nextItemAddress)
			{
				FCodeInfo *pCodeInfo = state.GetCodeInfoForAddress(addr);
				if (pCodeInfo != nullptr && pCodeInfo->bDisabled == false)
				{
					nextItemAddress = addr + pCodeInfo->ByteSize;
					state.ItemList.push_back(pCodeInfo);
				}
				else // code and data are mutually exclusive
				{
					FDataInfo *pDataInfo = state.GetReadDataInfoForAddress(addr);
					if (pDataInfo != nullptr)
					{
						if (pDataInfo->DataType != DataType::Blob && pDataInfo->DataType != DataType::Graphics)	// not sure why we want this
							nextItemAddress = addr + pDataInfo->ByteSize;
						else
							nextItemAddress = addr + 1;

						state.ItemList.push_back(pDataInfo);
					}
				}
			}

			// update cursor item index
			for (int i = 0; i < FCodeAnalysisState::kNoViewStates; i++)
			{
				if (state.ViewState[i].pCursorItem == state.ItemList.back())
					state.ViewState[i].CursorItemIndex = (int)state.ItemList.size() - 1;
			}
		}

		// Maybe this needs to follow the same algorithm as the main view?
		ImGui::SetScrollY(state.GetFocussedViewState().CursorItemIndex * line_height);
		state.bCodeAnalysisDataDirty = false;
	}

}

void DoItemContextMenu(FCodeAnalysisState& state, FItem *pItem)
{
	if (ImGui::BeginPopupContextItem("code item context menu"))
	{		
		if (pItem->Type == ItemType::Data)
		{
			if (ImGui::Selectable("Toggle data type (D)"))
			{
				SetItemData(state, pItem);
			}
			if (ImGui::Selectable("Set as text (T)"))
			{
				SetItemText(state, pItem);
			}
			if (ImGui::Selectable("Set as Code (C)"))
			{
				SetItemCode(state, pItem);
			}
#ifdef ENABLE_IMAGE_TYPE
			if (ImGui::Selectable("Set as Image (I)"))
			{
				SetItemImage(state, pItem);
			}
#endif
			if (ImGui::Selectable("Toggle Data Breakpoint"))
				state.CPUInterface->ToggleDataBreakpointAtAddress(pItem->Address, pItem->ByteSize);
			if (ImGui::Selectable("Add Watch"))
				state.AddWatch(pItem->Address);

		}

		if (pItem->Type == ItemType::Label)
		{
			if (ImGui::Selectable("Remove label"))
			{
				RemoveLabelAtAddress(state, pItem->Address);
			}
		}
		else
		{
			if (ImGui::Selectable("Add label (L)"))
			{
				AddLabelAtAddress(state, pItem->Address);
			}
		}

		// breakpoints
		if (pItem->Type == ItemType::Code)
		{
			if (ImGui::Selectable("Toggle Exec Breakpoint"))
				state.CPUInterface->ToggleExecBreakpointAtAddress(pItem->Address);
		}
				
		if (ImGui::Selectable("View in graphics viewer"))
		{
			FDataInfo* pDataItem = state.GetReadDataInfoForAddress(pItem->Address);
			int byteSize = 1;
			if (pDataItem->DataType == DataType::Bitmap)
			{
				byteSize = pDataItem->ByteSize;
			}
			state.CPUInterface->GraphicsViewerSetView(pItem->Address, byteSize);
		}

		ImGui::EndPopup();
	}
}

void DrawCodeAnalysisItemAtIndex(FCodeAnalysisState& state, FCodeAnalysisViewState& viewState, int i)
{
	assert(i < (int)state.ItemList.size());
	FItem* pItem = state.ItemList[i];
	bool bHighlight = (viewState.HighlightAddress >= pItem->Address && viewState.HighlightAddress < pItem->Address + pItem->ByteSize);
	uint32_t kHighlightColour = 0xff00ff00;
	ImGui::PushID(i);

	// Highlight formatting selection
	/*if (state.DataFormattingOptions.IsValid() &&
		pItem->Address >= state.DataFormattingOptions.StartAddress &&
		pItem->Address <= state.DataFormattingOptions.EndAddress)
	{
		bHighlight = true;
		kHighlightColour = 0xffffff00;
	}*/

	//const FItem *pPrevItem = i > 0 ? state.ItemList[i-1] : nullptr;
	//if (pPrevItem != nullptr && pItem->Address > pPrevItem->Address + pPrevItem->ByteSize)
	//	ImGui::Separator();

	// selectable
	const uint16_t endAddress = viewState.DataFormattingOptions.CalcEndAddress();
	const bool bSelected = (pItem == viewState.pCursorItem) || 
		(viewState.DataFormattingTabOpen && pItem->Address >= viewState.DataFormattingOptions.StartAddress && pItem->Address <= endAddress);
	if (ImGui::Selectable("##codeanalysisline", bSelected, ImGuiSelectableFlags_SelectOnNav))
	{
		viewState.pCursorItem = state.ItemList[i];
		viewState.CursorItemIndex = i;

		// Select Data Formatting Range
		if (viewState.DataFormattingTabOpen && pItem->Type == ItemType::Data)
		{
			ImGuiIO& io = ImGui::GetIO();
			if (io.KeyShift)
			{
				//viewState.DataFormattingOptions.EndAddress = viewState.pCursorItem->Address;
				if (viewState.DataFormattingOptions.ItemSize > 0)
					viewState.DataFormattingOptions.NoItems = (viewState.DataFormattingOptions.StartAddress - viewState.pCursorItem->Address) / viewState.DataFormattingOptions.ItemSize;
			}
			else
			{
				//viewState.DataFormattingOptions.EndAddress = viewState.pCursorItem->Address;
				viewState.DataFormattingOptions.StartAddress = viewState.pCursorItem->Address;
			}
		}
	}
	DoItemContextMenu(state, pItem);
	ImGui::SetItemAllowOverlap();	// allow buttons
	ImGui::SameLine();

	switch (pItem->Type)
	{
	case ItemType::Label:
		DrawLabelInfo(state, viewState,static_cast<const FLabelInfo *>(pItem));
		break;
	case ItemType::Code:
		if (bHighlight)
			ImGui::PushStyleColor(ImGuiCol_Text, kHighlightColour);
		DrawCodeInfo(state, viewState, static_cast<const FCodeInfo *>(pItem));
		if (bHighlight)
			ImGui::PopStyleColor();
		break;
	case ItemType::Data:
		if (bHighlight)
			ImGui::PushStyleColor(ImGuiCol_Text, kHighlightColour);
		DrawDataInfo(state, viewState, static_cast<const FDataInfo *>(pItem),false,state.bAllowEditing);
		if (bHighlight)
			ImGui::PopStyleColor();
		break;
	case ItemType::CommentLine:
		DrawCommentLine(state, static_cast<const FCommentLine*>(pItem));
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
	const char* operandTypes[] = { "Unknown", "Pointer", "JumpAddress", "Decimal", "Hex"};
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

void DrawDetailsPanel(FCodeAnalysisState &state, FCodeAnalysisViewState& viewState)
{
	if (viewState.pCursorItem)
	{
		FItem *pItem = viewState.pCursorItem;
		switch (pItem->Type)
		{
		case ItemType::Label:
			DrawLabelDetails(state, viewState, static_cast<FLabelInfo *>(pItem));
			break;
		case ItemType::Code:
			DrawCodeDetails(state, viewState, static_cast<FCodeInfo *>(pItem));
			break;
		case ItemType::Data:
			DrawDataDetails(state, viewState, static_cast<FDataInfo *>(pItem));
			break;
		case ItemType::CommentLine:
			{
				FCommentBlock* pCommentBlock = state.GetCommentBlockForAddress(pItem->Address);
				if (pCommentBlock != nullptr)
					DrawCommentBlockDetails(state, pCommentBlock);
			}
			break;
		}



		if(pItem->Type != ItemType::CommentLine)
		{
			static std::string commentString;
			static FItem *pCurrItem = nullptr;
			if (pCurrItem != pItem)
			{
				commentString = pItem->Comment;
				pCurrItem = pItem;
			}

			//ImGui::Text("Comments");
			if (ImGui::InputTextWithHint("Comments", "Comments", &commentString))
			{
				SetItemCommentText(state, pItem, commentString.c_str());
			}
		}

	}
}

void DrawDebuggerButtons(FCodeAnalysisState &state, FCodeAnalysisViewState& viewState)
{
	if (state.CPUInterface->ShouldExecThisFrame())
	{
		if (ImGui::Button("Break (F5)"))
		{
			state.CPUInterface->Break();
			//viewState.TrackPCFrame = true;
		}
	}
	else
	{
		if (ImGui::Button("Continue (F5)"))
		{
			state.CPUInterface->Continue();
		}
	}
	ImGui::SameLine();
	if (ImGui::Button("Step Over (F10)"))
	{
		state.CPUInterface->StepOver();
		viewState.TrackPCFrame = true;
	}
	ImGui::SameLine();
	if (ImGui::Button("Step Into (F11)"))
	{
		state.CPUInterface->StepInto();
		viewState.TrackPCFrame = true;
	}
	ImGui::SameLine();
	if (ImGui::Button("Step Frame (F6)"))
	{
		state.CPUInterface->StepFrame();
	}
	ImGui::SameLine();
	if (ImGui::Button("Step Screen Write (F7)"))
	{
		state.CPUInterface->StepScreenWrite();
	}
	//ImGui::SameLine();
	//ImGui::Checkbox("Jump to PC on break", &bJumpToPCOnBreak);
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
	viewState.HoverAddress = -1;

	if (state.CPUInterface->ShouldExecThisFrame())
		state.CurrentFrameNo++;

	UpdateItemList(state);

	if (ImGui::ArrowButton("##btn", ImGuiDir_Left))
		GoToPreviousAddress(viewState);
	ImGui::SameLine();
	if (ImGui::Button("Jump To PC"))
		GoToAddress(viewState,state.CPUInterface->GetPC());
	ImGui::SameLine();
	static int addrInput = 0;
	const ImGuiInputTextFlags inputFlags = (GetNumberDisplayMode() == ENumberDisplayMode::Decimal) ? ImGuiInputTextFlags_CharsDecimal : ImGuiInputTextFlags_CharsHexadecimal;
	if (ImGui::InputInt("Jump To", &addrInput, 1, 100, inputFlags))
		GoToAddress(viewState, addrInput);

	if (viewState.TrackPCFrame == true)
	{
		viewState.GoToAddress = state.CPUInterface->GetPC();
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

	// StackInfo
	ImGui::SameLine();
	ImGui::Text("Stack range: ");
	DrawAddressLabel(state, viewState, state.StackMin);
	ImGui::SameLine();
	DrawAddressLabel(state, viewState, state.StackMax);

	if(ImGui::BeginChild("##analysis", ImVec2(ImGui::GetWindowContentRegionWidth() * 0.75f, 0), true))
	{
		//int scrollToItem = -1;
		// jump to address
		if (viewState.GoToAddress != -1)
		{
			const float currScrollY = ImGui::GetScrollY();
			const float currWindowHeight = ImGui::GetWindowHeight();
			const int kJumpViewOffset = 5;
			for (int item = 0; item < (int)state.ItemList.size(); item++)
			{
				if ((state.ItemList[item]->Address >= viewState.GoToAddress) && (viewState.GoToLabel || state.ItemList[item]->Type != ItemType::Label))
				{
					// set cursor
					viewState.pCursorItem = state.ItemList[item];
					viewState.CursorItemIndex = item;
					//scrollToItem = item;

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
					ImGuiContext& g = *GImGui;

					ImRect rect;
					rect.Min = ImVec2(0, itemY);
					rect.Max = ImVec2(100, itemY + lineHeight);
					//ImGui::ScrollToRect(g.CurrentWindow, rect, ImGuiScrollFlags_KeepVisibleEdgeY);
					
					break;
				}
			}

			viewState.GoToAddress = -1;
			viewState.GoToLabel = false;
		}

		// draw clipped list
		ImGuiListClipper clipper((int)state.ItemList.size(), lineHeight);

		//clipper.ForceDisplayRangeByIndices(viewState.CursorItemIndex, viewState.CursorItemIndex+1);
		while (clipper.Step())
		{
			for (int i = clipper.DisplayStart; i < clipper.DisplayEnd; i++)
			{
				DrawCodeAnalysisItemAtIndex(state, viewState, i);
			}
		}

		/*if (scrollToItem != -1)
		{
			ImGuiContext& g = *GImGui;

			float itemPosY = clipper.StartPosY + (clipper.ItemsHeight * (scrollToItem + 10));
			ImRect rect;
			rect.Min = ImVec2(0, itemPosY - lineHeight);
			rect.Max = ImVec2(100, itemPosY + lineHeight);
			ImGui::ScrollToRect(g.CurrentWindow, rect, ImGuiScrollFlags_KeepVisibleEdgeY);
		}*/
		//float item_pos_y = clipper.StartPosY + (clipper.ItemsHeight * viewState.CursorItemIndex);
		//ImGui::SetScrollFromPosY(item_pos_y - ImGui::GetWindowPos().y);

		// only handle keypresses for focussed window
		if(state.FocussedWindowId == windowId)
			ProcessKeyCommands(state, viewState);

		UpdatePopups(state, viewState);
	}
	ImGui::EndChild();
	ImGui::SameLine();
	if(ImGui::BeginChild("##rightpanel", ImVec2(0, 0), true))
	{
		float height = ImGui::GetWindowContentRegionMax().y - ImGui::GetWindowContentRegionMin().y;
		if (ImGui::BeginChild("##cadetails", ImVec2(0, height / 2), true))
			DrawDetailsPanel(state, viewState);
		ImGui::EndChild();
		if (ImGui::BeginChild("##caglobals", ImVec2(0, 0), true))
			DrawGlobals(state, viewState);
		ImGui::EndChild();
	}
	ImGui::EndChild(); // right panel
}

void DrawExecutionInfo(FCodeAnalysisState& state)
{
	if (ImGui::BeginTabBar("ExeInfoTabs"))
	{
		if (ImGui::BeginTabItem("CallStack"))
		{
			DrawCallStack(state);
			ImGui::EndTabItem();
		}

		if (ImGui::BeginTabItem("Trace"))
		{
			DrawTrace(state);
			ImGui::EndTabItem();
		}

		if (ImGui::BeginTabItem("Registers"))
		{
			DrawRegisters(state);
			ImGui::EndTabItem();
		}		

		ImGui::EndTabBar();
	}
}

void DrawLabelList(FCodeAnalysisState &state, FCodeAnalysisViewState& viewState, std::vector<FLabelInfo *> labelList)
{
	static std::string filterText;
	ImGui::InputText("Filter", &filterText);
	if (ImGui::BeginChild("GlobalLabelList", ImVec2(0, 0), false))
	{
		for (FLabelInfo *pLabelInfo : labelList)
		{
			if (filterText.empty() || pLabelInfo->Name.find(filterText) != std::string::npos)
			{
				if (ImGui::Selectable(pLabelInfo->Name.c_str(), viewState.pCursorItem == pLabelInfo))
				{
					GoToAddress(viewState, pLabelInfo->Address, true);
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
		if (viewState.pCursorItem)
		{
			formattingOptions.StartAddress = viewState.pCursorItem->Address;
			//formattingOptions.EndAddress = viewState.pCursorItem->Address;
		}

		viewState.DataFormattingTabOpen = true;
	}

	// Set Start address of region to format
	ImGui::PushID("Start");
	ImGui::InputInt("Start Address", &formattingOptions.StartAddress, 1, 100, inputFlags);
	ImGui::PopID();

	// Set End address of region to format
	ImGui::PushID("End");
	ImGui::Text("End Address: %s", NumStr(formattingOptions.CalcEndAddress()));
	//ImGui::InputInt("End Address", &formattingOptions.EndAddress, 1, 100, inputFlags);
	ImGui::PopID();

	
	const char* dataTypes[] = { "Byte", "Word", "Bitmap", "Char Map", "Col Attr" };
	static int dataTypeIndex = 0; // Here we store our selection data as an index.
	const char* combo_preview_value = dataTypes[dataTypeIndex];  // Pass in the preview value visible before opening the combo (it could be anything)
	if (ImGui::BeginCombo("Data Type", combo_preview_value, 0))
	{
		for (int n = 0; n < IM_ARRAYSIZE(dataTypes); n++)
		{
			const bool isSelected = (dataTypeIndex == n);
			if (ImGui::Selectable(dataTypes[n], isSelected))
				dataTypeIndex = n;

			// Set the initial focus when opening the combo (scrolling + keyboard navigation focus)
			if (isSelected)
				ImGui::SetItemDefaultFocus();
		}
		ImGui::EndCombo();
	}

	switch (dataTypeIndex)
	{
	case 0:
		formattingOptions.DataType = DataType::Byte;
		formattingOptions.ItemSize = 1;
		ImGui::InputInt("Item Count", &formattingOptions.NoItems);
		break;
	case 1:
		formattingOptions.DataType = DataType::Word;
		formattingOptions.ItemSize = 2;
		ImGui::InputInt("Item Count", &formattingOptions.NoItems);

		break;
	case 2:
		formattingOptions.DataType = DataType::Bitmap;
		{
			static int size[2];
			if (ImGui::InputInt2("Bitmap Size(X,Y)", size))
			{
				formattingOptions.ItemSize = std::max(1,size[0] / 8);
				formattingOptions.NoItems = size[1];
			}
			//ImGui::InputInt("Item Size", &formattingOptions.ItemSize);
		}
		break;
	case 3:
		formattingOptions.DataType = DataType::CharacterMap;
		{
			static int size[2];
			if (ImGui::InputInt2("CharMap Size(X,Y)", size))
			{
				formattingOptions.ItemSize = std::max(1, size[0]);
				formattingOptions.NoItems = size[1];
			}

			DrawCharacterSetComboBox(state, &formattingOptions.CharacterSet);
			const char* format = "%02X";
			int flags = ImGuiInputTextFlags_EnterReturnsTrue | ImGuiInputTextFlags_CharsHexadecimal;
			ImGui::InputScalar("Null Character", ImGuiDataType_U8, &formattingOptions.EmptyCharNo, 0, 0, format, flags);
			//ImGui::InputInt("Item Size", &formattingOptions.ItemSize);
		}
		//ImGui::InputInt("Item Size", &formattingOptions.ItemSize);
		break;
	case 4:
		formattingOptions.DataType = DataType::ColAttr;
		ImGui::InputInt("Item Size", &formattingOptions.ItemSize);
		ImGui::InputInt("Item Count", &formattingOptions.NoItems);
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
			state.bCodeAnalysisDataDirty = true;
		}
		ImGui::SameLine();
		if (ImGui::Button("Format & Advance"))
		{
			FormatData(state, formattingOptions);
			formattingOptions.StartAddress += formattingOptions.ItemSize * formattingOptions.NoItems;
			state.bCodeAnalysisDataDirty = true;
		}
	}

	if (ImGui::Button("Clear Selection"))
	{
		formattingOptions = FDataFormattingOptions();
	}
}

void DrawGlobals(FCodeAnalysisState &state, FCodeAnalysisViewState& viewState)
{
	if(ImGui::BeginTabBar("GlobalsTabBar"))
	{
		if(ImGui::BeginTabItem("Functions"))
		{
			DrawLabelList(state, viewState, state.GlobalFunctions);
			ImGui::EndTabItem();
		}

		if (ImGui::BeginTabItem("Data"))
		{
			DrawLabelList(state, viewState, state.GlobalDataItems);
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

		ImGui::EndTabBar();
	}
}


