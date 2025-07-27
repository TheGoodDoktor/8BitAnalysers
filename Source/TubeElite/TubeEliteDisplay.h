#pragma once

#include <cstdint>

class FTubeElite;

class FTubeEliteDisplay
{
public:
	bool Init(FTubeElite* pSys);
	bool ProcessVDUChar(uint8_t ch);
	bool ProcessMOSVDUChar(uint8_t ch);
	bool ProcessEliteChar(uint8_t ch);
	void DrawCharAtCursor(uint8_t ch);
	
	void SetCursorX(int x);
	void SetCursorY(int y);

	void ClearTextScreen(uint8_t clearChar = 0);
	void ClearTextScreenFromRow(uint8_t rowNo,uint8_t claerChar=0);

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

	bool	bShowLog = true;
	bool	bLastCharCtrl = false;	// last char was a control char
};