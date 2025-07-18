#pragma once

#include <cstdint>

class FTubeEliteDisplay
{
public:
	bool ProcessVDUChar(uint8_t ch);

	void DrawUI(void);
private:
	static const int kCharMapSizeX = 40;
	static const int kCharMapSizeY = 24;

	uint8_t CharMap[kCharMapSizeX][kCharMapSizeY] = { 0 };
	int		CursorX = 0;
	int		CursorY = 0;
};