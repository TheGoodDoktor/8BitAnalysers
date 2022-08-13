#pragma once
#include <stdio.h>
#include <cstdint>

//namespace nlohmann { class json; }
struct FLabelInfo;
struct FCodeInfo;
struct FDataInfo;

class FMemoryBuffer;

struct FCodeAnalysisPage
{
	void Initialise(uint16_t address);
	void Reset(void);
	void WriteToBuffer(FMemoryBuffer& buffer);
	bool ReadFromBuffer(FMemoryBuffer& buffer);
	//void WriteToJSon(nlohmann::json& jsonOutput);

	static const int kPageSize = 1024;	// 1Kb page

	uint16_t	BaseAddress; // physical base address
	FLabelInfo* Labels[kPageSize];
	FCodeInfo* CodeInfo[kPageSize];
	FDataInfo* DataInfo[kPageSize];
	uint16_t		LastWriter[kPageSize];
};