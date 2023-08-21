#pragma once

#include <cstdint>
#include <string>
#include <vector>

// Enums

// CPU abstraction
enum class ECPUType
{
	Unknown,
	Z80,
	M6502
};

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

// NOTE: only add to this enum at the end - there are loose dependencies on it (file format, combo box)
enum class EOperandType
{
	Unknown = 0,
	Pointer,
	JumpAddress,
	Decimal,
	Hex,
	Binary
};

// NOTE: only add to this enum at the end - there are loose dependencies on it (file format, combo box)
enum class EDataItemDisplayType
{
	Unknown = 0,
	Pointer,
	JumpAddress,
	Decimal,
	Hex,
	Binary,
	Bitmap,
	ColMap2Bpp_CPC,	// Amstrad CPC mode 1 format
	ColMap4Bpp_CPC,	// Amstrad CPC mode 0 format
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

// Structures

struct FAddressRef
{
	FAddressRef() :BankId(-1), Address(0) {}
	FAddressRef(int16_t bankId, uint16_t address) :BankId(bankId), Address(address) {}

	bool IsValid() const { return BankId != -1; }
	void SetInvalid() { BankId = -1; }
	bool operator<(const FAddressRef& other) const { return Val < other.Val; }
	bool operator==(const FAddressRef& other) const { return Val == other.Val; }
	bool operator!=(const FAddressRef& other) const { return Val != other.Val; }
	FAddressRef operator++(int) { Address++; return *this; }
	union
	{
		struct
		{
			int16_t		BankId;
			uint16_t	Address;
		};
		uint32_t	Val;
	};
};

struct FFoundString
{
	FAddressRef		Address;
	std::string		String;
};

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


struct FItem
{
	EItemType		Type = EItemType::Unknown;
	std::string		Comment;
	uint16_t		ByteSize = 0;
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
	int				ExecutionCount = 0;

	union
	{
		struct
		{
			bool			bDisabled : 1;
			bool			bSelfModifyingCode : 1;
			bool			bUnused : 1;
			bool			bIsCall : 1;
		};
		uint32_t	Flags = 0;
	};

	bool	bNOPped = false;
	uint8_t	OpcodeBkp[4] = { 0 };
private:
	FCodeInfo() :FItem() { Type = EItemType::Code; }
	~FCodeInfo() = default;

	static std::vector<FCodeInfo*>	AllocatedList;
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
		DisplayType = EDataItemDisplayType::Unknown;
		Comment.clear();
		ReadCount = 0;
		LastFrameRead = -1;
		Reads.Reset();
		WriteCount = 0;
		LastFrameWritten = -1;
		Writes.Reset();
	}

	EDataType	DataType = EDataType::Byte;
	EDataItemDisplayType	DisplayType = EDataItemDisplayType::Unknown;

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
		FAddressRef	CharSetAddress;	// address of character set
		FAddressRef	GraphicsSetRef;	// for bitmap data
		FAddressRef	InstructionAddress;	// for operand data types
	};
	uint8_t		EmptyCharNo = 0;

	// Reads
	int						ReadCount = 0;
	int						LastFrameRead = -1;
	FItemReferenceTracker	Reads;	// address and counts of data access instructions

	// Writes
	int						WriteCount = 0;
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
