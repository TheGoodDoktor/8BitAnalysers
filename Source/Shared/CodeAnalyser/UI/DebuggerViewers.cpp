#include "DebuggerViewers.h"

#include <imgui.h>
#include "Misc/EmuBase.h"
#include "CodeAnalyser/Debugger.h"

bool FDebuggerViewBase::Init(void)
{
	pDebugger = &pEmulator->GetCodeAnalysis().Debugger;
	return true;
}

#if TABBED_DEBUGGER

void FDebuggerTabsViewer::DrawUI(void)
{
	if (ImGui::BeginTabBar("DebuggerTabBar"))
	{
		if (ImGui::BeginTabItem("Breakpoints"))
		{
			pDebugger->DrawBreakpoints();
			ImGui::EndTabItem();
		}

		if (ImGui::BeginTabItem("Watches"))
		{
			pDebugger->DrawWatches();
			ImGui::EndTabItem();
		}

		if (ImGui::BeginTabItem("Registers"))
		{
			pDebugger->DrawRegisters();
			ImGui::EndTabItem();
		}

		if (ImGui::BeginTabItem("Stack"))
		{
			pDebugger->DrawStack();
			ImGui::EndTabItem();
		}

		if (ImGui::BeginTabItem("Call Stack"))
		{
			pDebugger->DrawCallStack();
			ImGui::EndTabItem();
		}

		if (ImGui::BeginTabItem("Trace"))
		{
			pDebugger->DrawTrace();
			ImGui::EndTabItem();
		}

		if (ImGui::BeginTabItem("Events"))
		{
			pDebugger->DrawEvents();
			ImGui::EndTabItem();
		}

		ImGui::EndTabBar();
	}
}

#else

void FBreakpointsViewer::DrawUI(void)
{
	pDebugger->DrawBreakpoints();
}

void FWatchesViewer::DrawUI(void)
{
	pDebugger->DrawWatches();
}

void FRegistersViewer::DrawUI(void)
{
	pDebugger->DrawRegisters();
}

void FStackViewer::DrawUI(void)
{
	pDebugger->DrawStack();
}

void FCallStackViewer::DrawUI(void)
{
	pDebugger->DrawCallStack();
}

void FTraceViewer::DrawUI(void)
{
	pDebugger->DrawTrace();
}

void FEventsViewer::DrawUI(void)
{
	pDebugger->DrawEvents();
}

#endif // TABBED_DEBUGGER
