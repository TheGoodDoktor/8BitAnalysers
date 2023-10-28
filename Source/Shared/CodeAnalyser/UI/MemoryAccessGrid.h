#pragma once

#include "CodeAnalyser/CodeAnalyserTypes.h"

class FCodeAnalysisState;

class FMemoryAccessGrid
{
public:
	FMemoryAccessGrid(FCodeAnalysisState* pCodeAnalysis, int xGridSize, int yGridSize);

	virtual FAddressRef GetGridSquareAddress(int x, int y) = 0;
	virtual void OnDraw() = 0;

	bool	GetAddressGridPosition(FAddressRef address, int& outX, int& outY);
	void	DrawAt(float x,float y);
protected:
	FCodeAnalysisState*	CodeAnalysis = nullptr;

	float	GridSquareSize = 10.0f;
	int		GridSizeX = -1;
	int		GridSizeY = -1;

	bool	bShowReadWrites = true;

	FAddressRef	SelectedCharAddress;
	int			SelectedCharX = -1;
	int			SelectedCharY = -1;
};