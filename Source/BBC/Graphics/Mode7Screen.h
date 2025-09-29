#pragma once

class FGraphicsView;
class FBBCEmulator;

void InitMode7ScreenViewer(FBBCEmulator* pEmu);
void DrawMode7ScreenToGraphicsView(FBBCEmulator* pEmu, FGraphicsView *pScreenView);