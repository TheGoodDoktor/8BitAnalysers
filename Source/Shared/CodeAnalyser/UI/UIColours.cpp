#include "UIColours.h"
#include <imgui.h>

#define IM_COLRGB(R,G,B) IM_COL32(R,G,B,255)

// Colour Presets
namespace Colours
{
	// ABGR
	uint32_t defaultValue	= IM_COLRGB(255, 255, 255);		// default value 
	uint32_t unknownValue = IM_COLRGB(128, 128, 128);		// unknown value
	uint32_t unknownDataRead = IM_COLRGB(0, 128, 0);		// unknown data read
	uint32_t unknownDataWrite = IM_COLRGB(215, 0, 0);		// unknown data write
	uint32_t localLabel		= IM_COLRGB(128, 128, 128);		// local label
	uint32_t globalLabel	= IM_COLRGB(128, 255, 128);		// global label
	uint32_t function		= IM_COLRGB(255, 200, 128);		// function
	uint32_t functionDesc	= IM_COLRGB(255, 200, 128);		// function desc
	uint32_t comment		= IM_COLRGB(87, 166, 74);		// comment
	uint32_t reg			= IM_COLRGB(175, 255, 255);		// registers
	uint32_t address		= IM_COLRGB(175, 175, 255);		// addresses
	uint32_t mnemonic		= IM_COLRGB(207, 255, 255);		// opcode
	uint32_t noppedMnemonic	= IM_COLRGB(128, 128, 128);		// nopped opcode
	uint32_t immediate		= IM_COLRGB(255, 255, 255);		// immediate
	uint32_t text			= IM_COLRGB(255, 200, 128);		// text 

	uint32_t highlight		= IM_COLRGB(0,255,0);	// highlighted addresses/labels
    uint32_t error          = IM_COLRGB(255,0,0);
	static int FrameCounter = 0;

	void Tick()
	{
		FrameCounter++;
	}

	uint32_t GetFlashColour()
	{
		// generate flash colour
		uint32_t flashCol = 0xff000000;
		const int flashCounter = FrameCounter >> 2;
		if (flashCounter & 1) flashCol |= 0xff << 0;
		if (flashCounter & 2) flashCol |= 0xff << 8;
		if (flashCounter & 4) flashCol |= 0xff << 16;
		return flashCol;
	}


}//namespace Colours
