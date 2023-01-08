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

enum class LabelType;

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


enum class Key
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
	DataType	DataType = DataType::Byte;
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
		DataType = DataType::Bitmap;
		StartAddress = address;
		ItemSize = xSize / 8;
		NoItems = ySize;
	}

	void		SetupForCharmap(uint16_t address, int xSize, int ySize)
	{
		DataType = DataType::CharacterMap;
		StartAddress = address;
		ItemSize = xSize;
		NoItems = ySize;
	}
};

// view state for code analysis window
struct FCodeAnalysisViewState
{
	bool	Enabled = false;
	FItem*	pCursorItem	= nullptr;
	int		CursorItemIndex = -1;
	bool	TrackPCFrame = false;
	int		GoToAddress = -1;
	int		HoverAddress = -1;		// address being hovered over
	int		HighlightAddress = -1;	// address to highlight
	bool	GoToLabel = false;
	std::vector<uint16_t>	AddressStack;

	bool					DataFormattingTabOpen = false;
	FDataFormattingOptions	DataFormattingOptions;
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

	FCodeAnalysisPage*		ReadPageTable[kAddressSize / FCodeAnalysisPage::kPageSize];
	FCodeAnalysisPage*		WritePageTable[kAddressSize / FCodeAnalysisPage::kPageSize];
	void					SetCodeAnalysisReadPage(int pageNo, FCodeAnalysisPage* pPage) { ReadPageTable[pageNo] = pPage; }
	void					SetCodeAnalysisWritePage(int pageNo, FCodeAnalysisPage* pPage) { WritePageTable[pageNo] = pPage; }
	void					SetCodeAnalysisRWPage(int pageNo, FCodeAnalysisPage* pReadPage, FCodeAnalysisPage *pWritePage)
	{
		SetCodeAnalysisReadPage(pageNo, pReadPage);
		SetCodeAnalysisWritePage(pageNo, pWritePage);
	}

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
	int32_t		NextPageId = 0;
	std::map<std::string, int>	LabelUsage;

public:
	bool					bCodeAnalysisDataDirty = false;

	bool					bRegisterDataAccesses = true;

	std::vector< FItem *>	ItemList;
	std::vector< FLabelInfo *>	GlobalDataItems;
	std::vector< FLabelInfo *>	GlobalFunctions;

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

	int						KeyConfig[(int)Key::Count];

	std::vector< class FCommand *>	CommandStack;

	bool					bAllowEditing = false;

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

// Commands
class FCommand
{
public:
	virtual void Do(FCodeAnalysisState &state) = 0;
	virtual void Undo(FCodeAnalysisState &state) = 0;
};


// Analysis
void InitialiseCodeAnalysis(FCodeAnalysisState &state, ICPUInterface* pCPUInterface);
bool GenerateLabelForAddress(FCodeAnalysisState &state, uint16_t pc, LabelType label);
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

FLabelInfo* AddLabel(FCodeAnalysisState& state, uint16_t address, const char* name, LabelType type);
FCommentBlock* AddCommentBlock(FCodeAnalysisState& state, uint16_t address);
void AddLabelAtAddress(FCodeAnalysisState &state, uint16_t address);
void RemoveLabelAtAddress(FCodeAnalysisState &state, uint16_t address);
void SetLabelName(FCodeAnalysisState &state, FLabelInfo *pLabel, const char *pText);
void SetItemCode(FCodeAnalysisState& state, uint16_t addr);
void SetItemCode(FCodeAnalysisState &state, FItem *pItem);
void SetItemData(FCodeAnalysisState &state, FItem *pItem);
void SetItemText(FCodeAnalysisState &state, FItem *pItem);
void SetItemImage(FCodeAnalysisState& state, FItem* pItem);
void SetItemCommentText(FCodeAnalysisState &state, FItem *pItem, const char *pText);

void FormatData(FCodeAnalysisState& state, const FDataFormattingOptions& options);

// number output abstraction
IDasmNumberOutput* GetNumberOutput();
void SetNumberOutput(IDasmNumberOutput* pNumberOutputObj);
