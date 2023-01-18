#pragma once

#include <cstdint>
#include "SpectrumEmu.h"

// Rom routines
// obtained from:
// http://www.primrosebank.net/computers/zxspectrum/docs/CompleteSpectrumROMDisassemblyThe.pdf

struct FROMLabelInfo
{
	uint16_t		Address;
	const char *	pLabelName;
	LabelType		LabelType = LabelType::Function;
};

FROMLabelInfo g_RomLabels[] =
{
	// Restarts
	{0x0000, "START"},
	{0x0008, "ERROR-1"},
	{0x0010, "PRINT-A-1"},
	{0x0018, "GET-CHAR"},
	{0x001C, "TEST-CHAR"},
	// collect character restart
	{0x0020, "NEXT-CHAR"},
	// calculator restart
	{0x0028, "FP-CALC"},
	// make bc spaces restart
	{0x0030, "BC-SPACES"},
	// maskable interrupt routine
	{0x0038, "MASK-INT"},
	{0x0048, "KEY-INT"},

	// Rom Routines

	// 'Error-2' routine
	{0x0053, "ERROR-2"},
	{0x0055, "ERROR-3"},
	// NMI routine
	{0x0066, "RESET"},
	{0x0070, "NO-RESET"},
	// 'ch-add+1' subroutine
	{0x0074, "CH-ADD+1"},
	{0x0077, "TEMP-PTR1"},
	{0x0078, "TEMP-PTR2"},
	// 'skip over' subroutine
	{0x007D, "SKIP-OVER"},
	{0x0090, "SKIPS"},
	// keyboard scanning subroutine
	{0x028e, "KEY-SCAN"},
	{0x0296, "KEY-LINE"},
	{0x029F, "KEY-3KEYS"},
	{0x02A1, "KEY-BITS"},
	{0x02AB, "KEY-DONE"},
	// keyboard subroutine
	{0x02bf, "KEYBOARD"},
	{0x02C6, "K-ST-LOOP"},
	{0x02D1, "K-CH-SET"},
	{0x02F1, "K-NEW"},
	{0x0308, "K-END"},
	// the 'repeating key' subroutine
	{0x0310, "K-REPEAT"},
	//k-test subroutine
	{0x031e, "KEY-TEST"},
	{0x032C, "KEY-MAIN"},
	// keyboard decoding Subroutine
	{0x0333, "K-DECODE"},
	{0x0341, "K-E-LET"},
	{0x034A, "K-LOOK-UP"},
	{0x034F, "K-KLC-LET"},
	{0x0364, "K-TOKENS"},
	{0x0367, "K-DIGIT"},
	{0x0382, "K-8-&-9"},
	{0x0389, "K-GRA-DGT"},
	{0x039D, "K-KLC-DGT"},
	{0x03B2, "K-@-CHAR"},
	// Loudspeaker Routines

	{0x0a80, "PO-Change"},
	{0x0b03, "PO-Fetch"},
	{0x0b24, "PO-Any"},
	{0x0bdb, "PO-Attr"},
	{0x0e9b, "CL-Addr"},
	{0x15f2, "Print-A2"},
};

// ROM System variables (in RAM!)
// https://skoolkid.github.io/rom/buffers/sysvars.html
//
struct FROMSysVariable
{
	uint16_t		Address;
	uint16_t		Length;
	const char *	pLabelName;
};


FROMSysVariable g_SysVariables[] =
{
	{0x5C00, 8,"KSTATE"},
	{0x5C08, 1,"LAST-K"},
	{0x5C09, 1,"REPDEL"},
	{0x5C0A, 1,"REPPER"},
	{0x5C0B, 2,"DEFADD"},
	{0x5C0D, 1,"KDATA"},
	{0x5C0E, 2,"TVDATA"},
	{0x5C10, 38,"STRMS"},
};
