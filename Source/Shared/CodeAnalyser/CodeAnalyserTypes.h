#pragma once

#include <cstdint>
#include <string>
#include <cstring>
#include <vector>
#include <unordered_map>

// forward declarations
class FDisplayTypeBase;

// Enums

// CPU abstraction
enum class ECPUType
{
	Unknown,
	Z80,
	M6502,
	HuC6280,
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
	FunctionDescLine,

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
	Binary,
	UnsignedNumber,
	SignedNumber,
	Enum,
	Flags,
	Struct,
	Ascii,
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
	Bitmap,			// 1 bit per pixel
	ColMap2Bpp_CPC,	// Amstrad CPC mode 1 format
	ColMap4Bpp_CPC,	// Amstrad CPC mode 0 format
	ColMapMulticolour_C64,	// C64 Multicolour format
	UnsignedNumber,
	SignedNumber,
	Ascii,
	XPos,
	YPos,
	XCharPos,
	YCharPos,
};

// NOTE: only add to this enum at the end - there are loose dependencies on it (file format, combo box)
enum class EBitmapFormat
{
	Bitmap_1Bpp,		// 1 bit per pixel
	
	ColMap2Bpp_CPC,	// Amstrad CPC mode 1 format
	ColMap4Bpp_CPC,	// Amstrad CPC mode 0 format

	ColMapMulticolour_C64,	// C64 Multicolour format

	Max,
	None = Max
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
	Struct,		// structure
	Flags,		// flags

	Max,
	None = Max
};

// don't change order! loose references!
enum class EFunctionSortMode : int
{
	Location = 0,
	Alphabetical,
	CallFrequency,
	NoReferences
};

// don't change order! loose references!
enum class EDataSortMode : int
{
	Location = 0,
	Alphabetical,
	NoReferences
};

enum class EGlobalSortMode : int
{
	Location = 0,
	Alphabetical,
	CallFrequency,
	NoReferences
};

// Entries for the data type filter drop-down
enum class EDataTypeFilter : int
{
	All,
	Pointer,
	Text,
	Bitmap,
	CharacterMap,
	ColAttr,
};

enum class EBankAccess
{
	None = 0x00,
	Read = 0x01,
	Write = 0x02,
	ReadWrite = 0x03,

};

enum class EInstructionType
{
	AddToSelf,
	LoopBack,
	PortInput,
	PortOutput,
	EnableInterrupts,
	DisableInterrupts,
	ChangeInterruptMode,
	JumpToPointer,
	Halt,
	SetStackPointer,
	FunctionCall,

	Unknown,
};

// Structures

// This holds a reference to a memory address in a bank based memory architecture
struct FAddressRef
{
	FAddressRef() :BankId(-1), Address(0) {}
	FAddressRef(uint32_t val) :Val(val) {}
	FAddressRef(int16_t bankId, uint16_t address) :BankId(bankId), Address(address) {}

	bool IsValid() const { return BankId != -1; }
	void SetInvalid() { BankId = -1; }
	bool operator<(const FAddressRef& other) const { return CompVal() < other.CompVal(); }
	bool operator<=(const FAddressRef& other) const { return CompVal() <= other.CompVal(); }
	bool operator>=(const FAddressRef & other) const { return CompVal() >= other.CompVal(); }
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

private:
	// this is for the comparison operator overloads - we can't use Val because we'd have to re-order the union
	// too much code relies on the order of BankId and Address, e.g. load/save json
	uint32_t CompVal() const { return (BankId << 16) | Address; }

};


// Hash function for FAddresRef so we can use unordered sets/maps
template <>
struct std::hash<FAddressRef>
{
	std::size_t operator()(const FAddressRef& k) const
	{
		return k.Val;
	}
};

struct FFoundString
{
	FAddressRef		Address;
	std::string		String;
};

class FItemReferenceTracker
{
public:
	void Reset() 
	{
		References.clear(); 
		WriteCounter =0;	
	}

	bool	HasReferenceTo(FAddressRef addrRef)
	{
		const auto size = References.size();
		for (int i = 0; i < size; i++)
		{
			if (References[i] == addrRef)
				return true;
		}

		return false;
	}

	void	RegisterAccess(const FAddressRef& addrRef)
	{
		if(HasReferenceTo(addrRef))	// already has reference
			return;

		if(WriteCounter < MaxEntryCount)
			References.emplace_back(addrRef);
		else
			References[WriteCounter % MaxEntryCount] = addrRef;

		WriteCounter++;
	}

	bool RemoveReference(const FAddressRef& addrRef)
	{
		for(auto it = References.begin();it!=References.end();++it)
		{
			if(*it == addrRef)
			{
				References.erase(it);
				return true;
			}
		}

		return false;
	}
	bool IsEmpty() const { return References.empty(); }
	int NumReferences() const { return (int)References.size(); }
	const std::vector<FAddressRef>& GetReferences() const { return References; }
	std::vector<FAddressRef>& GetReferences() { return References; }
private:
	int		MaxEntryCount = 32;
	int		WriteCounter = 0;
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
	static FLabelInfo* Duplicate(const FLabelInfo* pSourceLabel);
	static void FreeAll();

	bool EnsureUniqueName(FAddressRef addr)
	{
		if(Global == false)	// local labels don't need to be unique
			return false;

		auto labelIt = GlobalLabelAddress.find(Name);
		if (labelIt == GlobalLabelAddress.end())
		{
			GlobalLabelAddress[Name] = addr;
			return false;
		}

		if (labelIt->second != addr)
		{
			char postFix[32];
			snprintf(postFix, 32, "_%X", addr.Address);
			Name += std::string(postFix);
			GlobalLabelAddress[Name] = addr;
		}	

		return true;
	}

	void SanitizeName(void)
	{
		for (int i = 0; i < Name.size(); i++)
		{
			const char ch = Name[i];
			if(ch == ' ')
				Name[i] = '_';
		}
	}

	static bool RemoveLabelName(const std::string& labelName)
	{
		auto labelIt = GlobalLabelAddress.find(labelName);
		//assert(labelIt != LabelUsage.end());	// shouldn't happen - it does though - investigate
		if (labelIt == GlobalLabelAddress.end())
			return false;

		GlobalLabelAddress.erase(labelIt);
		return true;
	}

	static void	ResetLabelNames() { GlobalLabelAddress.clear(); }


	void			InitialiseName(const char* pNewName) { Name = pNewName; }
	void			ChangeName(const char* pNewName, FAddressRef addr) 
	{
		if (strlen(pNewName) == 0)	// don't let a label be empty
			return;

		if(Global)
			RemoveLabelName(Name);
		Name = pNewName;
		EnsureUniqueName(addr);
		Edited = true;
	}
	const char*		GetName() const {return Name.c_str(); }

	bool					Global = false;
	bool					Edited = false;	// has the name been changed since generation?
	ELabelType				LabelType = ELabelType::Data;
	FItemReferenceTracker	References;
	uint16_t				MemoryRange = 1;	// range for this label in bytes
	//std::map<uint16_t, int>	References;
private:
	FLabelInfo() { Type = EItemType::Label; }
	~FLabelInfo() = default;

	std::string				Name;

	static std::vector<FLabelInfo*>	AllocatedList;
	//static std::unordered_map<std::string, int>	LabelUsage;
	static std::unordered_map<std::string, FAddressRef>	GlobalLabelAddress;

};

struct FCodeInfo : FItem
{
	static FCodeInfo* Allocate();
	static void FreeAll();

	EOperandType	OperandType = EOperandType::Unknown;
	int				StructId = -1;
	std::string		Text;				// Disassembly text
	FAddressRef		OperandAddress;	// optional operand address
	int				FrameLastExecuted = -1;
	int				LastExecuted = -1;
	int				ExecutionCount = 0;

	union
	{
		struct
		{
			bool			bDisabled : 1;
			bool			bSelfModifyingCode : 1;
			bool			bUnused : 1;
			bool			bIsCall : 1;
			bool			bHasLuaHandler : 1;
			bool			bHasBreakpoint : 1;
		};
		uint32_t	Flags = 0;
	};

	bool	bNOPped = false;
	uint8_t	OpcodeBkp[4] = { 0 };

	FItemReferenceTracker	Reads;	// addresses read by this instruction
	FItemReferenceTracker	Writes;	// addresses written to by this function

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
		pDisplayType = nullptr;
		Comment.clear();
		ReadCount = 0;
		LastFrameRead = -1;
		Reads.Reset();
		WriteCount = 0;
		LastFrameWritten = -1;
		Writes.Reset();
	}

	bool IsUninitialised() const
	{
		if (DataType == EDataType::Byte && DisplayType == EDataItemDisplayType::Unknown)
			return true;

		return false;
	}

	EDataType				DataType = EDataType::Byte;
	EDataItemDisplayType	DisplayType = EDataItemDisplayType::Unknown;
	FDisplayTypeBase*		pDisplayType = nullptr;	// display type for this data item

	// Flags
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
			bool			bStructMember : 1;	// is item a member of a structure
			bool			bHasBreakpoint : 1;	// data breakpoint
		};
		uint32_t	Flags = 0;
	};

	// Address references
	union
	{
		FAddressRef	CharSetAddress = FAddressRef();	// address of character set
		FAddressRef	GraphicsSetRef;	// for bitmap data
		FAddressRef	InstructionAddress;	// for operand data types
	};

	union 
	{
		uint8_t		EmptyCharNo = 0;
		uint8_t		StructByteOffset;
	};

	// sub type references
	union 
	{
		int			PaletteNo = -1;
		int			SubTypeId;
	};

	// Reads
	int						ReadCount = 0;
	int						LastFrameRead = -1;
	int						LastRead = -1;
	FItemReferenceTracker	Reads;	// address and counts of data access instructions

	// Writes
	int						WriteCount = 0;
	int						LastFrameWritten = -1;
	int						LastWritten = -1;
	FItemReferenceTracker	Writes;	// address and counts of data access instructions
	FAddressRef				LastWriter;
};

struct FCommentBlock : FItem
{
	static FCommentBlock* Allocate();
	static FCommentBlock* Duplicate(const FCommentBlock* pSourceCommentBlock);
	static void FreeAll();

private:
	FCommentBlock() : FItem() { Type = EItemType::CommentBlock; }
	~FCommentBlock() = default;
	static std::vector<FCommentBlock*>	AllocatedList;
};

struct FCommentLine : FItem
{
	class FAllocator
	{
		public:
			FCommentLine* Allocate()
			{
				if (FreeList.size() == 0)
					FreeList.push_back(new FCommentLine);

				FCommentLine* pLine = FreeList.back();
				AllocatedList.push_back(pLine);
				FreeList.pop_back();

				return pLine;
			}
			void FreeAll()
			{
				for (auto it : AllocatedList)
					FreeList.push_back(it);

				AllocatedList.clear();
			}
		private:
			std::vector<FCommentLine*>	AllocatedList;
			std::vector<FCommentLine*>	FreeList;
	};

private:
	FCommentLine() : FItem() { Type = EItemType::CommentLine; }
	~FCommentLine() = default;

};

struct FFunctionDescLine : FItem
{
	class FAllocator
	{
	public:
		FFunctionDescLine* Allocate()
		{
			if (FreeList.size() == 0)
				FreeList.push_back(new FFunctionDescLine);

			FFunctionDescLine* pLine = FreeList.back();
			AllocatedList.push_back(pLine);
			FreeList.pop_back();

			return pLine;
		}
		void FreeAll()
		{
			for (auto it : AllocatedList)
				FreeList.push_back(it);

			AllocatedList.clear();
		}
	private:
		std::vector<FFunctionDescLine*>	AllocatedList;
		std::vector<FFunctionDescLine*>	FreeList;
	};
private:
	FFunctionDescLine() : FItem() { Type = EItemType::FunctionDescLine; }
	~FFunctionDescLine() = default;
};

struct FCodeAnalysisItem
{
	FCodeAnalysisItem() {}
	FCodeAnalysisItem(FItem* pItem, int16_t bankId, uint16_t addr) :Item(pItem), AddressRef(bankId, addr) {}
	FCodeAnalysisItem(FItem* pItem, FAddressRef addr) :Item(pItem), AddressRef(addr) {}

	bool IsValid() const { return Item != nullptr; }

	FItem* Item = nullptr;
	FAddressRef	AddressRef;
};


struct FCPUFunctionCall
{
	FAddressRef		FunctionAddr;
	FAddressRef		CallAddr;
	FAddressRef		ReturnAddr;

	bool operator==(const FCPUFunctionCall& other) const
	{
		return FunctionAddr == other.FunctionAddr && CallAddr == other.CallAddr && ReturnAddr == other.ReturnAddr;
	}
};