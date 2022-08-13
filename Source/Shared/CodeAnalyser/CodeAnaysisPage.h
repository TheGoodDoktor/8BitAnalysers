#pragma once
#include <stdio.h>
#include <cstdint>
#include <string>
#include <map>

class FMemoryBuffer;


enum class LabelType
{
	Data,
	Function,
	Code,
};

enum class ItemType
{
	Label,
	Code,
	Data,
};

struct FItem
{
	ItemType		Type;
	std::string		Comment;
	uint16_t		Address;	// note: this might be a problem if pages are mapped to different physical addresses
	uint16_t		ByteSize;
	int				FrameLastAccessed = -1;
	bool			bBreakpointed = false;
};

struct FLabelInfo : FItem
{
	FLabelInfo() { Type = ItemType::Label; }

	std::string				Name;
	bool					Global = false;
	LabelType				LabelType;
	std::map<uint16_t, int>	References;
};

struct FCodeInfo : FItem
{
	FCodeInfo() :FItem()
	{
		Type = ItemType::Code;
	}

	std::string		Text;				// Disassembly text
	uint16_t		JumpAddress = 0;	// optional jump address
	uint16_t		PointerAddress = 0;	// optional pointer address

	union
	{
		struct
		{
			bool			bDisabled : 1;
			bool			bSelfModifyingCode : 1;
		};
		uint32_t	Flags = 0;
	};
};

enum class DataType
{
	Byte,
	Word,
	Text,		// ascii text
	Graphics,	// pixel data
	Blob,		// opaque data blob
};

struct FDataInfo : FItem
{
	FDataInfo() :FItem() { Type = ItemType::Data; }

	DataType	DataType = DataType::Byte;

	int						LastFrameRead = -1;
	std::map<uint16_t, int>	Reads;	// address and counts of data access instructions
	int						LastFrameWritten = -1;
	std::map<uint16_t, int>	Writes;	// address and counts of data access instructions
};


struct FCodeAnalysisPage
{
	void Initialise(uint16_t address);
	void Reset(void);
	void WriteToBuffer(FMemoryBuffer& buffer);
	bool ReadFromBuffer(FMemoryBuffer& buffer);
	//void WriteToJSon(nlohmann::json& jsonOutput);

	static const int kPageSize = 1024;	// 1Kb page

	uint16_t		BaseAddress; // physical base address
	FLabelInfo*		Labels[kPageSize];
	FCodeInfo*		CodeInfo[kPageSize];
	FDataInfo		DataInfo[kPageSize];
	uint16_t		LastWriter[kPageSize];
};