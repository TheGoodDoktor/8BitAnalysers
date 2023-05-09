#pragma once
#include <stdio.h>
#include <cstdint>
#include <string>
//#include <map>
#include <vector>

#include <Util/Misc.h>

#include "CodeAnalyserTypes.h"

class FMemoryBuffer;

// don't change order or you'll mess up the load/save
enum class ELabelType
{
	Data,
	Function,
	Code,
	Text,

	Max,
	None = Max
};

enum class EItemType
{
	Label,
	Code,
	Data,
	CommentBlock,
	CommentLine,

	Unknown
};

enum class EOperandType
{
	Unknown = 0,
	Pointer,
	JumpAddress,
	Decimal,
	Hex,
	Binary
};



struct FItem
{
	EItemType		Type = EItemType::Unknown;
	std::string		Comment;
	uint16_t		ByteSize = 0;
};

/*struct FItemReference
{
	FItemReference() = default;
	//FItemReference(uint16_t pc) : InstructionAddress(pc) {}
	FItemReference(uint16_t pc, int16_t bankId) : InstructionRef(pc, bankId) {}
	FItemReference(const FAddressRef& addrRef) : InstructionRef(addrRef) {}

	FAddressRef	InstructionRef;
	//uint16_t	InstructionAddress = 0;
	//int16_t		InstructionPageId = 0;
};*/

class FItemReferenceTracker
{
public:
	void Reset() { References.clear(); }
	
	void	RegisterAccess(const FAddressRef& addrRef)
	{
		const auto size = References.size();
		for (int i = 0; i < size; i++)
		{
			if (References[i] == addrRef)
				return;
		}

		References.emplace_back(addrRef);
	}

	bool IsEmpty() const { return References.empty(); }
	const std::vector<FAddressRef>& GetReferences() const { return References; }
private:
	std::vector<FAddressRef>	References;
};

struct FLabelInfo : FItem
{
	static FLabelInfo* Allocate();
	static void FreeAll();

	std::string				Name;
	bool					Global = false;
	ELabelType				LabelType = ELabelType::Data;
	FItemReferenceTracker	References;
	//std::map<uint16_t, int>	References;
private:
	FLabelInfo() { Type = EItemType::Label; }
	~FLabelInfo() = default;

	static std::vector<FLabelInfo*>	AllocatedList;
};

struct FCodeInfo : FItem
{
	static FCodeInfo* Allocate();
	static void FreeAll();

	EOperandType	OperandType = EOperandType::Unknown;
	std::string		Text;				// Disassembly text
	FAddressRef		JumpAddress;	// optional jump address
	FAddressRef		PointerAddress;	// optional pointer address
	int				FrameLastExecuted = -1;

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
	uint8_t	OpcodeBkp[4] = { 0 };
private:
	FCodeInfo() :FItem(){Type = EItemType::Code;	}
	~FCodeInfo() = default;

	static std::vector<FCodeInfo*>	AllocatedList;
};



enum class EDataType
{
	Byte,
	ByteArray,
	Word,
	WordArray,
	Text,		// ascii text
	Bitmap,
	CharacterMap,
	ScreenPixels,	// screen pixel data
	Image,		// character/sprite image
	Blob,		// opaque data blob
	ColAttr,	// colour attribute
	InstructionOperand,	// an operand for an instruction

	Max,
	None = Max
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
	FDataInfo() :FItem() { Type = EItemType::Data; }

	void Reset()
	{
		Flags = 0;
		ByteSize = 1;
		DataType = EDataType::Byte;
		OperandType = EOperandType::Unknown;
		Comment.clear();
		LastFrameRead = -1;
		Reads.Reset();
		LastFrameWritten = -1;
		Writes.Reset();
	}

	EDataType	DataType = EDataType::Byte;
	EOperandType	OperandType = EOperandType::Unknown;

	union
	{
		struct
		{
			bool			bGameState : 1;
			bool			bUnused1 : 1;	
			bool			bUnused2 : 1;
			bool			bBit7Terminator : 1;	// for bit 7 terminated strings
			bool			bShowBinary : 1;	// display the value(s) as binary
			bool			bShowCharMap : 1;	// display memory as character map
		};
		uint32_t	Flags = 0;
	};

	union
	{
		FImageData* ImageData = nullptr;	// additional data for images
		FAddressRef	CharSetAddress;	// address of character set
		/*struct
		{
		};*/
		FAddressRef	InstructionAddress;	// for operand data types
	};
	uint8_t		EmptyCharNo = 0;

	int						LastFrameRead = -1;
	FItemReferenceTracker	Reads;	// address and counts of data access instructions
	int						LastFrameWritten = -1;
	FItemReferenceTracker	Writes;	// address and counts of data access instructions
	FAddressRef				LastWriter;
};

struct FCommentBlock : FItem
{
	static FCommentBlock* Allocate();
	static void FreeAll();

private:
	FCommentBlock() : FItem() { Type = EItemType::CommentBlock; }
	~FCommentBlock() = default;
	static std::vector<FCommentBlock*>	AllocatedList;
};

struct FCommentLine : FItem
{

	static FCommentLine* Allocate();
	static void FreeAll();
private:
	FCommentLine() : FItem() { Type = EItemType::CommentLine; }
	~FCommentLine() = default;

	static std::vector<FCommentLine*>	AllocatedList;
	static std::vector<FCommentLine*>	FreeList;
};

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

	void SetLabelAtAddress(const char* pLabelName, ELabelType type, uint16_t addr);
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