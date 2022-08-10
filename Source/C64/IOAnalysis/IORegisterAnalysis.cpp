#include "IORegisterAnalysis.h"

#include <imgui.h>

void DrawRegValueDefault(uint8_t val)
{
	ImGui::Text("$%X", val);
}

void DrawRegValueDecimal(uint8_t val)
{
	ImGui::Text("%d", val);
}