#pragma once
#include <cstdint>
#include <string>
#include <map>
#include <unordered_set>
#include <vector>
#include <algorithm>
#include <assert.h>

#include "CodeAnalyserTypes.h"
#include "CodeAnalysisPage.h"
#include "Debugger.h"
#include "IOAnalyser.h"
#include <Misc/GlobalConfig.h>
#include "Commands/FormatDataCommand.h"
#include "Debug/DebugLog.h"

// forward dec
class FGraphicsView;
class FCodeAnalysisState;
class FEmuBase;
class FDataTypes;
class FFunctionInfoCollection;
class FDataRegionList;
class FMemoryAnalyser;

enum class ELabelType;

struct FCodeAnalysisBank;
extern FCodeAnalysisBank Banks[];

/* the input callback type */
typedef uint8_t(*FDasmInput)(void* user_data);
/* the output callback type */
typedef void (*FDasmOutput)(char c, void* user_data);

class ICPUEmulator
{
public:
	virtual void* GetImpl() const = 0;
};

class ICPUEmulator6502 : public ICPUEmulator
{
public:
	static const uint8_t kFlagCarry		= 1 << 0;
	static const uint8_t kFlagZero		= 1 << 1;
	static const uint8_t kFlagInterrupt = 1 << 2;
	static const uint8_t kFlagDecimal	= 1 << 3;
	static const uint8_t kFlagBreak		= 1 << 4;
	static const uint8_t kFlagTransfer	= 1 << 5;
	static const uint8_t kFlagOverflow	= 1 << 6;
	static const uint8_t kFlagNegative	= 1 << 7;

	virtual uint16_t GetPC() const = 0;
	virtual uint8_t GetA() const = 0;
	virtual uint8_t GetX() const = 0;
	virtual uint8_t GetY() const = 0;
	virtual uint8_t GetS() const = 0;
	virtual uint8_t GetP() const = 0;
};

class ICPUEmulatorZ80 : public ICPUEmulator
{
public:
	//virtual uint8_t GetA() const = 0;
	//virtual uint8_t GetB() const = 0;
	//virtual uint8_t GetC() const = 0;
	// etc
};

class ICPUInterface
{
public:
	// Memory Access
	virtual uint8_t		ReadByte(uint16_t address) const = 0;
	virtual uint16_t	ReadWord(uint16_t address) const = 0;
	virtual const uint8_t*	GetMemPtr(uint16_t address) const = 0;
	virtual void		WriteByte(uint16_t address, uint8_t value) = 0;

	virtual FAddressRef	GetPC(void) = 0;
	virtual uint16_t	GetSP(void) = 0;

    //virtual void	GraphicsViewerSetView(FAddressRef address) {};

	virtual ICPUEmulator*	GetCPUEmulator(void) const { return nullptr; }	// get pointer to emulator - a bit of a hack

	ECPUType	CPUType = ECPUType::Unknown;
};

struct FMemoryAccess
{
	FAddressRef	Address;
	uint8_t		Value;
	FAddressRef	PC;
};

enum class EKey
{
	SetItemData,
	SetItemText,
	SetItemCode,
	SetItemImage,
	SetItemBinary,
	SetItemPointer,
	SetItemJumpAddress,
	SetItemNumber,
	SetItemAscii,
	SetItemUnknown,

	AddLabel,
	Rename,
	Comment,
	CommentLegacy,

	GoToAddress,

	// Debugger
	BreakContinue,
	StepOver,
	StepInto,
	StepFrame,
	StepScreenWrite,
	Breakpoint,

	Count
};









struct FAddressCoord
{
	FAddressRef		Address;
	float			YPos;
};


// view state for code analysis window
struct FCodeAnalysisViewState
{
	void Reset()
	{
		CursorItem = FCodeAnalysisItem();
		ViewingBankId = -1;
	}
	// accessor functions
	const FCodeAnalysisItem& GetCursorItem() const { return CursorItem; }
	void SetCursorItem(const FCodeAnalysisItem& item)
	{
		CursorItem = item;
	}

	FAddressRef& GetGotoAddress() { return GoToAddressRef; }
	const FAddressRef& GetGotoAddress() const { return GoToAddressRef; }
	void GoToAddress(FAddressRef address, bool bLabel = false);
	bool GoToPreviousAddress();
	bool GoToNextAddress();
	void BookmarkAddress(int bookmarkNo, FAddressRef address)
	{
		if(bookmarkNo>=0 && bookmarkNo < kNoBookmarks)
			Bookmarks[bookmarkNo] = address;
	}

	void GoToBookmarkAddress(int bookmarkNo)
	{
		if (bookmarkNo >= 0 && bookmarkNo < kNoBookmarks)
			GoToAddress(Bookmarks[bookmarkNo]);
	}


	bool GetYPosForAddress(FAddressRef addr, float& ypos)
	{
		for (const auto& item : AddressCoords)
		{
			if (item.Address == addr)
			{
				ypos = item.YPos;
				return true;
			}
		}

		return false;
	}
	
	void FixupAddressRefs(const FCodeAnalysisState& state);
	
	bool			Enabled = false;
	bool			TrackPCFrame = false;
	FAddressRef		HoverAddress;			// address being hovered over
	FAddressRef		HighlightAddress;		// address to highlight
	int				HighlightScanline = -1;	// scanline to highlight
	bool			GoToLabel = false;
	uint16_t		JumpAddress = 0;
	int16_t			ViewingBankId = -1;
	const FLabelInfo*		pLabelScope = nullptr;
	FItem*		pKeyboardFocusItem = nullptr; // item to set keyboard focus on

	
	std::vector< FAddressCoord>		AddressCoords;
	int								JumpLineIndent;

	// find
	std::string					FindText;
	std::vector<FAddressRef>	FindResults;
	bool						SearchROM = false;

	// formatting
	bool						DataFormattingTabOpen = false;
	FDataFormattingOptions		DataFormattingOptions;
	FBatchDataFormattingOptions	BatchFormattingOptions;

	EBitmapFormat CurBitmapFormat = EBitmapFormat::None;

private:
	FCodeAnalysisItem			CursorItem;
	FAddressRef					GoToAddressRef;
	std::vector<FAddressRef>	PreviousAddressStack;
	std::vector<FAddressRef>	NextAddressStack;
	static const int			kNoBookmarks = 5;
	FAddressRef					Bookmarks[kNoBookmarks];
};

struct FCodeAnalysisConfig
{
	//bool				bShowOpcodeValues = false;
	bool				bShowBanks = false;
	//int					BranchLinesDisplayMode = 1;
	const uint32_t*		CharacterColourLUT = nullptr;

	bool				bSupportedBitmapTypes[(int)EBitmapFormat::Max] = { false };

	// horizontal positions
	bool	bShowConfigWindow = false;
	float	LabelPos = 42.0f;
	float	CommentLinePos = 42.0f;
	float	InlineCommentPos = 0.0f;
	float	AddressPos = 48.0f;
	float	AddressSpaceRatio = 6.0f;
	//float	BranchLineIndentStart = 0;
	float	BranchSpacing = 4.0f;
	int		BranchMaxIndent = 8;
	int		BranchLinesPerIndent = 5;
};

struct FCodeAnalysisBank
{
	int16_t				Id = -1;
	int					NoPages = 0;
	uint32_t			SizeMask = 0;
	//std::vector<int>	MappedPages;	// banks can be mapped to multiple pages
	std::unordered_set<int>	MappedReadPages;
	std::unordered_set<int>	MappedWritePages;
	int					PrimaryMappedPage = -1;	// the page this bank is normally mapped to
	uint8_t*			Memory = nullptr;	// pointer to memory bank occupies
	FCodeAnalysisPage*	Pages = nullptr;
	std::string			Name;
	std::string			Description;	// where we can describe what the bank is used for
	//bool				bReadOnly = false;
	bool				bMachineROM = false;
	bool				bFixed = false;	// bank is never remapped
	bool				bIsDirty = false;
	bool				bEverBeenMapped = false;
	bool				bHidden = false;
	std::vector<FCodeAnalysisItem>		ItemList;

	FCommentLine::FAllocator	CommentLineAllocator;
	FFunctionDescLine::FAllocator	FunctionDescLineAllocator;

	EBankAccess			Mapping = EBankAccess::None;

	void UpdateMapping()
	{
		int mapping = 0;
		if(MappedReadPages.empty() == false)
			mapping |= 1;
		if (MappedWritePages.empty() == false)
			mapping |= 2;

		Mapping = (EBankAccess)mapping;
	}
	void MapToPage(int startPageNo, EBankAccess access)
	{
		if((int)access & 1)
			MappedReadPages.insert(startPageNo);
		if ((int)access & 2)
			MappedWritePages.insert(startPageNo);

		bEverBeenMapped = true;
		UpdateMapping();
	}
	void UnmapFromPage(int startPageNo, EBankAccess access)
	{
		if ((int)access & 1)
			MappedReadPages.erase(startPageNo);
		if ((int)access & 2)
			MappedWritePages.erase(startPageNo);
		UpdateMapping();
	}

	bool		AddressValid(uint16_t addr) const { return addr >= GetMappedAddress() && addr < GetMappedAddress() + (NoPages * FCodeAnalysisPage::kPageSize);	}
	bool		IsUsed() const { return Pages[0].bUsed; }
	bool		IsMapped() const { return Mapping!= EBankAccess::None; }
	EBankAccess	GetBankMapping() const { return Mapping;}
	uint16_t	GetMappedAddress() const { return PrimaryMappedPage * FCodeAnalysisPage::kPageSize; }
	uint16_t	GetSizeBytes() const { return NoPages * FCodeAnalysisPage::kPageSize; }
};

#ifdef NDEBUG
#define CHECK_BANK_ADDR_VALID(addrRef, bankAddr, pBank) 
#else
void LogInvalidAddressRefForBank(const FCodeAnalysisBank* pBank, FAddressRef addrRef);
#define CHECK_BANK_ADDR_VALID(addrRef, bankAddr, pBank) if (bankAddr >= pBank->NoPages * FCodeAnalysisPage::kPageSize) LogInvalidAddressRefForBank(pBank, addrRef); 
#endif // NDEBUG

// code analysis information
class FCodeAnalysisState
{
public:
	// constants
	static const int kAddressSize = 1 << 16;
	static const int kPageShift = 10;
	static const int kPageMask = 1023;
	static const int kNoPagesInAddressSpace = kAddressSize / FCodeAnalysisPage::kPageSize;

	static const int kMaxBanks = 1024;
	static int16_t BankCount;

	FCodeAnalysisState();
	void	Init(FEmuBase* pEmu);
	void	OnFrameStart();
	void	OnFrameEnd();
	void	OnMachineFrameStart();
	void	OnMachineFrameEnd();
	void	OnCPUTick(uint64_t pins);

	const FEmuBase* GetEmulator() const { return pEmulator; }
	FEmuBase* GetEmulator() { return pEmulator; }
    
    FDataTypes* GetDataTypes() {return pDataTypes;}

	const ICPUInterface* GetCPUInterface() const { return CPUInterface; }

	ICPUInterface* CPUInterface = nullptr;	// Make private
	int			CurrentFrameNo = 0;
	int			ExecutionCounter = 0;

	void	SetGlobalConfig(FGlobalConfig *pConfig) { pGlobalConfig = pConfig; }

	// Memory Banks & Pages
	int16_t		GetNextBankId() const { return FCodeAnalysisState::BankCount++; }
	int16_t		CreateBank(const char* name, int noKb, uint8_t* pMemory, bool bReadOnly, uint16_t initialAddress, bool bFixed = false);
	bool		FreeBanksFrom(int16_t bankId);
	bool		SetBankPrimaryPage(int16_t bankId, int startPageNo);
	bool		MapBank(int16_t bankId, int startPageNo, EBankAccess access = EBankAccess::ReadWrite);
	// sam. reinstated this.
	bool		UnMapBank(int16_t bankId, int startPageNo, EBankAccess access);

	bool		IsBankIdMapped(int16_t bankId) const;
	bool		IsAddressValid(FAddressRef addr) const;

	bool		MapBankForAnalysis(FCodeAnalysisBank& bank);
	void		UnMapAnalysisBanks();

	bool		IsAddressBreakpointed(FAddressRef addr) const;
	bool		ToggleExecBreakpointAtAddress(FAddressRef addr);
	bool		ToggleDataBreakpointAtAddress(FAddressRef addr, uint16_t dataSize);

	FCodeAnalysisBank* GetBank(int16_t bankId) { return (bankId >= 0 && bankId < FCodeAnalysisState::BankCount) ? &Banks[bankId] : nullptr; }
	const FCodeAnalysisBank* GetBank(int16_t bankId) const { return (bankId >= 0 && bankId < FCodeAnalysisState::BankCount) ? &Banks[bankId] : nullptr;	}
	int16_t		GetBankFromAddress(uint16_t address) const { return MappedReadBanks[address >> kPageShift]; }
	int16_t		GetReadBankFromAddress(uint16_t address) const { return MappedReadBanks[address >> kPageShift]; }
	int16_t		GetWriteBankFromAddress(uint16_t address) const { return MappedWriteBanks[address >> kPageShift]; }
	const FCodeAnalysisBank* GetBanks() const { return Banks; }
	FCodeAnalysisBank* GetBanks() { return Banks; }

	FAddressRef	AddressRefFromPhysicalAddress(uint16_t physAddr) const { return FAddressRef(GetBankFromAddress(physAddr), physAddr); }
	FAddressRef	AddressRefFromPhysicalReadAddress(uint16_t physAddr) const { return FAddressRef(GetReadBankFromAddress(physAddr), physAddr); }
	FAddressRef	AddressRefFromPhysicalWriteAddress(uint16_t physAddr) const { return FAddressRef(GetWriteBankFromAddress(physAddr), physAddr); }

	uint8_t		ReadByte(uint16_t address) const
	{
		if (MappedMem[address >> kPageShift] == nullptr)
			return CPUInterface->ReadByte(address);
		else
			return *(MappedMem[(address >> kPageShift)] + (address & kPageMask));
	}

	uint8_t		ReadByte(FAddressRef address) const
	{
		const FCodeAnalysisBank* pBank = GetBank(address.GetBankId());
		assert(pBank != nullptr);
		// this is silly
		return pBank->Memory[address.GetAddress() - pBank->GetMappedAddress()];
	}

	uint16_t	ReadWord(uint16_t address) const
	{
		if (MappedMem[address >> kPageShift] == nullptr)
			return CPUInterface->ReadWord(address);
		else
			return *(uint16_t*)(MappedMem[address >> kPageShift] + (address & kPageMask));
	}

	uint16_t		ReadWord(FAddressRef address) const
	{
		const FCodeAnalysisBank* pBank = GetBank(address.GetBankId());
		assert(pBank != nullptr);
		return *(uint16_t*)(&pBank->Memory[address.GetAddress() - pBank->GetMappedAddress()]);
	}

	void		WriteByte(uint16_t address, uint8_t value) 
	{ 
		if (MappedMem[address >> kPageShift] == nullptr)
			CPUInterface->WriteByte(address, value);
		else
			*(MappedMem[(address >> kPageShift)] + (address & kPageMask)) = value;
	}
	
	int GetNoPages() const { return (int)RegisteredPages.size();}
	bool IsValidPageId(int16_t id) const { return id >=0 && id < RegisteredPages.size(); }
	FCodeAnalysisPage* GetPage(int16_t id) { return RegisteredPages[id]; }

	// Advance an address ref by a number of bytes, may go to next bank in physical memory
	bool AdvanceAddressRef(FAddressRef& addressRef, int amount = 1) const
	{
		if(addressRef.IsValid() == false)
			return false;

		const FCodeAnalysisBank* pBank = GetBank(addressRef.GetBankId());
		if (addressRef.GetAddress() + amount < pBank->GetMappedAddress() + pBank->GetSizeBytes())
		{
			const uint16_t addr = addressRef.GetAddress();
			addressRef.SetAddress(addr + amount);
			return true;
		}
		else
		{
			// we might come of the end here, find the bank that's physically mapped after?
			addressRef = AddressRefFromPhysicalAddress(addressRef.GetAddress() + amount);
		}
		return addressRef.IsValid();
	}

	void	SetCodeAnalysisDirty(FAddressRef addrRef)
	{
		FCodeAnalysisBank* pBank = GetBank(addrRef.GetBankId());
		if (pBank != nullptr)
			pBank->bIsDirty = true;
		bCodeAnalysisDataDirty = true;
	}

	void	SetCodeAnalysisDirty(uint16_t address)	
	{ 
		SetCodeAnalysisDirty({ GetBankFromAddress(address),address });
	}

	void	SetAddressRangeDirty()
	{
		for (int i = 0; i < kNoPagesInAddressSpace; i++)
		{
			FCodeAnalysisBank* pReadBank = GetBank(MappedReadBanks[i]);
			if (pReadBank != nullptr)
				pReadBank->bIsDirty = true;
			FCodeAnalysisBank* pWriteBank = GetBank(MappedWriteBanks[i]);
			if (pWriteBank != nullptr)
				pWriteBank->bIsDirty = true;
			bCodeAnalysisDataDirty = true;
		}
	}

	void	SetAllBanksDirty()
	{
		for (int b = 0; b < FCodeAnalysisState::BankCount; b++)
		{
			Banks[b].bIsDirty;
		}
		bCodeAnalysisDataDirty = true;
	}

	void	ClearDirtyStatus(void)
	{
		bCodeAnalysisDataDirty = false;
	}
	
	bool IsCodeAnalysisDataDirty() const { return bCodeAnalysisDataDirty; }
	void ClearRemappings() { bMemoryRemapped = false; }
	bool HasMemoryBeenRemapped() const { return bMemoryRemapped; }

	void FixupAddressRefs();
	void FixupBankAddressRefs();
	void UpdateFocussedViewState();

public:

	bool					bRegisterDataAccesses = true;

	std::vector<FCodeAnalysisItem>	ItemList;

	std::vector<FCodeAnalysisItem>	GlobalDataItems;
	std::vector<FCodeAnalysisItem>	GlobalFunctions;

	bool						bTraceFunctionExecution = false;
	FFunctionInfoCollection*	pFunctions = nullptr;
	FDataRegionList*			pDataRegions = nullptr;

	static const int kNoViewStates = 4;
	FCodeAnalysisViewState	ViewState[kNoViewStates];	// new multiple view states
	int						FocussedWindowId = 0;
	FCodeAnalysisViewState& GetFocussedViewState() { return ViewState[FocussedWindowId]; }
	FCodeAnalysisViewState& GetAltViewState() { return ViewState[FocussedWindowId ^ 1]; }
	
	FDebugger				Debugger;
	FMemoryAnalyser	*		pMemoryAnalyser = nullptr;
	FIOAnalyser				IOAnalyser;

	FAddressRef				CopiedAddress;

	int				XPosHighlight = -1;
	int				YPosHighlight = -1;

	int				KeyConfig[(int)EKey::Count] = { 0 };

	std::vector< class FCommand *>	CommandStack;

	bool					bAllowEditing = false;

	FCodeAnalysisConfig		Config;
	FGlobalConfig*			pGlobalConfig = nullptr;
public:
	// Access functions for code analysis

	FCodeAnalysisPage* GetReadPage(uint16_t addr)  
	{
		const int pageNo = addr >> kPageShift;
		FCodeAnalysisPage* pPage = ReadPageTable[pageNo];
		if (pPage == nullptr)
		{
			LOGERROR("Read page %d NOT mapped", pageNo);
		}

		return pPage;
	}
	
	const FCodeAnalysisPage* GetReadPage(uint16_t addr) const { return ((FCodeAnalysisState *)this)->GetReadPage(addr); }
	FCodeAnalysisPage* GetWritePage(uint16_t addr) 
	{
		const int pageNo = addr >> kPageShift;
		FCodeAnalysisPage* pPage = WritePageTable[pageNo];
		if (pPage == nullptr)
		{
			LOGERROR("Write page %d NOT mapped", pageNo);
		}

		return pPage;
	}
	const FCodeAnalysisPage* GetWritePage(uint16_t addr) const { return ((FCodeAnalysisState*)this)->GetWritePage(addr); }

	const FLabelInfo* GetLabelForPhysicalAddress(uint16_t addr) const { return GetReadPage(addr)->Labels[addr & kPageMask]; }
	const FLabelInfo* GetScopeLabelForPhysicalAddress(uint16_t addr) const { return GetReadPage(addr)->ScopeLabel[addr & kPageMask]; }
	FLabelInfo* GetLabelForPhysicalAddress(uint16_t addr) { return GetReadPage(addr)->Labels[addr & kPageMask]; }
	FLabelInfo* GetLabelForAddress(FAddressRef addrRef)
	{
		const FCodeAnalysisBank* pBank = GetBank(addrRef.GetBankId());
		if (pBank != nullptr)
		{
			const uint16_t bankAddr = addrRef.GetAddress() - (pBank->PrimaryMappedPage * FCodeAnalysisPage::kPageSize);
			CHECK_BANK_ADDR_VALID(addrRef, bankAddr, pBank);
			assert(bankAddr < pBank->NoPages * FCodeAnalysisPage::kPageSize);	// This assert gets caused by banks being mapped into more than one location in physical memory
			return pBank->Pages[(bankAddr >> FCodeAnalysisPage::kPageShift) & pBank->SizeMask].Labels[bankAddr & FCodeAnalysisPage::kPageMask];
		}
		else
		{
			return nullptr;
		}
	}
	FLabelInfo* GetScopeForAddress(FAddressRef addrRef)
	{
		const FCodeAnalysisBank* pBank = GetBank(addrRef.GetBankId());
		if (pBank != nullptr)
		{
			const uint16_t bankAddr = addrRef.GetAddress() - (pBank->PrimaryMappedPage * FCodeAnalysisPage::kPageSize);
			CHECK_BANK_ADDR_VALID(addrRef, bankAddr, pBank);
			assert(bankAddr < pBank->NoPages * FCodeAnalysisPage::kPageSize);	// This assert gets caused by banks being mapped into more than one location in physical memory
			return pBank->Pages[(bankAddr >> FCodeAnalysisPage::kPageShift) & pBank->SizeMask].ScopeLabel[bankAddr & FCodeAnalysisPage::kPageMask];
		}
		else
		{
			return nullptr;
		}
	}
	void SetLabelForPhysicalAddress(uint16_t addr, FLabelInfo* pLabel)
	{
		if(pLabel != nullptr)	// ensure no name clashes
			pLabel->EnsureUniqueName(AddressRefFromPhysicalAddress(addr));
		GetReadPage(addr)->Labels[addr & kPageMask] = pLabel; 
	}
	bool SetLabelForAddress(FAddressRef addrRef, FLabelInfo* pLabel);


	//FCommentBlock* GetCommentBlockForAddress(uint16_t addr) const { return GetReadPage(addr)->CommentBlocks[addr & kPageMask]; }
	FCommentBlock* GetCommentBlockForAddress(FAddressRef addrRef)
	{
		const FCodeAnalysisBank* pBank = GetBank(addrRef.GetBankId());
		if (pBank != nullptr)
		{
			const uint16_t bankAddr = addrRef.GetAddress() - (pBank->PrimaryMappedPage * FCodeAnalysisPage::kPageSize);
			CHECK_BANK_ADDR_VALID(addrRef, bankAddr, pBank);
			assert(bankAddr < pBank->NoPages * FCodeAnalysisPage::kPageSize);	// This assert gets caused by banks being mapped into more than one location in physical memory
			return pBank->Pages[(bankAddr >> FCodeAnalysisPage::kPageShift) & pBank->SizeMask].CommentBlocks[bankAddr & FCodeAnalysisPage::kPageMask];
		}
		else
		{
			return nullptr;
		}
	}
	void SetCommentBlockForAddress(FAddressRef addrRef, FCommentBlock* pCommentBlock)
	{
		FCodeAnalysisBank* pBank = GetBank(addrRef.GetBankId());
		if (pBank != nullptr)
		{
			const uint16_t bankAddr = addrRef.GetAddress() - (pBank->PrimaryMappedPage * FCodeAnalysisPage::kPageSize);
			CHECK_BANK_ADDR_VALID(addrRef, bankAddr, pBank);
			assert(bankAddr < pBank->NoPages * FCodeAnalysisPage::kPageSize);	// This assert gets caused by banks being mapped into more than one location in physical memory
			pBank->Pages[(bankAddr >> FCodeAnalysisPage::kPageShift) & pBank->SizeMask].CommentBlocks[bankAddr & FCodeAnalysisPage::kPageMask] = pCommentBlock;
		}
		//GetReadPage(addr)->CommentBlocks[addr & kPageMask] = pCommentBlock;
	}

	const FCodeInfo* GetCodeInfoForPhysicalAddress(uint16_t addr) const { return GetReadPage(addr)->CodeInfo[addr & kPageMask]; }
	FCodeInfo* GetCodeInfoForPhysicalAddress(uint16_t addr) { return GetReadPage(addr)->CodeInfo[addr & kPageMask]; }
	FCodeInfo* GetCodeInfoForAddress(FAddressRef addrRef)
	{
		const FCodeAnalysisBank* pBank = GetBank(addrRef.GetBankId());
		if (pBank != nullptr)
		{
			const uint16_t bankAddr = addrRef.GetAddress() - (pBank->PrimaryMappedPage * FCodeAnalysisPage::kPageSize);
			CHECK_BANK_ADDR_VALID(addrRef, bankAddr, pBank);
			assert(bankAddr < pBank->NoPages * FCodeAnalysisPage::kPageSize);	// This assert gets caused by banks being mapped into more than one location in physical memory
			return pBank->Pages[(bankAddr >> FCodeAnalysisPage::kPageShift) & pBank->SizeMask].CodeInfo[bankAddr & FCodeAnalysisPage::kPageMask];
		}
		else
		{
			return nullptr;
		}
	}

	void SetCodeInfoForAddress(uint16_t addr, FCodeInfo* pCodeInfo) { GetReadPage(addr)->CodeInfo[addr & kPageMask] = pCodeInfo; }
	void SetCodeInfoForAddress(FAddressRef addrRef, FCodeInfo* pCodeInfo)
	{ 
		FCodeAnalysisBank* pBank = GetBank(addrRef.GetBankId());
		if (pBank != nullptr)
		{
			const uint16_t bankAddr = addrRef.GetAddress() - (pBank->PrimaryMappedPage * FCodeAnalysisPage::kPageSize);
			CHECK_BANK_ADDR_VALID(addrRef, bankAddr, pBank);
			assert(bankAddr < pBank->NoPages * FCodeAnalysisPage::kPageSize);	// This assert gets caused by banks being mapped into more than one location in physical memory
			pBank->Pages[(bankAddr >> FCodeAnalysisPage::kPageShift) & pBank->SizeMask].CodeInfo[bankAddr & FCodeAnalysisPage::kPageMask] = pCodeInfo;
		}
	}

	FDataInfo* GetDataInfoForAddress(FAddressRef addrRef) const
	{
		const FCodeAnalysisBank* pBank = GetBank(addrRef.GetBankId());
		if (pBank != nullptr)
		{
			const uint16_t bankAddr = addrRef.GetAddress() - pBank->GetMappedAddress();
			CHECK_BANK_ADDR_VALID(addrRef, bankAddr, pBank);
			assert(bankAddr < pBank->NoPages * FCodeAnalysisPage::kPageSize);	// This assert gets caused by banks being mapped into more than one location in physical memory
			return &pBank->Pages[(bankAddr >> FCodeAnalysisPage::kPageShift) & pBank->SizeMask].DataInfo[bankAddr & FCodeAnalysisPage::kPageMask];
		}
		else
		{
			return nullptr;
		}
	}

	const FDataInfo* GetReadDataInfoForAddress(uint16_t addr) const { return &GetReadPage(addr)->DataInfo[addr & kPageMask]; }
	FDataInfo* GetReadDataInfoForAddress(uint16_t addr) { return &GetReadPage(addr)->DataInfo[addr & kPageMask]; }
	
	const FDataInfo* GetWriteDataInfoForAddress(uint16_t addr) const { return  &GetWritePage(addr)->DataInfo[addr & kPageMask]; }
	FDataInfo* GetWriteDataInfoForAddress(uint16_t addr) { return &GetWritePage(addr)->DataInfo[addr & kPageMask]; }
	
	FAddressRef GetLastWriterForAddress(uint16_t addr) const { return GetWritePage(addr)->DataInfo[addr & kPageMask].LastWriter; }
	void SetLastWriterForAddress(uint16_t addr, FAddressRef lastWriter) { GetWritePage(addr)->DataInfo[addr & kPageMask].LastWriter = lastWriter; }

	FMachineState* GetMachineState(uint16_t addr) { return GetReadPage(addr)->MachineState[addr & kPageMask];}
	void SetMachineStateForAddress(uint16_t addr, FMachineState* pMachineState) { GetReadPage(addr)->MachineState[addr & kPageMask] = pMachineState; }

	//FAddressRef FindMemoryPattern(uint8_t* pData, size_t dataSize);
	std::vector<FAddressRef> FindAllMemoryPatterns(const uint8_t* pData, size_t dataSize, bool bROM, bool bPhysicalOnly);
	std::vector<FFoundString> FindAllStrings(bool bROM, bool bPhysicalOnly);
	std::vector<FAddressRef> FindInAnalysis(const char* pString, bool bSearchROM);

	//bool FindMemoryPatternInPhysicalMemory(uint8_t* pData, size_t dataSize, uint16_t offset, uint16_t& outAddr);

	void FindAsciiStrings(uint16_t startAddress);


private:
	// private methods
	bool					RegisterPage(FCodeAnalysisPage* pPage, const char* pName)
	{
		if (pPage->PageId != -1)
			return false;
		pPage->PageId = (int16_t)RegisteredPages.size();
		RegisteredPages.push_back(pPage);
		PageNames.push_back(pName);
		return true;
	}
	const char* GetPageName(int16_t id) { return PageNames[id].c_str(); }
	int16_t					GetAddressReadPageId(uint16_t addr) { return GetReadPage(addr)->PageId; }
	int16_t					GetAddressWritePageId(uint16_t addr) { return GetWritePage(addr)->PageId; }
	const std::vector< FCodeAnalysisPage*>& GetRegisteredPages() const { return RegisteredPages; }


	void					SetCodeAnalysisReadPage(int pageNo, FCodeAnalysisPage* pPage) 
	{ 
		ReadPageTable[pageNo] = pPage; if (pPage != nullptr) 
		pPage->bUsed = true; 
	}
	void					SetCodeAnalysisWritePage(int pageNo, FCodeAnalysisPage* pPage) 
	{ 
			WritePageTable[pageNo] = pPage; if(pPage != nullptr) 
			pPage->bUsed = true; 
	}
	void					SetCodeAnalysisRWPage(int pageNo, FCodeAnalysisPage* pReadPage, FCodeAnalysisPage* pWritePage)
	{
		SetCodeAnalysisReadPage(pageNo, pReadPage);
		SetCodeAnalysisWritePage(pageNo, pWritePage);
	}

	// private data members
	FEmuBase*						pEmulator = nullptr;
    FDataTypes*                     pDataTypes = nullptr;

	FCodeAnalysisPage*				ReadPageTable[kNoPagesInAddressSpace];
	FCodeAnalysisPage*				WritePageTable[kNoPagesInAddressSpace];

//	std::vector<FCodeAnalysisBank>	Banks;

	int16_t							MappedReadBanks[kNoPagesInAddressSpace];	// banks mapped into address space
	int16_t							MappedWriteBanks[kNoPagesInAddressSpace];	// banks mapped into address space
	int16_t							MappedReadBanksBackup[kNoPagesInAddressSpace];	// banks mapped into address space
	int16_t							MappedWriteBanksBackup[kNoPagesInAddressSpace];	// banks mapped into address space

	uint8_t*						MappedMem[kNoPagesInAddressSpace];	// mapped analysis memory
				
	std::vector<FCodeAnalysisPage*>	RegisteredPages;
	std::vector<std::string>	PageNames;
	int32_t						NextPageId = 0;

	bool						bCodeAnalysisDataDirty = false;
	bool						bMemoryRemapped = true;

	FCodeAnalysisState(const FCodeAnalysisState&) = delete;                 // Prevent copy-construction
	FCodeAnalysisState& operator=(const FCodeAnalysisState&) = delete;      // Prevent assignment


};

// Analysis
FLabelInfo* GenerateLabelForAddress(FCodeAnalysisState &state, FAddressRef addrRef, ELabelType label);
void RunStaticCodeAnalysis(FCodeAnalysisState &state, uint16_t pc);
bool RegisterCodeExecuted(FCodeAnalysisState &state, uint16_t pc, uint16_t oldpc);
void RegisterCall(FCodeAnalysisState& state, const FCPUFunctionCall& callInfo);
void RegisterReturn(FCodeAnalysisState& state, FAddressRef returnAddress);
void ReAnalyseCode(FCodeAnalysisState &state);
uint16_t WriteCodeInfoForAddress(FCodeAnalysisState& state, uint16_t pc);
void GenerateGlobalInfo(FCodeAnalysisState &state);
void RegisterDataRead(FCodeAnalysisState& state, uint16_t pc, uint16_t dataAddr);
void RegisterDataWrite(FCodeAnalysisState &state, uint16_t pc, uint16_t dataAddr, uint8_t value);
void UpdateCodeInfoForAddress(FCodeAnalysisState &state, uint16_t pc);
void ResetReferenceInfo(FCodeAnalysisState& state, bool bReads, bool bWrites);
void ResetReadWriteCounts(FCodeAnalysisState& state, bool bReads, bool bWrites);
void ResetExecutionCounts(FCodeAnalysisState &state);

std::string GetItemText(const FCodeAnalysisState& state, FAddressRef address);

// Commands
FLabelInfo* AddLabel(FCodeAnalysisState& state, FAddressRef address, const char* name, ELabelType type, uint16_t memoryRange = 0);
FCommentBlock* AddCommentBlock(FCodeAnalysisState& state, FAddressRef address);
FLabelInfo* AddLabelAtAddress(FCodeAnalysisState &state, FAddressRef address);
void RemoveLabelAtAddress(FCodeAnalysisState &state, FAddressRef address);
void SetItemCode(FCodeAnalysisState& state, FAddressRef addr);
//void SetItemCode(FCodeAnalysisState &state, const FCodeAnalysisItem& item);
void SetItemData(FCodeAnalysisState &state, const FCodeAnalysisItem& item);
void SetDataItemDisplayType(FCodeAnalysisState& state, const FCodeAnalysisItem& item, EDataItemDisplayType displayType);
void SetItemText(FCodeAnalysisState &state, const FCodeAnalysisItem& item);
void SetItemImage(FCodeAnalysisState& state, const FCodeAnalysisItem& item);
void SetItemCommentText(FCodeAnalysisState &state, const FCodeAnalysisItem& item, const char *pText);

void FormatData(FCodeAnalysisState& state, const FDataFormattingOptions& options);
void BatchFormatData(FCodeAnalysisState& state, const FBatchDataFormattingOptions& options);

// machine state
FMachineState* AllocateMachineState(FCodeAnalysisState& state);
void FreeMachineStates(FCodeAnalysisState& state);
void CaptureMachineState(FMachineState* pMachineState, ICPUInterface* pCPUInterface);

void FixupAddressRefForBank(const FCodeAnalysisBank* pBank, FAddressRef& addr);
void FixupAddressRef(const FCodeAnalysisState& state, FAddressRef& addr);
void FixupAddressRefList(const FCodeAnalysisState& state, std::vector<FAddressRef>& addrList);
void FixupAddressRefListForBank(const FCodeAnalysisBank* pBank, std::vector<FAddressRef>& addrList);

// static analysis functions
EInstructionType GetInstructionType(FCodeAnalysisState& state, FAddressRef addr);
