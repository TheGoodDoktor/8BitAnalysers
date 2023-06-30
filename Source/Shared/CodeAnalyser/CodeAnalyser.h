#pragma once
#include <cstdint>
#include <string>
#include <map>
#include <set>
#include <vector>
#include <algorithm>
#include <assert.h>

#include "CodeAnalyserTypes.h"
#include "CodeAnalysisPage.h"
#include "Debugger.h"

class FGraphicsView;
class FCodeAnalysisState;

enum class ELabelType;

/* the input callback type */
typedef uint8_t(*FDasmInput)(void* user_data);
/* the output callback type */
typedef void (*FDasmOutput)(char c, void* user_data);

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

    // FIXME - no other implementation for the method - unable to instantiate abstract class
    virtual void	GraphicsViewerSetView(FAddressRef address, int charWidth) {};
	//virtual void	GraphicsViewerSetView(FAddressRef address, int charWidth) = 0;

	virtual void*	GetCPUEmulator(void) const { return nullptr; }	// get pointer to emulator - a bit of a hack

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
	ToggleItemBinary,

	AddLabel,
	Rename,
	Comment,
	AddCommentBlock,

	// Debugger
	BreakContinue,
	StepOver,
	StepInto,
	StepFrame,
	StepScreenWrite,
	Breakpoint,

	Count
};


struct FDataFormattingOptions
{
	EDataType	DataType = EDataType::Byte;
	int			StartAddress = 0;	// TODO: use Address Ref
	int			ItemSize = 1;
	int			NoItems = 1;
	FAddressRef	CharacterSet;
	uint8_t		EmptyCharNo = 0;
	bool		ClearCodeInfo = false;
	bool		ClearLabels = false;
	bool		AddLabelAtStart = false;

	bool		IsValid() const {	return NoItems > 0 && ItemSize > 0;	}
	uint16_t	CalcEndAddress() const { return StartAddress + (NoItems * ItemSize) - 1; }
	void		SetupForBitmap(uint16_t address, int xSize, int ySize)
	{
		DataType = EDataType::Bitmap;
		StartAddress = address;
		ItemSize = xSize / 8;
		NoItems = ySize;
	}

	void		SetupForCharmap(uint16_t address, int xSize, int ySize)
	{
		DataType = EDataType::CharacterMap;
		StartAddress = address;
		ItemSize = xSize;
		NoItems = ySize;
	}
};

struct FLabelListFilter
{
	std::string		FilterText;
	uint16_t		MinAddress = 0x0000;
	uint16_t		MaxAddress = 0xffff;
	bool			bRAMOnly = false;
};

struct FCodeAnalysisItem
{
	FCodeAnalysisItem() {}
	FCodeAnalysisItem(FItem* pItem, int16_t bankId, uint16_t addr) :Item(pItem), AddressRef(bankId,addr) {}
	FCodeAnalysisItem(FItem* pItem, FAddressRef addr) :Item(pItem), AddressRef(addr) {}

	bool IsValid() const { return Item != nullptr; }
	
	FItem*		Item = nullptr;
	FAddressRef	AddressRef;

};

// view state for code analysis window
struct FCodeAnalysisViewState
{
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

	bool			Enabled = false;
	bool			TrackPCFrame = false;
	FAddressRef		HoverAddress;		// address being hovered over
	FAddressRef		HighlightAddress;	// address to highlight
	bool			GoToLabel = false;
	int16_t			ViewingBankId = -1;
	// for global Filters
	bool						ShowROMLabels = false;
	FLabelListFilter			GlobalDataItemsFilter;
	std::vector<FCodeAnalysisItem>	FilteredGlobalDataItems;
	FLabelListFilter				GlobalFunctionsFilter;
	std::vector<FCodeAnalysisItem>	FilteredGlobalFunctions;


	bool					DataFormattingTabOpen = false;
	FDataFormattingOptions	DataFormattingOptions;
private:
	FCodeAnalysisItem			CursorItem;
	FAddressRef					GoToAddressRef;
	std::vector<FAddressRef>	AddressStack;
};

struct FCodeAnalysisConfig
{
	bool				bShowOpcodeValues = false;
	bool				bShowBanks = false;
	const uint32_t*		CharacterColourLUT = nullptr;
};

struct FCodeAnalysisBank
{
	int16_t				Id = -1;
	int					NoPages = 0;
	uint32_t			SizeMask = 0;
	std::vector<int>	MappedPages;	// banks can be mapped to multiple pages
	int					PrimaryMappedPage = -1;
	uint8_t*			Memory = nullptr;	// pointer to memory bank occupies
	FCodeAnalysisPage*	Pages = nullptr;
	std::string			Name;
	std::string			Description;	// where we can describe what the bank is used for
	bool				bReadOnly = false;
	bool				bIsDirty = false;
	std::vector<FCodeAnalysisItem>		ItemList;

	bool		AddressValid(uint16_t addr) const { return addr >= GetMappedAddress() && addr < GetMappedAddress() + (NoPages * FCodeAnalysisPage::kPageSize);	}
	bool		IsUsed() const { return Pages[0].bUsed; }
	bool		IsMapped() const { return MappedPages.empty() == false; }
	uint16_t	GetMappedAddress() const { return PrimaryMappedPage * FCodeAnalysisPage::kPageSize; }
	uint16_t	GetSizeBytes() const { return NoPages * FCodeAnalysisPage::kPageSize; }
};

// code analysis information
class FCodeAnalysisState
{
public:
	// constants
	static const int kAddressSize = 1 << 16;
	static const int kPageShift = 10;
	static const int kPageMask = 1023;
	static const int kNoPagesInAddressSpace = kAddressSize / FCodeAnalysisPage::kPageSize;

	FCodeAnalysisState();
	void	Init(ICPUInterface* pCPUInterface);
	void	OnFrameStart();
	void	OnFrameEnd();

	const ICPUInterface* GetCPUInterface() const { return CPUInterface; }

	ICPUInterface* CPUInterface = nullptr;	// Make private
	int						CurrentFrameNo = 0;

	// Memory Banks & Pages
	int16_t		CreateBank(const char* name, int noKb, uint8_t* pMemory, bool bReadOnly);
	bool		MapBank(int16_t bankId, int startPageNo);
	bool		UnMapBank(int16_t bankId, int startPageNo);
	bool		IsBankIdMapped(int16_t bankId) const;
	bool		IsAddressValid(FAddressRef addr) const;

	bool		MapBankForAnalysis(FCodeAnalysisBank& bank);
	void		UnMapAnalysisBanks();

	bool		IsAddressBreakpointed(FAddressRef addr) const;
	bool		ToggleExecBreakpointAtAddress(FAddressRef addr);
	bool		ToggleDataBreakpointAtAddress(FAddressRef addr, uint16_t dataSize);

	
	FCodeAnalysisBank* GetBank(int16_t bankId) { return (bankId >= 0 && bankId < Banks.size()) ? &Banks[bankId] : nullptr; }
	const FCodeAnalysisBank* GetBank(int16_t bankId) const { return (bankId >= 0 && bankId < Banks.size()) ? &Banks[bankId] : nullptr;	}
	int16_t		GetBankFromAddress(uint16_t address) const { return MappedBanks[address >> kPageShift]; }
	const std::vector<FCodeAnalysisBank>& GetBanks() const { return Banks; }
	std::vector<FCodeAnalysisBank>& GetBanks() { return Banks; }

	FAddressRef	AddressRefFromPhysicalAddress(uint16_t physAddr) const { return FAddressRef(GetBankFromAddress(physAddr), physAddr); }

	uint8_t		ReadByte(uint16_t address) const
	{
		if (MappedMem[address >> kPageShift] == nullptr)
			return CPUInterface->ReadByte(address);
		else
			return *(MappedMem[(address >> kPageShift)] + (address & kPageMask));
	}

	uint8_t		ReadByte(FAddressRef address) const
	{
		const FCodeAnalysisBank* pBank = GetBank(address.BankId);
		assert(pBank != nullptr);
		return pBank->Memory[address.Address - pBank->GetMappedAddress()];
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
		const FCodeAnalysisBank* pBank = GetBank(address.BankId);
		assert(pBank != nullptr);
		return *(uint16_t*)(&pBank->Memory[address.Address - pBank->GetMappedAddress()]);
	}

	void		WriteByte(uint16_t address, uint8_t value) 
	{ 
		if (MappedMem[address >> kPageShift] == nullptr)
			CPUInterface->WriteByte(address, value);
		else
			*(MappedMem[(address >> kPageShift)] + (address & kPageMask)) = value;
	}
	
	FCodeAnalysisPage* GetPage(int16_t id) { return RegisteredPages[id]; }

	void	SetCodeAnalysisDirty(FAddressRef addrRef)
	{
		FCodeAnalysisBank* pBank = GetBank(addrRef.BankId);
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
			FCodeAnalysisBank* pBank = GetBank(MappedBanks[i]);
			if (pBank != nullptr)
				pBank->bIsDirty = true;
			bCodeAnalysisDataDirty = true;
		}
	}

	void	SetAllBanksDirty()
	{
		for (auto& bank : Banks)
			bank.bIsDirty = true;
		bCodeAnalysisDataDirty = true;
	}

	void	ClearDirtyStatus(void)
	{
		bCodeAnalysisDataDirty = false;
	}
	
	bool IsCodeAnalysisDataDirty() const { return bCodeAnalysisDataDirty; }
	void ClearRemappings() { bMemoryRemapped = false; }
	bool HasMemoryBeenRemapped() const { return bMemoryRemapped; }
	//const std::vector<int16_t>& GetDirtyBanks() const { return RemappedBanks; }

	void	ResetLabelNames() { LabelUsage.clear(); }
	bool	EnsureUniqueLabelName(std::string& lableName);
	bool	RemoveLabelName(const std::string& labelName);	// for changing label names

public:

	bool					bRegisterDataAccesses = true;

	std::vector<FCodeAnalysisItem>	ItemList;

	std::vector<FCodeAnalysisItem>	GlobalDataItems;
	bool						bRebuildFilteredGlobalDataItems = true;
	
	std::vector<FCodeAnalysisItem>	GlobalFunctions;
	bool						bRebuildFilteredGlobalFunctions = true;

	static const int kNoViewStates = 4;
	FCodeAnalysisViewState	ViewState[kNoViewStates];	// new multiple view states
	int						FocussedWindowId = 0;
	FCodeAnalysisViewState& GetFocussedViewState() { return ViewState[FocussedWindowId]; }
	FCodeAnalysisViewState& GetAltViewState() { return ViewState[FocussedWindowId ^ 1]; }
	
	FDebugger				Debugger;

	FAddressRef				CopiedAddress;

	int						KeyConfig[(int)EKey::Count] = { -1 };

	std::vector< class FCommand *>	CommandStack;

	bool					bAllowEditing = false;

	FCodeAnalysisConfig Config;
public:
	// Access functions for code analysis

	FCodeAnalysisPage* GetReadPage(uint16_t addr)  
	{
		const int pageNo = addr >> kPageShift;
		FCodeAnalysisPage* pPage = ReadPageTable[pageNo];
		if (pPage == nullptr)
		{
			printf("Read page %d NOT mapped", pageNo);
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
			printf("Write page %d NOT mapped", pageNo);
		}

		return pPage;
	}
	const FCodeAnalysisPage* GetWritePage(uint16_t addr) const { return ((FCodeAnalysisState*)this)->GetWritePage(addr); }

	const FLabelInfo* GetLabelForAddress(uint16_t addr) const { return GetReadPage(addr)->Labels[addr & kPageMask]; }
	FLabelInfo* GetLabelForAddress(uint16_t addr) { return GetReadPage(addr)->Labels[addr & kPageMask]; }
	FLabelInfo* GetLabelForAddress(FAddressRef addrRef)
	{
		const FCodeAnalysisBank* pBank = GetBank(addrRef.BankId);
		if (pBank != nullptr)
		{
			const uint16_t bankAddr = addrRef.Address - (pBank->PrimaryMappedPage * FCodeAnalysisPage::kPageSize);
			return pBank->Pages[bankAddr >> FCodeAnalysisPage::kPageShift].Labels[bankAddr & FCodeAnalysisPage::kPageMask];
		}
		else
		{
			return nullptr;
		}
	}
	void SetLabelForAddress(uint16_t addr, FLabelInfo* pLabel) 
	{
		if(pLabel != nullptr)	// ensure no name clashes
			EnsureUniqueLabelName(pLabel->Name);
		GetReadPage(addr)->Labels[addr & kPageMask] = pLabel; 
	}
	void SetLabelForAddress(FAddressRef addrRef, FLabelInfo* pLabel)
	{
		if (pLabel != nullptr)	// ensure no name clashes
			EnsureUniqueLabelName(pLabel->Name);

		FCodeAnalysisBank* pBank = GetBank(addrRef.BankId);
		if (pBank != nullptr)
		{
			const uint16_t bankAddr = addrRef.Address - (pBank->PrimaryMappedPage * FCodeAnalysisPage::kPageSize);
			pBank->Pages[bankAddr >> FCodeAnalysisPage::kPageShift].Labels[bankAddr & FCodeAnalysisPage::kPageMask] = pLabel;
		}
	}

	FCommentBlock* GetCommentBlockForAddress(uint16_t addr) const { return GetReadPage(addr)->CommentBlocks[addr & kPageMask]; }
	void SetCommentBlockForAddress(uint16_t addr, FCommentBlock* pCommentBlock)
	{
		GetReadPage(addr)->CommentBlocks[addr & kPageMask] = pCommentBlock;
	}

	const FCodeInfo* GetCodeInfoForAddress(uint16_t addr) const { return GetReadPage(addr)->CodeInfo[addr & kPageMask]; }
	FCodeInfo* GetCodeInfoForAddress(uint16_t addr) { return GetReadPage(addr)->CodeInfo[addr & kPageMask]; }
	FCodeInfo* GetCodeInfoForAddress(FAddressRef addrRef)
	{
		const FCodeAnalysisBank* pBank = GetBank(addrRef.BankId);
		if (pBank != nullptr)
		{
			const uint16_t bankAddr = addrRef.Address - (pBank->PrimaryMappedPage * FCodeAnalysisPage::kPageSize);
			return pBank->Pages[bankAddr >> FCodeAnalysisPage::kPageShift].CodeInfo[bankAddr & FCodeAnalysisPage::kPageMask];
		}
		else
		{
			return nullptr;
		}
	}

	void SetCodeInfoForAddress(uint16_t addr, FCodeInfo* pCodeInfo) { GetReadPage(addr)->CodeInfo[addr & kPageMask] = pCodeInfo; }

	const FDataInfo* GetReadDataInfoForAddress(uint16_t addr) const { return &GetReadPage(addr)->DataInfo[addr & kPageMask]; }
	FDataInfo* GetReadDataInfoForAddress(uint16_t addr) { return &GetReadPage(addr)->DataInfo[addr & kPageMask]; }
	FDataInfo* GetReadDataInfoForAddress(FAddressRef addrRef)
	{
		const FCodeAnalysisBank* pBank = GetBank(addrRef.BankId);
		if (pBank != nullptr)
		{
			const uint16_t bankAddr = addrRef.Address - pBank->GetMappedAddress();
			return &pBank->Pages[bankAddr >> FCodeAnalysisPage::kPageShift].DataInfo[bankAddr & FCodeAnalysisPage::kPageMask];
		}
		else
		{
			return nullptr;
		}
	}
	const FDataInfo* GetWriteDataInfoForAddress(uint16_t addr) const { return  &GetWritePage(addr)->DataInfo[addr & kPageMask]; }
	FDataInfo* GetWriteDataInfoForAddress(uint16_t addr) { return &GetWritePage(addr)->DataInfo[addr & kPageMask]; }
	FDataInfo* GetWriteDataInfoForAddress(FAddressRef addrRef)
	{
		const FCodeAnalysisBank* pBank = GetBank(addrRef.BankId);
		if (pBank != nullptr)
		{
			const uint16_t bankAddr = addrRef.Address - pBank->GetMappedAddress();
			return &pBank->Pages[bankAddr >> FCodeAnalysisPage::kPageShift].DataInfo[bankAddr & FCodeAnalysisPage::kPageMask];
		}
		else
		{
			return nullptr;
		}
	}
	FAddressRef GetLastWriterForAddress(uint16_t addr) const { return GetWritePage(addr)->DataInfo[addr & kPageMask].LastWriter; }
	void SetLastWriterForAddress(uint16_t addr, FAddressRef lastWriter) { GetWritePage(addr)->DataInfo[addr & kPageMask].LastWriter = lastWriter; }

	FMachineState* GetMachineState(uint16_t addr) { return GetReadPage(addr)->MachineState[addr & kPageMask];}
	void SetMachineStateForAddress(uint16_t addr, FMachineState* pMachineState) { GetReadPage(addr)->MachineState[addr & kPageMask] = pMachineState; }

	bool FindMemoryPattern(uint8_t* pData, size_t dataSize, uint16_t offset, uint16_t& outAddr);

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


	void					SetCodeAnalysisReadPage(int pageNo, FCodeAnalysisPage* pPage) { ReadPageTable[pageNo] = pPage; if (pPage != nullptr) pPage->bUsed = true; }
	void					SetCodeAnalysisWritePage(int pageNo, FCodeAnalysisPage* pPage) { WritePageTable[pageNo] = pPage; if(pPage != nullptr) pPage->bUsed = true; }
	void					SetCodeAnalysisRWPage(int pageNo, FCodeAnalysisPage* pReadPage, FCodeAnalysisPage* pWritePage)
	{
		SetCodeAnalysisReadPage(pageNo, pReadPage);
		SetCodeAnalysisWritePage(pageNo, pWritePage);
	}

	// private data members

	FCodeAnalysisPage*				ReadPageTable[kNoPagesInAddressSpace];
	FCodeAnalysisPage*				WritePageTable[kNoPagesInAddressSpace];

	std::vector<FCodeAnalysisBank>	Banks;
	int16_t							MappedBanks[kNoPagesInAddressSpace];	// banks mapped into address space
	int16_t							MappedBanksBackup[kNoPagesInAddressSpace];	// banks mapped into address space

	uint8_t*						MappedMem[kNoPagesInAddressSpace];	// mapped analysis memory
				
	std::vector<FCodeAnalysisPage*>	RegisteredPages;
	std::vector<std::string>	PageNames;
	int32_t						NextPageId = 0;
	std::map<std::string, int>	LabelUsage;

	bool						bCodeAnalysisDataDirty = false;
	bool						bMemoryRemapped = true;

};

// Analysis
FLabelInfo* GenerateLabelForAddress(FCodeAnalysisState &state, FAddressRef addrRef, ELabelType label);
void RunStaticCodeAnalysis(FCodeAnalysisState &state, uint16_t pc);
bool RegisterCodeExecuted(FCodeAnalysisState &state, uint16_t pc, uint16_t oldpc);
void ReAnalyseCode(FCodeAnalysisState &state);
uint16_t WriteCodeInfoForAddress(FCodeAnalysisState& state, uint16_t pc);
void GenerateGlobalInfo(FCodeAnalysisState &state);
void RegisterDataRead(FCodeAnalysisState& state, uint16_t pc, uint16_t dataAddr);
void RegisterDataWrite(FCodeAnalysisState &state, uint16_t pc, uint16_t dataAddr, uint8_t value);
void UpdateCodeInfoForAddress(FCodeAnalysisState &state, uint16_t pc);
void ResetReferenceInfo(FCodeAnalysisState &state);

std::string GetItemText(FCodeAnalysisState& state, FAddressRef address);

// Commands
void Undo(FCodeAnalysisState &state);

FLabelInfo* AddLabel(FCodeAnalysisState& state, uint16_t address, const char* name, ELabelType type);
FCommentBlock* AddCommentBlock(FCodeAnalysisState& state, uint16_t address);
FLabelInfo* AddLabelAtAddress(FCodeAnalysisState &state, FAddressRef address);
void RemoveLabelAtAddress(FCodeAnalysisState &state, FAddressRef address);
void SetLabelName(FCodeAnalysisState &state, FLabelInfo *pLabel, const char *pText);
void SetItemCode(FCodeAnalysisState& state, FAddressRef addr);
//void SetItemCode(FCodeAnalysisState &state, const FCodeAnalysisItem& item);
void SetItemData(FCodeAnalysisState &state, const FCodeAnalysisItem& item);
void SetItemText(FCodeAnalysisState &state, const FCodeAnalysisItem& item);
void SetItemImage(FCodeAnalysisState& state, const FCodeAnalysisItem& item);
void SetItemCommentText(FCodeAnalysisState &state, const FCodeAnalysisItem& item, const char *pText);

void FormatData(FCodeAnalysisState& state, const FDataFormattingOptions& options);

// machine state
FMachineState* AllocateMachineState(FCodeAnalysisState& state);
void FreeMachineStates(FCodeAnalysisState& state);
void CaptureMachineState(FMachineState* pMachineState, ICPUInterface* pCPUInterface);