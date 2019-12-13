#include "CodeAnalyserUI.h"
#include "CodeAnalyser.h"

#include "UI/SpeccyUI.h"

#include "imgui.h"
#include "misc/cpp/imgui_stdlib.h"
#include <algorithm>
#include "chips/z80.h"

// UI
void DrawCodeAnalysisItemAtIndex(FCodeAnalysisState& state, int i);

void GoToAddress(FCodeAnalysisState &state, uint16_t newAddress)
{
	if(state.pCursorItem != nullptr)
		state.AddressStack.push_back(state.pCursorItem->Address);
	state.GoToAddress = newAddress;
}

bool GoToPreviousAddress(FCodeAnalysisState &state)
{
	if (state.AddressStack.empty())
		return false;

	state.GoToAddress = state.AddressStack.back();
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
			// TODO: bring up snippet in tool tip
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
				GoToAddress(state, addr);

		}

		ImGui::PopStyleColor();
	}
}

void DrawCodeAddress(FCodeAnalysisState &state, uint16_t addr)
{
	ImGui::PushStyleColor(ImGuiCol_Text, 0xff00ffff);
	ImGui::Text("%04Xh", addr);
	ImGui::PopStyleColor();
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
	ImGui::Text("%s: ", pLabelInfo->Name.c_str());
	if (ImGui::IsItemHovered())
	{
		ImGui::BeginTooltip();
		ImGui::Text("Callers:");
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

	ImGui::Text("\t%04Xh", pCodeInfo->Address);
	const float line_start_x = ImGui::GetCursorPosX();
	ImGui::SameLine(line_start_x + cell_width * 4 + glyph_width * 2);
	ImGui::Text("%s", pCodeInfo->Text.c_str());

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
		const uint8_t val = ReadySpeccyByte(state.pSpeccy, pDataInfo->Address);
		if (val == '\n')// carriage return messes up list
			ImGui::Text("db %02Xh '<cr>'", val, val);
		else
			ImGui::Text("db %02Xh '%c'", val, val);
	}
	break;

	case DataType::Word:
	{
		const uint16_t val = ReadySpeccyByte(state.pSpeccy, pDataInfo->Address) | (ReadySpeccyByte(state.pSpeccy, pDataInfo->Address + 1) << 8);
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
			const char ch = ReadySpeccyByte(state.pSpeccy, pDataInfo->Address + i);
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

void DrawDataDetails(FCodeAnalysisState &state, FDataInfo *pDataInfo)
{
	switch (pDataInfo->DataType)
	{
	case DataType::Byte:
	break;

	case DataType::Word:
	break;

	case DataType::Text:
	{
		std::string textString;
		for (int i = 0; i < pDataInfo->ByteSize; i++)
		{
			const char ch = ReadySpeccyByte(state.pSpeccy, pDataInfo->Address + i);
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
}





void ProcessKeyCommands(FCodeAnalysisState &state)
{
	if (ImGui::IsWindowFocused() && state.pCursorItem != nullptr)
	{
		if (ImGui::IsKeyPressed(state.KeyConfig[(int)Key::SetItemData]))
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
	}
}

void UpdateItemList(FCodeAnalysisState &state)
{
	// build item list - not every frame please!
	if (state.bCodeAnalysisDataDirty)
	{
		const float line_height = ImGui::GetTextLineHeight();
		
		state.ItemList.clear();

		int nextDataAddress = 0;

		// loop across address range
		for (int addr = 0; addr < (1 << 16); addr++)
		{
			FLabelInfo *pLabelInfo = state.Labels[addr];
			if (pLabelInfo != nullptr)
			{
				state.ItemList.push_back(pLabelInfo);

			}
			FCodeInfo *pCodeInfo = state.CodeInfo[addr];
			if (pCodeInfo != nullptr)
			{
				nextDataAddress = addr + pCodeInfo->ByteSize;
				state.ItemList.push_back(pCodeInfo);
			}
			else if (addr >= nextDataAddress)// code and data are mutually exclusive
			{
				FDataInfo *pDataInfo = state.DataInfo[addr];
				if (pDataInfo != nullptr)
				{
					nextDataAddress = addr + pDataInfo->ByteSize;
					state.ItemList.push_back(pDataInfo);
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

void DrawCodeAnalysisItemAtIndex(FCodeAnalysisState& state, int i)
{
	assert(i < state.ItemList.size());
	const FItem* pItem = state.ItemList[i];

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
	ImGui::SetItemAllowOverlap();	// allow buttons
	ImGui::SameLine();

	switch (pItem->Type)
	{
	case ItemType::Label:
		DrawLabelInfo(state, static_cast<const FLabelInfo *>(pItem));
		break;
	case ItemType::Code:
		DrawCodeInfo(state, static_cast<const FCodeInfo *>(pItem));
		break;
	case ItemType::Data:
		DrawDataInfo(state, static_cast<const FDataInfo *>(pItem));
		break;
	}


	ImGui::PopID();
}

void DrawCodeAnalysisData(FSpeccyUI *pUI)
{
	FCodeAnalysisState &state = pUI->CodeAnalysis;
	const float line_height = ImGui::GetTextLineHeight();
	const float glyph_width = ImGui::CalcTextSize("F").x;
	const float cell_width = 3 * glyph_width;

	if (state.pSpeccy->ExecThisFrame)
		state.CurrentFrameNo++;

	UpdateItemList(state);

	if (ImGui::ArrowButton("##btn", ImGuiDir_Left))
		GoToPreviousAddress(state);
	ImGui::SameLine();
	if (ImGui::Button("Jump To PC"))
		GoToAddress(state,z80_pc(&pUI->pSpeccy->CurrentState.cpu));
	ImGui::SameLine();
	static int addrInput = 0;
	if (ImGui::InputInt("Jump To", &addrInput, 1, 100, ImGuiInputTextFlags_CharsHexadecimal))
		GoToAddress(state, addrInput);

	ImGui::BeginChild("##analysis", ImVec2(ImGui::GetWindowContentRegionWidth() * 0.5f, 0), true);
	{
		// jump to address
		if (state.GoToAddress != -1)
		{
			for (int item = 0; item < state.ItemList.size(); item++)
			{
				if ((state.ItemList[item]->Address == state.GoToAddress) && (state.ItemList[item]->Type != ItemType::Label))
				{
					// set cursor
					state.pCursorItem = state.ItemList[item];
					state.CursorItemIndex = item;

					//ImGui::SetScrollY(maxY * fraction);
					ImGui::SetScrollY(item * line_height);
					break;
				}
			}

			state.GoToAddress = -1;
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
	ImGui::BeginChild("##detail", ImVec2(0, 0), true);
	if(state.pCursorItem)
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
	ImGui::EndChild();


	
}
