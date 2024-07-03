#include "ComboBoxes.h"

#include <vector>
#include <imgui.h>
#include <imgui_internal.h>
#include <functional>

#include "CodeAnalyserUI.h"
#include "Util/Misc.h"
#include "../CodeAnalyser.h"
#include "Util/GraphicsView.h"

#include "GraphicsViewer.h"

// Type combo boxes
// TODO: move to its own file

// Generic combo function for enums
template <typename EnumType>
bool DrawEnumCombo(const char* pLabel,
	EnumType& operandType,
	const std::vector<std::pair<const char*, EnumType>>& enumLookup,
	std::function<bool(EnumType)> validEnumFunc = [](EnumType) {return true; }
)
{
	bool bChanged = false;
	const char* pPreviewStr = nullptr;
	for (const auto& val : enumLookup)
	{
		if (val.second == operandType)
		{
			pPreviewStr = val.first;
			break;
		}
	}

	assert(pPreviewStr != nullptr);
	if (ImGui::BeginCombo(pLabel, pPreviewStr))
	{
		for (int n = 0; n < (int)enumLookup.size(); n++)
		{
			if (validEnumFunc(enumLookup[n].second))
			{
				const bool isSelected = (operandType == enumLookup[n].second);
				if (ImGui::Selectable(enumLookup[n].first, isSelected))
				{
					operandType = enumLookup[n].second;
					bChanged = true;
				}
			}
		}
		ImGui::EndCombo();
	}

	return bChanged;
}

static const std::vector<std::pair<const char*, ENumberDisplayMode>> g_NumberTypes =
{
	{ "None",       ENumberDisplayMode::None },
	{ "Decimal",    ENumberDisplayMode::Decimal },
	{ "$ Hex",      ENumberDisplayMode::HexDollar },
	{ "Hex h",      ENumberDisplayMode::HexAitch },
};

bool DrawNumberTypeCombo(const char* pLabel, ENumberDisplayMode& numberMode)
{
	return DrawEnumCombo<ENumberDisplayMode>(pLabel, numberMode, g_NumberTypes);
}

static const std::vector<std::pair<const char*, EOperandType>> g_OperandTypes =
{
	{ "Unknown" ,		EOperandType::Unknown},
	{ "Number",			EOperandType::SignedNumber},
	{ "Pointer" ,		EOperandType::Pointer},
	{ "Jump Address",	EOperandType::JumpAddress},
	{ "Decimal",		EOperandType::Decimal},
	{ "Hex",			EOperandType::Hex},
	{ "Binary",			EOperandType::Binary},
	{ "Unsigned Number",			EOperandType::UnsignedNumber},
	{ "Ascii",			EOperandType::Ascii},
	//{ "Struct",			EOperandType::Struct},
	//{ "Signed Number",	EOperandType::SignedNumber},
};

bool IsOperandTypeSupported(EOperandType operandType, const FCodeInfo* pCodeInfo)
{
	switch (operandType)
	{
	case EOperandType::Pointer:
	case EOperandType::JumpAddress:
		return pCodeInfo->OperandAddress.IsValid();
	default:
		return true;
	}
}

bool DrawOperandTypeCombo(const char* pLabel, FCodeInfo* pCodeInfo)
{
	return DrawEnumCombo<EOperandType>(pLabel, pCodeInfo->OperandType, g_OperandTypes,
		[pCodeInfo](EOperandType operandType) { return IsOperandTypeSupported(operandType, pCodeInfo); });
}

bool IsDisplayTypeSupported(EDataItemDisplayType displayType, const FCodeAnalysisState& state)
{
	switch (displayType)
	{
	case EDataItemDisplayType::ColMap2Bpp_CPC:
		return state.Config.bSupportedBitmapTypes[(int)EBitmapFormat::ColMap2Bpp_CPC];
	case EDataItemDisplayType::ColMap4Bpp_CPC:
		return state.Config.bSupportedBitmapTypes[(int)EBitmapFormat::ColMap4Bpp_CPC];
	case EDataItemDisplayType::ColMapMulticolour_C64:
		return state.Config.bSupportedBitmapTypes[(int)EBitmapFormat::ColMapMulticolour_C64];
	default:
		return true;
	}
}

static const std::vector<std::pair<const char*, EDataItemDisplayType>> g_DisplayTypes =
{
	{ "Unknown" ,		EDataItemDisplayType::Unknown},
	{ "Signed Number",		EDataItemDisplayType::SignedNumber},
	{ "Unsigned Number",	EDataItemDisplayType::UnsignedNumber},
	{ "Pointer" ,		EDataItemDisplayType::Pointer},
	{ "JumpAddress",	EDataItemDisplayType::JumpAddress},
	{ "Decimal",		EDataItemDisplayType::Decimal},
	{ "Hex",			EDataItemDisplayType::Hex},
	{ "Ascii",			EDataItemDisplayType::Ascii},
	{ "Binary",			EDataItemDisplayType::Binary},
	{ "Bitmap",			EDataItemDisplayType::Bitmap},
	{ "ColMap2Bpp CPC",			EDataItemDisplayType::ColMap2Bpp_CPC},
	{ "ColMap4Bpp CPC",			EDataItemDisplayType::ColMap4Bpp_CPC},
	{ "Multicolour C64",			EDataItemDisplayType::ColMapMulticolour_C64},

};

bool DrawDataDisplayTypeCombo(const char* pLabel, EDataItemDisplayType& displayType, const FCodeAnalysisState& state)
{

	return DrawEnumCombo<EDataItemDisplayType>(pLabel, displayType, g_DisplayTypes, [&state](EDataItemDisplayType type) { return IsDisplayTypeSupported(type, state); });
}

static const std::vector<std::pair<const char*, EDataType>> g_DataTypes =
{
	{ "Byte",           EDataType::Byte },
	{ "Byte Array",     EDataType::ByteArray },
	{ "Word",           EDataType::Word },
	{ "Word Array",     EDataType::WordArray } ,
	{ "Bitmap",         EDataType::Bitmap },
	{ "Char Map",       EDataType::CharacterMap },
	{ "Col Attr",       EDataType::ColAttr },
	{ "Text",           EDataType::Text },
	{ "Struct",         EDataType::Struct },
};

bool DrawDataTypeCombo(const char* pLabel, EDataType& displayType)
{
	return DrawEnumCombo<EDataType>(pLabel, displayType, g_DataTypes);
}

static const std::vector<std::pair<const char*, EDataTypeFilter>> g_DataFilterTypes =
{
	{ "All",        EDataTypeFilter::All },
	{ "Pointer",    EDataTypeFilter::Pointer },
	{ "Text",       EDataTypeFilter::Text },
	{ "Bitmap",     EDataTypeFilter::Bitmap },
	{ "Char Map",   EDataTypeFilter::CharacterMap },
	{ "Col Attr",   EDataTypeFilter::ColAttr },
};

bool DrawDataTypeFilterCombo(const char* pLabel, EDataTypeFilter& filterType)
{
	return DrawEnumCombo<EDataTypeFilter>(pLabel, filterType, g_DataFilterTypes);
}

bool DrawBitmapFormatCombo(EBitmapFormat& bitmapFormat, const FCodeAnalysisState& state)
{
	assert(bitmapFormat < EBitmapFormat::Max);

	int numFormatsSupported = 0;
	for (int n = 0; n < (int)EBitmapFormat::Max; n++)
		numFormatsSupported += state.Config.bSupportedBitmapTypes[n] ? 1 : 0;

	if (numFormatsSupported == 1)
	{
		// don't draw the combo box if there is only 1 format supported
		return false;
	}

	const int index = (int)bitmapFormat;
	const char* bitmapFormats[] = { "1bpp", "2bpp (CPC Mode 1)", "4bpp (CPC Mode 0)", "C64 Multicolour" };

	bool bChanged = false;

	if (ImGui::BeginCombo("Bitmap Format", bitmapFormats[index]))
	{
		for (int n = 0; n < IM_ARRAYSIZE(bitmapFormats); n++)
		{
			if (state.Config.bSupportedBitmapTypes[n])
			{
				const bool isSelected = (index == n);
				if (ImGui::Selectable(bitmapFormats[n], isSelected))
				{
					bitmapFormat = (EBitmapFormat)n;
					bChanged = true;
				}
			}
		}
		ImGui::EndCombo();
	}
	return bChanged;
}

static const std::vector<std::pair<const char*, EGraphicsViewMode>> g_GraphicsViewModeTypes =
{
	{ "Bitmap",			EGraphicsViewMode::Bitmap },
	{ "Bitmap Chars",	EGraphicsViewMode::BitmapChars },
	{ "Bitmap Winding",	EGraphicsViewMode::BitmapWinding },
};

bool DrawGraphicsViewModeCombo(const char* pLabel, EGraphicsViewMode& viewMode)
{
	return DrawEnumCombo<EGraphicsViewMode>(pLabel, viewMode, g_GraphicsViewModeTypes);

}

bool DrawPaletteCombo(const char* pLabel, const char* pFirstItemLabel, int& paletteEntryIndex, int numColours /* = -1 */)
{
	int index = paletteEntryIndex;

	bool bChanged = false;
	if (ImGui::BeginCombo(pLabel, nullptr, ImGuiComboFlags_CustomPreview))
	{
		if (ImGui::Selectable(pFirstItemLabel, index == -1))
		{
			paletteEntryIndex = -1;
		}

		const int numPalettes = GetNoPaletteEntries();
		for (int p = 0; p < numPalettes; p++)
		{
			if (const FPaletteEntry* pEntry = GetPaletteEntry(p))
			{
				if (numColours == -1 || pEntry->NoColours == numColours)
				{
					const std::string str = "Palette " + std::to_string(p);
					const bool isSelected = (index == p);
					if (ImGui::Selectable(str.c_str(), isSelected))
					{
						paletteEntryIndex = p;
						bChanged = true;
					}

					const uint32_t* pPalette = GetPaletteFromPaletteNo(p);
					if (pPalette)
					{
						ImGui::SameLine();
						DrawPalette(pPalette, pEntry->NoColours);
					}
				}
			}
		}

		ImGui::EndCombo();
	}

	const std::string palettePreview = "Palette " + std::to_string(index);
	const char* pComboPreview = index == -1 ? pFirstItemLabel : palettePreview.c_str();
	if (ImGui::BeginComboPreview())
	{
		ImGui::Text("%s", pComboPreview);
		if (const FPaletteEntry* pEntry = GetPaletteEntry(index))
		{
			const uint32_t* pPalette = GetPaletteFromPaletteNo(index);
			if (pPalette)
			{
				ImGui::SameLine();
				DrawPalette(pPalette, numColours);
			}
		}
		ImGui::EndComboPreview();
	}
	return bChanged;
}
