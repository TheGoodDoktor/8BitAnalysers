#include "MCPSuggestionsUI.h"
#include "MCPSuggestions.h"

#include "Misc/EmuBase.h"
#include "CodeAnalyser/CodeAnalyser.h"
#include "CodeAnalyser/UI/CodeAnalyserUI.h"

#include <imgui.h>

static const char* GetSuggestionTypeLabel(EMCPSuggestionType type)
{
	switch (type)
	{
	case EMCPSuggestionType::RenameFunction:		return "Rename Function";
	case EMCPSuggestionType::SetFunctionDescription:return "Set Description";
	case EMCPSuggestionType::SetLabel:				return "Set Label";
	case EMCPSuggestionType::AddComment:			return "Add Comment";
	default:										return "Unknown";
	}
}

void DrawMCPSuggestionsUI(FEmuBase* pEmu, FMCPSuggestionQueue& queue)
{
	// Auto-load suggestions from disk when the active game changes
	if (pEmu->GetProjectConfig() != nullptr)
	{
		static std::string sLastLoadedPath;
		const std::string suggestionsPath = pEmu->GetGameWorkspaceRoot() + "Suggestions.json";
		if (suggestionsPath != sLastLoadedPath)
		{
			queue.Load(suggestionsPath);
			sLastLoadedPath = suggestionsPath;
		}
	}

	ImGui::SetNextWindowSize(ImVec2(700, 400), ImGuiCond_FirstUseEver);
	bool bOpen = true;
	if (!ImGui::Begin("MCP Suggestions"))//, &bOpen, ImGuiWindowFlags_NoCollapse))
	{
		ImGui::End();
		return;
	}

	const std::vector<FMCPSuggestion> suggestions = queue.GetAll();
	const int count = (int)suggestions.size();

	ImGui::Text("%d pending suggestion(s) from the AI analyser", count);
	ImGui::Separator();

	// Track which indices to accept/reject this frame (applied after the table loop)
	int acceptIndex = -1;
	int rejectIndex = -1;
	bool acceptAll = false;
	bool rejectAll = false;

	static const ImGuiTableFlags tableFlags =
		ImGuiTableFlags_Borders |
		ImGuiTableFlags_RowBg |
		ImGuiTableFlags_ScrollY |
		ImGuiTableFlags_SizingStretchProp;

	const float reservedHeight = ImGui::GetFrameHeightWithSpacing() * 2.5f;
	if (ImGui::BeginTable("suggestions", 6, tableFlags, ImVec2(0, -reservedHeight)))
	{
		ImGui::TableSetupScrollFreeze(0, 1);
		ImGui::TableSetupColumn("Type",       ImGuiTableColumnFlags_WidthFixed,   130.0f);
		ImGui::TableSetupColumn("Target",     ImGuiTableColumnFlags_WidthFixed,   130.0f);
		ImGui::TableSetupColumn("Old Value",  ImGuiTableColumnFlags_WidthStretch, 1.0f);
		ImGui::TableSetupColumn("New Value",  ImGuiTableColumnFlags_WidthStretch, 1.0f);
		ImGui::TableSetupColumn("Rationale",  ImGuiTableColumnFlags_WidthStretch, 2.0f);
		ImGui::TableSetupColumn("Actions",    ImGuiTableColumnFlags_WidthFixed,   110.0f);
		ImGui::TableHeadersRow();

		for (int i = 0; i < count; i++)
		{
			const FMCPSuggestion& s = suggestions[i];

			ImGui::TableNextRow();
			ImGui::TableSetColumnIndex(0);
			ImGui::TextUnformatted(GetSuggestionTypeLabel(s.Type));

			ImGui::TableSetColumnIndex(1);
			if (s.AddressRef.IsValid())
			{
				//ImGui::Text("%s ($%04X)", s.TargetName.c_str(), s.AddressRef.Address);
				DrawAddressLabel(pEmu->GetCodeAnalysis(),pEmu->GetCodeAnalysis().GetFocussedViewState(),s.AddressRef);
			}
			else
			{
				ImGui::TextUnformatted(s.TargetName.c_str());
			}

			ImGui::TableSetColumnIndex(2);
			ImGui::TextUnformatted(s.OldValue.c_str());

			ImGui::TableSetColumnIndex(3);
			ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0.4f, 1.0f, 0.4f, 1.0f));
			ImGui::TextUnformatted(s.NewValue.c_str());
			ImGui::PopStyleColor();

			ImGui::TableSetColumnIndex(4);
			ImGui::TextWrapped("%s", s.Rationale.c_str());

			ImGui::TableSetColumnIndex(5);
			ImGui::PushID(i);
			if (ImGui::SmallButton("Accept"))
				acceptIndex = i;
			ImGui::SameLine();
			if (ImGui::SmallButton("Reject"))
				rejectIndex = i;
			ImGui::PopID();
		}

		ImGui::EndTable();
	}

	ImGui::Separator();
	if (ImGui::Button("Accept All"))
		acceptAll = true;
	ImGui::SameLine();
	if (ImGui::Button("Reject All"))
		rejectAll = true;

	ImGui::End();

	// Apply decisions outside the table loop to avoid modifying the list while iterating
	bool bQueueChanged = false;
	if (acceptAll)
	{
		for (const FMCPSuggestion& s : suggestions)
			ApplySuggestion(pEmu, s);
		queue.Clear();
		bQueueChanged = true;
	}
	else if (rejectAll)
	{
		queue.Clear();
		bQueueChanged = true;
	}
	else if (acceptIndex >= 0)
	{
		ApplySuggestion(pEmu, suggestions[acceptIndex]);
		queue.Remove(acceptIndex);
		bQueueChanged = true;
	}
	else if (rejectIndex >= 0)
	{
		queue.Remove(rejectIndex);
		bQueueChanged = true;
	}

	if (bQueueChanged && pEmu->GetProjectConfig() != nullptr)
		queue.Save(pEmu->GetGameWorkspaceRoot() + "Suggestions.json");
}
