#include "BatchGameLoadViewer.h"

#include <algorithm>

#include <imgui.h>
#include "../PCEEmu.h"

#if !NEWADDRESSREF
extern int gTotalBanksProcessed;
#endif

FBatchGameLoadViewer::FBatchGameLoadViewer(FEmuBase* pEmu)
: FViewerBase(pEmu) 
{ 
	Name = "Batch Game Loader";
	pPCEEmu = static_cast<FPCEEmu*>(pEmu);
}

bool FBatchGameLoadViewer::Init()
{
	return true;
}

void FBatchGameLoadViewer::DrawUI()
{
#if !NEWADDRESSREF
	ImGui::Text("Banks fixed up %d", gTotalBanksProcessed);
#endif

	const double time = ImGui::GetTime();
	if (!bAutomationActive)
	{
		if (ImGui::Button("Start automation"))
		{
			bAutomationActive = true;
			NextGameTime = ImGui::GetTime();
		}
		
	}
	else
	{
		if (ImGui::Button("Stop automation mode"))
			bAutomationActive = false;
	}

	ImGui::InputInt("Game run time", &GameRunTime);

	ImGui::Text("Automation is %s", bAutomationActive ? "active." : "not active.");
	if (bAutomationActive)
	{
		//ImGui::SameLine();
		const float fTimeRemaining = (float)(NextGameTime - time);
		ImGui::Text("Game run time remaining %.1f", std::max(fTimeRemaining, 0.f));
	}

	auto findIt = pPCEEmu->GetGamesLists().find("Snapshot File");
	if (findIt == pPCEEmu->GetGamesLists().end())
		return;

	const FGamesList& gamesList = findIt->second;
	if (gamesList.GetNoGames())
	{
		static int gGameIndex = 0;
		bool bLoadGame = false;
		bool bNextGame = false;
		const bool bIsLastGameInList = gGameIndex == gamesList.GetNoGames() - 1;

		if (bAutomationActive)
		{
			if (ImGui::GetTime() >= NextGameTime)
			{
				if (!bIsLastGameInList)
					bNextGame = true;
				else
					bAutomationActive = false;
			}
		}

		const FEmulatorFile& game = gamesList.GetGame(gGameIndex);
		ImGui::Text("(%d/%d) %s", gGameIndex + 1, gamesList.GetNoGames(), game.DisplayName.c_str());

		if (ImGui::Button("Prev snap") || ImGui::IsKeyPressed(ImGuiKey_F1))
		{
			if (gGameIndex > 0)
			{
				gGameIndex--;
				bLoadGame = true;
			}
		}
		ImGui::SameLine();
		if (ImGui::Button("Next snap") || ImGui::IsKeyPressed(ImGuiKey_F2) || bNextGame)
		{
			if (!bIsLastGameInList)
			{
				gGameIndex++;
				bLoadGame = true;
			}
		}
		if (ImGui::IsKeyPressed(ImGuiKey_F3))
			bLoadGame = true;

		if (bLoadGame)
		{
#if !NEWADDRESSREF
			gTotalBanksProcessed = 0;
#endif
			LOGINFO("Load game '%s'", game.DisplayName.c_str());
			const FEmulatorFile& game = gamesList.GetGame(gGameIndex);
			if (!pPCEEmu->NewProjectFromEmulatorFile(game))
			{
				pPCEEmu->Reset();
				pPCEEmu->DisplayErrorMessage("Could not create project '%s'", game.DisplayName.c_str());
			}
			else
			{
				NextGameTime = time + GameRunTime;
			}
		}
	}

	ImGui::Text("Max dupe banks %d '%s'", pPCEEmu->DebugStats.MaxDupeMprBanks, pPCEEmu->DebugStats.GameWithMaxDupeMprBanks.c_str());
}
