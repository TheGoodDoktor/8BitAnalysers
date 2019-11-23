#include "StarquakeViewer.h"
#include "UI/SpeccyUI.h"

struct FStarquakeViewerData
{
	
};

void InitStarquakeViewer(FStarquakeViewerData *pStarquakeViewer)
{

	
}

void DrawStarquakeViewer(FSpeccyUI *pUI, FGameViewer &viewer)
{
	FStarquakeViewerData* pData = (FStarquakeViewerData*)viewer.pUserData;

}


void RegisterStarquakeViewer(FSpeccyUI *pUI)
{
	FGameViewer &viewer = AddGameViewer(pUI, "Starquake");
	viewer.pDrawFunction = DrawStarquakeViewer;

	FStarquakeViewerData* pData = new FStarquakeViewerData;

	InitStarquakeViewer(pData);
	viewer.pUserData = pData;
}