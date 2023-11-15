#include "ImGuiScaling.h"

#include <imgui.h>
#include  <math.h>

static float g_Scaling = 1.0f;

void ImGui_InitScaling(void)
{
	const float fontSize = ImGui::GetFontSize();
	g_Scaling = ceilf(fontSize / 13.0f);
}

float ImGui_GetScaling(void)
{
	return g_Scaling;
}