#pragma once
#include <stdio.h>
#include <cstdint>
#include <string>
#include <map>
#include <vector>

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
	CommentBlock,
	CommentLine,
};

struct FCPUFunctionCall
{
	uint16_t	FunctionAddr = 0;
	uint16_t	CallAddr = 0;
	uint16_t	ReturnAddr = 0;
};

struct FItem
{
	ItemType		Type;
	std::string		Comment;
	uint16_t		Address;	// note: this might be a problem if pages are mapped to different physical addresses
	uint16_t		ByteSize;
	int				FrameLastAccessed = -1;
	//bool			bBreakpointed = false;
};

struct FLabelInfo : FItem
{
	static FLabelInfo* Allocate();
	static void FreeAll();

	std::string				Name;
	bool					Global = false;
	LabelType				LabelType;
	std::map<uint16_t, int>	References;
private:
	FLabelInfo() { Type = ItemType::Label; }
	~FLabelInfo() = default;

	static std::vector<FLabelInfo*>	AllocatedList;
};

struct FCodeInfo : FItem
{
	static FCodeInfo* Allocate();
	static void FreeAll();

	std::string		Text;				// Disassembly text
	uint16_t		JumpAddress = 0;	// optional jump address
	uint16_t		PointerAddress = 0;	// optional pointer address

	union
	{
		struct
		{
			bool			bDisabled : 1;
			bool			bSelfModifyingCode : 1;
			bool			bUnused : 1;
		};
		uint32_t	Flags = 0;
	};

	bool	bNOPped = false;
	uint8_t	OpcodeBkp[4];
private:
	FCodeInfo() :FItem(){Type = ItemType::Code;	}
	~FCodeInfo() = default;

	static std::vector<FCodeInfo*>	AllocatedList;
};



enum class DataType
{
	Byte,
	ByteArray,
	Word,
	WordArray,
	Text,		// ascii text
	Bitmap,
	CharacterMap,
	Graphics,	// pixel data
	Image,		// character/sprite image
	Blob,		// opaque data blob
};

// struct for additional image data
struct FImageData
{
	~FImageData();

	int			SetSizeChars(int w, int h)	// return byte size
	{
		XSizeChars = (uint8_t)w;
		YSizeChars = (uint8_t)h;
		return w * h * 8;
	}
	uint8_t			XSizeChars = 1;	// x size in chars for images
	uint8_t			YSizeChars = 1;	// y size in chars for images
	uint8_t			ViewerId = 0;	// Id of image viewer
	class FGraphicsView* GraphicsView = nullptr;
};

struct FDataInfo : FItem
{
	FDataInfo() :FItem() { Type = ItemType::Data; }

	DataType	DataType = DataType::Byte;
	
	union
	{
		struct
		{
			bool			bGameState : 1;
			bool			bCodeOperand : 1;	// for self modifying code
			bool			bUnused : 1;
			bool			bBit7Terminator : 1;	// for bit 7 terminated strings
			bool			bShowBinary : 1;	// display the value(s) as binary
			bool			bShowCharMap : 1;	// display memory as character map
		};
		uint32_t	Flags = 0;
	};

	FImageData*		ImageData = nullptr;	// additional data for images

	int						LastFrameRead = -1;
	std::map<uint16_t, int>	Reads;	// address and counts of data access instructions
	int						LastFrameWritten = -1;
	std::map<uint16_t, int>	Writes;	// address and counts of data access instructions
};

struct FCommentBlock : FItem
{
	static FCommentBlock* Allocate();
	static void FreeAll();

private:
	FCommentBlock() : FItem() { Type = ItemType::CommentBlock; }
	~FCommentBlock() = default;
	static std::vector<FCommentBlock*>	AllocatedList;
};

struct FCommentLine : FItem
{

	static FCommentLine* Allocate();
	static void FreeAll();
private:
	FCommentLine() : FItem() { Type = ItemType::CommentLine; }
	~FCommentLine() = default;

	static std::vector<FCommentLine*>	AllocatedList;
	static std::vector<FCommentLine*>	FreeList;
};

struct FCodeAnalysisPage
{
	void Initialise(uint16_t address);
	void Reset(void);
	void WriteToBuffer(FMemoryBuffer& buffer);
	bool ReadFromBuffer(FMemoryBuffer& buffer);
	//void WriteToJSon(nlohmann::json& jsonOutput);

	void SetLabelAtAddress(const char* pLabelName, LabelType type, uint16_t addr);
	static const int kPageSize = 1024;	// 1Kb page

	int16_t			PageId = -1;
	uint16_t		BaseAddress; // physical base address
	FLabelInfo*		Labels[kPageSize];
	FCodeInfo*		CodeInfo[kPageSize];
	FDataInfo		DataInfo[kPageSize];
	FCommentBlock*	CommentBlocks[kPageSize];
	uint16_t		LastWriter[kPageSize];
};