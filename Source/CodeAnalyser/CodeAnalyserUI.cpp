#include "CodeAnalyserUI.h"
#include "CodeAnalyser.h"

#include "UI/SpeccyUI.h"

#include "imgui.h"
#include "misc/cpp/imgui_stdlib.h"
#include <algorithm>
#include "chips/z80.h"

// UI

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

void DrawLabelInfo(FSpeccyUI *pUI, const FLabelInfo *pLabelInfo)
{
	ImGui::Text("%s: ", pLabelInfo->Name.c_str());
	if (ImGui::IsItemHovered())
	{
		ImGui::BeginTooltip();
		ImGui::Text("Callers:");
		for (const auto & caller : pLabelInfo->References)
		{
			ImGui::Text("%04Xh", caller.first);
		}
		ImGui::EndTooltip();
	}

	DrawComment(pLabelInfo);	
}

void DrawLabelDetails(FSpeccyUI *pUI, FLabelInfo *pLabelInfo)
{
	ImGui::Text("Callers:");
	for (const auto & caller : pLabelInfo->References)
	{
		ImGui::Text("%04Xh", caller.first);
	}
}

void DrawCodeInfo(FSpeccyUI *pUI, const FCodeInfo *pCodeInfo)
{
	FCodeAnalysisState &state = pUI->CodeAnalysis;
	
	const float line_height = ImGui::GetTextLineHeight();
	const float glyph_width = ImGui::CalcTextSize("F").x;
	const float cell_width = 3 * glyph_width;

	const int framesSinceAccessed = pUI->CurrentFrameNo - pCodeInfo->FrameLastAccessed;
	const int brightVal = (255 - std::min(framesSinceAccessed << 2, 255)) & 0xff;

	const ImU32 col = 0xff000000 | (brightVal << 16) | (brightVal << 8) | (brightVal << 0);

	const bool bPCLine = pCodeInfo->Address == z80_pc(&pUI->pSpeccy->CurrentState.cpu);

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
		const FLabelInfo *pLabelInfo = state.Labels[pCodeInfo->JumpAddress];
		if (pLabelInfo != nullptr)
		{
			ImGui::SameLine();
			ImGui::PushStyleColor(ImGuiCol_Text, 0xff808080);
			ImGui::Text("[%s]", pLabelInfo->Name.c_str());
			/*ImGui::SameLine();

			if (ImGui::ArrowButton("##btn", ImGuiDir_Right))
			{
				GoToAddress(state,pCodeInfo->JumpAddress);
			}*/

			if (ImGui::IsItemHovered())
			{
				ImGui::SetTooltip("Goto %04Xh", pCodeInfo->JumpAddress);
				if(ImGui::IsMouseDoubleClicked(0))
					GoToAddress(state, pCodeInfo->JumpAddress);

			}
				
			ImGui::PopStyleColor();
		}
	}

	DrawComment(pCodeInfo);

}

void DrawCodeDetails(FSpeccyUI *pUI, FCodeInfo *pCodeInfo)
{
}

void DrawDataInfo(FSpeccyUI *pUI, const FDataInfo *pDataInfo)
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
		const uint8_t val = ReadySpeccyByte(pUI->pSpeccy, pDataInfo->Address);
		ImGui::Text("db %02Xh '%c'", val, val);
	}
	break;

	case DataType::Word:
	{
		const uint16_t val = ReadySpeccyByte(pUI->pSpeccy, pDataInfo->Address) | (ReadySpeccyByte(pUI->pSpeccy, pDataInfo->Address + 1) << 8);
		ImGui::Text("dw %04Xh", val);
	}
	break;

	case DataType::Text:
	{
		std::string textString;
		for (int i = 0; i < pDataInfo->ByteSize; i++)
		{
			const char ch = ReadySpeccyByte(pUI->pSpeccy, pDataInfo->Address + i);
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

void DrawDataDetails(FSpeccyUI *pUI, FDataInfo *pDataInfo)
{
}

void SetItemData(FCodeAnalysisState &state, FItem *pItem)
{
	if (pItem->Type == ItemType::Data)
	{
		FDataInfo *pDataItem = static_cast<FDataInfo *>(pItem);
		if (pDataItem->DataType == DataType::Byte)
		{
			pDataItem->DataType = DataType::Word;
			pDataItem->ByteSize = 2;
			state.bCodeAnalysisDataDirty = true;
		}
		else if (pDataItem->DataType == DataType::Word)
		{
			pDataItem->DataType = DataType::Byte;
			pDataItem->ByteSize = 1;
			state.bCodeAnalysisDataDirty = true;

		}
	}
}

void SetItemText(FCodeAnalysisState &state, FItem *pItem)
{
	if (pItem->Type == ItemType::Data)
	{
		FDataInfo *pDataItem = static_cast<FDataInfo *>(pItem);
		if (pDataItem->DataType == DataType::Byte)
		{
			// set to ascii
			pDataItem->ByteSize = 0;	// reset byte counter

			uint16_t charAddr = pDataItem->Address;
			while (state.DataInfo[charAddr] != nullptr && state.DataInfo[charAddr]->DataType == DataType::Byte)
			{
				const uint8_t val = ReadySpeccyByte(state.pSpeccy, charAddr);
				if (val == 0 || val > 0x80)
					break;
				pDataItem->ByteSize++;
				charAddr++;
			}

			// did the operation fail? -revert to byte
			if (pDataItem->ByteSize == 0)
			{
				pDataItem->DataType = DataType::Byte;
				pDataItem->ByteSize = 1;
			}
			else
			{
				pDataItem->DataType = DataType::Text;
				state.bCodeAnalysisDataDirty = true;
			}
		}
	}
}
void AddLabelAtAddress(FCodeAnalysisState &state, uint16_t address)
{
	if (state.Labels[address] == nullptr)
	{
		LabelType labelType = LabelType::Data;
		if (state.CodeInfo[address] != nullptr)
			labelType = LabelType::Code;

		GenerateLabelForAddress(state, address, labelType);
		state.bCodeAnalysisDataDirty = true;
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

void DrawCodeAnalysisData(FSpeccyUI *pUI)
{
	FCodeAnalysisState &state = pUI->CodeAnalysis;
	const float line_height = ImGui::GetTextLineHeight();
	const float glyph_width = ImGui::CalcTextSize("F").x;
	const float cell_width = 3 * glyph_width;

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
		const FItem *pPrevItem = nullptr;

		while (clipper.Step())
		{
			for (int i = clipper.DisplayStart; i < clipper.DisplayEnd; i++)
			{
				const FItem* pItem = state.ItemList[i];

				ImGui::PushID(i);

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
					DrawLabelInfo(pUI, static_cast<const FLabelInfo *>(pItem));
					break;
				case ItemType::Code:
					DrawCodeInfo(pUI, static_cast<const FCodeInfo *>(pItem));
					break;
				case ItemType::Data:
					DrawDataInfo(pUI, static_cast<const FDataInfo *>(pItem));
					break;
				}

				pPrevItem = pItem;

				ImGui::PopID();
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
			DrawLabelDetails(pUI, static_cast<FLabelInfo *>(pItem));
			break;
		case ItemType::Code:
			DrawCodeDetails(pUI, static_cast<FCodeInfo *>(pItem));
			break;
		case ItemType::Data:
			DrawDataDetails(pUI, static_cast<FDataInfo *>(pItem));
			break;
		}
		ImGui::Text("Comments");
		ImGui::InputTextWithHint("##source", "Comments", &pItem->Comment);

	}
	ImGui::EndChild();


	
}