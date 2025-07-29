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
		uint8_t bytes[4] = { 0, 0, 0, 0 };
	};
	uint32_t colour = 0;	// line colour
};

struct FPixel
{
	union 
	{
		struct  
		{
			uint8_t dist;
			uint8_t x, y;
		};
		uint32_t val;	// packed dist,x,y
		uint8_t bytes[4] = { 0, 0, 0, 0 };
	};
	uint32_t colour = 0;	// line colour
};

// https://elite.bbcelite.com/6502sp/i_o_processor/workspace/i_o_variables.html#params
struct FDashboardParams
{
	union
	{
		struct
		{
			uint8_t		Energy;		// energy level (0-255)
			uint8_t		Apl1;		// Magnitude of the roll angle alpha, i.e. |alpha|, which is a positive value between 0 and 31
			uint8_t		Apl2;		// sign of roll angle
			uint8_t		Beta;
			uint8_t		Bet1;
			uint8_t		Speed;		// speed (1-40)
			uint8_t		Altitude;	// altitude
			uint8_t		MainLoopCounter;
			uint8_t		ForwardShield;	// forward shield strength
			uint8_t		AftShield;	// aft shield strength
			uint8_t		Fuel;		// fuel level
			uint8_t		LaserTemp;	// laser temperature
			uint8_t		CabinTemp;	// cabin temperature
			uint8_t		FlashingCOnsoleBarsConfig;		// flashing console bars configuration
			uint8_t		EscapePod;
		};
		uint8_t Bytes[15];
	};
};

class FTubeEliteDisplay
{
public:
	bool Init(FTubeElite* pSys);
	bool ProcessVDUChar(uint8_t ch);
	bool ProcessMOSVDUChar(uint8_t ch);
	bool ProcessEliteCommandByte(uint8_t cmdByte);
	bool ProcessEliteChar(uint8_t ch);
	void DrawCharAtCursor(uint8_t ch);
	
	void SetCursorX(int x);
	void SetCursorY(int y);

	void ClearTextScreen(uint8_t clearChar = 0);
	void ClearScreenBottom(void);
	void ClearTextScreenFromRow(uint8_t rowNo,uint8_t claerChar=0);

	bool AddLine(const FLine& line);
	void ReceivePixelData(const uint8_t* pPixelData);

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

	// Command related
	uint8_t ProcessingCommand = 0;	// currently processing command ID
	int		NoCommandBytesRead = 0;	// number of bytes read for the current command

	// Character map related
	uint8_t CharMap[kCharMapSizeX][kCharMapSizeY] = { 0 };
	int		CursorX = 0;
	int		CursorY = 0;

	uint8_t	CurrentColour = 0;
	uint8_t ColourPalette = 0;

	// Line related
	static const int kMaxLines = 256;
	FLine	LineHeap[kMaxLines];
	int		NoLines = 0;
	FLine	NewLine;
	int		NumLineBytesToRead = 0;	// number of bytes to read for the next line

	// pixel related
	static const int kMaxPixels = 256;
	FPixel	PixelHeap[kMaxPixels];
	int		NoPixels = 0;

	FDashboardParams	DashboardParams;	// dashboard parameters

	// VDU log
	bool	bShowLog = true;
	bool	bLastCharCtrl = false;	// last char was a control char

	// Input handling
	bool	BBCKeyDown[256] = { false };	// BBC key down state
	bool	bWindowFocused = false;	// true if the display window is focused
};