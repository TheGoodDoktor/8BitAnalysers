#pragma once

#include "Misc/GlobalConfig.h"

enum class ECPCModel;

// CPC specific config
struct FCPCConfig : public FGlobalConfig
{
	FCPCConfig();
	bool Init(void) override;
	
	const char* GetUpperROMSlotName(int slotIndex) const { return slotIndex < UpperROMSlot.size() ? UpperROMSlot[slotIndex].c_str() : nullptr; }

	ECPCModel GetDefaultModel() const;

	std::vector<std::string>	UpperROMSlot;
	bool bDefaultMachineIs6128 = true;

protected:

	void ReadFromJson(const nlohmann::json& jsonConfigFile) override;
	void WriteToJson(nlohmann::json& jsonConfigFile) const override;
};
