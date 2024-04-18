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
	void	DrawAt(float x, float y);
	void	Draw();

	void	SetGridSize(int x,int y) { GridSizeX = x; GridSizeY = y;}

	virtual void FixupAddressRefs();

protected:
	virtual void	DrawBackground(float x, float y) {}
	void	DrawGrid(float x, float y);

	FCodeAnalysisState*	CodeAnalysis = nullptr;

	float	GridSquareSize = 10.0f;
	int		GridSizeX = -1;
	int		GridSizeY = -1;
	int		GridStride = -1;
	int		OffsetX = 0;
	int		OffsetY = 0;


	bool	bDrawGrid = true;
	bool	bShowValues = false;
	bool	bShowReadWrites = true;
	bool	bOutlineAllSquares = false;
	bool	bUseIgnoreValue = false;
	uint8_t	IgnoreValue = 0;

	FAddressRef	SelectedCharAddress;
	int			SelectedCharX = -1;
	int			SelectedCharY = -1;
	bool		bDetailsToSide = false;
};