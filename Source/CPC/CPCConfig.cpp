#include "CPCConfig.h"

#include "CPCEmu.h"
#include "json.hpp"
#include "ExternalROMSupport.h"
#include "Util/FileUtil.h"

FCPCConfig::FCPCConfig()
{
	UpperROMSlot.resize(kNumUpperROMSlots);
}

ECPCModel FCPCConfig::GetDefaultModel() const
{
	return bDefaultMachineIs6128 ? ECPCModel::CPC_6128 : ECPCModel::CPC_464;
}

bool FCPCConfig::Init(void)
{
	if (FGlobalConfig::Init() == false)
		return false;

	LuaBaseFiles.push_back("Lua/CPCBase.lua");
	SnapshotFolder = GetDocumentsPath("CPCGames");
	WorkspaceRoot = GetDocumentsPath("CPCAnalyserProjects");

	FixupPaths();

	return true;
}

void FCPCConfig::ReadFromJson(const nlohmann::json& jsonConfigFile)
{
	FGlobalConfig::ReadFromJson(jsonConfigFile);

	if (jsonConfigFile.contains("DefaultMachineIs6128"))
		bDefaultMachineIs6128 = jsonConfigFile["DefaultMachineIs6128"];

	for (int i = 1; i < UpperROMSlot.size(); i++)
	{
		char temp[32] = { 0 };
		sprintf(temp, "UpperROMSlot%02d", i);
		
		if (jsonConfigFile.contains(temp))
		{
			UpperROMSlot[i] = jsonConfigFile[temp];
		}
	}

	FixupPaths();
}

void FCPCConfig::WriteToJson(nlohmann::json& jsonConfigFile) const
{
	FGlobalConfig::WriteToJson(jsonConfigFile);

	jsonConfigFile["DefaultMachineIs6128"] = bDefaultMachineIs6128;

	for (int i = 1; i < UpperROMSlot.size(); i++)
	{
		char temp[32] = { 0 };
		sprintf(temp, "UpperROMSlot%02d", i);
		jsonConfigFile[temp] = UpperROMSlot[i].c_str();
	}
}