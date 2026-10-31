#include "ImGuiSettingsHandler.h"

#include <imgui.h>
#include "imgui_internal.h"	// for ImGuiSettingsHandler
#include <cstring>
#include <cstdio>

#include "Misc/EmuBase.h"
#include "CodeAnalyser/UI/ViewerBase.h"

// Persist each viewer window's open/closed state as a custom "[ViewerWindow][<name>]" section
// in imgui.ini, alongside the built-in window position/size/collapsed data.
// Like this:
//
// [ViewerWindow][Static Analysis]
// Open=0

static void* ImguiSettings_ReadOpen(ImGuiContext*, ImGuiSettingsHandler* handler, const char* name)
{
	FEmuBase* pEmu = (FEmuBase*)handler->UserData;
	for (FViewerBase* pViewer : pEmu->GetViewers())
	{
		if (strcmp(pViewer->GetName(), name) == 0)
			return pViewer;
	}
	return nullptr;	// unknown viewer (removed/renamed) - line(s) will be ignored
}

static void ImguiSettings_ReadLine(ImGuiContext*, ImGuiSettingsHandler*, void* entry, const char* line)
{
	FViewerBase* pViewer = (FViewerBase*)entry;
	int open = 1;
	if (sscanf(line, "Open=%d", &open) == 1)
		pViewer->bOpen = open != 0;
}

static void ImguiSettings_WriteAll(ImGuiContext*, ImGuiSettingsHandler* handler, ImGuiTextBuffer* buf)
{
	FEmuBase* pEmu = (FEmuBase*)handler->UserData;
	for (FViewerBase* pViewer : pEmu->GetViewers())
	{
		buf->appendf("[%s][%s]\n", handler->TypeName, pViewer->GetName());
		buf->appendf("Open=%d\n\n", pViewer->bOpen ? 1 : 0);
	}
}

void RegisterImguiSettingsHandler(FEmuBase* pEmu)
{
	ImGuiSettingsHandler handler;
	handler.TypeName = "ViewerWindow";
	handler.TypeHash = ImHashStr("ViewerWindow");
	handler.UserData = pEmu;
	handler.ReadOpenFn = ImguiSettings_ReadOpen;
	handler.ReadLineFn = ImguiSettings_ReadLine;
	handler.WriteAllFn = ImguiSettings_WriteAll;
	ImGui::AddSettingsHandler(&handler);
}
