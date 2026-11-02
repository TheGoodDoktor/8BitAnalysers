#pragma once

#include "../CodeAnalyserTypes.h"
#include "Util/Misc.h"

enum class EGraphicsViewMode;

struct FCodeInfo;
class FCodeAnalysisState;

bool DrawNumberTypeCombo(const char* pLabel, ENumberDisplayMode& numberMode);
bool DrawOperandTypeCombo(const char* pLabel, FCodeInfo* pCodeInfo);
bool DrawOperandInfoCombo(const char* pLabel, FOperandInfo& operandInfo); // sam. Added this
bool DrawDataTypeCombo(const char* pLabel, EDataType& dataType, const bool* pSupportedDataTypes = nullptr); // sam. pSupportedDataTypes: optional bool[(int)EDataType::Max] to restrict which types are offered.
bool DrawDataDisplayTypeCombo(const char* pLabel, EDataItemDisplayType& displayType, const FCodeAnalysisState& state);
bool DrawBitmapFormatCombo(EBitmapFormat& bitmapFormat, const FCodeAnalysisState& state);
bool DrawPaletteCombo(const char* pLabel, const char* pFirstItemLabel, int& paletteEntryIndex, int numColours = -1);
bool DrawGraphicsViewModeCombo(const char* pLabel, EGraphicsViewMode& viewMode);
