#pragma once
#include <cstdint>
#include <string>
#include "SpeccyUI.h"

enum class LabelType;
struct FSpeccyUI;

struct FCodeInfo : FItem
{
	FCodeInfo() { Type = ItemType::Code; }

	//uint16_t	Address;
	std::string	Text;
	uint16_t	JumpAddress = 0;
	//int			ByteSize;
	//bool		EndPoint = false;
};

struct FCodeAnalysisState
{
	uint16_t	CurrentAddress;
	FSpeccyUI *	pUI;

	FCodeInfo *	pInstructionInfo = nullptr;	// current instruction info
	//std::string	InstructionStr;

	FCodeInfo*	CodeInfo[0x10000];

	bool		bDirty = false;
};



bool GenerateLabelsForAddress(FSpeccyUI *pUI, uint16_t pc, LabelType label);

void RunStaticCodeAnalysis(FSpeccyUI *pUI, uint16_t pc);

void DrawCodeAnalysisData(FSpeccyUI *pUI);