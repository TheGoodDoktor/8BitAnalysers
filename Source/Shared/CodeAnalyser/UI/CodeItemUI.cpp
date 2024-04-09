#include "CodeAnalyserUI.h"

#include "../CodeAnalyser.h"
#include "CodeToolTips.h"
#include "CodeAnalyser/DataTypes.h"

#include <math.h>

#include "imgui.h"
#include "UIColours.h"

void ShowCodeAccessorActivity(FCodeAnalysisState& state, const FAddressRef accessorCodeAddr)
{
	const FCodeInfo* pCodeInfo = state.GetCodeInfoForAddress(accessorCodeAddr);
	const int FrameLastExecuted = pCodeInfo != nullptr ? pCodeInfo->FrameLastExecuted : -1;
	// (pCodeInfo != nullptr)
	{
		const int framesSinceExecuted = FrameLastExecuted != -1 ? state.CurrentFrameNo - FrameLastExecuted : 255;
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

void DrawBranchLines(FCodeAnalysisState& state, FCodeAnalysisViewState& viewState, const FCodeAnalysisItem& item)
{
	const FCodeInfo* pCodeInfo = static_cast<const FCodeInfo*>(item.Item);
	ImDrawList* dl = ImGui::GetWindowDrawList();
	const ImVec2 pos = ImGui::GetCursorScreenPos();
	const float lineHeight = ImGui::GetTextLineHeight();

	ImU32 lineCol = 0xff7f7f7f;	// grey
	if (viewState.HighlightAddress == pCodeInfo->OperandAddress)
		lineCol = 0xff00ff00;	// green
	float ypos;
	if (viewState.GetYPosForAddress(pCodeInfo->OperandAddress, ypos))
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
		const int jumpIndex = GetItemIndexForAddress(state, pCodeInfo->OperandAddress);
		const int noLines = abs(thisIndex - jumpIndex);
		const int maxIndent = state.Config.BranchMaxIndent;
		const int indentAmount = maxIndent - std::min(noLines / 5, maxIndent);

		const bool bDirectionUp = pCodeInfo->OperandAddress.Address < item.AddressRef.Address;
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

bool EditHexDataItem(FCodeAnalysisState& state, uint16_t address)
{
	uint8_t val = state.ReadByte(address);

	ImGui::PushID(address);
	ImGui::SetNextItemWidth(ImGui::CalcTextSize("0").x * 2.0f);
	ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(0, 0));
	ImGui::SetItemAllowOverlap();	// allow controls

	bool bChanged = false;
	if (ImGui::InputScalar("##hexbyteinput", ImGuiDataType_U8, &val, NULL, NULL, "%02X", ImGuiInputTextFlags_EnterReturnsTrue | ImGuiInputTextFlags_EnterReturnsTrue))
	{
		// Write value
		state.CPUInterface->WriteByte(address, val);
		bChanged = true;
	}
	ImGui::PopStyleVar();

	ImGui::PopID();
	return bChanged;
}

// Get an offset from an instruction
// this looks for specific instructions that have an offset
// so far it's only Z80 IX/IY indexing instructions
// this should probably be moved to the CPU abstraction
int GetInstructionByteOffset(const FCodeAnalysisState& state, FAddressRef addr)
{
	if(state.CPUInterface->CPUType != ECPUType::Z80)
		return -1;

	const uint8_t instr = state.ReadByte(addr);
	switch (instr)
	{
	case 0xDD:	// IX
	case 0xFD:	// IY
			if(state.AdvanceAddressRef(addr,2))
				return state.ReadByte(addr);
			break;
	}
	return -1;
}

// this assumes that the code item is mapped into physical memory
void DrawCodeInfo(FCodeAnalysisState& state, FCodeAnalysisViewState& viewState, const FCodeAnalysisItem& item)
{
	FCodeInfo* pCodeInfo = static_cast<FCodeInfo*>(item.Item);
	FDebugger& debugger = state.Debugger;

	const float line_height = ImGui::GetTextLineHeight();
	const float glyph_width = ImGui::CalcTextSize("F").x;
	const float cell_width = 3 * glyph_width;
	const uint16_t physAddress = item.AddressRef.Address;
	const bool bHighlight = (viewState.HighlightAddress.IsValid() && viewState.HighlightAddress.Address >= physAddress && viewState.HighlightAddress.Address < physAddress + item.Item->ByteSize);

	ImDrawList* dl = ImGui::GetWindowDrawList();
	const ImVec2 pos = ImGui::GetCursorScreenPos();

	ShowCodeAccessorActivity(state, item.AddressRef);

	bool bDisplayBranchLine = state.pGlobalConfig->BranchLinesDisplayMode == 2;
	if (state.pGlobalConfig->BranchLinesDisplayMode == 1)
	{
		const bool bSelected = (item.Item == viewState.GetCursorItem().Item);
		const bool bHighlighted = (viewState.HighlightAddress == pCodeInfo->OperandAddress);
		bDisplayBranchLine = bSelected || bHighlighted;
	}

	if (bDisplayBranchLine)
	{
		// draw branch lines
		if (pCodeInfo->OperandType == EOperandType::JumpAddress && pCodeInfo->OperandAddress.IsValid() && pCodeInfo->bIsCall == false)
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
	ImGui::PushStyleColor(ImGuiCol_Text, bHighlight ? Colours::highlight : Colours::address);
	ImGui::Text("%s  ", NumStr(physAddress));
	ImGui::PopStyleColor();
	ImGui::SameLine(lineStartX + state.Config.AddressPos + state.Config.AddressSpace);

	if (state.pGlobalConfig->bShowOpcodeValues)
	{
		// Draw hex values of the instruction's opcode
		char tmp[16] = { 0 };
		const ImVec4 byteChangedCol(1.0f, 1.0f, 0.0f, 1.0f);
		const ImVec4 byteNormalCol(1.0f, 1.0f, 1.0f, 1.0f);
		bool bByteModified = false;
		for (int i = 0; i < 4; i++)
		{
			if (i < pCodeInfo->ByteSize)
				snprintf(tmp, 16, "%02X", state.ReadByte(item.AddressRef.Address + i));
			else
				snprintf(tmp, 16, "  ");

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

			if (state.bAllowEditing && i < pCodeInfo->ByteSize)
			{
				if (EditHexDataItem(state, physAddress + i))
				{
					pCodeInfo->Text.clear();
				}
			}
			else
			{
				ImGui::TextColored(bByteModified ? byteChangedCol : byteNormalCol, "%s", tmp);
			}
			
			ImGui::SameLine();
		}

		ImGui::Text(" ");
		ImGui::SameLine();
	}

	Markup::SetCodeInfo(pCodeInfo);
    const bool bNOPed = state.bAllowEditing && pCodeInfo->bNOPped;
	ImGui::PushStyleColor(ImGuiCol_Text, bNOPed ? Colours::noppedMnemonic : Colours::mnemonic);
	const bool bShownTooltip = Markup::DrawText(state,viewState,pCodeInfo->Text.c_str()); // draw the disassembly output for this instruction
	ImGui::PopStyleColor();
	Markup::SetCodeInfo(nullptr);

	// show struct members
	if(pCodeInfo->StructId != -1)
		state.GetDataTypes()->DrawStructMember(pCodeInfo->StructId, GetInstructionByteOffset(state, item.AddressRef));

	if (bShownTooltip == false && ImGui::IsItemHovered())
	{
		ShowCodeToolTip(state, physAddress);
	}

	#if 0
	// draw jump address label name
	if (pCodeInfo->OperandType == EOperandType::JumpAddress && pCodeInfo->JumpAddress.IsValid())
	{
		DrawAddressLabel(state, viewState, pCodeInfo->JumpAddress);
	}
	else if (pCodeInfo->OperandType == EOperandType::Pointer && pCodeInfo->PointerAddress.IsValid())
	{
		DrawAddressLabel(state, viewState, pCodeInfo->PointerAddress);
	}
	#endif
	DrawComment(state, viewState, pCodeInfo);

}

// this code assumes the item is in physical address space
void DrawCodeDetails(FCodeAnalysisState& state, FCodeAnalysisViewState& viewState, const FCodeAnalysisItem& item)
{
	FCodeInfo* pCodeInfo = static_cast<FCodeInfo*>(item.Item);
	const uint16_t physAddress = item.AddressRef.Address;

	if (DrawOperandTypeCombo("Operand Type", pCodeInfo))
		pCodeInfo->Text.clear();	// clear for a rewrite

	//if (pCodeInfo->OperandType == EOperandType::Struct)
	if(GetInstructionByteOffset(state, item.AddressRef) != -1)
	{
		state.GetDataTypes()->DrawStructComboBox("Struct", pCodeInfo->StructId);
	}

	if (state.Config.bShowBanks && pCodeInfo->OperandType == EOperandType::Pointer)
	{
		DrawBankInput(state, "Bank", pCodeInfo->OperandAddress.BankId);
	}

	if (state.bAllowEditing && ImGui::Checkbox("NOP out instruction", &pCodeInfo->bNOPped))
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

	if(pCodeInfo->Reads.IsEmpty() == false)
	{
		ImGui::Text("Reads:");
		for (const auto& read : pCodeInfo->Reads.GetReferences())
		{
			ShowDataItemActivity(state, read);

			ImGui::Text("   ");
			ImGui::SameLine();
			DrawCodeAddress(state, viewState, read);
		}
	}

	if (pCodeInfo->Writes.IsEmpty() == false)
	{
		ImGui::Text("Writes:");
		for (const auto& written : pCodeInfo->Writes.GetReferences())
		{
			ShowDataItemActivity(state, written);

			ImGui::Text("   ");
			ImGui::SameLine();
			DrawCodeAddress(state, viewState, written);
		}
	}
}

