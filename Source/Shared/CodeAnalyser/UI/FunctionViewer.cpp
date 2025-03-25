#include "FunctionViewer.h"

#include <imgui.h>

bool FFunctionViewer::Init()
{
	return true;
}

void FFunctionViewer::Shutdown()
{
}

void FFunctionViewer::DrawUI()
{
	ImGui::Text("Function Viewer");
}

