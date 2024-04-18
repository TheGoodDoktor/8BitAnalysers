#include "GameConfig.h"

#include "Util/FileUtil.h"
#include "json.hpp"
//#include "magic_enum.hpp"
#include <iomanip>
#include <fstream>
#include <sstream>

#include "Debug/DebugLog.h"
#include "Util/Misc.h"
#include <Util/GraphicsView.h>

using json = nlohmann::json;
static std::vector< FProjectConfig *>	g_GameConfigs;

bool AddGameConfig(FProjectConfig *pConfig)
{
	for (const auto& pGameConfig : GetGameConfigs())
	{
		// Dont add game configs with identical names
		if (pGameConfig->Name == pConfig->Name)
			return false;
	}

	g_GameConfigs.push_back(pConfig);
	return true;
}

bool RemoveGameConfig(const char* pName)
{
	for (std::vector< FProjectConfig*>::iterator it = g_GameConfigs.begin(); it != g_GameConfigs.end(); ++it)
	{
		FProjectConfig* pConfig = *it;
		if (pConfig->Name == pName)
		{
			g_GameConfigs.erase(it);
			return true;
		}
	}
	return false;
}

const std::vector< FProjectConfig *>& GetGameConfigs()
{
	return g_GameConfigs;
}

FProjectConfig* GetGameConfigForName(const char* pGameName)
{
	for (const auto& pGameConfig : g_GameConfigs)
	{
		if (pGameConfig->Name == pGameName)
			return pGameConfig;
	}
	return nullptr;
}

FProjectConfig* GetGameConfigForSnapshot(const char* pSnapshotName)
{
	for (const auto& pGameConfig : g_GameConfigs)
	{
		if (pGameConfig->EmulatorFile.FileName == pSnapshotName)
			return pGameConfig;
	}
	return nullptr;
}


bool SaveGameConfigToFile(const FProjectConfig &config, const char *fname) 
{
	json jsonConfigFile;

	config.SaveToJson(jsonConfigFile);

	std::ofstream outFileStream(fname);
	if (outFileStream.is_open())
	{
		outFileStream << std::setw(4) << jsonConfigFile << std::endl;
		return true;
	}

	return false;
}

void FProjectConfig::SaveToJson(nlohmann::json & jsonConfigFile) const
{
	jsonConfigFile["Name"] = Name;
	jsonConfigFile["Machine"] = Machine;
	//jsonConfigFile["SnapshotFile"] = SnapshotFile;

	// snapshot
	json snapshotJson;
	snapshotJson["Type"] = EmulatorFile.Type;
	snapshotJson["DisplayName"] = EmulatorFile.DisplayName;
	snapshotJson["FileName"] = EmulatorFile.FileName;
	snapshotJson["ListName"] = EmulatorFile.ListName;
	jsonConfigFile["Snapshot"] = snapshotJson;

	// save character sets

	// save options
	json optionsJson;

	//optionsJson["WriteSnapshot"] = config.WriteSnapshot;

	//optionsJson["NumberMode"] = (int)config.NumberDisplayMode;
	//optionsJson["ShowScanlineIndicator"] = config.bShowScanLineIndicator;
	//optionsJson["Audio"] = config.Sou

	// Output view options
	for (int i = 0; i < FCodeAnalysisState::kNoViewStates; i++)
	{
		const FCodeAnalysisViewConfig& viewConfig = ViewConfigs[i];
		json viewConfigJson;
		viewConfigJson["Enabled"] = viewConfig.bEnabled;
		viewConfigJson["ViewAddress"] = viewConfig.ViewAddress.Address;
		viewConfigJson["ViewAddressBank"] = viewConfig.ViewAddress.BankId;

		optionsJson["ViewConfigs"].push_back(viewConfigJson);
	}

	jsonConfigFile["Options"] = optionsJson;

	for (const auto& luaSrc : LuaSourceFiles)
	{
		jsonConfigFile["LuaSourceFiles"].push_back(luaSrc);
	}
}

void FProjectConfig::FixupAddressRefs(FCodeAnalysisState& state)
{
	for (int i = 0; i < FCodeAnalysisState::kNoViewStates; i++)
	{
		FixupAddressRef(state, ViewConfigs[i].ViewAddress);
	}
}


bool LoadGameConfigFromFile(FProjectConfig &config, const char *fname)
{
	std::ifstream inFileStream(fname);
	if (inFileStream.is_open() == false)
		return false;

	json jsonConfigFile;

	inFileStream >> jsonConfigFile;
	inFileStream.close();
	config.LoadFromJson(jsonConfigFile);
	return true;
}

void FProjectConfig::LoadFromJson(const nlohmann::json & jsonConfigFile)
{
	Name = jsonConfigFile["Name"].get<std::string>();

	if (jsonConfigFile.contains("Machine"))
		Machine = jsonConfigFile["Machine"];

	// old - patch up
	if (jsonConfigFile.contains("SnapshotFile"))
	{ 
		EmulatorFile.FileName = GetFileFromPath(jsonConfigFile["SnapshotFile"].get<std::string>().c_str());
		EmulatorFile.ListName = "";
		EmulatorFile.Type = GetEmuFileTypeFromFileName(EmulatorFile.FileName);
		EmulatorFile.DisplayName = RemoveFileExtension(GetFileFromPath(EmulatorFile.FileName.c_str()).c_str());
	}

	// snapshot
	if (jsonConfigFile.contains("Snapshot"))
	{ 
		// snapshot
		const json& snapshotJson = jsonConfigFile["Snapshot"];
		EmulatorFile.Type = snapshotJson["Type"];
		EmulatorFile.DisplayName = snapshotJson["DisplayName"];
		EmulatorFile.FileName = snapshotJson["FileName"];
		EmulatorFile.ListName = snapshotJson["ListName"];
	}

	// load options
	if (jsonConfigFile.contains("Options"))
	{
		const json& optionsJson = jsonConfigFile["Options"];
		//if(optionsJson.contains("WriteSnapshot"))
		//	config.WriteSnapshot = optionsJson["WriteSnapshot"];
		//config.bShowScanLineIndicator = optionsJson["ShowScanlineIndicator"];

		if (optionsJson.contains("EnableCodeAnalysisView"))
		{
			for (int i = 0; i < FCodeAnalysisState::kNoViewStates; i++)
				ViewConfigs[i].bEnabled = optionsJson["EnableCodeAnalysisView"][i];
		}
		else if (optionsJson.contains("ViewConfigs"))
		{
			for (int i = 0; i < FCodeAnalysisState::kNoViewStates; i++)
			{
				FCodeAnalysisViewConfig& viewConfig = ViewConfigs[i];
				const json& viewConfigJson = optionsJson["ViewConfigs"][i];
				viewConfig.bEnabled = viewConfigJson["Enabled"];
				viewConfig.ViewAddress.Address = viewConfigJson["ViewAddress"];
				if (viewConfigJson.contains("ViewAddressBank"))
					viewConfig.ViewAddress.BankId = viewConfigJson["ViewAddressBank"];
				else
					viewConfig.ViewAddress.BankId = -1;
			}
		}
	}

	// Lua related config
	if (jsonConfigFile.contains("LuaSourceFiles"))
	{
		LuaSourceFiles.clear();
		for (const auto& srcJson : jsonConfigFile["LuaSourceFiles"])
		{
			LuaSourceFiles.push_back(srcJson);
		}
	}
}

bool FProjectConfig::AddLuaSourceFile(const char* pFilename)
{
	for (const auto& luaFile : LuaSourceFiles)
	{
		if(luaFile == pFilename)
			return false;
	}

	LuaSourceFiles.push_back(pFilename);
	return true;
}
