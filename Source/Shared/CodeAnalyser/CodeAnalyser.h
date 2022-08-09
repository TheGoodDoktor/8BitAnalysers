#pragma once
#include <cstdint>
#include <string>
#include <map>
#include <vector>

#include "CodeAnaysisPage.h"

#define USE_PAGING 1

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
	virtual uint8_t		ReadByte(uint16_t address) = 0;
	virtual uint16_t	ReadWord(uint16_t address) = 0;
	virtual uint16_t	GetPC(void) = 0;

	// commands
	virtual void	Break() = 0;
	virtual void	Continue() = 0;
	virtual void	GraphicsViewerSetAddress(uint16_t address) = 0;

	virtual bool	ExecThisFrame(void) = 0;

	virtual void InsertROMLabels(struct FCodeAnalysisState& state) = 0;
	virtual void InsertSystemLabels(struct FCodeAnalysisState& state) = 0;

	ECPUType	CPUType = ECPUType::Unknown;
};

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


enum class Key
{
	SetItemData,
	SetItemText,
	SetItemCode,

	AddLabel,
	Rename,
	Comment,

	Count
};



// code analysis information
struct FCodeAnalysisState
{
	ICPUInterface*			CPUInterface = nullptr;
	int						CurrentFrameNo = 0;

	static const int kAddressSize = 1 << 16;

	// TODO: use page table
	FCodeAnalysisPage*		ReadPageTable[kAddressSize / FCodeAnalysisPage::kPageSize];
	FCodeAnalysisPage*		WritePageTable[kAddressSize / FCodeAnalysisPage::kPageSize];
	void					SetCodeAnalysisReadPage(int pageNo, FCodeAnalysisPage* pPage) { ReadPageTable[pageNo] = pPage; }
	void					SetCodeAnalysisWritePage(int pageNo, FCodeAnalysisPage* pPage) { WritePageTable[pageNo] = pPage; }
	void					SetCodeAnalysisRWPage(int pageNo, FCodeAnalysisPage* pReadPage, FCodeAnalysisPage *pWritePage)
	{
		SetCodeAnalysisReadPage(pageNo, pReadPage);
		SetCodeAnalysisWritePage(pageNo, pWritePage);
	}


	// TODO: replace below with above
private:
	FLabelInfo*				Labels[kAddressSize];
	FCodeInfo*				CodeInfo[kAddressSize];
	FDataInfo*				DataInfo[kAddressSize];
	uint16_t				LastWriter[kAddressSize];
public:
	bool					bCodeAnalysisDataDirty = false;

	bool					bRegisterDataAccesses = true;

	std::vector< FItem *>	ItemList;
	std::vector< FLabelInfo *>	GlobalDataItems;
	std::vector< FLabelInfo *>	GlobalFunctions;
	FItem*					pCursorItem = nullptr;
	int						CursorItemIndex = -1;
	int						GoToAddress = -1;
	int						HoverAddress = -1;		// address being hovered over
	int						HighlightAddress = -1;	// address to highlight
	bool					GoToLabel = false;
	std::vector<uint16_t>	AddressStack;

	int						KeyConfig[(int)Key::Count];

	std::vector< class FCommand *>	CommandStack;

	// Access functions for code analysis
#if USE_PAGING
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
	void SetLabelForAddress(uint16_t addr, FLabelInfo* pLabel) { GetReadPage(addr)->Labels[addr & kPageMask] = pLabel; }

	const FCodeInfo* GetCodeInfoForAddress(uint16_t addr) const { return GetReadPage(addr)->CodeInfo[addr & kPageMask]; }
	FCodeInfo* GetCodeInfoForAddress(uint16_t addr) { return GetReadPage(addr)->CodeInfo[addr & kPageMask]; }
	void SetCodeInfoForAddress(uint16_t addr, FCodeInfo* pCodeInfo) { GetReadPage(addr)->CodeInfo[addr & kPageMask] = pCodeInfo; }

	const FDataInfo* GetReadDataInfoForAddress(uint16_t addr) const { return GetReadPage(addr)->DataInfo[addr & kPageMask]; }
	FDataInfo* GetReadDataInfoForAddress(uint16_t addr) { return GetReadPage(addr)->DataInfo[addr & kPageMask]; }
	void SetReadDataInfoForAddress(uint16_t addr, FDataInfo* pDataInfo) { GetReadPage(addr)->DataInfo[addr & kPageMask] = pDataInfo; }

	const FDataInfo* GetWriteDataInfoForAddress(uint16_t addr) const { return  GetWritePage(addr)->DataInfo[addr & kPageMask]; }
	FDataInfo* GetWriteDataInfoForAddress(uint16_t addr) { return GetWritePage(addr)->DataInfo[addr & kPageMask]; }
	void SetWriteDataInfoForAddress(uint16_t addr, FDataInfo* pDataInfo) { GetWritePage(addr)->DataInfo[addr & kPageMask] = pDataInfo; }

	uint16_t GetLastWriterForAddress(uint16_t addr) const { return GetWritePage(addr)->LastWriter[addr & kPageMask]; }
	void SetLastWriterForAddress(uint16_t addr, uint16_t lastWriter) { GetWritePage(addr)->LastWriter[addr & kPageMask] = lastWriter; }
#else
	const FLabelInfo* GetLabelForAddress(uint16_t addr) const { return Labels[addr]; }
	FLabelInfo* GetLabelForAddress(uint16_t addr) { return Labels[addr]; }
	void SetLabelForAddress(uint16_t addr, FLabelInfo* pLabel) { Labels[addr] = pLabel; }
	
	const FCodeInfo* GetCodeInfoForAddress(uint16_t addr) const { return CodeInfo[addr]; }
	FCodeInfo* GetCodeInfoForAddress(uint16_t addr) { return CodeInfo[addr]; }
	void SetCodeInfoForAddress(uint16_t addr, FCodeInfo* pCodeInfo) { CodeInfo[addr] = pCodeInfo; }
	
	const FDataInfo* GetReadDataInfoForAddress(uint16_t addr) const { return DataInfo[addr]; }
	FDataInfo* GetReadDataInfoForAddress(uint16_t addr) { return DataInfo[addr]; }
	void SetReadDataInfoForAddress(uint16_t addr, FDataInfo* pDataInfo) { DataInfo[addr] = pDataInfo; }

	const FDataInfo* GetWriteDataInfoForAddress(uint16_t addr) const { return DataInfo[addr]; }
	FDataInfo* GetWriteDataInfoForAddress(uint16_t addr) { return DataInfo[addr]; }
	void SetWriteDataInfoForAddress(uint16_t addr, FDataInfo* pDataInfo) { DataInfo[addr] = pDataInfo; }

	uint16_t GetLastWriterForAddress(uint16_t addr) const { return LastWriter[addr]; }
	void SetLastWriterForAddress(uint16_t addr, uint16_t lastWriter) { LastWriter[addr] = lastWriter; }
#endif
};

// Commands
class FCommand
{
public:
	virtual void Do(FCodeAnalysisState &state) = 0;
	virtual void Undo(FCodeAnalysisState &state) = 0;
};

void InitialiseCodeAnalysisPage(FCodeAnalysisPage* pPage, uint16_t address);

// Analysis
void InitialiseCodeAnalysis(FCodeAnalysisState &state, ICPUInterface* pCPUInterface);
bool GenerateLabelForAddress(FCodeAnalysisState &state, uint16_t pc, LabelType label);
void RunStaticCodeAnalysis(FCodeAnalysisState &state, uint16_t pc);
void RegisterCodeExecuted(FCodeAnalysisState &state, uint16_t pc);
void ReAnalyseCode(FCodeAnalysisState &state);
void GenerateGlobalInfo(FCodeAnalysisState &state);
void RegisterDataAccess(FCodeAnalysisState &state, uint16_t pc, uint16_t dataAddr, bool bWrite);
void UpdateCodeInfoForAddress(FCodeAnalysisState &state, uint16_t pc);
void ResetMemoryLogs(FCodeAnalysisState &state);

// Commands
void Undo(FCodeAnalysisState &state);

FLabelInfo* AddLabel(FCodeAnalysisState& state, uint16_t address, const char* name, LabelType type);
void AddLabelAtAddress(FCodeAnalysisState &state, uint16_t address);
void RemoveLabelAtAddress(FCodeAnalysisState &state, uint16_t address);
void SetLabelName(FCodeAnalysisState &state, FLabelInfo *pLabel, const char *pText);
void SetItemCode(FCodeAnalysisState &state, FItem *pItem);
void SetItemData(FCodeAnalysisState &state, FItem *pItem);
void SetItemText(FCodeAnalysisState &state, FItem *pItem);
void SetItemCommentText(FCodeAnalysisState &state, FItem *pItem, const char *pText);


bool OutputCodeAnalysisToTextFile(FCodeAnalysisState &state, const char *pTextFileName, uint16_t startAddr, uint16_t endAddr);