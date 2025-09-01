#pragma once
#include <cstdint>

#include "../CodeAnalyserTypes.h"

struct FCodeAnalysisItem;
class FCodeAnalysisState;
struct FCodeAnalysisViewState;
struct FDataInfo;
struct FItem;
class FGraphicsView;

enum class ENumberDisplayMode;
enum class EDataItemDisplayType;
enum class EOperandType;


class FMemoryRegionDescGenerator
{
public:
	bool	InRegion(FAddressRef addr) const
	{
		return (addr.GetBankId() == RegionBankId || RegionBankId == -1) && addr.GetAddress() >= RegionMin && addr.GetAddress() <= RegionMax;
	}
    void SetRegionBankId(int16_t bankId) { RegionBankId = bankId; }

	virtual const char* GenerateAddressString(FAddressRef addr) = 0;

	virtual void FrameTick() {}
    
protected:
	int16_t		RegionBankId = -1;	// -1 means any bank (needed for regions that span banks)
	uint16_t	RegionMin = 0xffff;
	uint16_t	RegionMax = 0;
};

const uint32_t kAddressLabelFlag_NoBank		= 0x0001;
const uint32_t kAddressLabelFlag_NoBrackets	= 0x0002;
const uint32_t kAddressLabelFlag_White		= 0x0004;

// UI
void ResetRegionDescs(void);
bool AddMemoryRegionDescGenerator(FMemoryRegionDescGenerator* pGen);
void UpdateRegionDescs(void);

void ShowCodeAccessorActivity(FCodeAnalysisState& state, const FAddressRef accessorCodeAddr);
void DrawCodeAddress(FCodeAnalysisState& state, FCodeAnalysisViewState& viewState, FAddressRef addr, uint32_t displayFlags = 0);
std::string GenerateAddressLabelString(FCodeAnalysisState& state, FAddressRef addr);
bool DrawAddressLabel(FCodeAnalysisState& state, FCodeAnalysisViewState& viewState, uint16_t addr, uint32_t displayFlags = 0);
bool DrawAddressLabel(FCodeAnalysisState& state, FCodeAnalysisViewState& viewState, FAddressRef addr, uint32_t displayFlags = 0);
int GetItemIndexForAddress(const FCodeAnalysisState& state, FAddressRef addr);
void DrawCodeAnalysisItem(FCodeAnalysisState& state, FCodeAnalysisViewState& viewState, const FCodeAnalysisItem& item);
void DrawPalette(const uint32_t* palette, int numColours, float height = 0.f);
EDataItemDisplayType GetDisplayTypeForBitmapFormat(EBitmapFormat bitmapFormat);

void DrawSnippetToolTip(FCodeAnalysisState& state, FCodeAnalysisViewState& viewState, const FAddressRef addr, int noLines = 10);

void DrawCodeAnalysisData(FCodeAnalysisState &state, int windowId);
void DrawGlobals(FCodeAnalysisState& state, FCodeAnalysisViewState& viewState);

float DrawDataBinary(FCodeAnalysisState& state, const FDataInfo* pDataInfo);
void DrawDataInfo(FCodeAnalysisState& state, FCodeAnalysisViewState& viewState, const FCodeAnalysisItem &item, bool bDrawLabel = false, bool bEdit = true);
void DrawDataDetails(FCodeAnalysisState &state, FCodeAnalysisViewState& viewState, const FCodeAnalysisItem& item);
void ShowDataItemActivity(FCodeAnalysisState& state, FAddressRef addr);
void ShowDataItemReadActivity(FCodeAnalysisState& state, FAddressRef addr);
void ShowDataItemWriteActivity(FCodeAnalysisState& state, FAddressRef addr);
void DrawDataAccesses(FCodeAnalysisState& state, FCodeAnalysisViewState& viewState, const FDataInfo* pDataInfo);

void DrawInlineComment(FCodeAnalysisState& state, FCodeAnalysisViewState& viewState, const FItem* pItem);
void DrawComment(FCodeAnalysisState& state, FCodeAnalysisViewState& viewState, const FItem* pItem, float offset = 0.0f, bool bIsInline = true);

// util functions - move?
bool DrawU8Input(const char* label, uint8_t* value);
bool DrawAddressInput(FCodeAnalysisState& state, const char* label, FAddressRef& address);
bool DrawAddressInput(const char* label, uint16_t* value);
const char* GetBankText(const FCodeAnalysisState& state, int16_t bankId);
bool DrawBankInput(const FCodeAnalysisState& state, const char* label, int16_t& bankId, bool bAllowNone = false);
EBitmapFormat GetBitmapFormatForDisplayType(EDataItemDisplayType displayType);
int GetBppForBitmapFormat(EBitmapFormat bitmapFormat);
int GetNumColoursForBitmapFormat(EBitmapFormat bitmapFormat);
bool BitmapFormatHasPalette(EBitmapFormat bitmapFormat);
bool IsBitmapFormatDoubleWidth(EBitmapFormat bitmapFormat);

// config - move?
void DrawCodeAnalysisConfigWindow(FCodeAnalysisState& state);

namespace Markup
{ 
void SetCodeInfo(const FCodeInfo* pCodeInfo);
bool DrawText(FCodeAnalysisState& state, FCodeAnalysisViewState& viewState, const char* pText);
std::string ExpandString(FCodeAnalysisState& state, const char* pText);
}


