#include "ImGuiDrawing.h"

void DrawArrow(ImDrawList* dl, ImVec2 pos, bool bLeftDirection)
{
	ImVec2 arrowPos = ImVec2(pos.x, pos.y);
	const float h = 13.f;
	float r = h * 0.40f;
	ImVec2 center = ImVec2(arrowPos.x + h * 0.5f, arrowPos.y + h * 0.5f);
	ImVec2 a, b, c;
	if (bLeftDirection) r = -r;
	a = ImVec2(+0.750f * r, +0.000f * r);
	b = ImVec2(-0.750f * r, +0.866f * r);
	c = ImVec2(-0.750f * r, -0.866f * r);
	dl->AddTriangleFilled(ImVec2(center.x + a.x, center.y + a.y), ImVec2(center.x + b.x, center.y + b.y), ImVec2(center.x + c.x, center.y + c.y), 0xffffffff);
}