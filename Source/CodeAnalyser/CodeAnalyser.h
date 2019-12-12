#pragma once
#include <cstdint>
#include <string>
#include <map>
#include <vector>


enum class LabelType;
struct FSpeccyUI;
struct FSpeccy;

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
	uint16_t		Address;
	uint16_t		ByteSize;
	int				FrameLastAccessed = -1;
};

struct FLabelInfo : FItem
{
	FLabelInfo() { Type = ItemType::Label; }

	std::string				Name;
	LabelType				LabelType;
	std::map<uint16_t, int>	References;
};

struct FCodeInfo : FItem
{
	FCodeInfo() :FItem() { Type = ItemType::Code; }

	std::string		Text;				// Disassembly text
	uint16_t		JumpAddress = 0;	// optional jump address
	uint16_t		PointerAddress = 0;	// optional pointer address
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

	std::map<uint16_t, int>	References;	// address and counts of data access instructions
};


enum class Key
{
	SetItemData,
	SetItemText,
	SetItemCode,

	AddLabel,
	Rename,

	Count
};

// code analysis information
struct FCodeAnalysisState
{
	FSpeccy *				pSpeccy = nullptr;
	int						CurrentFrameNo = 0;

	static const int kAddressSize = 1 << 16;
	FLabelInfo*				Labels[kAddressSize];
	FCodeInfo*				CodeInfo[kAddressSize];
	FDataInfo*				DataInfo[kAddressSize];
	bool					bCodeAnalysisDataDirty = false;

	std::vector< FItem *>	ItemList;
	FItem*					pCursorItem = nullptr;
	int						CursorItemIndex = -1;
	int						GoToAddress = -1;
	std::vector<uint16_t>	AddressStack;

	int						KeyConfig[(int)Key::Count];

	std::vector< class FCommand *>	CommandStack;

};

// Commands
class FCommand
{
public:
	virtual void Do(FCodeAnalysisState &state) = 0;
	virtual void Undo(FCodeAnalysisState &state) = 0;
};


// Analysis
void InitialiseCodeAnalysis(FCodeAnalysisState &state, FSpeccy* pSpeccy);
bool GenerateLabelForAddress(FCodeAnalysisState &state, uint16_t pc, LabelType label);
void RunStaticCodeAnalysis(FCodeAnalysisState &state, uint16_t pc);
void ReAnalyseCode(FCodeAnalysisState &state);

// Commands
void Undo(FCodeAnalysisState &state);

void AddLabelAtAddress(FCodeAnalysisState &state, uint16_t address); 
void SetLabelName(FCodeAnalysisState &state, FLabelInfo *pLabel, const char *pText);
void SetItemData(FCodeAnalysisState &state, FItem *pItem);
void SetItemText(FCodeAnalysisState &state, FItem *pItem);
void SetItemCommentText(FCodeAnalysisState &state, FItem *pItem, const char *pText);
