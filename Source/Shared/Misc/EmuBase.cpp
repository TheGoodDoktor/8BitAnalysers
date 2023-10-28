#include "EmuBase.h"

#include <imgui.h>
#include <CodeAnalyser/UI/CodeAnalyserUI.h>

bool	FEmuBase::Init()
{

    return true;
}


void FEmuBase::Shutdown()
{

}

void FEmuBase::Tick()
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
        sprintf(name, "Code Analysis %d", codeAnalysisNo + 1);
        if (CodeAnalysis.ViewState[codeAnalysisNo].Enabled)
        {
            if (ImGui::Begin(name, &CodeAnalysis.ViewState[codeAnalysisNo].Enabled))
            {
                DrawCodeAnalysisData(CodeAnalysis, codeAnalysisNo);
            }
            ImGui::End();
        }

    }
    DrawEmulatorUI();
}


void FEmuBase::FileMenu()
{
    // New game from snapshot

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
            CodeAnalysis.SetAllBanksDirty();
            bClearCode = true;
        }
        if (ImGui::MenuItem("Hex - $FE", 0, GetNumberDisplayMode() == ENumberDisplayMode::HexDollar))
        {
            SetNumberDisplayMode(ENumberDisplayMode::HexDollar);
            CodeAnalysis.SetAllBanksDirty();
            bClearCode = true;
        }

        // clear code text so it can be written again
        if (bClearCode)
        {
            for (int i = 0; i < 1 << 16; i++)
            {
                FCodeInfo* pCodeInfo = CodeAnalysis.GetCodeInfoForAddress(i);
                if (pCodeInfo && pCodeInfo->Text.empty() == false)
                    pCodeInfo->Text.clear();

            }
        }

        ImGui::EndMenu();
    }
    ImGui::MenuItem("Scan Line Indicator", 0, &CodeAnalysis.pGlobalConfig->bShowScanLineIndicator);
    ImGui::MenuItem("Enable Audio", 0, &CodeAnalysis.pGlobalConfig->bEnableAudio);
    ImGui::MenuItem("Edit Mode", 0, &CodeAnalysis.bAllowEditing);
    ImGui::MenuItem("Show Opcode Values", 0, &CodeAnalysis.pGlobalConfig->bShowOpcodeValues);

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

    //if(pActiveGame!=nullptr)
    //	ImGui::MenuItem("Save Snapshot with game", 0, &pActiveGame->pConfig->WriteSnapshot);

#ifndef NDEBUG
    ImGui::MenuItem("Show Config", 0, &CodeAnalysis.Config.bShowConfigWindow);
    //TODO: ImGui::MenuItem("ImGui Demo", 0, &bShowImGuiDemo);
    //TODO: ImGui::MenuItem("ImPlot Demo", 0, &bShowImPlotDemo);
#endif // NDEBUG
}

void FEmuBase::WindowsMenu()
{
    //TODO: ImGui::MenuItem("DebugLog", 0, &bShowDebugLog);
    if (ImGui::BeginMenu("Code Analysis"))
    {
        for (int codeAnalysisNo = 0; codeAnalysisNo < FCodeAnalysisState::kNoViewStates; codeAnalysisNo++)
        {
            char menuName[32];
            sprintf(menuName, "Code Analysis %d", codeAnalysisNo + 1);
            ImGui::MenuItem(menuName, 0, &CodeAnalysis.ViewState[codeAnalysisNo].Enabled);
        }

        ImGui::EndMenu();
    }

    for (auto Viewer : Viewers)
    {
        ImGui::MenuItem(Viewer->GetName(), 0, &Viewer->bOpen);

    }
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

        if (ImGui::BeginMenu("Options"))
        {
            OptionsMenu();
            ImGui::EndMenu();
        }

        if (ImGui::BeginMenu("Windows"))
        {
            WindowsMenu();
            ImGui::EndMenu();
        }

        ImGui::EndMainMenuBar();
    }
}

// Viewers
void FEmuBase::AddViewer(FViewerBase* pViewer)
{
    Viewers.push_back(pViewer);
}

void FEmuBase::InitViewers()
{
    for (auto Viewer : Viewers)
    {
        if (Viewer->Init() == false)
        {
            // TODO: report error
        }
    }
}