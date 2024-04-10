#include "ImGuiScaling.h"

#include <imgui.h>
#include  <math.h>

static float g_Scaling = 1.0f;
static float g_GlyphWidth = 8.0f;

void ImGui_InitScaling(void)
{
	const float fontSize = ImGui::GetFontSize();
	g_Scaling = ceilf(fontSize / 13.0f);
	if(ImGui::GetFont() != nullptr)
		g_GlyphWidth = ImGui::CalcTextSize("F").x;

}

float ImGui_GetScaling(void)
{
	return g_Scaling;
}

float ImGui_GetFontCharWidth()
{
	return g_GlyphWidth;
}