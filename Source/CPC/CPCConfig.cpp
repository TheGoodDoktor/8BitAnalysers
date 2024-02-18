#include "CPCConfig.h"

#include "json.hpp"
#include "ExternalROMSupport.h"
#include "Util/FileUtil.h"

FCPCConfig::FCPCConfig()
{
	UpperROMSlot.resize(kNumUpperROMSlots);
}

bool FCPCConfig::Init(void)
{
	if (FGlobalConfig::Init() == false)
		return false;

	LuaBaseFiles.push_back("Lua/CPCBase.lua");
	
	return true;
}

void FCPCConfig::ReadFromJson(const nlohmann::json& jsonConfigFile)
{
	FGlobalConfig::ReadFromJson(jsonConfigFile);

	if (jsonConfigFile.contains("SnapshotFolder128"))
		SnapshotFolder128 = jsonConfigFile["SnapshotFolder128"];

	// fixup paths
	if (SnapshotFolder128.back() != '/')
		SnapshotFolder128 += "/";

	for (int i = 1; i < UpperROMSlot.size(); i++)
	{
		char temp[32] = { 0 };
		sprintf(temp, "UpperROMSlot%02d", i);
		
		if (jsonConfigFile.contains(temp))
		{
			UpperROMSlot[i] = jsonConfigFile[temp];
		}
	}
}

void FCPCConfig::WriteToJson(nlohmann::json& jsonConfigFile) const
{
	FGlobalConfig::WriteToJson(jsonConfigFile);
	jsonConfigFile["SnapshotFolder128"] = SnapshotFolder128;

	for (int i = 1; i < UpperROMSlot.size(); i++)
	{
		char temp[32] = { 0 };
		sprintf(temp, "UpperROMSlot%02d", i);
		jsonConfigFile[temp] = UpperROMSlot[i].c_str();
	}
}