#include "ZXSpectrumGameConfig.h"

#include <json.hpp>
#include <iomanip>
#include <fstream>
#include <sstream>

#include "GameViewers/GameViewer.h"
#include <Util/FileUtil.h>
#include "ZXSpectrumConfig.h"
#include "SpectrumEmu.h"

// Spectrum specific

void FZXSpectrumGameConfig::LoadFromJson(const nlohmann::json& jsonConfigFile)
{
	FProjectConfig::LoadFromJson(jsonConfigFile);

	if(EmulatorFile.ListName.empty())
		EmulatorFile.ListName = "Snapshot File";

	// Patch up old field that assumed everything was in the 'Games' dir
	//if (jsonConfigFile.contains("Z80File"))
	//	SnapshotFile = std::string("./Games/") + jsonConfigFile["Z80File"].get<std::string>();

	// spectrum specific
	if (jsonConfigFile.contains("128KGame"))
		Spectrum128KGame = jsonConfigFile["128KGame"];

	pViewerConfig = GetViewConfigForGame(Name.c_str());

	if (jsonConfigFile.contains("SpriteConfigs"))
	{
		for (const auto& jsonSprConfig : jsonConfigFile["SpriteConfigs"])
		{
			const std::string& name = jsonSprConfig["Name"].get<std::string>();

			FSpriteDefConfig& sprConfig = SpriteConfigs[name];
			sprConfig.BaseAddress = ParseHexString16bit(jsonSprConfig["BaseAddress"].get<std::string>());
			sprConfig.Count = jsonSprConfig["Count"].get<int>();
			sprConfig.Width = jsonSprConfig["Width"].get<int>();
			sprConfig.Height = jsonSprConfig["Height"].get<int>();
		}
	}
}

void FZXSpectrumGameConfig::SaveToJson(nlohmann::json& jsonConfigFile) const
{
	FProjectConfig::SaveToJson(jsonConfigFile);

	// Spectrum specific
	jsonConfigFile["128KGame"] = Spectrum128KGame;

	for (const auto& sprConfigIt : SpriteConfigs)
	{
		nlohmann::json spriteConfig;
		const FSpriteDefConfig& spriteDefConfig = sprConfigIt.second;

		spriteConfig["Name"] = sprConfigIt.first;
		spriteConfig["BaseAddress"] = MakeHexString(spriteDefConfig.BaseAddress);
		spriteConfig["Count"] = spriteDefConfig.Count;
		spriteConfig["Width"] = spriteDefConfig.Width;
		spriteConfig["Height"] = spriteDefConfig.Height;

		jsonConfigFile["SpriteConfigs"].push_back(spriteConfig);
	}


}

FZXSpectrumGameConfig* CreateNewZXGameConfigFromSnapshot(const FEmulatorFile& snapshot)
{
	FZXSpectrumGameConfig* pNewConfig = new FZXSpectrumGameConfig;

	pNewConfig->Name = RemoveFileExtension(snapshot.DisplayName.c_str());
	pNewConfig->EmulatorFile = snapshot;
	pNewConfig->pViewerConfig = GetViewConfigForGame(pNewConfig->Name.c_str());

	return pNewConfig;
}

FZXSpectrumGameConfig* CreateNewZXBasicConfig(void)
{
	FZXSpectrumGameConfig* pNewConfig = new FZXSpectrumGameConfig;

	pNewConfig->Name = "ZXBasic";
	//pNewConfig->SnapshotFile = "";
	pNewConfig->pViewerConfig = GetViewConfigForGame(pNewConfig->Name.c_str());

	return pNewConfig;
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

bool LoadPOKFile(FZXSpectrumGameConfig& config, const char* fname)
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


bool LoadZXSpectrumGameConfigs(FSpectrumEmu* pEmu)
{
	FDirFileList listing;
	
	const std::string root = pEmu->GetGlobalConfig()->WorkspaceRoot;

	// New method - search through each game directory
	if (EnumerateDirectory(root.c_str(), listing) == false)
		return false;

	for (const auto& file : listing)
	{
		if (file.FileType == FDirEntry::Directory)
		{
			if(file.FileName == "." || file.FileName == "..")
				continue;

			FDirFileList directoryListing;
			std::string gameDir = root + file.FileName;
			if (EnumerateDirectory(gameDir.c_str(), directoryListing))
			{
				for (const auto& gameFile : directoryListing)
				{
					if (gameFile.FileName == "Config.json")
					{
						const std::string& configFileName = gameDir + "/" + gameFile.FileName;
						FZXSpectrumGameConfig* pNewConfig = new FZXSpectrumGameConfig;
						if (LoadGameConfigFromFile(*pNewConfig, configFileName.c_str()))
						{
							//if (pNewConfig->Spectrum128KGame == (pEmu->ZXEmuState.type == ZX_TYPE_128))
								AddGameConfig(pNewConfig);
						}
						else
						{
							delete pNewConfig;
						}

					}
				}
			}
		}
	}

	// Old method - keep this in while we have need of it
	const std::string configDir = root + "Configs/";

	if (EnumerateDirectory(configDir.c_str(), listing) == false)
		return false;

	for (const auto& file : listing)
	{
		const std::string& fn = configDir + file.FileName;
		if ((fn.substr(fn.find_last_of(".") + 1) == "json"))
		{
			FZXSpectrumGameConfig* pNewConfig = new FZXSpectrumGameConfig;
			if (LoadGameConfigFromFile(*pNewConfig, fn.c_str()))
			{
				//if (pNewConfig->Spectrum128KGame == (pEmu->ZXEmuState.type == ZX_TYPE_128))
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
