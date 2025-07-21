#include "DisplayTypes.h"
#include "../CodeAnalyser.h"

#include <imgui.h>
#include "CodeAnalyserUI.h"

class FUnknownType : public FDisplayTypeBase
{
public:
	FUnknownType() : FDisplayTypeBase("Unknown") {}
	virtual void DrawValue(FCodeAnalysisState& state, uint16_t value) const override
	{
		ImGui::Text("%s", NumStr(value));
	}
};

class FDecimalNumberType : public FDisplayTypeBase
{
public:
	FDecimalNumberType() : FDisplayTypeBase("Decimal") {}
	virtual void DrawValue(FCodeAnalysisState& state, uint16_t value) const override
	{
		ImGui::Text("%d", value);
	}
};

class FHexNumberType : public FDisplayTypeBase
{
public:
	FHexNumberType() : FDisplayTypeBase("Hex") {}
	virtual void DrawValue(FCodeAnalysisState& state, uint16_t value) const override
	{
		ImGui::Text("%04X", value);
	}
};

class FAddressType : public FDisplayTypeBase
{
public:
	FAddressType() : FDisplayTypeBase("Address") {}
	virtual void DrawValue(FCodeAnalysisState& state, uint16_t value) const override
	{
		DrawAddressLabel(state, state.GetFocussedViewState(), state.AddressRefFromPhysicalAddress(value));
	}
};

class FXPosType : public FDisplayTypeBase
{
public:
	FXPosType() : FDisplayTypeBase("XPos") {}
	virtual void DrawValue(FCodeAnalysisState& state, uint16_t value) const override
	{
		ImGui::Text("%d", value);
		if (ImGui::IsItemHovered())
			state.XPosHighlight = value;
	}
};

class FYPosType : public FDisplayTypeBase
{	
public:
	FYPosType() : FDisplayTypeBase("YPos") {}
	virtual void DrawValue(FCodeAnalysisState& state, uint16_t value) const override
	{
		ImGui::Text("%d", value);
		if (ImGui::IsItemHovered())
			state.YPosHighlight = value;
	}
};

class FXCharPosType : public FDisplayTypeBase
{	
public:
	FXCharPosType() : FDisplayTypeBase("XCharPos") {}
	virtual void DrawValue(FCodeAnalysisState& state, uint16_t value) const override
	{
		ImGui::Text("%d", value);
		if (ImGui::IsItemHovered())
			state.XPosHighlight = value * 8;
	}
};

class FYCharPosType : public FDisplayTypeBase
{
public:
	FYCharPosType() : FDisplayTypeBase("YCharPos") {}
	virtual void DrawValue(FCodeAnalysisState& state, uint16_t value) const override
	{
		ImGui::Text("%d", value);
		if (ImGui::IsItemHovered())
			state.YPosHighlight = value * 8;
	}
};
class FXYPosType : public FDisplayTypeBase
{
public:
	FXYPosType() : FDisplayTypeBase("XYPos") {}
	virtual void DrawValue(FCodeAnalysisState& state, uint16_t value) const override
	{
		ImGui::Text("%d,%d", value & 0xFF, (value >> 8) & 0xFF);
		if (ImGui::IsItemHovered())
		{
			state.XPosHighlight = value & 0xFF;
			state.YPosHighlight = (value >> 8) & 0xFF;
		}
	}
};

class FXYCharPosType : public FDisplayTypeBase
{
public:
	FXYCharPosType() : FDisplayTypeBase("XYCharPos") {}
	virtual void DrawValue(FCodeAnalysisState& state, uint16_t value) const override
	{
		ImGui::Text("%d,%d", value & 0xFF, (value >> 8) & 0xFF);
		if (ImGui::IsItemHovered())
		{
			state.XPosHighlight = (value & 0xFF) * 8;
			state.YPosHighlight = ((value >> 8) & 0xFF) * 8;
		}
	}
};

class FX2YPosType : public FDisplayTypeBase
{
public:
	FX2YPosType() : FDisplayTypeBase("X2YPos") {}
	virtual void DrawValue(FCodeAnalysisState& state, uint16_t value) const override
	{
		ImGui::Text("%d,%d", (value & 0xFF) * 2, (value >> 8) & 0xFF);
		if (ImGui::IsItemHovered())
		{
			state.XPosHighlight = (value & 0xFF) * 2;
			state.YPosHighlight = (value >> 8) & 0xFF;
		}
	}
};

// map name to type
std::map<std::string, const FDisplayTypeBase*> g_DisplayTypeMap;
// linear list of types for combo box
std::vector<const FDisplayTypeBase*> g_DisplayTypeList;

bool RegisterDisplayType(const FDisplayTypeBase* pType)
{
	if (g_DisplayTypeMap.find(pType->GetTypeName()) != g_DisplayTypeMap.end())
	{
		return false; // already registered
	}
	g_DisplayTypeMap[pType->GetTypeName()] = pType;
	g_DisplayTypeList.push_back(pType);
	return true;
}

const FDisplayTypeBase* GetDisplayType(const char* name)
{
	auto it = g_DisplayTypeMap.find(name);
	if (it != g_DisplayTypeMap.end())
	{
		return it->second;
	}
	return nullptr;
}

// combo box of types
bool DrawDisplayTypeComboBox(const FDisplayTypeBase** pChangeType)
{
	bool bChanged = false;

	if (ImGui::BeginCombo("##DisplayType", (*pChangeType)->GetTypeName()))
	{
		for (const FDisplayTypeBase* pType : g_DisplayTypeList)
		{
			if (ImGui::Selectable(pType->GetTypeName(), *pChangeType == pType))
			{
				*pChangeType = pType;
				bChanged = true;
			}
		}
		ImGui::EndCombo();
	}

	return bChanged;
}

void RegisterBasicDisplayTypes()
{
	RegisterDisplayType(new FUnknownType());
	RegisterDisplayType(new FDecimalNumberType());
	RegisterDisplayType(new FHexNumberType());
	RegisterDisplayType(new FAddressType());
	RegisterDisplayType(new FXPosType());
	RegisterDisplayType(new FYPosType());
	RegisterDisplayType(new FXCharPosType());
	RegisterDisplayType(new FYCharPosType());
	RegisterDisplayType(new FXYPosType());
	RegisterDisplayType(new FXYCharPosType());
	RegisterDisplayType(new FX2YPosType());
}


// util 
const FDisplayTypeBase* GetDisplayTypeFromEnum(EDataItemDisplayType type)
{
	switch (type)
	{
	case EDataItemDisplayType::Unknown: 
		return GetDisplayType("Unknown");
	case EDataItemDisplayType::Decimal:
		return GetDisplayType("Decimal");
	case EDataItemDisplayType::Hex:
		return GetDisplayType("Hex");
	}
	return nullptr;
}
