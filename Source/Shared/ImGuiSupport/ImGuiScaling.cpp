#include "ImGuiScaling.h"

#include <imgui.h>
#include  <math.h>

static float g_Scaling = 1.0f;

void ImGui_InitScaling(void)
{
	const float fontSize = ImGui::GetFontSize();
	g_Scaling = fontSize / 13.0f;
}

float ImGui_GetScaling(void)
{
	//return 1.0f;	// temp
	const float fontSize = ImGui::GetFontSize();
	g_Scaling = ceilf(fontSize / 13.0f);
	return g_Scaling;
}