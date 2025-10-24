#include "BatchGameLoadViewer.h"

#include <algorithm>

#include <imgui.h>

#include "../PCEEmu.h"
#include "Misc/GameConfig.h"

#include <geargrafx_core.h>

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
	// uncomment to make automation begin when tool boots up.
	//bLoadExistingProject = true;
	//StartAutomation();

	return true;
}

void FBatchGameLoadViewer::StartAutomation()
{
	bAutomationActive = true;
	bLoadGame = true;
	if (bPressRandomButtons)
		NextButtonPressTime = GetNextButtonPressTime();
}

void FBatchGameLoadViewer::DrawUI()
{
#if !NEWADDRESSREF
	ImGui::Text("Banks fixed up %d", gTotalBanksProcessed);
#endif

	ImGui::SeparatorText("Automation");
	ImGui::Text("Automation is %s", bAutomationActive ? "active." : "not active.");

	const double time = ImGui::GetTime();
	if (!bAutomationActive)
	{
		if (ImGui::Button("Start automation"))
		{
			StartAutomation();
		}	
	}
	else
	{
		if (ImGui::Button("Stop automation mode"))
			bAutomationActive = false;
	}

	ImGui::InputInt("Game run time", &GameRunTime);

	if (ImGui::Checkbox("Press random buttons", &bPressRandomButtons))
	{
		NextButtonPressTime = GetNextButtonPressTime();
	}
	ImGui::InputFloat("Input delay", &InputDelay);
	
	ImGui::Checkbox("Load existing project", &bLoadExistingProject);

	float fGameTimeRemaining = 0;
	if (bAutomationActive)
	{
		fGameTimeRemaining = (float)(NextGameTime - time);
		ImGui::Text("Game time remaining: %.1fs", MAX(fGameTimeRemaining, 0.f));
	}

	auto findIt = pPCEEmu->GetGamesLists().find("Snapshot File");
	if (findIt == pPCEEmu->GetGamesLists().end())
		return;

	const FGamesList& gamesList = findIt->second;
	const int numGamesInList = gamesList.GetNoGames();
	if (numGamesInList)
	{
		bool bNextGame = false;
		const bool bIsLastGameInList = GameIndex == numGamesInList - 1;

		if (bAutomationActive)
		{
			if (time >= NextGameTime)
			{
				if (bIsLastGameInList)
				{
					bAutomationActive = false;
					GameIndex = 0;
				}
				else
					bNextGame = true;
			}

			if (bPressRandomButtons)
			{
				if (time >= NextButtonPressTime)
				{
					pPCEEmu->GetCore()->KeyReleased(GG_CONTROLLER_1, GG_KEY_RUN);
					pPCEEmu->GetCore()->KeyReleased(GG_CONTROLLER_1, GG_KEY_SELECT);
					pPCEEmu->GetCore()->KeyReleased(GG_CONTROLLER_1, GG_KEY_I);
					pPCEEmu->GetCore()->KeyReleased(GG_CONTROLLER_1, GG_KEY_II);
					pPCEEmu->GetCore()->KeyReleased(GG_CONTROLLER_1, GG_KEY_UP);
					pPCEEmu->GetCore()->KeyReleased(GG_CONTROLLER_1, GG_KEY_DOWN);
					pPCEEmu->GetCore()->KeyReleased(GG_CONTROLLER_1, GG_KEY_LEFT);
					pPCEEmu->GetCore()->KeyReleased(GG_CONTROLLER_1, GG_KEY_RIGHT);

					switch (rand() % 8)
					{
					case 0:
						pPCEEmu->GetCore()->KeyPressed(GG_CONTROLLER_1, GG_KEY_RUN);
						break;
					case 1:
						pPCEEmu->GetCore()->KeyPressed(GG_CONTROLLER_1, GG_KEY_SELECT);
						break;
					case 2:
						pPCEEmu->GetCore()->KeyPressed(GG_CONTROLLER_1, GG_KEY_I);
						break;
					case 3:
						pPCEEmu->GetCore()->KeyPressed(GG_CONTROLLER_1, GG_KEY_II);
						break;
					case 4:
						pPCEEmu->GetCore()->KeyPressed(GG_CONTROLLER_1, GG_KEY_UP);
						break;
					case 5:
						pPCEEmu->GetCore()->KeyPressed(GG_CONTROLLER_1, GG_KEY_DOWN);
						break;
					case 6:
						pPCEEmu->GetCore()->KeyPressed(GG_CONTROLLER_1, GG_KEY_LEFT);
						break;
					case 7:
						pPCEEmu->GetCore()->KeyPressed(GG_CONTROLLER_1, GG_KEY_RIGHT);
						break;
					}

					NextButtonPressTime = GetNextButtonPressTime();
				}
			}
		}
		const int numGamesToLoad = numGamesInList - GameIndex;
		const int totSecs = (numGamesToLoad * GameRunTime) + (int)fGameTimeRemaining;
		const int totMins = totSecs / 60;
		const int totHours = totMins / 60;
		ImGui::Text("Total time remaining: %dh %dm %ds", (int)totHours, totMins % 60, totSecs % 60);

		ImGui::SeparatorText("File list");

		{
			const FEmulatorFile& game = gamesList.GetGame(GameIndex);
			ImGui::Text("(%d/%d) %s", GameIndex + 1, numGamesInList, game.DisplayName.c_str());
			ImGui::InputInt("Game index", &GameIndex);
		}

		if (ImGui::Button("Prev game") || ImGui::IsKeyPressed(ImGuiKey_F1))
		{
			if (GameIndex > 0)
			{
				GameIndex--;
				bLoadGame = true;
			}
		}
		ImGui::SameLine();
		if (ImGui::Button("Next game") || ImGui::IsKeyPressed(ImGuiKey_F2) || bNextGame)
		{
			if (!bIsLastGameInList)
			{
				GameIndex++;
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
			const FEmulatorFile& game = gamesList.GetGame(GameIndex);
			bool bOk = false;
			FProjectConfig* pConfig = nullptr;

			if (bLoadExistingProject)
				pConfig = GetGameConfigForName(game.DisplayName.c_str());
			
			if (pConfig)
			{
				LOGINFO("%d Loading existing project '%s'", GameIndex, game.DisplayName.c_str());
				bOk = pPCEEmu->LoadProject(pConfig, true);
			}
			else
			{
				LOGINFO("%d Creating new project for '%s'", GameIndex, game.DisplayName.c_str());
				bOk = pPCEEmu->NewProjectFromEmulatorFile(game);
			}
			
			if (bOk)
			{
				pPCEEmu->GetCodeAnalysis().Debugger.Continue();
				NextGameTime = time + GameRunTime;
			}
			else
			{
				pPCEEmu->Reset();
				pPCEEmu->DisplayErrorMessage("Could not %s project '%s'", bLoadExistingProject ? "load" : "create", game.DisplayName.c_str());
			}
			bLoadGame = false;
		}
	}
}

double FBatchGameLoadViewer::GetNextButtonPressTime() const 
{ 
	return ImGui::GetTime() + ((double)(rand() / RAND_MAX) * InputDelay); 
}

