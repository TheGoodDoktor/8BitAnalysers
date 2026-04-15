#pragma once

#include "../CodeAnalyser.h"
#include "CodeAnalyserUI.h"

#include <imgui.h>

const ImVec4 g_RegNormalCol(1.0f, 1.0f, 1.0f, 1.0f);
const ImVec4 g_RegChangedCol(1.0f, 1.0f, 0.0f, 1.0f);

ImVec4 GetRegNormalCol()
{
	return g_RegNormalCol;
}

ImVec4 GetRegChangedCol()
{
	return g_RegChangedCol;
}

void EditByte(uint8_t* pByteData)
{
	const ENumberDisplayMode numMode = GetNumberDisplayMode();
	const float glyphWidth = ImGui::CalcTextSize("0").x;
	int flags = ImGuiInputTextFlags_EnterReturnsTrue;
	float width = glyphWidth * 2.0f;
	const char* format = "%02X";

	switch (numMode)
	{
	case ENumberDisplayMode::Decimal:
		width += glyphWidth;
		format = "%d";
		break;
	case ENumberDisplayMode::HexAitch:
	case ENumberDisplayMode::HexDollar:
		flags |= ImGuiInputTextFlags_CharsHexadecimal;
		break;
	default:
		break;
	}
	ImGui::PushID(pByteData);
	if (numMode == ENumberDisplayMode::HexDollar)
	{
		ImGui::Text("$");
		ImGui::SameLine(0, 0);
	}
	ImGui::SetNextItemWidth((float)width);
	ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(0, 0));
	ImGui::SetItemAllowOverlap();	// allow controls
	ImGui::InputScalar("##byteinput", ImGuiDataType_U8, pByteData, NULL, NULL, format, ImGuiInputTextFlags_EnterReturnsTrue | ImGuiInputTextFlags_EnterReturnsTrue);
	if (numMode == ENumberDisplayMode::HexAitch)
	{
		ImGui::SameLine(0, 0);
		ImGui::Text("h");
	}
	ImGui::PopStyleVar();
	ImGui::PopID();
}

void EditWord(uint16_t* pWordData)
{
	const ENumberDisplayMode numMode = GetNumberDisplayMode();
	int flags = ImGuiInputTextFlags_EnterReturnsTrue;
	const float glyphWidth = ImGui::CalcTextSize("0").x;
	float width = glyphWidth * 4.0f;
	const char* format = "%04X";

	switch (numMode)
	{
	case ENumberDisplayMode::Decimal:
		width += glyphWidth;
		format = "%d";
		break;
	case ENumberDisplayMode::HexAitch:
	case ENumberDisplayMode::HexDollar:
		flags |= ImGuiInputTextFlags_CharsHexadecimal;
		break;
	default:
		break;
	}

	ImGui::PushID(pWordData);
	if (numMode == ENumberDisplayMode::HexDollar)
	{
		ImGui::Text("$");
		ImGui::SameLine(0, 0);
	}
	ImGui::SetNextItemWidth((float)width);
	ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(0, 0));
	ImGui::SetItemAllowOverlap();	// allow controls
	ImGui::InputScalar("##wordinput", ImGuiDataType_U16, pWordData, NULL, NULL, format, flags);
	
	if (numMode == ENumberDisplayMode::HexAitch)
	{
		ImGui::SameLine(0, 0);
		ImGui::Text("h");
	}
	ImGui::PopStyleVar();

	ImGui::PopID();
}

void DoByteRegisterTooltip(uint8_t byteValue)
{
	if (ImGui::IsItemHovered())
	{
		ImGui::BeginTooltip();
		ImGui::Text("%s", NumStr(byteValue, ENumberDisplayMode::Decimal));
		ImGui::Text("%s", NumStr(byteValue, ENumberDisplayMode::Binary));
		ImGui::EndTooltip();
	}
}

void DrawByteRegister(FCodeAnalysisState& state, uint8_t* curByte, uint8_t oldByte, const char* fmt)
{
	const ImVec4 col = *curByte != oldByte ? g_RegChangedCol : g_RegNormalCol;
	ImGui::BeginGroup();
	if (state.bAllowEditing)
	{
		ImGui::TextColored(col, fmt, "");
		ImGui::SameLine();
		ImGui::PushStyleColor(ImGuiCol_Text, col);
		EditByte(curByte);
		ImGui::PopStyleColor();
	}
	else
	{
		ImGui::TextColored(col, fmt, NumStr(*curByte));
	}
	ImGui::EndGroup();

	DoByteRegisterTooltip(*curByte);
}

void DrawWordRegister(FCodeAnalysisState& state, uint16_t* curWord, uint16_t oldWord, const char* fmt)
{
	const ImVec4 col = *curWord != oldWord ? g_RegChangedCol : g_RegNormalCol;

	ImGui::BeginGroup();
	if (state.bAllowEditing)
	{
		ImGui::Text(fmt, "");
		ImGui::SameLine();
		ImGui::PushStyleColor(ImGuiCol_Text, col);
		EditWord(curWord);
		ImGui::PopStyleColor();
	}
	else
	{
		ImGui::TextColored(col, fmt, NumStr(*curWord));
	}
	DrawAddressLabel(state, state.GetFocussedViewState(), *curWord);
	ImGui::EndGroup();
}

void DrawFlag(FCodeAnalysisState& state, bool* curFlag, bool oldFlag)
{
	const ImVec4 col = *curFlag != oldFlag ? g_RegChangedCol : g_RegNormalCol;
	ImGui::PushID(curFlag);
	if (state.bAllowEditing)
	{
		ImGui::PushStyleColor(ImGuiCol_CheckMark, col);
		ImGui::Checkbox("##flag", curFlag);
		ImGui::PopStyleColor();
	}
	else
	{
		ImGui::TextColored(col, "%s", *curFlag ? "Y" : "N");
	}
	ImGui::PopID();
}