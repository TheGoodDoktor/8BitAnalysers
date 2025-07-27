#pragma once

#include <cstdint>

class FTubeElite;

struct FLine
{
	union 
	{
		struct
		{
			uint8_t x1, y1, x2, y2;
		};
		uint32_t val;	// packed x1,y1,x2,y2
	};
};

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
	void ClearScreenBottom(void);
	void ClearTextScreenFromRow(uint8_t rowNo,uint8_t claerChar=0);

	bool AddLine(uint8_t x1, uint8_t y1, uint8_t x2, uint8_t y2);

	bool UpdateKeyboardBuffer(uint8_t* pBuffer);
	bool IsKeyDown(uint8_t key)
	{
		return BBCKeyDown[key];
	}
	void Tick(void);
	void DrawUI(void);
private:
	static const int kCharMapSizeX = 40;
	static const int kCharMapSizeY = 24;

	FTubeElite*	pTubeSys = nullptr;
	uint8_t CharMap[kCharMapSizeX][kCharMapSizeY] = { 0 };
	int		CursorX = 0;
	int		CursorY = 0;

	static const int kMaxLines = 256;
	FLine	LineHeap[kMaxLines];
	int		NoLines = 0;

	uint8_t		LastKeyCode = 0;	// last key code processed

	bool	bWindowFocused = false;	// true if the display window is focused

	bool	bShowLog = true;
	bool	bLastCharCtrl = false;	// last char was a control char

	bool	BBCKeyDown[256] = { false };	// BBC key down state
};