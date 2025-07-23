#pragma once

#include <cstdint>

class FTubeElite;

class FTubeEliteDisplay
{
public:
	bool Init(FTubeElite* pSys);
	bool ProcessVDUChar(uint8_t ch);
	
	void SetCursorX(int x) { CursorX = x; }
	void SetCursorY(int y) { CursorY = y; }

	void Tick(void);
	void DrawUI(void);
private:
	static const int kCharMapSizeX = 40;
	static const int kCharMapSizeY = 24;

	FTubeElite*	pTubeSys = nullptr;
	uint8_t CharMap[kCharMapSizeX][kCharMapSizeY] = { 0 };
	int		CursorX = 0;
	int		CursorY = 0;

	bool	bWindowFocused = false;	// true if the display window is focused
};