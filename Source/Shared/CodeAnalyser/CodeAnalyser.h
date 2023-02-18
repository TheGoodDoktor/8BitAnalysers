#pragma once
#include <cstdint>
#include <string>
#include <map>
#include <set>
#include <vector>
#include <algorithm>

#include "CodeAnaysisPage.h"

#define USE_PAGING 1

class FGraphicsView;
struct FCodeAnalysisState;

enum class ELabelType;

// CPU abstraction
enum class ECPUType
{
	Unknown,
	Z80,
	M6502
};

/* the input callback type */
typedef uint8_t(*FDasmInput)(void* user_data);
/* the output callback type */
typedef void (*FDasmOutput)(char c, void* user_data);

class ICPUInterface
{
public:
	virtual uint8_t		ReadByte(uint16_t address) const = 0;
	virtual uint16_t	ReadWord(uint16_t address) const = 0;
	virtual const uint8_t*	GetMemPtr(uint16_t address) const = 0;
	virtual void		WriteByte(uint16_t address, uint8_t value) = 0;
	virtual uint16_t	GetPC(void) = 0;
	virtual uint16_t	GetSP(void) = 0;

	// breakpoints
	virtual bool	IsAddressBreakpointed(uint16_t addr) = 0;
	virtual bool	ToggleExecBreakpointAtAddress(uint16_t addr) = 0;
	virtual bool	ToggleDataBreakpointAtAddress(uint16_t addr, uint16_t dataSize) = 0;

	// commands
	virtual void	Break() = 0;
	virtual void	Continue() = 0;
	virtual void	StepOver() = 0;
	virtual void	StepInto() = 0;
	virtual void	StepFrame() = 0;
	virtual void	StepScreenWrite() = 0;
	virtual void	GraphicsViewerSetView(uint16_t address, int charWidth) = 0;

	virtual bool	ShouldExecThisFrame(void) const = 0;
	virtual bool	IsStopped(void) const = 0;

	virtual void* GetCPUEmulator(void) { return nullptr; }	// get pointer to emulator - a bit of a hack

	ECPUType	CPUType = ECPUType::Unknown;
};

typedef void (*z80dasm_output_t)(char c, void* user_data);

class IDasmNumberOutput
{
public:

	virtual void OutputU8(uint8_t val, z80dasm_output_t out_cb) = 0;
	virtual void OutputU16(uint16_t val, z80dasm_output_t out_cb) = 0;
	virtual void OutputD8(int8_t val, z80dasm_output_t out_cb) = 0;
};

class FDasmStateBase : public IDasmNumberOutput
{
public:
	FCodeAnalysisState*		CodeAnalysisState;
	uint16_t				CurrentAddress;
	std::string				Text;
};


struct FMemoryAccess
{
	uint16_t	Address;
	uint8_t		Value;
	uint16_t	PC;
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
	int			StartAddress = 0;
	int			ItemSize = 1;
	int			NoItems = 1;
	uint16_t	CharacterSet = 0;
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
	uint16_t		MinAddress = 0;
	uint16_t		MaxAddress = 0xffff;
};

struct FCodeAnalysisItem
{
	FCodeAnalysisItem() = default;
	FCodeAnalysisItem(FItem* pItem, uint16_t addr) :Item(pItem), Address(addr) {}

	bool IsValid() const { return Item != nullptr; }
	
	FItem* Item = nullptr;
	uint16_t	Address = 0;	// address in address space
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

	bool	Enabled = false;
	int		CursorItemIndex = -1;
	bool	TrackPCFrame = false;
	int		GoToAddress = -1;
	int		HoverAddress = -1;		// address being hovered over
	int		HighlightAddress = -1;	// address to highlight
	bool	GoToLabel = false;

	// for global Filters
	bool						ShowROMLabels = true;
	FLabelListFilter			GlobalDataItemsFilter;
	std::vector<FCodeAnalysisItem>	FilteredGlobalDataItems;
	FLabelListFilter				GlobalFunctionsFilter;
	std::vector<FCodeAnalysisItem>	FilteredGlobalFunctions;

	std::vector<uint16_t>	AddressStack;

	bool					DataFormattingTabOpen = false;
	FDataFormattingOptions	DataFormattingOptions;
private:
	FCodeAnalysisItem	CursorItem;
};

struct FCodeAnalysisConfig
{
	bool bShowOpcodeValues = false;
};


// code analysis information
// TODO: make this a class
struct FCodeAnalysisState
{
	ICPUInterface*			CPUInterface = nullptr;
	int						CurrentFrameNo = 0;

	static const int kAddressSize = 1 << 16;

	bool					RegisterPage(FCodeAnalysisPage* pPage, const char* pName) 
	{
		if (pPage->PageId != -1)
			return false;
		pPage->PageId = (int16_t)RegisteredPages.size();
		RegisteredPages.push_back(pPage);
		PageNames.push_back(pName);
		return true;
	}
	FCodeAnalysisPage*		GetPage(int16_t id) { return RegisteredPages[id]; }
	const char*				GetPageName(int16_t id) { return PageNames[id].c_str(); }
	int16_t					GetAddressReadPageId(uint16_t addr) { return GetReadPage(addr)->PageId; }
	int16_t					GetAddressWritePageId(uint16_t addr) { return GetWritePage(addr)->PageId; }
	const std::vector< FCodeAnalysisPage*>& GetRegisteredPages() const { return RegisteredPages; }

	FCodeAnalysisPage*		ReadPageTable[kAddressSize / FCodeAnalysisPage::kPageSize];
	FCodeAnalysisPage*		WritePageTable[kAddressSize / FCodeAnalysisPage::kPageSize];
	void					SetCodeAnalysisReadPage(int pageNo, FCodeAnalysisPage* pPage) { ReadPageTable[pageNo] = pPage; pPage->bUsed = true; }
	void					SetCodeAnalysisWritePage(int pageNo, FCodeAnalysisPage* pPage) { WritePageTable[pageNo] = pPage; pPage->bUsed = true;}
	void					SetCodeAnalysisRWPage(int pageNo, FCodeAnalysisPage* pReadPage, FCodeAnalysisPage *pWritePage)
	{
		SetCodeAnalysisReadPage(pageNo, pReadPage);
		SetCodeAnalysisWritePage(pageNo, pWritePage);
	}

	void	SetCodeAnalysisDirty(bool val = true) 
	{ 
		bCodeAnalysisDataDirty = val; 
	}

	bool IsCodeAnalysisDataDirty() const { return bCodeAnalysisDataDirty; }
	void SetMemoryRemapped(bool bRemapped = true) { bMemoryRemapped = bRemapped; }
	bool HasMemoryBeenRemapped() const { return bMemoryRemapped; }

	void	ResetLabelNames() { LabelUsage.clear(); }
	bool	EnsureUniqueLabelName(std::string& lableName);
	bool	RemoveLabelName(const std::string& labelName);	// for changing label names

	// Watches
	void InitWatches() { Watches.clear(); }
	bool	AddWatch(uint16_t address)
	{
		return Watches.insert(address).second;
	}

	bool	RemoveWatch(uint16_t address)
	{
		Watches.erase(address);
		return true;
	}

	const std::set<uint16_t>& GetWatches() const { return Watches; }
private:
	std::vector<FCodeAnalysisPage*>	RegisteredPages;
	std::vector<std::string>	PageNames;
	int32_t						NextPageId = 0;
	std::map<std::string, int>	LabelUsage;

	bool						bCodeAnalysisDataDirty = false;
	bool						bMemoryRemapped = false;

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
	
	std::set<uint16_t>		Watches;	// addresses to use as watches
	std::vector<FCPUFunctionCall>	CallStack;
	uint16_t				StackMin;
	uint16_t				StackMax;

	std::vector<uint16_t>	FrameTrace;

	int						KeyConfig[(int)EKey::Count];

	std::vector< class FCommand *>	CommandStack;

	bool					bAllowEditing = false;

	FCodeAnalysisConfig Config;
public:
	// Access functions for code analysis
	static const int kPageShift = 10;
	static const int kPageMask = 1023;

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
	void SetLabelForAddress(uint16_t addr, FLabelInfo* pLabel) 
	{
		if(pLabel != nullptr)	// ensure no name clashes
			EnsureUniqueLabelName(pLabel->Name);
		GetReadPage(addr)->Labels[addr & kPageMask] = pLabel; 
	}

	FCommentBlock* GetCommentBlockForAddress(uint16_t addr) const { return GetReadPage(addr)->CommentBlocks[addr & kPageMask]; }
	void SetCommentBlockForAddress(uint16_t addr, FCommentBlock* pCommentBlock)
	{
		GetReadPage(addr)->CommentBlocks[addr & kPageMask] = pCommentBlock;
	}

	const FCodeInfo* GetCodeInfoForAddress(uint16_t addr) const { return GetReadPage(addr)->CodeInfo[addr & kPageMask]; }
	FCodeInfo* GetCodeInfoForAddress(uint16_t addr) { return GetReadPage(addr)->CodeInfo[addr & kPageMask]; }
	void SetCodeInfoForAddress(uint16_t addr, FCodeInfo* pCodeInfo) { GetReadPage(addr)->CodeInfo[addr & kPageMask] = pCodeInfo; }

	const FDataInfo* GetReadDataInfoForAddress(uint16_t addr) const { return &GetReadPage(addr)->DataInfo[addr & kPageMask]; }
	FDataInfo* GetReadDataInfoForAddress(uint16_t addr) { return &GetReadPage(addr)->DataInfo[addr & kPageMask]; }

	const FDataInfo* GetWriteDataInfoForAddress(uint16_t addr) const { return  &GetWritePage(addr)->DataInfo[addr & kPageMask]; }
	FDataInfo* GetWriteDataInfoForAddress(uint16_t addr) { return &GetWritePage(addr)->DataInfo[addr & kPageMask]; }

	uint16_t GetLastWriterForAddress(uint16_t addr) const { return GetWritePage(addr)->LastWriter[addr & kPageMask]; }
	void SetLastWriterForAddress(uint16_t addr, uint16_t lastWriter) { GetWritePage(addr)->LastWriter[addr & kPageMask] = lastWriter; }

	bool FindMemoryPattern(uint8_t* pData, size_t dataSize, uint16_t offset, uint16_t& outAddr);

	void FindAsciiStrings(uint16_t startAddress);
};

// Analysis
void InitialiseCodeAnalysis(FCodeAnalysisState &state, ICPUInterface* pCPUInterface);
bool GenerateLabelForAddress(FCodeAnalysisState &state, uint16_t pc, ELabelType label);
void RunStaticCodeAnalysis(FCodeAnalysisState &state, uint16_t pc);
bool RegisterCodeExecuted(FCodeAnalysisState &state, uint16_t pc, uint16_t nextpc);
void ReAnalyseCode(FCodeAnalysisState &state);
uint16_t WriteCodeInfoForAddress(FCodeAnalysisState& state, uint16_t pc);
void GenerateGlobalInfo(FCodeAnalysisState &state);
void RegisterDataRead(FCodeAnalysisState& state, uint16_t pc, uint16_t dataAddr);
void RegisterDataWrite(FCodeAnalysisState &state, uint16_t pc, uint16_t dataAddr);
void UpdateCodeInfoForAddress(FCodeAnalysisState &state, uint16_t pc);
void ResetReferenceInfo(FCodeAnalysisState &state);

std::string GetItemText(FCodeAnalysisState& state, uint16_t address);

// Commands
void Undo(FCodeAnalysisState &state);

FLabelInfo* AddLabel(FCodeAnalysisState& state, uint16_t address, const char* name, ELabelType type);
FCommentBlock* AddCommentBlock(FCodeAnalysisState& state, uint16_t address);
void AddLabelAtAddress(FCodeAnalysisState &state, uint16_t address);
void RemoveLabelAtAddress(FCodeAnalysisState &state, uint16_t address);
void SetLabelName(FCodeAnalysisState &state, FLabelInfo *pLabel, const char *pText);
void SetItemCode(FCodeAnalysisState& state, uint16_t addr);
//void SetItemCode(FCodeAnalysisState &state, const FCodeAnalysisItem& item);
void SetItemData(FCodeAnalysisState &state, const FCodeAnalysisItem& item);
void SetItemText(FCodeAnalysisState &state, const FCodeAnalysisItem& item);
void SetItemImage(FCodeAnalysisState& state, const FCodeAnalysisItem& item);
void SetItemCommentText(FCodeAnalysisState &state, const FCodeAnalysisItem& item, const char *pText);

void FormatData(FCodeAnalysisState& state, const FDataFormattingOptions& options);

// number output abstraction
IDasmNumberOutput* GetNumberOutput();
void SetNumberOutput(IDasmNumberOutput* pNumberOutputObj);
