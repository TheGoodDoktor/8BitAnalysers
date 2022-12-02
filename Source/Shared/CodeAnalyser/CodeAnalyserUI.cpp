#include "CodeAnalyserUI.h"
#include "CodeAnalyser.h"

#include "Util/Misc.h"
#include "Util/GraphicsView.h"
#include "ImageViewer.h"


#include "imgui.h"
#include "misc/cpp/imgui_stdlib.h"
#include <algorithm>
#include <sstream>
//#include "chips/z80.h"
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

void CodeAnalyserGoToAddress(FCodeAnalysisState& state, uint16_t newAddress, bool bLabel)
{
	GoToAddress(state, newAddress, bLabel);
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
	for(int i=0;i<(int)state.ItemList.size();i++)
	{
		if (state.ItemList[i] != nullptr && state.ItemList[i]->Address == addr)
			return i;
	}
	return -1;
}

void DrawAddressLabel(FCodeAnalysisState &state, uint16_t addr, bool bFunctionRel)
{
	int labelOffset = 0;
	const char *pLabelString = nullptr;
	
	for(int addrVal = addr; addrVal >= 0; addrVal--)
	{
		const FLabelInfo* pLabel = state.GetLabelForAddress(addrVal);
		if(pLabel != nullptr)
		{
			if (bFunctionRel == false || pLabel->LabelType == LabelType::Function)
			{
				pLabelString = pLabel->Name.c_str();
				break;
			}
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
					if(startIndex + line < (int)state.ItemList.size())
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

void DrawCodeAddress(FCodeAnalysisState &state, uint16_t addr, bool bFunctionRel)
{
	//ImGui::PushStyleColor(ImGuiCol_Text, 0xff00ffff);
	ImGui::Text("%s", NumStr(addr));
	//ImGui::PopStyleColor();
	ImGui::SameLine();
	DrawAddressLabel(state, addr, bFunctionRel);
}

void DrawCallStack(FCodeAnalysisState& state)
{
	if (state.CallStack.empty() == false)
	{
		const FLabelInfo* pLabel = state.GetLabelForAddress(state.CallStack.back().FunctionAddr);
		if (pLabel != nullptr)
		{
			ImGui::Text("%s :", pLabel->Name.c_str());
			ImGui::SameLine();
		}
	}
	DrawCodeAddress(state, state.CPUInterface->GetPC(), false);	// draw current PC

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
		DrawCodeAddress(state, state.CallStack[i].CallAddr, false);
	}
}

void DrawTrace(FCodeAnalysisState& state)
{
	const float line_height = ImGui::GetTextLineHeight();
	ImGuiListClipper clipper((int)state.FrameTrace.size(), line_height);

	while (clipper.Step())
	{
		for (int i = clipper.DisplayStart; i < clipper.DisplayEnd; i++)
		{
			DrawCodeAddress(state, state.FrameTrace[state.FrameTrace.size() - i - 1], false);	// draw current PC
		}
	}
	
}

void DrawComment(const FItem *pItem, float offset = 0.0f)
{
	if(pItem != nullptr && pItem->Comment.empty() == false)
	{
		ImGui::SameLine(offset);
		ImGui::PushStyleColor(ImGuiCol_Text, 0xff008000);
		ImGui::Text("\t// %s", pItem->Comment.c_str());
		ImGui::PopStyleColor();
	}
}

void DrawLabelInfo(FCodeAnalysisState &state, const FLabelInfo *pLabelInfo)
{
	const FCodeInfo* pCodeInfo = state.GetCodeInfoForAddress(pLabelInfo->Address);	// for self-modifying code

	ImVec4 labelColour = ImVec4(0.5f, 0.5f, 0.5f, 1.0f);
	if (state.HighlightAddress == pLabelInfo->Address)
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
			DrawCodeAddress(state, caller.first);
		}
		ImGui::EndTooltip();
	}

	DrawComment(pLabelInfo);	
}

void DrawLabelDetails(FCodeAnalysisState &state, FLabelInfo *pLabelInfo)
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
		//DrawAddressLabel( state, caller.first );
		//ImGui::Text( "%s", NumStr(caller.first) );
		//DrawAddressLabel( state, caller.first );

		/*const int index = GetItemIndexForAddress(state, caller.first);
		if ( index != -1 )
		{
			DrawCodeAnalysisItemAtIndex( state, index );
		}*/
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
					DrawCodeAddress(state, caller.first);
				}
				break;
			}
		}
	}
}

float DrawDataCharMapLine(FCodeAnalysisState& state, const FDataInfo* pDataInfo)
{
	const float line_height = ImGui::GetTextLineHeight();
	const float rectSize = line_height + 4;
	ImDrawList* dl = ImGui::GetWindowDrawList();
	ImVec2 pos = ImGui::GetCursorScreenPos();
	pos.x += 150.0f;
	const float startPos = pos.x;
	pos.y -= rectSize + 2;

	for (int byte = 0; byte < pDataInfo->ByteSize; byte++)
	{
		const uint8_t val = state.CPUInterface->ReadByte(pDataInfo->Address + byte);
		const ImVec2 rectMin(pos.x, pos.y);
		const ImVec2 rectMax(pos.x + rectSize, pos.y + rectSize);
		char valTxt[8];
		sprintf_s(valTxt, "%02x", val);
		dl->AddRect(rectMin, rectMax, 0xffffffff);
		dl->AddText(ImVec2(pos.x + 1,pos.y + 1), 0xffffffff, valTxt);
		pos.x += rectSize;
	}
	return pos.x;

}

// returns how much space it took
float DrawDataBitmapLine(FCodeAnalysisState& state, const FDataInfo* pDataInfo)
{
	const float line_height = ImGui::GetTextLineHeight();
	const float rectSize = line_height + 4;
	ImDrawList* dl = ImGui::GetWindowDrawList();
	ImVec2 pos = ImGui::GetCursorScreenPos();
	pos.x += 200.0f;
	const float startPos = pos.x;
	pos.y -= rectSize + 2;

	for (int byte = 0; byte < pDataInfo->ByteSize; byte++)
	{
		const uint8_t val = state.CPUInterface->ReadByte(pDataInfo->Address + byte);

		for (int bit = 7; bit >= 0; bit--)
		{
			const ImVec2 rectMin(pos.x, pos.y);
			const ImVec2 rectMax(pos.x + rectSize, pos.y + rectSize);
			if (val & (1 << bit))
				dl->AddRectFilled(rectMin, rectMax, 0xffffffff);
			else
				dl->AddRect(rectMin, rectMax, 0xffffffff);

			pos.x += rectSize;
		}
	}

	//return pos.x - startPos;
	return pos.x;
}

void DrawDataInfo(FCodeAnalysisState &state, const FDataInfo *pDataInfo)
{
	const float line_height = ImGui::GetTextLineHeight();
	const float glyph_width = ImGui::CalcTextSize("F").x;
	const float cell_width = 3 * glyph_width;
	const int framesSinceWritten = state.CurrentFrameNo - pDataInfo->LastFrameWritten;
	const int framesSinceRead = state.CurrentFrameNo - pDataInfo->LastFrameRead;
	const int wBrightVal = (255 - std::min(framesSinceWritten << 2, 255)) & 0xff;
	const int rBrightVal = (255 - std::min(framesSinceRead << 2, 255)) & 0xff;
	float offset = 0;

	if (rBrightVal > 0 || wBrightVal > 0)
	{
		const ImU32 pc_color = 0xFF00FFFF;
		const ImU32 brd_color = 0xFF000000;

		ImVec2 pos = ImGui::GetCursorScreenPos();
		ImDrawList* dl = ImGui::GetWindowDrawList();
		const float lh2 = (float)(int)(line_height / 2);

		if (wBrightVal > 0)
		{
			const ImVec2 a(pos.x + 2, pos.y);
			const ImVec2 b(pos.x + 12, pos.y + lh2);
			const ImVec2 c(pos.x + 2, pos.y + line_height);

			const ImU32 col = 0xff000000 | (wBrightVal << 0);
			dl->AddTriangleFilled(a, b, c, col);
			dl->AddTriangle(a, b, c, brd_color);
		}

		pos.x += 10;

	
		if(rBrightVal>0)
		{
			const ImVec2 a(pos.x + 2, pos.y);
			const ImVec2 b(pos.x + 12, pos.y + lh2);
			const ImVec2 c(pos.x + 2, pos.y + line_height);
			
			const ImU32 col = 0xff000000 |  (rBrightVal << 8);
			dl->AddTriangleFilled(a, b, c, col);
			dl->AddTriangle(a, b, c, brd_color);
		}
	}

	// show if breakpointed
	if (state.CPUInterface->IsAddressBreakpointed(pDataInfo->Address))
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
	
	ImGui::Text("\t%s", NumStr(pDataInfo->Address));
	const float line_start_x = ImGui::GetCursorPosX();
	ImGui::SameLine(line_start_x + cell_width * 4 + glyph_width * 2);
		
	if (pDataInfo->bShowCharMap)
	{
		ImGui::Text("CM:");
		DrawDataCharMapLine(state, pDataInfo);
		return;
	}

	switch (pDataInfo->DataType)
	{
	case DataType::Byte:
	{
		const uint8_t val = state.CPUInterface->ReadByte(pDataInfo->Address);
		if (val == '\n')// carriage return messes up list
			ImGui::Text("db %s '<cr>'", NumStr(val), val);
		else
			ImGui::Text("db %s '%c'", NumStr(val), val);
	}
	break;

	case DataType::ByteArray:
	{
		uint8_t val = state.CPUInterface->ReadByte(pDataInfo->Address);
		
		ImGui::Text("db %s", NumStr(val));
		for (int i = 1; i < pDataInfo->ByteSize; i++)	// first word already written
		{
			val = state.CPUInterface->ReadByte(pDataInfo->Address + i);
			ImGui::SameLine();
			ImGui::Text(",%s", NumStr(val));
		}
	}
	break;

	case DataType::Word:
	{
		const uint16_t val = state.CPUInterface->ReadWord(pDataInfo->Address);
		ImGui::Text("dw %s", NumStr(val));
		// draw address as label - optional?
		ImGui::SameLine();	
		DrawAddressLabel(state, val);
	}
	break;

	case DataType::WordArray:
	{
		uint16_t val = state.CPUInterface->ReadWord(pDataInfo->Address);
		const int wordCount = pDataInfo->ByteSize / 2;

		ImGui::Text("dw %s", NumStr(val));
		for (int i = 1; i < wordCount; i++)	// first word already written
		{
			val = state.CPUInterface->ReadWord(pDataInfo->Address + (i * 2));
			ImGui::SameLine();
			ImGui::Text(",%s", NumStr(val));
		}
	}
	break;

	case DataType::Text:
	{
		std::string textString;
		for (int i = 0; i < pDataInfo->ByteSize; i++)
		{
			const char ch = state.CPUInterface->ReadByte(pDataInfo->Address + i);
			if (ch == '\n')
				textString += "<cr>";
			if (pDataInfo->bBit7Terminator && ch & (1 << 7))	// check bit 7 terminator flag
				textString += ch & ~(1 << 7);	// remove bit 7
			else
				textString += ch;
		}
		ImGui::Text("ascii '%s'", textString.c_str());
	}
	break;

	case DataType::Bitmap:
		ImGui::Text("Bitmap");
		offset = DrawDataBitmapLine(state, pDataInfo);
		break;
	case DataType::CharacterMap:
		ImGui::Text("Charmap");
		offset = DrawDataCharMapLine(state, pDataInfo);
		break;
	case DataType::Graphics:
	case DataType::Blob:
	default:
		ImGui::Text("%d Bytes", pDataInfo->ByteSize);
		break;
	}

	/*if (pDataInfo->bShowBinary)
	{
		offset = DrawDataBitmapLine(state, pDataInfo);
	}*/

	DrawComment(pDataInfo, offset);
}

void DrawDataValueGraph(float val, bool bReset)
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
		values[values_offset] = val;
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
		sprintf_s(overlay, "avg %f", average);
		ImGui::PlotLines("Lines", values, IM_ARRAYSIZE(values), values_offset, overlay, 0.0f, 255.0f, ImVec2(0, 80));
	}
	//ImGui::PlotHistogram("Histogram", arr, IM_ARRAYSIZE(arr), 0, NULL, 0.0f, 1.0f, ImVec2(0, 80));
}

void DrawDataValueGraph(uint8_t val, bool bReset)
{
	DrawDataValueGraph(static_cast<float>(val), bReset);
}

void DrawDataValueGraph(uint16_t val, bool bReset)
{
	DrawDataValueGraph(static_cast<float>(val), bReset);
}

void DrawDataDetails(FCodeAnalysisState &state, FDataInfo *pDataInfo)
{
	switch (pDataInfo->DataType)
	{
	case DataType::Byte:
		DrawDataValueGraph(state.CPUInterface->ReadByte(pDataInfo->Address),false);
	break;

	case DataType::Word:
		DrawDataValueGraph(state.CPUInterface->ReadWord(pDataInfo->Address), false);
		break;

	case DataType::Text:
	{
		std::string textString;
		for (int i = 0; i < pDataInfo->ByteSize; i++)
		{
			const char ch = state.CPUInterface->ReadByte(pDataInfo->Address + i);
			if (ch == '\n')
				textString += "<cr>";
			if (pDataInfo->bBit7Terminator && ch & (1 << 7))	// check bit 7 terminator flag
				textString += ch & ~(1 << 7);	// remove bit 7
			else
				textString += ch;
		}
		ImGui::Text("%s", textString.c_str());
		int length = pDataInfo->ByteSize;
		if(ImGui::InputInt("Length",&length))
		{
			pDataInfo->ByteSize = length;
			state.bCodeAnalysisDataDirty = true;
		}
	}
	break;

	case DataType::Graphics:
		// TODO: GFX as texture?
		//point gfx viewer?
		break;

	case DataType::Blob:
		// draw memory detail
		break;

	case DataType::Image:
	// This is currently disabled
	if(0)
	{
		FImageData* pImageData = pDataInfo->ImageData;
		bool bRebuildImage = false;
		int sz[] = { pImageData->XSizeChars,pImageData->YSizeChars };
		
		if (ImGui::InputInt2("Image Size (chars)", sz))
		{
			pDataInfo->ByteSize = pImageData->SetSizeChars(sz[0], sz[1]);	
			state.bCodeAnalysisDataDirty = true;	// force redraw of items
			bRebuildImage = true;
		}

		// viewer drop down
		const auto& viewerList = GetImageViewerList();
		if (viewerList.empty() == false)
		{
			if (ImGui::BeginCombo("Viewer", viewerList[pImageData->ViewerId]->GetName()))
			{
				for (int i = 0; i < (int)viewerList.size(); i++)
				{
					const bool bSelected = i == pImageData->ViewerId;
					if (ImGui::Selectable(viewerList[i]->GetName(), bSelected))
					{
						pImageData->ViewerId = i;
						bRebuildImage = true;
					}

					if (bSelected)
						ImGui::SetItemDefaultFocus();
				}
				ImGui::EndCombo();
			}
		}
		
		// Draw image
		if (pDataInfo->ImageData->GraphicsView == nullptr)
			bRebuildImage = true;

		if(bRebuildImage)
		{ 
			delete pImageData->GraphicsView;

			pImageData->GraphicsView = new FGraphicsView(64, 64);// pDataInfo->ImgData.XSizeChars * 8, pDataInfo->ImgData.YSizeChars * 8);
			pImageData->GraphicsView->Clear();

			viewerList[pImageData->ViewerId]->DrawImageToView(
				pDataInfo->Address,
				pImageData->XSizeChars, pImageData->YSizeChars,
				pImageData->GraphicsView,
				state.CPUInterface);
		}
		pImageData->GraphicsView->Draw();
	}
	break;
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

	ImGui::Text("Last Writer: ");
	ImGui::SameLine();
	DrawCodeAddress(state,state.GetLastWriterForAddress(pDataInfo->Address));
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
		state.bCodeAnalysisDataDirty = true;
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
#ifdef ENABLE_IMAGE_TYPE
		else if (ImGui::IsKeyPressed(state.KeyConfig[(int)Key::SetItemImage]))
		{
			SetItemImage(state, state.pCursorItem);
		}
#endif
		else if (ImGui::IsKeyPressed(state.KeyConfig[(int)Key::ToggleItemBinary]))
		{
			if (state.pCursorItem->Type == ItemType::Data)
			{
				FDataInfo* pDataItem = static_cast<FDataInfo*>(state.pCursorItem);
				if(pDataItem->DataType != DataType::Bitmap)
					pDataItem->DataType = DataType::Bitmap;
				else
					pDataItem->DataType = DataType::Byte;
				//pDataItem->bShowBinary = !pDataItem->bShowBinary;
			}
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
		else if (ImGui::IsKeyPressed(state.KeyConfig[(int)Key::AddCommentBlock]))
		{
			AddCommentBlock(state, state.pCursorItem->Address);
		}
		else if (ImGui::IsKeyPressed(state.KeyConfig[(int)Key::Breakpoint]))
		{
			state.CPUInterface->ToggleDataBreakpointAtAddress(state.pCursorItem->Address, state.pCursorItem->ByteSize);
		}
	}
	 
	if (ImGui::IsKeyPressed(state.KeyConfig[(int)Key::BreakContinue]))
	{
		if (state.CPUInterface->ShouldExecThisFrame())
		{
			state.CPUInterface->Break();
			state.TrackPCFrame = true;
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


	if (ImGui::BeginPopup("Enter Comment Text", ImGuiWindowFlags_AlwaysAutoResize))
	{
		ImGui::SetKeyboardFocusHere();
		if (ImGui::InputText("##comment", &state.pCursorItem->Comment, ImGuiInputTextFlags_EnterReturnsTrue))
		{
			ImGui::CloseCurrentPopup();
		}
		ImGui::SetItemDefaultFocus();
		ImGui::EndPopup();
	}

	if (ImGui::BeginPopup("Enter Label Text", ImGuiWindowFlags_AlwaysAutoResize))
	{
		FLabelInfo *pLabel = (FLabelInfo *)state.pCursorItem;
		
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

		// loop across address range
		for (int addr = 0; addr < (1 << 16); addr++)
		{
			FCommentBlock* pCommentBlock = state.GetCommentBlockForAddress(addr);
			if (pCommentBlock != nullptr)
			{
				// split comment into lines
				std::stringstream stringStream(pCommentBlock->Comment);
				std::string line;

				while (std::getline(stringStream, line, '\n'))
				{
					if (line.empty() || line[0] == '@')	// skip lines starting with @ - we might want to create items from them in future
						continue;

					FCommentLine* pLine = FCommentLine::Allocate();
					pLine->Comment = line;
					pLine->Address = addr;
					state.ItemList.push_back(pLine);	
				}
			}
			
			FLabelInfo* pLabelInfo = state.GetLabelForAddress(addr);
			if (pLabelInfo != nullptr)
			{
				state.ItemList.push_back(pLabelInfo);

			}

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
#ifdef ENABLE_IMAGE_TYPE
			if (ImGui::Selectable("Set as Image (I)"))
			{
				SetItemImage(state, pItem);
			}
#endif
			if (ImGui::Selectable("Toggle Data Breakpoint"))
				state.CPUInterface->ToggleDataBreakpointAtAddress(pItem->Address, pItem->ByteSize);
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
			state.CPUInterface->GraphicsViewerSetAddress(pItem->Address);
		}

		ImGui::EndPopup();
	}
}

void DrawCodeAnalysisItemAtIndex(FCodeAnalysisState& state, int i)
{
	assert(i < (int)state.ItemList.size());
	FItem* pItem = state.ItemList[i];
	bool bHighlight = (pItem->Address == state.HighlightAddress);
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
	bool bSelected = (pItem == state.pCursorItem) || (pItem->Address >= state.DataFormattingOptions.StartAddress && pItem->Address <= state.DataFormattingOptions.EndAddress);
	if (ImGui::Selectable("##codeanalysisline", bSelected, 0))
	{
		state.pCursorItem = state.ItemList[i];
		state.CursorItemIndex = i;

		// Select Data Formatting Range
		if (state.DataFormattingTabOpen && pItem->Type == ItemType::Data)
		{
			ImGuiIO& io = ImGui::GetIO();
			if (io.KeyShift)
			{
				state.DataFormattingOptions.EndAddress = state.pCursorItem->Address;
			}
			else
			{
				state.DataFormattingOptions.EndAddress = state.pCursorItem->Address;
				state.DataFormattingOptions.StartAddress = state.pCursorItem->Address;
			}
		}
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
	case ItemType::CommentLine:
		DrawCommentLine(state, static_cast<const FCommentLine*>(pItem));
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

void DrawDebuggerButtons(FCodeAnalysisState &state)
{
	if (state.CPUInterface->ShouldExecThisFrame())
	{
		if (ImGui::Button("Break (F5)"))
		{
			state.CPUInterface->Break();
			state.TrackPCFrame = true;
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
		state.TrackPCFrame = true;
	}
	ImGui::SameLine();
	if (ImGui::Button("Step Into (F11)"))
	{
		state.CPUInterface->StepInto();
		state.TrackPCFrame = true;
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

void DrawCodeAnalysisData(FCodeAnalysisState &state)
{
	//FSpeccy *pSpeccy = state.pSpeccy;
	const float line_height = ImGui::GetTextLineHeight();
	const float glyph_width = ImGui::CalcTextSize("F").x;
	const float cell_width = 3 * glyph_width;

	state.HighlightAddress = state.HoverAddress;
	state.HoverAddress = -1;

	if (state.CPUInterface->ShouldExecThisFrame())
		state.CurrentFrameNo++;

	UpdateItemList(state);

	if (ImGui::ArrowButton("##btn", ImGuiDir_Left))
		GoToPreviousAddress(state);
	ImGui::SameLine();
	if (ImGui::Button("Jump To PC"))
		GoToAddress(state,state.CPUInterface->GetPC());
	ImGui::SameLine();
	static int addrInput = 0;
	const ImGuiInputTextFlags inputFlags = (GetNumberDisplayMode() == ENumberDisplayMode::Decimal) ? ImGuiInputTextFlags_CharsDecimal : ImGuiInputTextFlags_CharsHexadecimal;
	if (ImGui::InputInt("Jump To", &addrInput, 1, 100, inputFlags))
		GoToAddress(state, addrInput);

	if (state.TrackPCFrame == true)
	{
		state.GoToAddress = state.CPUInterface->GetPC();
		state.TrackPCFrame = false;
	}
	
	DrawDebuggerButtons(state);

	if (ImGui::Button("Reset Memory Logs"))
	{
		ResetMemoryLogs(state);
	}
	ImGui::SameLine();
	ImGui::Text("Stack range: ");
	DrawAddressLabel(state, state.StackMin);
	ImGui::SameLine();
	DrawAddressLabel(state, state.StackMax);

	if(ImGui::BeginChild("##analysis", ImVec2(ImGui::GetWindowContentRegionWidth() * 0.75f, 0), true))
	{
		// jump to address
		if (state.GoToAddress != -1)
		{
			const int kJumpViewOffset = 5;
			for (int item = 0; item < (int)state.ItemList.size(); item++)
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
		float height = ImGui::GetWindowContentRegionMax().y - ImGui::GetWindowContentRegionMin().y;
		if (ImGui::BeginChild("##cadetails", ImVec2(0, height / 2), true))
			DrawDetailsPanel(state);
		ImGui::EndChild();
		if (ImGui::BeginChild("##caglobals", ImVec2(0, 0), true))
			DrawGlobals(state);
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

		ImGui::EndTabBar();
	}
}

void DrawLabelList(FCodeAnalysisState &state, std::vector<FLabelInfo *> labelList)
{
	static std::string filterText;
	ImGui::InputText("Filter", &filterText);
	if (ImGui::BeginChild("GlobalLabelList", ImVec2(0, 0), false))
	{
		for (FLabelInfo *pLabelInfo : labelList)
		{
			if (filterText.empty() || pLabelInfo->Name.find(filterText) != std::string::npos)
			{
				if (ImGui::Selectable(pLabelInfo->Name.c_str(), state.pCursorItem == pLabelInfo))
				{
					GoToAddress(state, pLabelInfo->Address, true);
				}
			}
		}
	}
	ImGui::EndChild();
}

void DrawFormatTab(FCodeAnalysisState& state)
{
	FDataFormattingOptions& formattingOptions = state.DataFormattingOptions;
	const ImGuiInputTextFlags inputFlags = (GetNumberDisplayMode() == ENumberDisplayMode::Decimal) ? ImGuiInputTextFlags_CharsDecimal : ImGuiInputTextFlags_CharsHexadecimal;

	if (state.DataFormattingTabOpen == false)
	{
		if (state.pCursorItem)
		{
			formattingOptions.StartAddress = state.pCursorItem->Address;
			formattingOptions.EndAddress = state.pCursorItem->Address;
		}

		state.DataFormattingTabOpen = true;
	}

	// Set Start address of region to format
	ImGui::PushID("Start");
	ImGui::InputInt("Start Address", &formattingOptions.StartAddress, 1, 100, inputFlags);
	ImGui::PopID();

	// Set End address of region to format
	ImGui::PushID("End");
	ImGui::InputInt("End Address", &formattingOptions.EndAddress, 1, 100, inputFlags);
	ImGui::PopID();

	if (ImGui::Button("Clear Selection"))
	{
		formattingOptions = FDataFormattingOptions();
	}


	const char* dataTypes[] = { "Byte", "Word", "Bitmap", "Char Map" };
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
		break;
	case 1:
		formattingOptions.DataType = DataType::Word;
		formattingOptions.ItemSize = 2;
		break;
	case 2:
		formattingOptions.DataType = DataType::Bitmap;
		ImGui::InputInt("Item Size", &formattingOptions.ItemSize);
		break;
	case 3:
		formattingOptions.DataType = DataType::CharacterMap;
		ImGui::InputInt("Item Size", &formattingOptions.ItemSize);
		break;
	}

	static int itemCount = 1;
	ImGui::InputInt("Item Count", &itemCount);
	ImGui::SameLine();
	if (ImGui::Button("Set"))
		formattingOptions.EndAddress = formattingOptions.StartAddress + (itemCount * formattingOptions.ItemSize);

	//ImGui::Checkbox("Binary Visualisation", &formattingOptions.BinaryVisualisation);
	//ImGui::Checkbox("Char Map Visualisation", &formattingOptions.CharMapVisualisation);
	ImGui::Checkbox("Clear Code Info", &formattingOptions.ClearCodeInfo);
	ImGui::Checkbox("Clear Labels", &formattingOptions.ClearLabels);

	if (formattingOptions.IsValid())
	{
		if (ImGui::Button("Format"))
		{
			FormatData(state, formattingOptions);
			state.bCodeAnalysisDataDirty = true;
			//formattingOptions = FDataFormattingOptions();	// clear selection
		}
	}
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

		if (ImGui::BeginTabItem("Format"))	
		{
			DrawFormatTab(state);
			ImGui::EndTabItem();
		}
		else
		{
			state.DataFormattingTabOpen = false;
		}

		ImGui::EndTabBar();
	}
}


