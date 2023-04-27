#include "GameConfig.h"

#include "Util/FileUtil.h"
#include "json.hpp"
//#include "magic_enum.hpp"
#include <iomanip>
#include <fstream>
#include <sstream>

#include "SpectrumEmu.h"
#include "GameViewers/GameViewer.h"
#include "SnapshotLoaders/GamesList.h"
#include "Debug/DebugLog.h"
#include "Util/Misc.h"
#include <Util/GraphicsView.h>
#include "GlobalConfig.h"

using json = nlohmann::json;
static std::vector< FGameConfig *>	g_GameConfigs;

bool AddGameConfig(FGameConfig *pConfig)
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

const std::vector< FGameConfig *>& GetGameConfigs()
{
	return g_GameConfigs;
}

FGameConfig * CreateNewGameConfigFromSnapshot(const FGameSnapshot& snapshot)
{
	FGameConfig *pNewConfig = new FGameConfig;

	pNewConfig->Name = RemoveFileExtension(snapshot.DisplayName.c_str());
	pNewConfig->SnapshotFile = GetFileFromPath(snapshot.FileName.c_str());
	pNewConfig->pViewerConfig = GetViewConfigForGame(pNewConfig->Name.c_str());

	return pNewConfig;
}

bool SaveGameConfigToFile(const FGameConfig &config, const char *fname)
{
	json jsonConfigFile;
	jsonConfigFile["Name"] = config.Name;
	jsonConfigFile["SnapshotFile"] = config.SnapshotFile;
	jsonConfigFile["128KGame"] = config.Spectrum128KGame;

	for (const auto&sprConfigIt : config.SpriteConfigs)
	{
		json spriteConfig;
		const FSpriteDefConfig& spriteDefConfig = sprConfigIt.second;
		
		spriteConfig["Name"] = sprConfigIt.first;
		spriteConfig["BaseAddress"] = MakeHexString(spriteDefConfig.BaseAddress);
		spriteConfig["Count"] = spriteDefConfig.Count;
		spriteConfig["Width"] = spriteDefConfig.Width;
		spriteConfig["Height"] = spriteDefConfig.Height;

		jsonConfigFile["SpriteConfigs"].push_back(spriteConfig);
	}

	// save character sets

	// save options
	json optionsJson;

	optionsJson["WriteSnapshot"] = config.WriteSnapshot;

	//optionsJson["NumberMode"] = (int)config.NumberDisplayMode;
	//optionsJson["ShowScanlineIndicator"] = config.bShowScanLineIndicator;
	//optionsJson["Audio"] = config.Sou

	// Output view options
	for (int i = 0; i < FCodeAnalysisState::kNoViewStates; i++)
	{
		const FCodeAnalysisViewConfig& viewConfig = config.ViewConfigs[i];
		json viewConfigJson;
		viewConfigJson["Enabled"] = viewConfig.bEnabled;
		viewConfigJson["ViewAddress"] = viewConfig.ViewAddress.Address;
		viewConfigJson["ViewAddressBank"] = viewConfig.ViewAddress.BankId;

		optionsJson["ViewConfigs"].push_back(viewConfigJson);
	}

	jsonConfigFile["Options"] = optionsJson;

	std::ofstream outFileStream(fname);
	if (outFileStream.is_open())
	{
		outFileStream << std::setw(4) << jsonConfigFile << std::endl;
		return true;
	}

	return false;
}


bool LoadGameConfigFromFile(const FCodeAnalysisState& state, FGameConfig &config, const char *fname)
{
	std::ifstream inFileStream(fname);
	if (inFileStream.is_open() == false)
		return false;

	json jsonConfigFile;

	inFileStream >> jsonConfigFile;
	inFileStream.close();

	config.Name = jsonConfigFile["Name"].get<std::string>();

	// Patch up old field that assumed everything was in the 'Games' dir
	if (jsonConfigFile.contains("Z80File"))
	{
		config.SnapshotFile = std::string("./Games/") + jsonConfigFile["Z80File"].get<std::string>();
	}
	if (jsonConfigFile.contains("SnapshotFile"))
	{
		config.SnapshotFile = GetFileFromPath(jsonConfigFile["SnapshotFile"].get<std::string>().c_str());
	}
	if (jsonConfigFile.contains("128KGame"))
	{
		config.Spectrum128KGame = jsonConfigFile["128KGame"];
	}

	config.pViewerConfig = GetViewConfigForGame(config.Name.c_str());

	for(const auto & jsonSprConfig : jsonConfigFile["SpriteConfigs"])
	{
		const std::string &name = jsonSprConfig["Name"].get<std::string>();
		
		FSpriteDefConfig &sprConfig = config.SpriteConfigs[name];
		sprConfig.BaseAddress = ParseHexString16bit(jsonSprConfig["BaseAddress"].get<std::string>());
		sprConfig.Count = jsonSprConfig["Count"].get<int>();
		sprConfig.Width = jsonSprConfig["Width"].get<int>();
		sprConfig.Height = jsonSprConfig["Height"].get<int>();
	}

	// load options
	if (jsonConfigFile.contains("Options"))
	{
		const json& optionsJson = jsonConfigFile["Options"];
		if(optionsJson.contains("WriteSnapshot"))
			config.WriteSnapshot = optionsJson["WriteSnapshot"];
		//config.bShowScanLineIndicator = optionsJson["ShowScanlineIndicator"];

		if (optionsJson.contains("EnableCodeAnalysisView"))
		{
			for (int i = 0; i < FCodeAnalysisState::kNoViewStates; i++)
				config.ViewConfigs[i].bEnabled = optionsJson["EnableCodeAnalysisView"][i];
		}
		else if (optionsJson.contains("ViewConfigs"))
		{
			for (int i = 0; i < FCodeAnalysisState::kNoViewStates; i++)
			{
				FCodeAnalysisViewConfig& viewConfig = config.ViewConfigs[i];
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

	return true;
}

enum PokeFileToken
{
  Description = 'N',
  MorePoke = 'M',
  LastPoke = 'Z',
  EndOfFile = 'Y'
};

enum class PokeReaderState
{
	ProcessDescription,
	ProcessPokeEntries,
};

bool LoadPOKFile(FGameConfig &config, const char *fname)
{
	std::ifstream inFileStream(fname);
	if (inFileStream.is_open() == false)
		return false;

	config.Cheats.clear();

	// Read entire file 
	std::ostringstream buffer;
    buffer << inFileStream.rdbuf();

	PokeReaderState state = PokeReaderState::ProcessDescription;

	std::istringstream iss(buffer.str());
	for (std::string line; std::getline(iss, line); )
	{
		if (!line.empty())
		{
			const char token = line[0];
			switch (token)
			{
			case PokeFileToken::Description:
			{
				// todo deal with state != PokeReaderState::ProcessDescription

				FCheat cheat;
				cheat.Description = line.substr(1);
				config.Cheats.push_back(cheat);
				state = PokeReaderState::ProcessPokeEntries;
				break;
			}
			case PokeFileToken::MorePoke:
				// Intentional drop through
			case PokeFileToken::LastPoke:
			{	
				// todo check state is PokeReaderState::ProcessPokeEntries

				std::vector<std::string> tokens;
				Tokenize(line, ' ', tokens);				

				// todo deal with incorrect number of entries

				FCheatMemoryEntry pokeEntry;
				pokeEntry.Address = std::stoi(tokens[2]);
				uint16_t value = std::stoi(tokens[3]);

				// todo deal with invalid values (<0 and >256)

				if (value < 256)
				{
					pokeEntry.Value = static_cast<uint8_t>(value);
				}
				else if (value == 256)
				{
					pokeEntry.Value = 0;
					pokeEntry.bUserDefined = true;
					config.Cheats.back().bHasUserDefinedEntries = true;

				}
				config.Cheats.back().Entries.push_back(pokeEntry);

				if (token == PokeFileToken::LastPoke)
					state = PokeReaderState::ProcessDescription;
				break;
			}
			case PokeFileToken::EndOfFile:
				return true;
			default:
				// todo handle this
				break;
			}
		}
	}

	return false;
}


bool LoadGameConfigs(FSpectrumEmu *pEmu)
{
	FDirFileList listing;

	const std::string root = GetGlobalConfig().WorkspaceRoot;
	const std::string configDir = root + "Configs/";

	if (EnumerateDirectory(configDir.c_str(), listing) == false)
		return false;

	for (const auto &file : listing)
	{
		const std::string &fn = configDir + file.FileName;
		if ((fn.substr(fn.find_last_of(".") + 1) == "json"))
		{
			FGameConfig *pNewConfig = new FGameConfig;
			if (LoadGameConfigFromFile(pEmu->CodeAnalysis, *pNewConfig, fn.c_str()))
			{
				if(pNewConfig->Spectrum128KGame == (pEmu->ZXEmuState.type == ZX_TYPE_128))
					AddGameConfig(pNewConfig);
			}
			else
			{
				delete pNewConfig;
			}
		}
	}
	return true;
}