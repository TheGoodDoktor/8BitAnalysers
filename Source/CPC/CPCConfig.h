#pragma once

#include "Misc/GlobalConfig.h"

// CPC specific config
struct FCPCConfig : public FGlobalConfig
{
	FCPCConfig();
	bool Init(void) override;

	std::string						SnapshotFolder128 = "./Games128/";
	std::vector<std::string>	UpperROMSlot;
	const char* GetUpperROMSlotName(int slotIndex) const { return slotIndex < UpperROMSlot.size() ? UpperROMSlot[slotIndex].c_str() : nullptr; }
protected:

	void ReadFromJson(const nlohmann::json& jsonConfigFile) override;
	void WriteToJson(nlohmann::json& jsonConfigFile) const override;
};
