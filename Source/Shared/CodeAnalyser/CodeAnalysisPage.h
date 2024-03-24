#pragma once
#include <stdio.h>
#include <cstdint>
#include <string>
//#include <map>
#include <vector>

#include <Util/Misc.h>

#include "CodeAnalyserTypes.h"

class FMemoryBuffer;





// abstract machine state class - device specific
struct FMachineState
{

};

struct FCodeAnalysisPage
{
	void Initialise();
	void Reset(void);
	//void WriteToBuffer(FMemoryBuffer& buffer);
	//bool ReadFromBuffer(FMemoryBuffer& buffer);

	void SetLabelAtAddress(const char* pLabelName, ELabelType type, uint16_t addr, bool bGlobal = false);
	static const int kPageSize = 1024;	// 1Kb page
	static const int kPageShift = 10;	// 1Kb page
	static const int kPageMask = kPageSize - 1;

	bool			bUsed = false;	// has this page been used?
	int16_t			PageId = -1;
	FLabelInfo*		Labels[kPageSize];
	FCodeInfo*		CodeInfo[kPageSize];
	FDataInfo		DataInfo[kPageSize];
	FCommentBlock*	CommentBlocks[kPageSize];

	FMachineState*	MachineState[kPageSize];
};