#pragma once

#include <cstdint>
#include <string>

class FCodeAnalysisState;

// base class for all display types
class FDisplayTypeBase
{
public:
	FDisplayTypeBase(const char* name) :TypeName(name) {}
	virtual ~FDisplayTypeBase() {};

	virtual void DrawValue(FCodeAnalysisState& state, uint16_t value) const = 0;

	const char* GetTypeName() const
	{
		return TypeName.c_str();
	}
private:
	std::string TypeName;
};

bool RegisterDisplayType(const FDisplayTypeBase* pType);
const FDisplayTypeBase* GetDisplayType(const char* name);
bool DrawDisplayTypeComboBox(const FDisplayTypeBase** pType);

void RegisterBasicDisplayTypes();
