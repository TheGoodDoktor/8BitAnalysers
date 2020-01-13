#include "CodeAnalyserUI.h"
#include "CodeAnalyser.h"

#include "UI/SpeccyUI.h"

#include "imgui.h"
#include "misc/cpp/imgui_stdlib.h"
#include <algorithm>
#include "chips/z80.h"
#include "CodeToolTips.h"

// UI
void DrawCodeAnalysisItemAtIndex(FCodeAnalysisState& state, int i);

void GoToAddress(FCodeAnalysisState &state, uint16_t newAddress, bool bLabel = false)
{
	if(state.pCursorItem != nullptr)
		state.AddressStack.push_back(state.pCursorItem->Address);
	state.GoToAddress = newAddress;
	state.GoToLabel = bLabel;
}

void CodeAnalyserGoToAddress(uint16_t newAddress, bool bLabel)
{
	GoToAddress(GetSpeccyUI()->CodeAnalysis, newAddress, bLabel);
}

bool GoToPreviousAddress(FCodeAnalysisState &state)
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
	for(int i=0;i<state.ItemList.size();i++)
	{
		if (state.ItemList[i] != nullptr && state.ItemList[i]->Address == addr)
			return i;
	}
	return -1;
}

void DrawAddressLabel(FCodeAnalysisState &state, uint16_t addr)
{
	int labelOffset = 0;
	const char *pLabelString = nullptr;
	
	for(int addrVal = addr; addrVal >= 0; addrVal--)
	{
		if(state.Labels[addrVal] != nullptr)
		{
			pLabelString = state.Labels[addrVal]->Name.c_str();
			break;
		}

		labelOffset++;
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
				const int startIndex = index;// std::max(index - (kToolTipNoLines / 2), 0);
				for(int line=0;line < kToolTipNoLines;line++)
				{
					if(startIndex + line < state.ItemList.size())
						DrawCodeAnalysisItemAtIndex(state,startIndex + line);
				}
				ImGui::EndTooltip();
			}
			if (ImGui::IsMouseDoubleClicked(0))
				GoToAddress(state, addr, false);

			state.HoverAddress = addr;	
		}

		ImGui::PopStyleColor();
	}
}

void DrawCodeAddress(FCodeAnalysisState &state, uint16_t addr)
{
	//ImGui::PushStyleColor(ImGuiCol_Text, 0xff00ffff);
	ImGui::Text("%04Xh", addr);
	//ImGui::PopStyleColor();
	ImGui::SameLine();
	DrawAddressLabel(state, addr);
}

void DrawComment(const FItem *pItem)
{
	if(pItem != nullptr && pItem->Comment.empty() == false)
	{
		ImGui::SameLine();
		ImGui::PushStyleColor(ImGuiCol_Text, 0xff008000);
		ImGui::Text("\t// %s", pItem->Comment.c_str());
		ImGui::PopStyleColor();
	}
}

void DrawLabelInfo(FCodeAnalysisState &state, const FLabelInfo *pLabelInfo)
{
	if(pLabelInfo->Global || pLabelInfo->LabelType == LabelType::Function)
		ImGui::TextColored(ImVec4(1.0f, 1.0f, 1.0f, 1.0f),"%s: ", pLabelInfo->Name.c_str());
	else
		ImGui::TextColored(ImVec4(0.5f, 0.5f, 0.5f, 1.0f), "%s: ", pLabelInfo->Name.c_str());
	
	if (ImGui::IsItemHovered() && pLabelInfo->References.empty() == false)
	{
		ImGui::BeginTooltip();
		ImGui::Text("References:");
		for (const auto & caller : pLabelInfo->References)
		{
			DrawCodeAddress(state, caller.first);
		}
		ImGui::EndTooltip();
	}

	DrawComment(pLabelInfo);	
}

void DrawLabelDetails(FCodeAnalysisState &state, FLabelInfo *pLabelInfo)
{
	static std::string labelString;
	static FItem *pCurrItem = nullptr;
	if (pCurrItem != pLabelInfo)
	{
		labelString = pLabelInfo->Name;
		pCurrItem = pLabelInfo;
	}

	//ImGui::Text("Comments");
	if (ImGui::InputText("Name", &labelString))
	{
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

	ImGui::Text("Callers:");
	for (const auto & caller : pLabelInfo->References)
	{
		DrawCodeAddress(state, caller.first);
	}
}



void DrawCodeInfo(FCodeAnalysisState &state, const FCodeInfo *pCodeInfo)
{
	const float line_height = ImGui::GetTextLineHeight();
	const float glyph_width = ImGui::CalcTextSize("F").x;
	const float cell_width = 3 * glyph_width;

	const int framesSinceAccessed = state.CurrentFrameNo - pCodeInfo->FrameLastAccessed;
	const int brightVal = (255 - std::min(framesSinceAccessed << 2, 255)) & 0xff;

	const ImU32 col = 0xff000000 | (brightVal << 16) | (brightVal << 8) | (brightVal << 0);

	const bool bPCLine = pCodeInfo->Address == z80_pc(&state.pSpeccy->CurrentState.cpu);

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

	//ImGui::PushStyleColor(ImGuiCol_Text, col);
	//ImGui::Text(">> ");
	//ImGui::SameLine();
	//ImGui::PopStyleColor();
	if(pCodeInfo->bSelfModifyingCode == true)
	{
		UpdateCodeInfoForAddress(state, pCodeInfo->Address);
	}

	ImGui::Text("\t%04Xh", pCodeInfo->Address);
	const float line_start_x = ImGui::GetCursorPosX();
	ImGui::SameLine(line_start_x + cell_width * 4 + glyph_width * 2);
	ImGui::Text("%s", pCodeInfo->Text.c_str());
	if(ImGui::IsItemHovered())
	{
		ShowCodeToolTip(state, pCodeInfo);
	}

	// draw jump address label name
	if (pCodeInfo->JumpAddress != 0)
	{
		DrawAddressLabel(state, pCodeInfo->JumpAddress);
	}

	if (pCodeInfo->PointerAddress != 0)
	{
		DrawAddressLabel(state, pCodeInfo->PointerAddress);
	}

	DrawComment(pCodeInfo);

}

void DrawCodeDetails(FCodeAnalysisState &state, FCodeInfo *pCodeInfo)
{
	if (pCodeInfo->bSelfModifyingCode == true)
		ImGui::Text("Self modifying code");
}

void DrawDataInfo(FCodeAnalysisState &state, const FDataInfo *pDataInfo)
{
	const float glyph_width = ImGui::CalcTextSize("F").x;
	const float cell_width = 3 * glyph_width;

	ImGui::Text("\t%04Xh", pDataInfo->Address);
	const float line_start_x = ImGui::GetCursorPosX();
	ImGui::SameLine(line_start_x + cell_width * 4 + glyph_width * 2);

	
	
	switch (pDataInfo->DataType)
	{
	case DataType::Byte:
	{
		const uint8_t val = ReadSpeccyByte(state.pSpeccy, pDataInfo->Address);
		if (val == '\n')// carriage return messes up list
			ImGui::Text("db %02Xh '<cr>'", val, val);
		else
			ImGui::Text("db %02Xh '%c'", val, val);
	}
	break;

	case DataType::Word:
	{
		const uint16_t val = ReadSpeccyByte(state.pSpeccy, pDataInfo->Address) | (ReadSpeccyByte(state.pSpeccy, pDataInfo->Address + 1) << 8);
		ImGui::Text("dw %04Xh", val);
		// draw address as label - optional?
		ImGui::SameLine();	
		DrawAddressLabel(state, val);
	}
	break;

	case DataType::Text:
	{
		std::string textString;
		for (int i = 0; i < pDataInfo->ByteSize; i++)
		{
			const char ch = ReadSpeccyByte(state.pSpeccy, pDataInfo->Address + i);
			if (ch == '\n')
				textString += "<cr>";
			else
				textString += ch;
		}
		ImGui::Text("ascii '%s'", textString.c_str());
	}
	break;

	case DataType::Graphics:
	case DataType::Blob:
	default:
		ImGui::Text("%d Bytes", pDataInfo->ByteSize);
		break;
	}

	DrawComment(pDataInfo);

}

void DrawDataValueGraph(uint8_t val, bool bReset)
{
	// Create a dummy array of contiguous float values to plot
		// Tip: If your float aren't contiguous but part of a structure, you can pass a pointer to your first float and the sizeof() of your structure in the Stride parameter.
	static bool animate = true;
	static float values[90] = { 0 };
	static int values_offset = 0;
	static double refresh_time = 0.0;
	if (!animate || refresh_time == 0.0)
		refresh_time = ImGui::GetTime();

	while (refresh_time < ImGui::GetTime()) // Create dummy data at fixed 60 hz rate for the demo
	{
		static float phase = 0.0f;
		values[values_offset] = (float)val;
		values_offset = (values_offset + 1) % IM_ARRAYSIZE(values);
		phase += 0.10f*values_offset;
		refresh_time += 1.0f / 60.0f;
	}

	// Plots can display overlay texts
	// (in this example, we will display an average value)
	{
		float average = 0.0f;
		for (int n = 0; n < IM_ARRAYSIZE(values); n++)
			average += values[n];
		average /= (float)IM_ARRAYSIZE(values);
		char overlay[32];
		sprintf(overlay, "avg %f", average);
		ImGui::PlotLines("Lines", values, IM_ARRAYSIZE(values), values_offset, overlay, 0.0f, 255.0f, ImVec2(0, 80));
	}
	//ImGui::PlotHistogram("Histogram", arr, IM_ARRAYSIZE(arr), 0, NULL, 0.0f, 1.0f, ImVec2(0, 80));
}

void DrawDataDetails(FCodeAnalysisState &state, FDataInfo *pDataInfo)
{
	switch (pDataInfo->DataType)
	{
	case DataType::Byte:
		DrawDataValueGraph(ReadSpeccyByte(state.pSpeccy, pDataInfo->Address),false);
	break;

	case DataType::Word:
	break;

	case DataType::Text:
	{
		std::string textString;
		for (int i = 0; i < pDataInfo->ByteSize; i++)
		{
			const char ch = ReadSpeccyByte(state.pSpeccy, pDataInfo->Address + i);
			if (ch == '\n')
				textString += "<cr>";
			else
				textString += ch;
		}
		ImGui::Text("%s", textString.c_str());
		int length = pDataInfo->ByteSize;
		if(ImGui::InputInt("Length",&length))
		{
			pDataInfo->ByteSize = length;
		}
	}
	break;

	case DataType::Graphics:
		// TODO: GFX as texture?
		//point gfx viewer?
		break;

	case DataType::Blob:
		// draw memory detail
	default:
		break;
	}

	// List Data accesses
	if (pDataInfo->Reads.empty() == false)
	{
		ImGui::Text("Reads:");
		for (const auto & caller : pDataInfo->Reads)
		{
			DrawCodeAddress(state, caller.first);
		}
	}

	if (pDataInfo->Writes.empty() == false)
	{
		ImGui::Text("Writes:");
		for (const auto & caller : pDataInfo->Writes)
		{
			DrawCodeAddress(state, caller.first);
		}
	}
}


int CommentInputCallback(ImGuiInputTextCallbackData *pData)
{
	return 1;
}


void ProcessKeyCommands(FCodeAnalysisState &state)
{
	if (ImGui::IsWindowFocused() && state.pCursorItem != nullptr)
	{
		if (ImGui::IsKeyPressed(state.KeyConfig[(int)Key::SetItemCode]))
		{
			SetItemCode(state, state.pCursorItem);
		}
		else if (ImGui::IsKeyPressed(state.KeyConfig[(int)Key::SetItemData]))
		{
			SetItemData(state, state.pCursorItem);
		}
		else if (ImGui::IsKeyPressed(state.KeyConfig[(int)Key::SetItemText]))
		{
			SetItemText(state, state.pCursorItem);
		}
		else if (ImGui::IsKeyPressed(state.KeyConfig[(int)Key::AddLabel]))
		{
			AddLabelAtAddress(state, state.pCursorItem->Address);
		}
		else if (ImGui::IsKeyPressed(state.KeyConfig[(int)Key::Comment]))
		{
			//AddLabelAtAddress(state, state.pCursorItem->Address);
			ImGui::OpenPopup("Enter Comment Text");
			ImGui::SetWindowFocus("Enter Comment Text");
		}
		else if (state.pCursorItem->Type == ItemType::Label && ImGui::IsKeyPressed(state.KeyConfig[(int)Key::Rename]))
		{
			//AddLabelAtAddress(state, state.pCursorItem->Address);
			ImGui::OpenPopup("Enter Label Text");
			ImGui::SetWindowFocus("Enter Label Text");
		}
	}

	if (ImGui::BeginPopup("Enter Comment Text", ImGuiWindowFlags_AlwaysAutoResize))
	{
		ImGui::SetKeyboardFocusHere();
		if(ImGui::InputText("##comment", &state.pCursorItem->Comment, ImGuiInputTextFlags_EnterReturnsTrue))
			ImGui::CloseCurrentPopup();
		ImGui::SetItemDefaultFocus();
		ImGui::EndPopup();
	}

	if (ImGui::BeginPopup("Enter Label Text", ImGuiWindowFlags_AlwaysAutoResize))
	{
		FLabelInfo *pLabel = (FLabelInfo *)state.pCursorItem;
		
		ImGui::SetKeyboardFocusHere();
		if (ImGui::InputText("##comment", &pLabel->Name, ImGuiInputTextFlags_EnterReturnsTrue))
			ImGui::CloseCurrentPopup();
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

		int nextItemAddress = 0;

		// loop across address range
		for (int addr = 0; addr < (1 << 16); addr++)
		{
			FLabelInfo *pLabelInfo = state.Labels[addr];
			if (pLabelInfo != nullptr)
			{
				state.ItemList.push_back(pLabelInfo);

			}
			if (addr >= nextItemAddress)
			{
				FCodeInfo *pCodeInfo = state.CodeInfo[addr];
				if (pCodeInfo != nullptr && pCodeInfo->bDisabled == false)
				{
					nextItemAddress = addr + pCodeInfo->ByteSize;
					state.ItemList.push_back(pCodeInfo);
				}
				else // code and data are mutually exclusive
				{
					FDataInfo *pDataInfo = state.DataInfo[addr];
					if (pDataInfo != nullptr)
					{
						if (pDataInfo->DataType != DataType::Blob && pDataInfo->DataType != DataType::Graphics)
							nextItemAddress = addr + pDataInfo->ByteSize;
						else
							nextItemAddress = addr + 1;

						state.ItemList.push_back(pDataInfo);
					}
				}
			}

			// update cursor item index
			if (state.pCursorItem == state.ItemList.back())
				state.CursorItemIndex = (int)state.ItemList.size() - 1;
		}

		ImGui::SetScrollY(state.CursorItemIndex * line_height);
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
		
		if (ImGui::Selectable("View in graphics viewer"))
		{
			GraphicsViewerGoToAddress(pItem->Address);			
		}

		ImGui::EndPopup();
	}
}

void DrawCodeAnalysisItemAtIndex(FCodeAnalysisState& state, int i)
{
	assert(i < state.ItemList.size());
	FItem* pItem = state.ItemList[i];
	const bool bHighlight = (pItem->Address == state.HighlightAddress);
	const uint32_t kHighlightColour = 0xff00ff00;
	ImGui::PushID(i);

	const FItem *pPrevItem = i > 0 ? state.ItemList[i-1] : nullptr;
	if (pPrevItem != nullptr && pItem->Address > pPrevItem->Address + pPrevItem->ByteSize)
		ImGui::Separator();

	// selectable
	if (ImGui::Selectable("##codeanalysisline", pItem == state.pCursorItem, 0))
	{
		state.pCursorItem = state.ItemList[i];
		state.CursorItemIndex = i;
	}
	DoItemContextMenu(state, pItem);
	ImGui::SetItemAllowOverlap();	// allow buttons
	ImGui::SameLine();

	switch (pItem->Type)
	{
	case ItemType::Label:
		DrawLabelInfo(state, static_cast<const FLabelInfo *>(pItem));
		break;
	case ItemType::Code:
		if (bHighlight)
			ImGui::PushStyleColor(ImGuiCol_Text, kHighlightColour);
		DrawCodeInfo(state, static_cast<const FCodeInfo *>(pItem));
		if (bHighlight)
			ImGui::PopStyleColor();
		break;
	case ItemType::Data:
		if (bHighlight)
			ImGui::PushStyleColor(ImGuiCol_Text, kHighlightColour);
		DrawDataInfo(state, static_cast<const FDataInfo *>(pItem));
		if (bHighlight)
			ImGui::PopStyleColor();
		break;
	}


	ImGui::PopID();
}

void DrawDetailsPanel(FCodeAnalysisState &state)
{
	if (state.pCursorItem)
	{
		FItem *pItem = state.pCursorItem;
		switch (pItem->Type)
		{
		case ItemType::Label:
			DrawLabelDetails(state, static_cast<FLabelInfo *>(pItem));
			break;
		case ItemType::Code:
			DrawCodeDetails(state, static_cast<FCodeInfo *>(pItem));
			break;
		case ItemType::Data:
			DrawDataDetails(state, static_cast<FDataInfo *>(pItem));
			break;
		}

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

void DrawDebuggerButtons(FCodeAnalysisState &state)
{
	FSpeccy *pSpeccy = state.pSpeccy;
	static bool bJumpToPCOnBreak = false;

	if (ImGui::Button("Break"))
	{
		FSpeccyUI *pUI = GetSpeccyUI();
		pUI->UIZX.dbg.dbg.stopped = true;
		pUI->UIZX.dbg.dbg.step_mode = UI_DBG_STEPMODE_NONE;
		if(bJumpToPCOnBreak)
			GoToAddress(state,z80_pc(&pSpeccy->CurrentState.cpu));
	}
	ImGui::SameLine();
	if (ImGui::Button("Continue"))
	{
		FSpeccyUI *pUI = GetSpeccyUI();
		pUI->UIZX.dbg.dbg.stopped = false;
		pUI->UIZX.dbg.dbg.step_mode = UI_DBG_STEPMODE_NONE;
	}
	ImGui::SameLine();
	ImGui::Checkbox("Jump to PC on break", &bJumpToPCOnBreak);
}
void DrawCodeAnalysisData(FCodeAnalysisState &state)
{
	FSpeccy *pSpeccy = state.pSpeccy;
	const float line_height = ImGui::GetTextLineHeight();
	const float glyph_width = ImGui::CalcTextSize("F").x;
	const float cell_width = 3 * glyph_width;

	state.HighlightAddress = state.HoverAddress;
	state.HoverAddress = -1;

	if (state.pSpeccy->ExecThisFrame)
		state.CurrentFrameNo++;

	UpdateItemList(state);

	if (ImGui::ArrowButton("##btn", ImGuiDir_Left))
		GoToPreviousAddress(state);
	ImGui::SameLine();
	if (ImGui::Button("Jump To PC"))
		GoToAddress(state,z80_pc(&pSpeccy->CurrentState.cpu));
	ImGui::SameLine();
	static int addrInput = 0;
	if (ImGui::InputInt("Jump To", &addrInput, 1, 100, ImGuiInputTextFlags_CharsHexadecimal))
		GoToAddress(state, addrInput);
	DrawDebuggerButtons(state);
	ImGui::Checkbox("Analyse data accesses (slow)", &state.bRegisterDataAccesses);

	if(ImGui::BeginChild("##analysis", ImVec2(ImGui::GetWindowContentRegionWidth() * 0.75f, 0), true))
	{
		// jump to address
		if (state.GoToAddress != -1)
		{
			const int kJumpViewOffset = 5;
			for (int item = 0; item < state.ItemList.size(); item++)
			{
				if ((state.ItemList[item]->Address >= state.GoToAddress) && (state.GoToLabel || state.ItemList[item]->Type != ItemType::Label))
				{
					// set cursor
					state.pCursorItem = state.ItemList[item];
					state.CursorItemIndex = item;

					const int gotoItem = std::max(item - kJumpViewOffset, 0);
					ImGui::SetScrollY(gotoItem * line_height);
					break;
				}
			}

			state.GoToAddress = -1;
			state.GoToLabel = false;
		}

		// draw clipped list
		ImGuiListClipper clipper((int)state.ItemList.size(), line_height);

		while (clipper.Step())
		{
			for (int i = clipper.DisplayStart; i < clipper.DisplayEnd; i++)
			{
				DrawCodeAnalysisItemAtIndex(state, i);
			}
		}

		ProcessKeyCommands(state);
	}
	ImGui::EndChild();
	ImGui::SameLine();
	if(ImGui::BeginChild("##rightpanel", ImVec2(0, 0), true))
	{
		// Details Panel
		if (ImGui::BeginChild("##detail", ImVec2(0, 300), true))
		{
			DrawDetailsPanel(state);
		}
		ImGui::EndChild();

		// Globals Panel
		if (ImGui::BeginChild("##globallabelss", ImVec2(0, 0), true))
		{
			DrawGlobals(state);
		}
		ImGui::EndChild();

	}
	ImGui::EndChild(); // right panel

	
}

void DrawLabelList(FCodeAnalysisState &state, std::vector<FLabelInfo *> labelList)
{
	if (ImGui::BeginChild("GlobalLabelList", ImVec2(0, 0), false))
	{
		for (FLabelInfo *pLabelInfo : labelList)
		{
			if (ImGui::Selectable(pLabelInfo->Name.c_str(), state.pCursorItem == pLabelInfo))
			{
				GoToAddress(state, pLabelInfo->Address, true);
			}
		}
	}
	ImGui::EndChild();
}

void DrawGlobals(FCodeAnalysisState &state)
{
	if(ImGui::BeginTabBar("GlobalsTabBar"))
	{
		if(ImGui::BeginTabItem("Functions"))
		{
			DrawLabelList(state, state.GlobalFunctions);
			ImGui::EndTabItem();
		}

		if (ImGui::BeginTabItem("Data"))
		{
			DrawLabelList(state, state.GlobalDataItems);
			ImGui::EndTabItem();
		}

		ImGui::EndTabBar();
	}
}