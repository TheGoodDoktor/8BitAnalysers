#pragma once
#include <cstdint>
#include <string>

struct FSpeccyUI;

struct FCodeInfo
{
	uint16_t	Address;
	std::string	Text;
	int			ByteSize;
	bool		EndPoint = false;
};

struct FCodeAnalysisState
{
	uint16_t	CurrentAddress;
	FSpeccyUI *	pUI;

	FCodeInfo *	pInstructionInfo = nullptr;	// current instruction info
	//std::string	InstructionStr;

	FCodeInfo*	CodeInfo[0x10000];
};

bool GenerateLabelsForAddress(FSpeccyUI *pUI, uint16_t pc);

void RunStaticCodeAnalysis(FSpeccyUI *pUI, uint16_t pc);

void DrawCodeAnalysisData(FSpeccyUI *pUI);