#pragma once

#include <cinttypes>

// Colour Presets
namespace Colours
{
	extern uint32_t defaultValue;		// default value 
	extern uint32_t unknownValue;		// unknown value
	extern uint32_t unknownDataRead;		// unknown data read
	extern uint32_t unknownDataWrite;		// unknown data write
	extern uint32_t localLabel;	// local label
	extern uint32_t globalLabel;		// global label
	extern uint32_t function;		// function
	extern uint32_t functionDesc;		// function desc
	extern uint32_t comment;			// comment
	extern uint32_t reg;			// registers
	extern uint32_t address;			// addresses
	extern uint32_t mnemonic;
	extern uint32_t noppedMnemonic;
	extern uint32_t immediate;		// immediate values
	extern uint32_t text;
	extern uint32_t highlight;	// highlighted text
    extern uint32_t error;  // for errors

	void Tick();
	uint32_t GetFlashColour();

}//namespace Colours
