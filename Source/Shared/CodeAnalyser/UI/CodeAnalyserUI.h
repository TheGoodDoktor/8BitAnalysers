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
enum class EOperandType;


class FMemoryRegionDescGenerator
{
public:
	bool	InRegion(uint16_t addr) const
	{
		return addr >= RegionMin && addr <= RegionMax;
	}

	virtual const char* GenerateAddressString(uint16_t addr) = 0;
protected:
	uint16_t	RegionMin;
	uint16_t	RegionMax;
};


// UI

bool AddMemoryRegionDescGenerator(FMemoryRegionDescGenerator* pGen);

void ShowCodeAccessorActivity(FCodeAnalysisState& state, const FAddressRef accessorCodeAddr);
//void DrawCodeAddress(FCodeAnalysisState& state, FCodeAnalysisViewState& viewState, uint16_t addr, bool bFunctionRel = false);
void DrawCodeAddress(FCodeAnalysisState& state, FCodeAnalysisViewState& viewState, FAddressRef addr, bool bFunctionRel = false);
void DrawAddressLabel(FCodeAnalysisState& state, FCodeAnalysisViewState& viewState, uint16_t addr, bool bFunctionRel = false);
void DrawAddressLabel(FCodeAnalysisState& state, FCodeAnalysisViewState& viewState, FAddressRef addr, bool bFunctionRel = false);
int GetItemIndexForAddress(const FCodeAnalysisState& state, FAddressRef addr);
void DrawCodeAnalysisItem(FCodeAnalysisState& state, FCodeAnalysisViewState& viewState, const FCodeAnalysisItem& item);
bool DrawNumberTypeCombo(const char* pLabel, ENumberDisplayMode& numberMode);
bool DrawOperandTypeCombo(const char* pLabel, EOperandType& operandType);

void DrawCodeAnalysisData(FCodeAnalysisState &state, int windowId);
void DrawGlobals(FCodeAnalysisState& state, FCodeAnalysisViewState& viewState);
void DrawStackInfo(FCodeAnalysisState& state);
void DrawTrace(FCodeAnalysisState& state);
void DrawRegisters(FCodeAnalysisState& state);
void DrawWatchWindow(FCodeAnalysisState& state);

float DrawDataBinary(FCodeAnalysisState& state, const FDataInfo* pDataInfo);
void DrawDataInfo(FCodeAnalysisState& state, FCodeAnalysisViewState& viewState, const FCodeAnalysisItem &item, bool bDrawLabel = false, bool bEdit = true);
void DrawDataDetails(FCodeAnalysisState &state, FCodeAnalysisViewState& viewState, const FCodeAnalysisItem& item);
void ShowDataItemActivity(FCodeAnalysisState& state, FAddressRef addr);

void DrawComment(const FItem* pItem, float offset = 0.0f);

// util functions - move?
bool DrawU8Input(const char* label, uint8_t* value);
bool DrawAddressInput(const char* label, FAddressRef& address);
bool DrawAddressInput(const char* label, uint16_t* value);