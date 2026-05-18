#include "BatchGameLoadViewer.h"

#include <algorithm>

#include <imgui.h>

#include "../PCEEmu.h"
#include "../DebugStats.h"
#include "../GameDb.h"
#include "Misc/GameConfig.h"
#include "Util/FileUtil.h"
#include "SpriteViewer.h"

#include <geargrafx_core.h>

#if !NEWADDRESSREF
extern int gTotalBanksProcessed;
#endif

FBatchGameLoadViewer::FBatchGameLoadViewer(FEmuBase* pEmu)
: FViewerBase(pEmu) 
{ 
	Name = "Batch Game Loader";
	pPCEEmu = static_cast<FPCEEmu*>(pEmu);
	ActiveListName = FPCEEmu::kPCERomGameListName;
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
	GameFrameCount = 0;
	AutomationStartTime = ImGui::GetTime();

	if (bPressRandomButtons)
		NextButtonPressTime = GetNextButtonPressTime();

#ifdef _WIN32
	if (bExportAsm)
	{
		if (FileExists("AssembleLog.txt"))
		{
			if (FileExists("AssembleLogOld.txt"))
				std::system("del AssembleLogOld.txt");
			std::system("rename AssembleLog.txt AssembleLogOld.txt");
			std::system("del AssembleLog.txt");
		}
	}
#endif
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

	FCodeAnalysisState& state = pPCEEmu->GetCodeAnalysis();

	/*if (state.Debugger.IsStopped())
	{
		if (ImGui::Button("Continue"))
		{
			state.Debugger.Continue();
		}
	}
	else
	{
		if (ImGui::Button("Break"))
		{
			state.Debugger.Break();
		}
	}*/

	ImGui::InputInt("Game run time", &GameRunTime);
	ImGui::Checkbox("Use exact frames", &bUseFramesForRunTime);

	if (ImGui::Checkbox("Press random buttons", &bPressRandomButtons))
	{
		NextButtonPressTime = GetNextButtonPressTime();
	}
	if (ImGui::InputInt("Time until button presses", &TimeUntilButtonPresses))
	{
		if (TimeUntilButtonPresses)
			NextButtonPressTime = GetNextButtonPressTime();
	}

	ImGui::InputFloat("Input delay", &InputDelay);
	
	ImGui::Checkbox("Load existing project", &bLoadExistingProject);
	
	/*ImGui::SeparatorText("ASM");
	if (ImGui::Checkbox("Export ASM after game has run", &bExportAsm))
	{
		// make sure hack for correct asm export is turned on
		pPCEEmu->bWriteCodeInfoWhenCodeExecuted = bExportAsm;
	}

	if (bExportAsm)
		ImGui::Text("THIS WILL AFFECT PERFORMANCE");*/

	ImGui::SeparatorText("Bank Mapping");
	bool bMapped = false;
	if (const FProjectConfig* pConfig = pPCEEmu->GetProjectConfig())
	{
		if (pPCEEmu->pDebugStats)
			bMapped = pPCEEmu->pDebugStats->GameDebugStats[pConfig->Name].NumBanks == pPCEEmu->pDebugStats->GameDebugStats[pConfig->Name].NumBanksMapped;
		ImGui::Text("Fully mapped: %s", bMapped ? "Yes" : "No");
	}
	ImGui::Checkbox("Skip game when fully mapped", &bSkipWhenMapped);

	ImGui::SeparatorText("Status");

	float fGameTimeRemaining = 0;
	ElapsedGameRunTime = 0.f;
	if (bAutomationActive)
	{
		if (bUseFramesForRunTime)
		{
			ImGui::Text("Game frames elapsed %d / %d", GameFrameCount, GameRunTime * 60);
		}
		else
		{
			fGameTimeRemaining = (float)(NextGameTime - time);
			ImGui::Text("Game time remaining: %.1fs", MAX(fGameTimeRemaining, 0.f));
			ElapsedGameRunTime = GameRunTime - fGameTimeRemaining;
			ImGui::Text("Game time elapsed: %.1fs", ElapsedGameRunTime);
		}
	}

	const auto& gamesLists = pPCEEmu->GetGamesLists();
	if (ImGui::BeginCombo("Game List", ActiveListName.c_str()))
	{
		for (const auto& listIt : gamesLists)
		{
			const bool bSelected = listIt.first == ActiveListName;
			if (ImGui::Selectable(listIt.first.c_str(), bSelected))
			{
				ActiveListName = listIt.first;
				GameIndex = 0;
			}
			if (bSelected)
				ImGui::SetItemDefaultFocus();
		}
		ImGui::EndCombo();
	}

	auto findIt = gamesLists.find(ActiveListName);
	if (findIt == gamesLists.end())
		return;

	const FGamesList& gamesList = findIt->second;
	const int numGamesInList = gamesList.GetNoGames();
	if (numGamesInList)
	{
		bool bNextGame = false;
		const bool bIsLastGameInList = GameIndex == numGamesInList - 1;

		if (bAutomationActive)
		{
			bool bTimeIsUp = false;
			if (bUseFramesForRunTime)
			{
				if (GameFrameCount >= GameRunTime * 60)
					bTimeIsUp = true;
			}
			else
			{
				if (time >= NextGameTime)
					bTimeIsUp = true;
			}

			if (bTimeIsUp)
			{
				if (bIsLastGameInList)
				{
					bAutomationActive = false;
					GameIndex = 0;
				}
				else
					bNextGame = true;

				if (const FProjectConfig* pConfig = pPCEEmu->GetProjectConfig())
				{
					if (FGameDbEntry* pEntry = GetGameDbEntry(pConfig->Name))
					{
						int historySize = 0;
						pEntry->SpritesFoundInROM = pPCEEmu->GetSpriteViewer()->CountSpritesFoundInMemory(historySize);
						pEntry->SpritesInHistory = historySize;
					}
				}

				if (bExportAsm)
				{
					pPCEEmu->ExportAsmForCurrentGame();
				}
			}

			if (bSkipWhenMapped && bMapped)
				bNextGame = true;

			if (bPressRandomButtons && ElapsedGameRunTime > TimeUntilButtonPresses)
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
			GameFrameCount++;
		}
		const int numGamesToLoad = numGamesInList - GameIndex;
		const int totSecs = (numGamesToLoad * GameRunTime) + (int)fGameTimeRemaining;
		const int totMins = totSecs / 60;
		const int totHours = totMins / 60;
		ImGui::Text("Total time remaining: %dh %dm %ds", (int)totHours, totMins % 60, totSecs % 60);
		{
			const int elapsedSecs = bAutomationActive ? (int)(time - AutomationStartTime) : 0;
			const int elapsedMins = elapsedSecs / 60;
			const int elapsedHours = elapsedMins / 60;
			ImGui::Text("Total time elapsed: %dh %dm %ds", elapsedHours, elapsedMins % 60, elapsedSecs % 60);
		}

		ImGui::SeparatorText("File list");

		{
			const FEmulatorFile& game = gamesList.GetGame(GameIndex);
			ImGui::Text("(%d/%d) %s", GameIndex + 1, numGamesInList, game.DisplayName.c_str());
			ImGui::InputInt("Game index", &GameIndex);
			GameIndex = CLAMP(GameIndex, 0, numGamesInList);
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
				GameFrameCount = 0;
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

int FBatchGameLoadViewer::GetTestingMethodology() const
{
	if (bAutomationActive)
	{
		// Run for 1800 frames with no input
		if (bUseFramesForRunTime && GameRunTime == 30 && !bPressRandomButtons)
			return 1;
	}

	return -1;
}


