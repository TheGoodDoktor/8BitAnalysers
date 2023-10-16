#include "CodeAnalyserUI.h"

#include "../CodeAnalyser.h"
#include "CodeToolTips.h"

#include <math.h>

#include "imgui.h"

void ShowCodeAccessorActivity(FCodeAnalysisState& state, const FAddressRef accessorCodeAddr)
{
	const FCodeInfo* pCodeInfo = state.GetCodeInfoForAddress(accessorCodeAddr);
	if (pCodeInfo != nullptr)
	{
		const int framesSinceExecuted = pCodeInfo->FrameLastExecuted != -1 ? state.CurrentFrameNo - pCodeInfo->FrameLastExecuted : 255;
		const int brightVal = (255 - std::min(framesSinceExecuted << 2, 255)) & 0xff;
		const bool bPCLine = accessorCodeAddr == state.CPUInterface->GetPC();

		if (bPCLine || brightVal > 0)
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

			if (bPCLine)
			{
				dl->AddTriangleFilled(a, b, c, pc_color);
				dl->AddTriangle(a, b, c, brd_color);

				if (state.Debugger.IsStopped())
				{
					const ImVec2 lineStart(pos.x + 18, ImGui::GetItemRectMax().y);
					dl->AddLine(lineStart, ImGui::GetItemRectMax(), pc_color);
				}
			}
			else
			{
				dl->AddTriangleFilled(a, b, c, col);
				dl->AddTriangle(a, b, c, brd_color);
			}
		}
	}
}

#if 0
void DrawJumpIndicator(int nDirection)
{
	ImGui::SameLine();
	ImVec2 pos = ImGui::GetCursorScreenPos();
	ImGui::Text("");

	const float lineHeight = ImGui::GetTextLineHeight();
	const ImU32 col = 0xffffffff;

	const float ahh = 5.0f; // arrow head height
	const float ahw = 4.0f; // arrow head width
	const float vth = 4.0f; // vertical tail height
	const float htw = 6.0f; // horizontal tail width

	ImDrawList* dl = ImGui::GetWindowDrawList();

	// pos is the tip of the arrow

	pos.x += 4.0f;

	if (nDirection > 0) // up arrow
	{
		pos.y += 2.0f;
		dl->AddTriangleFilled(ImVec2(pos.x + ahw, pos.y + ahh), ImVec2(pos.x - ahw, pos.y + ahh), pos, col);
		dl->AddRectFilled(ImVec2(pos.x - 1.0f, pos.y + ahh), ImVec2(pos.x + 1.0f, pos.y + ahh + vth), col); // vertical tail
		dl->AddRectFilled(ImVec2(pos.x - htw, pos.y + ahh + vth - 1.0f), ImVec2(pos.x + 1.0f, pos.y + ahh + vth + 1.0f), col); // horizontal tail
	}
	else if (nDirection < 0) // down arrow
	{
		pos.y += lineHeight - 1.0f;
		dl->AddTriangleFilled(ImVec2(pos.x - ahw, pos.y - ahh), ImVec2(pos.x + ahw, pos.y - ahh), pos, col);
		dl->AddRectFilled(ImVec2(pos.x - 1.0f, pos.y - ahh), ImVec2(pos.x + 1.0f, pos.y - ahh - vth), col); // vertical tail
		dl->AddRectFilled(ImVec2(pos.x - htw, pos.y - ahh - vth - 1.0f), ImVec2(pos.x + 1.0f, pos.y - ahh - vth + 1.0f), col); // horizontal tail
	}
	else // left arrow 
	{
		// todo
	}
}
#endif

void DrawBranchLines(FCodeAnalysisState& state, FCodeAnalysisViewState& viewState, const FCodeAnalysisItem& item)
{
	const FCodeInfo* pCodeInfo = static_cast<const FCodeInfo*>(item.Item);
	ImDrawList* dl = ImGui::GetWindowDrawList();
	const ImVec2 pos = ImGui::GetCursorScreenPos();
	const float lineHeight = ImGui::GetTextLineHeight();

	ImU32 lineCol = 0xff7f7f7f;	// grey
	if (viewState.HighlightAddress == pCodeInfo->JumpAddress)
		lineCol = 0xff00ff00;	// green
	float ypos;
	if (viewState.GetYPosForAddress(pCodeInfo->JumpAddress, ypos))
	{
		const float distance = fabsf(ypos - pos.y);
		const float xEnd = pos.x + state.Config.AddressPos - 2.0f;
		ImVec2 lineStart = pos;
		const int noLines = static_cast<int>(distance / lineHeight);
		const int maxIndent = state.Config.BranchMaxIndent;
		const int indentAmount = maxIndent - std::min(noLines / state.Config.BranchLinesPerIndent, maxIndent);

		lineStart.x += indentAmount * state.Config.BranchSpacing;
		lineStart.y += lineHeight * 0.5f;	// middle

		ImVec2 lineEnd = lineStart;
		lineEnd.y = ypos + lineHeight * 0.5f;// middle

		viewState.JumpLineIndent++;

		dl->AddLine(lineStart, { xEnd, lineStart.y }, lineCol);	// -
		dl->AddLine(lineStart, lineEnd, lineCol);				// |
		dl->AddLine(lineEnd, { xEnd, lineEnd.y }, lineCol);		// -

		// arrow
		const ImVec2 a = { xEnd - 6, lineEnd.y - 3 };
		const ImVec2 b = { xEnd - 6, lineEnd.y + 3 };
		const ImVec2 c = { xEnd, lineEnd.y };
		dl->AddTriangleFilled(a, b, c, lineCol);
	}
	else // do off-screen lines
	{
		const int thisIndex = GetItemIndexForAddress(state, item.AddressRef);
		const int jumpIndex = GetItemIndexForAddress(state, pCodeInfo->JumpAddress);
		const int noLines = abs(thisIndex - jumpIndex);
		const int maxIndent = state.Config.BranchMaxIndent;
		const int indentAmount = maxIndent - std::min(noLines / 5, maxIndent);

		const bool bDirectionUp = pCodeInfo->JumpAddress.Address < item.AddressRef.Address;
		ImVec2 lineStart = pos;
		lineStart.x += indentAmount * state.Config.BranchSpacing;
		lineStart.y += lineHeight * 0.5f;// middle

		const float xEnd = pos.x + state.Config.AddressPos;
		dl->AddLine(lineStart, { xEnd, lineStart.y }, lineCol);	// -
		if (bDirectionUp)
		{
			dl->AddLine(lineStart, { lineStart.x,0 }, lineCol);				// |
		}
		else
		{
			const float yPos = ImGui::GetWindowPos().y + ImGui::GetWindowHeight();
			dl->AddLine(lineStart, { lineStart.x,yPos }, lineCol);				// |
		}
	}
}

// this assumes that the code item is mapped into physical memory
void DrawCodeInfo(FCodeAnalysisState& state, FCodeAnalysisViewState& viewState, const FCodeAnalysisItem& item)
{
	const FCodeInfo* pCodeInfo = static_cast<const FCodeInfo*>(item.Item);
	FDebugger& debugger = state.Debugger;

	const float line_height = ImGui::GetTextLineHeight();
	const float glyph_width = ImGui::CalcTextSize("F").x;
	const float cell_width = 3 * glyph_width;
	const uint16_t physAddress = item.AddressRef.Address;
	ImDrawList* dl = ImGui::GetWindowDrawList();
	const ImVec2 pos = ImGui::GetCursorScreenPos();

	ShowCodeAccessorActivity(state, item.AddressRef);

	bool bDisplayBranchLine = state.pGlobalConfig->BranchLinesDisplayMode == 2;
	if (state.pGlobalConfig->BranchLinesDisplayMode == 1)
	{
		const bool bSelected = (item.Item == viewState.GetCursorItem().Item);
		const bool bHighlighted = (viewState.HighlightAddress == pCodeInfo->JumpAddress);
		bDisplayBranchLine = bSelected || bHighlighted;
	}

	if (bDisplayBranchLine)
	{
		// draw branch lines
		if (pCodeInfo->OperandType == EOperandType::JumpAddress && pCodeInfo->JumpAddress.IsValid() && pCodeInfo->bIsCall == false)
			DrawBranchLines(state, viewState, item);
	}

	// show if breakpointed
	FBreakpoint* pBP = debugger.GetBreakpointForAddress(item.AddressRef);
	if (pBP != nullptr)
	{
		const ImU32 bp_enabled_color = 0xFF0000FF;
		const ImU32 bp_disabled_color = 0xFF000088;
		const ImU32 brd_color = 0xFF000000;
		const float lh2 = (float)(int)(line_height / 2);
		const ImVec2 mid(pos.x, pos.y + lh2);

		dl->AddCircleFilled(mid, 7, pBP->bEnabled ? bp_enabled_color : bp_disabled_color);
		dl->AddCircle(mid, 7, brd_color);

		// enable/disable by clicking on breakpoint indicator
		if (ImGui::IsMouseClicked(ImGuiMouseButton_Left))
		{
			const ImVec2 mousePos = ImGui::GetMousePos();
			const ImVec2 dist(mousePos.x - mid.x, mousePos.y - mid.y);
			if ((dist.x * dist.x + dist.y * dist.y) < (8 * 8))
				pBP->bEnabled = !pBP->bEnabled;
		}
	}

	if (state.GetMachineState(physAddress))
	{
		ImDrawList* dl = ImGui::GetWindowDrawList();
		const ImVec2 pos = ImGui::GetCursorScreenPos();

		dl->AddRectFilled(ImVec2(pos.x - 12, pos.y), ImVec2(pos.x - 8, pos.y + line_height), 0xFFFF0000);
	}

	// regenerate code text if it hasn't been generated or SMC
	if (pCodeInfo->bSelfModifyingCode == true || pCodeInfo->Text.empty())
	{
		//UpdateCodeInfoForAddress(state, pCodeInfo->Address);
		WriteCodeInfoForAddress(state, physAddress);
	}

	// draw instruction address
	const float lineStartX = ImGui::GetCursorPosX();
	ImGui::SameLine(lineStartX + state.Config.AddressPos);
	ImGui::Text("%s", NumStr(physAddress));
	ImGui::SameLine(lineStartX + state.Config.AddressPos + state.Config.AddressSpace);

	// grey out NOPed code
	if (pCodeInfo->bNOPped)
		ImGui::PushStyleColor(ImGuiCol_Text, 0xff808080);

	if (state.pGlobalConfig->bShowOpcodeValues)
	{
		// Draw hex values of the instruction's opcode
		char tmp[16] = { 0 };
		const ImVec4 byteChangedCol(1.0f, 1.0f, 0.0f, 1.0f);
		const ImVec4 byteNormalCol(1.0f, 1.0f, 1.0f, 1.0f);
		bool bByteModified = false;
		for (int i = 0; i < 4; i++)
		{
			std::string strHexValue;

			if (i < pCodeInfo->ByteSize)
				snprintf(tmp, 16, "%02X", state.ReadByte(item.AddressRef.Address + i));
			else
				snprintf(tmp, 16, "  ");

			strHexValue += tmp;

			if (pCodeInfo->bSelfModifyingCode)
			{
				FDataInfo* pOperandData = state.GetWriteDataInfoForAddress(physAddress + i);
				if (pOperandData->Writes.IsEmpty() == false)
				{
					// Change the colour if this is self modifying code and the byte has been modified.
					bByteModified = true;
				}
			}
			else
			{
				bByteModified = false;
			}

			ImGui::TextColored(bByteModified ? byteChangedCol : byteNormalCol, "%s%s", strHexValue.c_str(), i == 3 ? "  " : "");
			ImGui::SameLine();
		}
	}

	ImGui::Text("%s", pCodeInfo->Text.c_str());	// draw the disassembly output for this instruction

	if (pCodeInfo->bNOPped)
		ImGui::PopStyleColor();

	if (ImGui::IsItemHovered())
	{
		ShowCodeToolTip(state, physAddress);
	}

	// draw jump address label name
	if (pCodeInfo->OperandType == EOperandType::JumpAddress && pCodeInfo->JumpAddress.IsValid())
	{
		DrawAddressLabel(state, viewState, pCodeInfo->JumpAddress);
	}
	else if (pCodeInfo->OperandType == EOperandType::Pointer && pCodeInfo->PointerAddress.IsValid())
	{
		DrawAddressLabel(state, viewState, pCodeInfo->PointerAddress);
	}

	DrawComment(pCodeInfo);

}

// this code assumes the item is in physical address space
void DrawCodeDetails(FCodeAnalysisState& state, FCodeAnalysisViewState& viewState, const FCodeAnalysisItem& item)
{
	FCodeInfo* pCodeInfo = static_cast<FCodeInfo*>(item.Item);
	const uint16_t physAddress = item.AddressRef.Address;

	if (DrawOperandTypeCombo("Operand Type", pCodeInfo->OperandType))
		pCodeInfo->Text.clear();	// clear for a rewrite

	if (state.Config.bShowBanks && pCodeInfo->OperandType == EOperandType::Pointer)
	{
		DrawBankInput(state, "Bank", pCodeInfo->PointerAddress.BankId);
	}

	if (ImGui::Checkbox("NOP out instruction", &pCodeInfo->bNOPped))
	{
		if (pCodeInfo->bNOPped == true)
		{
			assert(pCodeInfo->ByteSize <= sizeof(pCodeInfo->OpcodeBkp));
			// backup code
			for (int i = 0; i < pCodeInfo->ByteSize; i++)
			{
				pCodeInfo->OpcodeBkp[i] = state.ReadByte(physAddress + i);

				// NOP it out
				// Note: This is not going to work with ROM
				if (state.CPUInterface->CPUType == ECPUType::Z80)
					state.WriteByte(physAddress + i, 0);
				else if (state.CPUInterface->CPUType == ECPUType::M6502)
					state.WriteByte(physAddress + i, 0xEA);
			}
		}
		else
		{
			// Restore
			for (int i = 0; i < pCodeInfo->ByteSize; i++)
				state.WriteByte(physAddress + i, pCodeInfo->OpcodeBkp[i]);

		}
	}

	if (pCodeInfo->bSelfModifyingCode == true)
	{
		ImGui::Text("Self modifying code");

		for (int i = 1; i < pCodeInfo->ByteSize; i++)
		{
			FDataInfo* pOperandData = state.GetWriteDataInfoForAddress(physAddress + i);
			if (pOperandData->Writes.IsEmpty() == false)
			{
				ImGui::Text("Operand Writes:");
				for (const auto& writer : pOperandData->Writes.GetReferences())
				{
					DrawCodeAddress(state, viewState, writer);
				}
				break;
			}
		}
	}
}

