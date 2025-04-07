#include "EmuBase.h"

#include <imgui.h>
#include <implot.h>
#include <CodeAnalyser/UI/CodeAnalyserUI.h>
#include <CodeAnalyser/AssemblerExport.h>
#include <CodeAnalyser/UI/GraphicsViewer.h>
#include <CodeAnalyser/UI/CharacterMapViewer.h>
#include <CodeAnalyser/UI/DisplayTypes.h>
#include <CodeAnalyser/DataTypes.h>
#include "GameConfig.h"

#include "Debug/DebugLog.h"
#include "Debug/ImGuiLog.h"
#include "Util/FileUtil.h"
#include "LuaScripting/LuaSys.h"
#include <CodeAnalyser/UI/UIColours.h>
#include "CodeAnalyser/CodeAnalysisDot.h"

// Separate viewer for Globals - put in its own file
class FGlobalsViewer : public FViewerBase
{
public:
	FGlobalsViewer(FEmuBase* pEmu) : FViewerBase(pEmu) { Name = "Globals"; }
	bool	Init(void) override
	{
		return true;
	}
	void	Shutdown(void) override
	{

	}

	void	DrawUI() override
	{
		FCodeAnalysisState& state = pEmulator->GetCodeAnalysis();
		DrawGlobals(state, state.GetFocussedViewState());
	}
};


void FEmulatorLaunchConfig::ParseCommandline(int argc, char** argv)
{
	std::vector<std::string> argList;
	for (int arg = 0; arg < argc; arg++)
	{
		argList.emplace_back(argv[arg]);
	}

	auto argIt = argList.begin();
	argIt++;	// skip exe name
	while (argIt != argList.end())
	{
		if (*argIt == std::string("-game"))
		{
			if (++argIt == argList.end())
			{
				LOGERROR("-game : No game specified");
				break;
			}
			SpecificGame = *argIt;
		}
		else if (*argIt == std::string("-nomultiwindow"))
		{
			bMultiWindow = false;
		}

		++argIt;
	}
}

bool	FEmuBase::Init(const FEmulatorLaunchConfig& launchConfig)
{
	FileInit();
	
	const char* pImGuiConfigFile = "imgui.ini";
	
	// check if we have an imgui.ini file in our app support dir
	if(FileExists(GetAppSupportPath(pImGuiConfigFile)) == false)
	{
		// copy it from the bundle
		size_t byteCount = 0;
		void *pFileData = LoadBinaryFile(GetBundlePath(pImGuiConfigFile), byteCount);
		if(pFileData == nullptr)
		{
			LOGERROR("Can't find imgui.ini file in bundle");
			return false;
		}
		
		SaveBinaryFile(GetAppSupportPath(pImGuiConfigFile), pFileData, byteCount);
	}
	
	static std::string iniFile = GetAppSupportPath(pImGuiConfigFile);
	ImGuiIO& io = ImGui::GetIO();
	io.IniFilename = iniFile.c_str();
	
	RegisterBasicDisplayTypes();
    AddViewer(new FDataTypesViewer(this));
	AddViewer(new FGlobalsViewer(this));
	return true;
}


void FEmuBase::Shutdown()
{
	LuaSys::Shutdown();
}

void FEmuBase::Tick()
{
	Colours::Tick();
	UpdateCharacterSets(CodeAnalysis);
}

void FEmuBase::Reset()
{

}

bool FEmuBase::DrawDockingView()
{
	//SCOPE_PROFILE_CPU("UI", "DrawUI", ProfCols::UI);

	static bool opt_fullscreen_persistant = true;
	bool opt_fullscreen = opt_fullscreen_persistant;
	//static ImGuiDockNodeFlags dockspace_flags = ImGuiDockNodeFlags_PassthruCentralNode;
	bool bOpen = false;
	ImGuiDockNodeFlags dockFlags = 0;

	// We are using the ImGuiWindowFlags_NoDocking flag to make the parent window not dockable into,
	// because it would be confusing to have two docking targets within each others.
	ImGuiWindowFlags window_flags = ImGuiWindowFlags_MenuBar | ImGuiWindowFlags_NoDocking;
	if (opt_fullscreen)
	{
		ImGuiViewport* viewport = ImGui::GetMainViewport();
		ImGui::SetNextWindowPos(viewport->Pos);
		ImGui::SetNextWindowSize(viewport->Size);
		ImGui::SetNextWindowViewport(viewport->ID);
		ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 0.0f);
		ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 0.0f);
		window_flags |= ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove;
		window_flags |= ImGuiWindowFlags_NoBringToFrontOnFocus | ImGuiWindowFlags_NoNavFocus;
	}

	// When using ImGuiDockNodeFlags_PassthruCentralNode, DockSpace() will render our background and handle the pass-thru hole, so we ask Begin() to not render a background.
	if (dockFlags & ImGuiDockNodeFlags_PassthruCentralNode)
		window_flags |= ImGuiWindowFlags_NoBackground;

	// Important: note that we proceed even if Begin() returns false (aka window is collapsed).
	// This is because we want to keep our DockSpace() active. If a DockSpace() is inactive, 
	// all active windows docked into it will lose their parent and become undocked.
	// We cannot preserve the docking relationship between an active window and an inactive docking, otherwise 
	// any change of dockspace/settings would lead to windows being stuck in limbo and never being visible.
	bool bQuit = false;
	ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0.0f, 0.0f));
	if (ImGui::Begin("DockSpace Demo", &bOpen, window_flags))
	{
		ImGui::PopStyleVar();

		if (opt_fullscreen)
			ImGui::PopStyleVar(2);

		// DockSpace
		ImGuiIO& io = ImGui::GetIO();
		if (io.ConfigFlags & ImGuiConfigFlags_DockingEnable)
		{
			const ImGuiID dockspaceId = ImGui::GetID("MyDockSpace");
			ImGui::DockSpace(dockspaceId, ImVec2(0.0f, 0.0f), dockFlags);
		}

		//bQuit = MainMenu();
		//DrawDebugWindows(uiState);
		DrawMainMenu();
		DrawUI();
		ImGui::End();
	}
	else
	{
		ImGui::PopStyleVar();
		bQuit = true;
	}

	return bQuit;
}

void FEmuBase::DrawUI()
{
	// TODO: Make these viewers
	if (ImGui::Begin("Debugger"))
	{
		CodeAnalysis.Debugger.DrawUI();
	}
	ImGui::End();

	if (ImGui::Begin("Memory Analyser"))
	{
		CodeAnalysis.MemoryAnalyser.DrawUI();
	}
	ImGui::End();

	if (ImGui::Begin("IO Analyser"))
	{
		CodeAnalysis.IOAnalyser.DrawUI();
	}
	ImGui::End();

	if (ImGui::Begin("Static Analysis"))
	{
		CodeAnalysis.StaticAnalysis.DrawUI();
	}
	ImGui::End();

	// Draw registered viewers
	for (auto Viewer : Viewers)
	{
		if (Viewer->bOpen)
		{
			if (ImGui::Begin(Viewer->GetName(), &Viewer->bOpen))
				Viewer->DrawUI();
			ImGui::End();
		}
	}

	// Code analysis views
	for (int codeAnalysisNo = 0; codeAnalysisNo < FCodeAnalysisState::kNoViewStates; codeAnalysisNo++)
	{
		char name[32];
		snprintf(name, 32,"Code Analysis %d", codeAnalysisNo + 1);
		if (CodeAnalysis.ViewState[codeAnalysisNo].Enabled)
		{
			if (ImGui::Begin(name, &CodeAnalysis.ViewState[codeAnalysisNo].Enabled))
			{
				DrawCodeAnalysisData(CodeAnalysis, codeAnalysisNo);
			}
			ImGui::End();
		}

	}

	if (bShowDebugLog)
		g_ImGuiLog.Draw("Debug Log", &bShowDebugLog);

    if (bShowImGuiDemo)
        ImGui::ShowDemoWindow(&bShowImGuiDemo);

    if (bShowImPlotDemo)
        ImPlot::ShowDemoWindow(&bShowImPlotDemo);

	DrawEmulatorUI();
    
    LuaSys::DrawUI();
}


void FEmuBase::FileMenu()
{
	// New game from snapshot

	for (const auto& gamesListIt : GamesLists)
	{
		const FGamesList& gamesList = gamesListIt.second;
		char menuTitle[128];
		snprintf(menuTitle,128,"New Project from %s",gamesList.GetFileType());

		if (ImGui::BeginMenu(menuTitle))
		{
			const int numGames = gamesList.GetNoGames();
			if (!numGames)
			{
				ImGui::Text("No %s found in directory:\n\n'%s'.\n\nDirectory is set in GlobalConfig.json", gamesList.GetFileType(), gamesList.GetRootDir());
			}
			else
			{
				for (int gameNo = 0; gameNo < numGames; gameNo++)
				{
					const FEmulatorFile& game = gamesList.GetGame(gameNo);

					if (ImGui::MenuItem(game.DisplayName.c_str()))
					{
						bool bGameExists = false;

						for (const auto& pGameConfig : GetGameConfigs())
						{
							if (pGameConfig->Name == game.DisplayName)
								bGameExists = true;
						}
						if (bGameExists)
						{
							EmulatorFileToLoad = game;
							bReplaceGamePopup = true;
							//ReplaceGameSnapshotIndex = gameNo;
						}
						else
						{
							if (!NewProjectFromEmulatorFile(game))
							{
								Reset();
								DisplayErrorMessage("Could not load emulator file '%s'", game.FileName.c_str());
							}
							break;
						}
					}
				}
			}
			ImGui::EndMenu();
		}
	}

	if (ImGui::BeginMenu("Open Project"))
	{
		if (GetGameConfigs().empty())
		{
			ImGui::Text("No projects found.\n\nFirst, create a project via the 'New Project ..' menu.");
		}
		else
		{
			for (const auto& pGameConfig : GetGameConfigs())
			{
				if (ImGui::MenuItem(pGameConfig->Name.c_str()))
				{
					SaveProject();  // save previous game
					if (LoadProject(pGameConfig, true) == false)
					{
						Reset();
						DisplayErrorMessage("Could not start project '%s'",pGameConfig->Name.c_str());
					}
				}
			}
		}

		ImGui::EndMenu();
	}

	if (ImGui::MenuItem("Save Project"))
	{
		SaveProject();
	}

	if (ImGui::MenuItem("Export ASM File"))
	{
		if (pCurrentProjectConfig->AsmExportPath.empty())
		{
			std::string outputFname = GetGameWorkspaceRoot() + pCurrentProjectConfig->Name + ".asm";
			ExportAssembler(this, outputFname.c_str(), ExportStartAddress, ExportEndAddress);
		}
		else
		{
			std::string outputFname = pCurrentProjectConfig->AsmExportPath + pCurrentProjectConfig->Name + ".asm";
			ExportAssembler(this, outputFname.c_str(), ExportStartAddress, ExportEndAddress);
		}
	}

	if (ImGui::MenuItem("Export ASM Range"))
	{
		bExportAsm = true;
	}

	if (ImGui::MenuItem("Export Binary File"))
	{
		// ImGui popup windows can't be activated from within a Menu so we set a flag to act on outside of the menu code.
		bExportBinary = true;
	}

	if (ImGui::MenuItem("Export Dot File"))
	{
		const std::string root = pGlobalConfig->WorkspaceRoot + pCurrentProjectConfig->Name + "/";

		ExportCodeAnalysisDot(CodeAnalysis, (root + pCurrentProjectConfig->Name + ".dot").c_str());
	}

	FileMenuAdditions();
}

void FEmuBase::OptionsMenu()
{
	if (ImGui::BeginMenu("Number Mode"))
	{
		bool bClearCode = false;
		if (ImGui::MenuItem("Decimal", 0, GetNumberDisplayMode() == ENumberDisplayMode::Decimal))
		{
			SetNumberDisplayMode(ENumberDisplayMode::Decimal);
			CodeAnalysis.SetAllBanksDirty();
			bClearCode = true;
		}
		if (ImGui::MenuItem("Hex - FEh", 0, GetNumberDisplayMode() == ENumberDisplayMode::HexAitch))
		{
			SetNumberDisplayMode(ENumberDisplayMode::HexAitch);
			SetHexNumberDisplayMode(ENumberDisplayMode::HexAitch);
			CodeAnalysis.SetAllBanksDirty();
			bClearCode = true;
		}
		if (ImGui::MenuItem("Hex - $FE", 0, GetNumberDisplayMode() == ENumberDisplayMode::HexDollar))
		{
			SetNumberDisplayMode(ENumberDisplayMode::HexDollar);
			SetHexNumberDisplayMode(ENumberDisplayMode::HexDollar);
			CodeAnalysis.SetAllBanksDirty();
			bClearCode = true;
		}

		// clear code text so it can be written again
		// TODO: this needs to work for banks
		if (bClearCode)
		{
			for (int i = 0; i < 1 << 16; i++)
			{
				FCodeInfo* pCodeInfo = CodeAnalysis.GetCodeInfoForPhysicalAddress(i);
				if (pCodeInfo && pCodeInfo->Text.empty() == false)
					pCodeInfo->Text.clear();

			}
		}

		ImGui::EndMenu();
	}
	ImGui::MenuItem("Scan Line Indicator", 0, &CodeAnalysis.pGlobalConfig->bShowScanLineIndicator);
	ImGui::MenuItem("Enable Audio", 0, &CodeAnalysis.pGlobalConfig->bEnableAudio);
	if (ImGui::MenuItem("Edit Mode", 0, &CodeAnalysis.bAllowEditing))
	{
		if(CodeAnalysis.bAllowEditing)
			OnEnterEditMode();
		else
			OnExitEditMode();
			
	}
	ImGui::MenuItem("Show Opcode Values", 0, &CodeAnalysis.pGlobalConfig->bShowOpcodeValues);
	if (ImGui::BeginMenu("Image Scale"))
	{
		for (int i = 0; i < 4; i++)
		{
			char numStr[4];
			snprintf(numStr,4,"%dx",i+1);
			if (ImGui::MenuItem(numStr, 0, CodeAnalysis.pGlobalConfig->ImageScale == i+1))
				CodeAnalysis.pGlobalConfig->ImageScale = i+1;
		}
		ImGui::EndMenu();
	}
	if (ImGui::BeginMenu("Display Branch Lines"))
	{
		if (ImGui::MenuItem("Off", 0, CodeAnalysis.pGlobalConfig->BranchLinesDisplayMode == 0))
		{
			CodeAnalysis.pGlobalConfig->BranchLinesDisplayMode = 0;
		}
		if (ImGui::MenuItem("Minimal", 0, CodeAnalysis.pGlobalConfig->BranchLinesDisplayMode == 1))
		{
			CodeAnalysis.pGlobalConfig->BranchLinesDisplayMode = 1;
		}
		if (ImGui::MenuItem("Full", 0, CodeAnalysis.pGlobalConfig->BranchLinesDisplayMode == 2))
		{
			CodeAnalysis.pGlobalConfig->BranchLinesDisplayMode = 2;
		}

		ImGui::EndMenu();
	}
	if (ImGui::BeginMenu("Font"))
	{
		if (const ImFont* pCurFont = ImGui::GetFont())
		{
			FGlobalConfig* pConfig = CodeAnalysis.pGlobalConfig;
			const bool bConfigFontIsLoaded = !strncmp(pConfig->Font.c_str(), pCurFont->GetDebugName(), pConfig->Font.length());

			ImGui::Checkbox("Use Built-In Font", &pConfig->bBuiltInFont);

			ImGui::Text("Current Font: %s", pCurFont->GetDebugName());
			if (!pConfig->bBuiltInFont)
			{
				ImGui::Separator();

				if (!pConfig->Font.empty())
				{
					if (bConfigFontIsLoaded)
					{
						if (ImGui::Button("Decrease Font Size"))
						{
							pConfig->FontSizePts--;
						}
						ImGui::SameLine();
						ImGui::Dummy(ImGui::CalcTextSize("FF"));
						ImGui::SameLine();
						if (ImGui::Button("Increase Font Size"))
						{
							pConfig->FontSizePts++;
						}

						ImGui::InputInt("Size (Pt)", &pConfig->FontSizePts, 0, 0, ImGuiInputTextFlags_EnterReturnsTrue);
						pConfig->FontSizePts = std::min(std::max(pConfig->FontSizePts, 8), 72);
					}
					else
					{
						ImGui::Text("Font '%s' could not be loaded.", pConfig->Font.c_str());
					}
				}
				else
				{
					ImGui::Text("No font defined in GlobalConfig.json.");
				}
			}
		}
		ImGui::EndMenu();
	}

	if (ImGui::BeginMenu("Assembler Exporter"))
	{
		const std::map<std::string, FASMExporter*>& exporters = GetAssemblerExporters();

		for (auto exporter : exporters)
		{
			if (ImGui::MenuItem(exporter.first.c_str(), 0, CodeAnalysis.pGlobalConfig->ExportAssembler == exporter.first))
			{
				CodeAnalysis.pGlobalConfig->ExportAssembler = exporter.first;
			}
		}

		ImGui::EndMenu();
	}
	//if(pActiveGame!=nullptr)
	//	ImGui::MenuItem("Save Snapshot with game", 0, &pActiveGame->pConfig->WriteSnapshot);

#ifndef NDEBUG
	ImGui::MenuItem("Show Config", 0, &CodeAnalysis.Config.bShowConfigWindow);
	ImGui::MenuItem("ImGui Demo", 0, &bShowImGuiDemo);
	ImGui::MenuItem("ImPlot Demo", 0, &bShowImPlotDemo);
#endif // NDEBUG

	OptionsMenuAdditions();
}

void FEmuBase::SystemMenu()
{
	if (pCurrentProjectConfig && ImGui::MenuItem("Reload Emulator File"))
	{
		if (!LoadEmulatorFile(&pCurrentProjectConfig->EmulatorFile))
		{
			DisplayErrorMessage("Could not load emulator file '%s'", pCurrentProjectConfig->EmulatorFile.FileName.c_str());
		}
	}

	if (ImGui::MenuItem("Reset"))
	{
		Reset();
	}

	SystemMenuAdditions();
}

void FEmuBase::ActionsMenu()
{
	if (ImGui::BeginMenu("Reset Reference Info"))
	{
		if (ImGui::MenuItem("Read"))
			ResetReferenceInfo(CodeAnalysis, true, false);
		if (ImGui::MenuItem("Write"))
			ResetReferenceInfo(CodeAnalysis, false, true);
		if (ImGui::MenuItem("Both"))
			ResetReferenceInfo(CodeAnalysis, true, true);

		ImGui::EndMenu();
	}

	if (ImGui::BeginMenu("Reset Read/Write Counts"))
	{
		if (ImGui::MenuItem("Read"))
			ResetReadWriteCounts(CodeAnalysis, true, false);
		if (ImGui::MenuItem("Write"))
			ResetReadWriteCounts(CodeAnalysis, false, true);
		if (ImGui::MenuItem("Both"))
			ResetReadWriteCounts(CodeAnalysis, true, true);

		ImGui::EndMenu();
	}

	if (ImGui::MenuItem("Reset Execution Counts"))
	{
		ResetExecutionCounts(CodeAnalysis);
	}

	if (ImGui::MenuItem("Run Static Analysis"))
	{
		CodeAnalysis.RunStaticAnalysis();
	}

	ActionMenuAdditions();
}

void FEmuBase::WindowsMenu()
{
	ImGui::MenuItem("DebugLog", 0, &bShowDebugLog);
	if (ImGui::BeginMenu("Code Analysis"))
	{
		for (int codeAnalysisNo = 0; codeAnalysisNo < FCodeAnalysisState::kNoViewStates; codeAnalysisNo++)
		{
			char menuName[32];
			snprintf(menuName,32, "Code Analysis %d", codeAnalysisNo + 1);
			ImGui::MenuItem(menuName, 0, &CodeAnalysis.ViewState[codeAnalysisNo].Enabled);
		}

		ImGui::EndMenu();
	}

	for (auto Viewer : Viewers)
	{
		ImGui::MenuItem(Viewer->GetName(), 0, &Viewer->bOpen);

	}

	WindowsMenuAdditions();
}

void FEmuBase::DrawMainMenu()
{
	if (ImGui::BeginMainMenuBar())
	{
		if (ImGui::BeginMenu("File"))
		{
			FileMenu();
			ImGui::EndMenu();
		}

		if (ImGui::BeginMenu("System"))
		{
			SystemMenu();
			ImGui::EndMenu();
		}

		if (ImGui::BeginMenu("Options"))
		{
			OptionsMenu();
			ImGui::EndMenu();
		}

		if (ImGui::BeginMenu("Actions"))
		{
			ActionsMenu();
			ImGui::EndMenu();
		}

		if (ImGui::BeginMenu("Windows"))
		{
			WindowsMenu();
			ImGui::EndMenu();
		}

		// draw emu timings
		const double timeMS = 1000.0f / ImGui::GetIO().Framerate;
		ImGui::SameLine(ImGui::GetWindowWidth() - 120);
		if (CodeAnalysis.Debugger.IsStopped())
			ImGui::Text("emu: stopped");
		else
			ImGui::Text("emu: %.2fms", timeMS);

		ImGui::EndMainMenuBar();
	}

	// Draw any modal popups that have been requested from clicking on menu items.
	// This is a workaround for an open bug.
	// https://github.com/ocornut/imgui/issues/331
	DrawExportAsmModalPopup();
	DrawReplaceGameModalPopup();
	DrawErrorMessageModalPopup();
}

void FEmuBase::DrawExportAsmModalPopup()
{
	if (bExportAsm || bExportBinary)
	{
		ImGui::OpenPopup("Export Address Range");
	}
	if (ImGui::BeginPopupModal("Export Address Range", NULL, ImGuiWindowFlags_AlwaysAutoResize))
	{
		// TODO: get defaults from system
		// Could initialise member variables
		//static ImU16 addrStart = 0;
		//static ImU16 addrEnd = 0xffff;

		ImGui::Text("Address range to export");
		bool bHex = GetNumberDisplayMode() != ENumberDisplayMode::Decimal;
		const char* formatStr = bHex ? "%x" : "%u";
		ImGuiInputTextFlags flags = bHex ? ImGuiInputTextFlags_CharsHexadecimal : ImGuiInputTextFlags_CharsDecimal;

		ImGui::InputScalar("Start", ImGuiDataType_U16, &ExportStartAddress, NULL, NULL, formatStr, flags);
		ImGui::SameLine();
		ImGui::InputScalar("End", ImGuiDataType_U16, &ExportEndAddress, NULL, NULL, formatStr, flags);

		if (ImGui::Button("Export", ImVec2(120, 0)))
		{
			if (ExportEndAddress > ExportStartAddress)
			{
				if (pCurrentProjectConfig != nullptr)
				{
					const std::string dir = GetGameWorkspaceRoot();
					EnsureDirectoryExists(dir.c_str());

					char addrRangeStr[16];
					if (bHex)
						snprintf(addrRangeStr, 16, "_%x_%x", ExportStartAddress, ExportEndAddress);
					else
						snprintf(addrRangeStr, 16, "_%u_%u", ExportStartAddress, ExportEndAddress);

					std::string outputFname = dir + pCurrentProjectConfig->Name + addrRangeStr;
					
					if(bExportAsm)
					{
						outputFname += ".asm";
						ExportAssembler(this, outputFname.c_str(), ExportStartAddress, ExportEndAddress);
					}
					else if (bExportBinary)
					{
						const int binarySize = ExportEndAddress - ExportStartAddress;
						outputFname += ".bin";

						uint8_t* pBinaryMem = (uint8_t *)malloc(binarySize);
						for(int i = 0;i < binarySize;i++)
							pBinaryMem[i] = ReadByte(ExportStartAddress + i);
						SaveBinaryFile(outputFname.c_str(), pBinaryMem, binarySize);
						free(pBinaryMem);
					}
				}
			}
			bExportAsm = false;
			bExportBinary = false;
			ImGui::CloseCurrentPopup();
		}
		ImGui::SetItemDefaultFocus();
		ImGui::SameLine();
		if (ImGui::Button("Cancel", ImVec2(120, 0)))
		{
			bExportAsm = false;
			ImGui::CloseCurrentPopup();
		}
		ImGui::EndPopup();
	}
}

void FEmuBase::DrawReplaceGameModalPopup()
{   
	if (bReplaceGamePopup)  // invoke popup if bool set
		ImGui::OpenPopup("Overwrite Game?");

	if (ImGui::BeginPopupModal("Overwrite Game?", NULL, ImGuiWindowFlags_AlwaysAutoResize))
	{
		ImGui::Text("Do you want to overwrite existing game data?\nAny reverse engineering progress will be lost!\n\n");
		ImGui::Separator();

		if (ImGui::Button("Overwrite", ImVec2(120, 0)))
		{
			//const FGameSnapshot& game = GamesList.GetGame(ReplaceGameSnapshotIndex);
			if (!NewProjectFromEmulatorFile(EmulatorFileToLoad))
			{
				Reset();
				DisplayErrorMessage("Could not load emulator file '%s'", EmulatorFileToLoad.FileName.c_str());
			}
			
			bReplaceGamePopup = false;
			
			ImGui::CloseCurrentPopup();
		}
		ImGui::SetItemDefaultFocus();
		ImGui::SameLine();
		if (ImGui::Button("Cancel", ImVec2(120, 0)))
		{
			bReplaceGamePopup = false;
			ImGui::CloseCurrentPopup();
		}
		ImGui::EndPopup();
	}
}

void FEmuBase::DrawErrorMessageModalPopup()
{
	if (bErrorMessagePopup)
	{
		ImGui::OpenPopup("Error");
		bErrorMessagePopup = false;
	}

	if (ImGui::BeginPopupModal("Error", NULL, ImGuiWindowFlags_AlwaysAutoResize))
	{
		ImGui::Text("%s",ErrorPopupText.c_str());

		if (!LastError.empty())
		{
			ImGui::Separator();
			ImGui::Text("Reason:");
			ImGui::SameLine();
			ImGui::Text("%s",LastError.c_str());
		}

		if (ImGui::Button("Ok", ImVec2(120, 0)) || ImGui::IsKeyPressed(ImGuiKey_Escape))
		{
			ImGui::CloseCurrentPopup();
			LastError = "";
		}
		
		ImGui::EndPopup();
	}
}

void FEmuBase::SetLastError(const char *fmt, ...)
{
    const int kBufSize = 1024;
    char buf[kBufSize];
    va_list ap;
    va_start(ap, fmt);
    vsnprintf(buf,kBufSize, fmt, ap);
    va_end(ap);
    
    LastError = buf;
}

void FEmuBase::DisplayErrorMessage(const char *fmt, ...)
{
    const int kBufSize = 1024;
    char buf[kBufSize];
    va_list ap;
    va_start(ap, fmt);
    vsnprintf(buf,kBufSize, fmt, ap);
    va_end(ap);
    
	bErrorMessagePopup = true;
	ErrorPopupText = buf;
}

void FEmuBase::LoadFont()
{
	if (FGlobalConfig* pGlobalConfig = CodeAnalysis.pGlobalConfig)
	{
		ImGuiIO& io = ImGui::GetIO();
		if (pGlobalConfig->bBuiltInFont)
		{
			io.Fonts->Clear();
			io.Fonts->AddFontDefault();
			io.Fonts->Build();
		}
		else if (!pGlobalConfig->Font.empty())
		{
			io.Fonts->Clear();
			const std::string fontPath = "./Fonts/" + pGlobalConfig->Font;

			// Calculate the font pixel size based on the DPI scale of the primary monitor.
			// Note: Monitors[0] is the primary monitor 
			const ImGuiPlatformIO& platformIO = ImGui::GetPlatformIO();
			const float dpiScale = !platformIO.Monitors.empty() ? platformIO.Monitors[0].DpiScale : 1.0f;
			if (!io.Fonts->AddFontFromFileTTF(GetBundlePath(fontPath.c_str()), (float)pGlobalConfig->FontSizePts * dpiScale))
			{
				LOGWARNING("Could not load font '%s'", fontPath.c_str());
			}
			io.Fonts->Build();
		}
	}
}

bool	FEmuBase::IsLabelStubbed(const char* pLabelName)
{
	if(pCurrentProjectConfig == nullptr)
		return false;

	const std::string labelName(pLabelName);

	for (const auto& stub : pCurrentProjectConfig->StubOutFunctions)
	{
		if(stub == labelName)
			return true;
	}

	return false;
}

bool	FEmuBase::AddStubbedLabel(const char* pLabelName)
{
	if(IsLabelStubbed(pLabelName))
		return false;

	pCurrentProjectConfig->StubOutFunctions.push_back(pLabelName);
	return true;
}

bool	FEmuBase::RemoveStubbedLabel(const char* pLabelName)
{
	if (pCurrentProjectConfig == nullptr)
		return false;

	const std::string labelName(pLabelName);

	for (auto stubIt = pCurrentProjectConfig->StubOutFunctions.begin(); stubIt != pCurrentProjectConfig->StubOutFunctions.end(); ++stubIt)
	{
		if (*stubIt == labelName)
		{
			pCurrentProjectConfig->StubOutFunctions.erase(stubIt);
			return true;
		}
	}

	return false;
}

// Viewers
void FEmuBase::AddViewer(FViewerBase* pViewer)
{
	pViewer->Init();
	Viewers.push_back(pViewer);
}

bool FEmuBase::StartGameFromName(const char* pGameName, bool bLoadGameData)
{
	FProjectConfig* pGameConfig = GetGameConfigForName(pGameName);
	if (pGameConfig)
	{
		if (LoadProject(pGameConfig, true) == false)
		{
			Reset();
			DisplayErrorMessage("Could not start game '%s'",pGameConfig->Name.c_str());
			return false;
		}
		else
		{
			return true;
		}
	}

	return false;
}

void FEmuBase::GraphicsViewerSetView(FAddressRef address)
{
	if(pGraphicsViewer)
		pGraphicsViewer->GoToAddress(address);
}

void FEmuBase::CharacterMapViewerSetView(FAddressRef address)
{
	if (pCharacterMapViewer)
		pCharacterMapViewer->GoToAddress(address);
}

bool	FEmuBase::AddGamesList(const char* pFileType, const char* pRootDir)
{
	auto insertRes = GamesLists.insert({std::string(pFileType),FGamesList(pFileType, pRootDir)});

	if(insertRes.second)
		insertRes.first->second.EnumerateGames();

	return true;
}



// Util

std::string FEmuBase::GetGameWorkspaceRoot() const
{
	return GetGlobalConfig()->WorkspaceRoot + GetProjectConfig()->Name + "/";
}
