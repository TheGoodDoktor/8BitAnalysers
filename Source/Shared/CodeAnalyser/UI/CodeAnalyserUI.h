#pragma once
#include <cstdint>

struct FCodeAnalysisState;
struct FCodeAnalysisViewState;
struct FDataInfo;
struct FItem;
class FGraphicsView;



// UI
void DrawCodeAddress(FCodeAnalysisState& state, FCodeAnalysisViewState& viewState, uint16_t addr, bool bFunctionRel = false);
void DrawAddressLabel(FCodeAnalysisState& state, FCodeAnalysisViewState& viewState, uint16_t addr, bool bFunctionRel = false);
int GetItemIndexForAddress(const FCodeAnalysisState& state, uint16_t addr);
void DrawCodeAnalysisItemAtIndex(FCodeAnalysisState& state, FCodeAnalysisViewState& viewState, int i);


void DrawCodeAnalysisData(FCodeAnalysisState &state, int windowId);
void DrawGlobals(FCodeAnalysisState& state, FCodeAnalysisViewState& viewState);
void DrawCallStack(FCodeAnalysisState& state);
void DrawTrace(FCodeAnalysisState& state);
void DrawRegisters(FCodeAnalysisState& state);
void DrawWatchWindow(FCodeAnalysisState& state);

float DrawDataBinary(FCodeAnalysisState& state, const FDataInfo* pDataInfo);
void DrawDataInfo(FCodeAnalysisState& state, FCodeAnalysisViewState& viewState, const FDataInfo* pDataInfo, bool bDrawLabel = false, bool bEdit = true);
void DrawDataDetails(FCodeAnalysisState &state, FCodeAnalysisViewState& viewState, FDataInfo *pDataInfo);

void CodeAnalyserGoToAddress(FCodeAnalysisViewState& state, uint16_t newAddress, bool bLabel = false);
void DrawComment(const FItem* pItem, float offset = 0.0f);

