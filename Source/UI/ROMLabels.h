#pragma once

#include <cstdint>
#include "SpeccyUI.h"

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
	{0x0000, "RST_Start"},
	{0x0008, "RST_Error"},
	{0x0010, "RST_PrintChar"},
	{0x0018, "RST_CollectChar"},
	{0x0020, "RST_CollectNextChar"},
	{0x0028, "RST_FPCalc"},
	{0x0030, "RST_MakeBCSpaces"},
	{0x0038, "RST_MaskInt"},

	// Rom Routines
	{0x028e, "Key-Scan"},
	{0x02bf, "Keyboard"},
	{0x031e, "Key-Test"},
	{0x0a80, "PO-Change"},
	{0x0b03, "PO-Fetch"},
	{0x0b24, "PO-Any"},
	{0x0bdb, "PO-Attr"},
	{0x0e9b, "CL-Addr"},
	{0x15f2, "Print-A2"},
};

