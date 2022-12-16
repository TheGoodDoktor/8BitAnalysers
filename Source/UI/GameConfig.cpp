#include "GameConfig.h"

#include "Util/FileUtil.h"
#include "json.hpp"
#include "magic_enum.hpp"
#include <iomanip>
#include <fstream>
#include <sstream>

#include "SpectrumEmu.h"
#include "GameViewers/GameViewer.h"
#include "SnapshotLoaders/GamesList.h"
#include "Debug/Debug.h"
#include "Shared/Util/Misc.h"

using json = nlohmann::json;
static std::vector< FGameConfig *>	g_GameConfigs;

bool AddGameConfig(FGameConfig *pConfig)
{
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
	pNewConfig->SnapshotFile = snapshot.FileName;
	pNewConfig->pViewerConfig = GetViewConfigForGame(pNewConfig->Name.c_str());

	return pNewConfig;
}



bool SaveGameConfigToFile(const FGameConfig &config, const char *fname)
{
	json jsonConfigFile;
	jsonConfigFile["Name"] = config.Name;
	jsonConfigFile["SnapshotFile"] = config.SnapshotFile;

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

	/*for (const FCheat& cheat : config.Cheats)
	{
		json cheatJson;
		cheatJson["Description"] = cheat.Description;
		for (const FCheatMemoryEntry& entry : cheat.Entries)
		{
			json cheatEntryJson;
			cheatEntryJson["Address"] = entry.Address;
			cheatEntryJson["Value"] = entry.Value;
			cheatJson["Entries"].push_back(cheatEntryJson);
		}

		jsonConfigFile["Cheats"].push_back(cheatJson);
	}*/

	// save options
	json optionsJson;
	optionsJson["NumberMode"] = (int)config.NumberDisplayMode;
	optionsJson["ShowScanlineIndicator"] = config.bShowScanLineIndicator;
	for (int i = 0; i < FCodeAnalysisState::kNoViewStates; i++)
		optionsJson["EnableCodeAnalysisView"].push_back(config.bCodeAnalysisViewEnabled[i]);
	jsonConfigFile["Options"] = optionsJson;

	std::ofstream outFileStream(fname);
	if (outFileStream.is_open())
	{
		outFileStream << std::setw(4) << jsonConfigFile << std::endl;
		return true;
	}

	return false;
}


bool LoadGameConfigFromFile(FGameConfig &config, const char *fname)
{
	std::ifstream inFileStream(fname);
	if (inFileStream.is_open() == false)
		return false;

	json jsonConfigFile;

	inFileStream >> jsonConfigFile;
	inFileStream.close();

	config.Name = jsonConfigFile["Name"].get<std::string>();

	// Patch up old field that assumed everything was in the 'Games' dir
	if (jsonConfigFile["Z80File"].is_null() == false)
	{
		config.SnapshotFile = std::string("./Games/") + jsonConfigFile["Z80File"].get<std::string>();
	}
	if (jsonConfigFile["SnapshotFile"].is_null() == false)
	{
		config.SnapshotFile = jsonConfigFile["SnapshotFile"].get<std::string>();
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

	/*for (const auto& cheatJson : jsonConfigFile["Cheats"])
	{
		FCheat cheat;
		cheat.Description = cheatJson["Description"].get<std::string>();

		for (const auto& cheatEntryJson : cheatJson["Entries"])
		{
			FCheatMemoryEntry entry;
			entry.Address = cheatEntryJson["Address"].get<int>();
			entry.Value = cheatEntryJson["Value"].get<int>();
			cheat.Entries.push_back(entry);
		}
		config.Cheats.push_back(cheat);
	}*/

	// load options
	if (jsonConfigFile["Options"].is_null() == false)
	{
		const json& optionsJson = jsonConfigFile["Options"];
		config.NumberDisplayMode = (ENumberDisplayMode)optionsJson["NumberMode"];
		config.bShowScanLineIndicator = optionsJson["ShowScanlineIndicator"];
		for (int i = 0; i < FCodeAnalysisState::kNoViewStates; i++)
			config.bCodeAnalysisViewEnabled[i] = optionsJson["EnableCodeAnalysisView"][i];
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

// Labels

void SaveLabelsBin(const FCodeAnalysisState &state, FILE *fp, uint16_t startAddress, uint16_t endAddress)
{
	int recordCount = 0;
	for (int i = startAddress; i <= endAddress; i++)
	{
		if (state.GetLabelForAddress(i) != nullptr)
			recordCount++;
	}

	fwrite(&recordCount, sizeof(int), 1, fp);

	for (int i = startAddress; i <= endAddress; i++)
	{
		const FLabelInfo *pLabel = state.GetLabelForAddress(i);
		if (pLabel != nullptr)
		{
			WriteStringToFile(std::string(magic_enum::enum_name(pLabel->LabelType)), fp);
			fwrite(&pLabel->Address, sizeof(pLabel->Address), 1, fp);
			fwrite(&pLabel->ByteSize, sizeof(pLabel->ByteSize), 1, fp);
			WriteStringToFile(pLabel->Name, fp);
			WriteStringToFile(pLabel->Comment, fp);
			fwrite(&pLabel->Global, sizeof(bool), 1, fp);

			// References?
			long refCountPos = ftell(fp);
			int noReferences = (int)pLabel->References.size();
			fwrite(&noReferences, sizeof(int), 1, fp);
			noReferences = 0;
			for (const auto &ref : pLabel->References)
			{
				const uint16_t refAddr = ref.first;
				if (refAddr >= startAddress && refAddr <= endAddress)	// only add references from region we are saving
				{
					fwrite(&refAddr, sizeof(refAddr), 1, fp);
					noReferences++;
				}
			}

			// fix up reference count
			fseek(fp, refCountPos, SEEK_SET);
			fwrite(&noReferences, sizeof(int), 1, fp);	// new reference count
			fseek(fp, 0, SEEK_END);	// point back to end of file
		}
	}
}

void LoadLabelsBin(FCodeAnalysisState &state, FILE *fp,int versionNo, uint16_t startAddress, uint16_t endAddress)
{
	int recordCount = 0;

	state.ResetLabelNames();

	fread(&recordCount, sizeof(int), 1, fp);

	for (int i = 0; i < recordCount; i++)
	{
		FLabelInfo *pLabel = FLabelInfo::Allocate();
		
		std::string enumVal;
		ReadStringFromFile(enumVal, fp);
		pLabel->LabelType = magic_enum::enum_cast<LabelType>(enumVal).value();
		fread(&pLabel->Address, sizeof(pLabel->Address), 1, fp);
		fread(&pLabel->ByteSize, sizeof(pLabel->ByteSize), 1, fp);
		ReadStringFromFile(pLabel->Name, fp);
		ReadStringFromFile(pLabel->Comment, fp);

		if (versionNo > 2)
			fread(&pLabel->Global, sizeof(bool), 1, fp);
		
		// References?
		if(versionNo > 1)
		{
			int noReferences = 0;
			fread(&noReferences, sizeof(int), 1, fp);
			for (int i=0;i< noReferences;i++)
			{
				uint16_t refAddr;
				fread(&refAddr, sizeof(refAddr), 1, fp);
				if (refAddr >= startAddress && refAddr <= endAddress)
				{
					pLabel->References[refAddr] = 1;
				}
				else
				{
					LOGWARNING("Label reference address %x outside of range",refAddr);
				}
			}
		}

		state.SetLabelForAddress(pLabel->Address,pLabel);
	}
}

// Code Info

void SaveCodeInfoBin(const FCodeAnalysisState &state, FILE *fp, uint16_t startAddress, uint16_t endAddress)
{
	int recordCount = 0;
	for (int i = startAddress; i <= endAddress; i++)
	{
		if (state.GetCodeInfoForAddress(i) != nullptr)
			recordCount++;
	}

	fwrite(&recordCount, sizeof(int), 1, fp);
	
	for (int i = startAddress; i <= endAddress; i++)
	{
		const FCodeInfo *pCodeInfo = state.GetCodeInfoForAddress(i);
		if (pCodeInfo != nullptr)
		{
			fwrite(&pCodeInfo->Flags, sizeof(pCodeInfo->Flags), 1, fp);
			fwrite(&pCodeInfo->Address, sizeof(pCodeInfo->Address), 1, fp);
			fwrite(&pCodeInfo->ByteSize, sizeof(pCodeInfo->ByteSize), 1, fp);
			fwrite(&pCodeInfo->JumpAddress, sizeof(pCodeInfo->JumpAddress), 1, fp);
			fwrite(&pCodeInfo->PointerAddress, sizeof(pCodeInfo->PointerAddress), 1, fp);
			WriteStringToFile(std::string(), fp);	// we can remove this - making sure backwards compatibility works of course!
			//WriteStringToFile(pCodeInfo->Text, fp);	// we can remove this - making sure backwards compatibility works of course!
			WriteStringToFile(pCodeInfo->Comment, fp);
		}
	}
}

void LoadCodeInfoBin(FCodeAnalysisState &state, FILE *fp, int versionNo, uint16_t startAddress, uint16_t endAddress)
{
	int recordCount;
	fread(&recordCount, sizeof(int), 1, fp);

	for (int i = 0; i < recordCount; i++)
	{
		FCodeInfo *pCodeInfo = FCodeInfo::Allocate();

		if(versionNo >= 4)
			fread(&pCodeInfo->Flags, sizeof(pCodeInfo->Flags), 1, fp);

		fread(&pCodeInfo->Address, sizeof(pCodeInfo->Address), 1, fp);
		fread(&pCodeInfo->ByteSize, sizeof(pCodeInfo->ByteSize), 1, fp);
		fread(&pCodeInfo->JumpAddress, sizeof(pCodeInfo->JumpAddress), 1, fp);
		fread(&pCodeInfo->PointerAddress, sizeof(pCodeInfo->PointerAddress), 1, fp);
		std::string tmp;
		ReadStringFromFile(tmp, fp);
		//ReadStringFromFile(pCodeInfo->Text, fp);
		ReadStringFromFile(pCodeInfo->Comment, fp);
		for(int codeByte = 0;codeByte < pCodeInfo->ByteSize;codeByte++)	// set for whole instruction address range
			state.SetCodeInfoForAddress(pCodeInfo->Address + codeByte, pCodeInfo);
	}
}

// Data Info

void SaveDataInfoBin(const FCodeAnalysisState& state, FILE *fp, uint16_t startAddress, uint16_t endAddress)
{
	int recordCount = 0;
	for (int i = startAddress; i <= endAddress; i++)
	{
		if (state.GetReadDataInfoForAddress(i) != nullptr)
			recordCount++;
	}

	fwrite(&recordCount, sizeof(int), 1, fp);

	for (int i = startAddress; i <= endAddress; i++)
	{
		const FDataInfo *pDataInfo = state.GetReadDataInfoForAddress(i);
		if (pDataInfo != nullptr)
		{
			WriteStringToFile(std::string(magic_enum::enum_name(pDataInfo->DataType)), fp);
			fwrite(&pDataInfo->Address, sizeof(pDataInfo->Address), 1, fp);
			fwrite(&pDataInfo->ByteSize, sizeof(pDataInfo->ByteSize), 1, fp);
			fwrite(&pDataInfo->Flags, sizeof(pDataInfo->Flags), 1, fp);
			WriteStringToFile(pDataInfo->Comment, fp);

			// Reads & Writes?
			int noReads = 0;
			const long noReadsFilePos = ftell(fp);
			fwrite(&noReads, sizeof(int), 1, fp);
			for (const auto &ref : pDataInfo->Reads)
			{
				const uint16_t refAddr = ref.first;
				if (refAddr >= startAddress && refAddr <= endAddress)
				{
					fwrite(&refAddr, sizeof(refAddr), 1, fp);
					noReads++;
				}
			}

			// patch number of reads
			fseek(fp, noReadsFilePos, SEEK_SET);
			fwrite(&noReads, sizeof(int), 1, fp);
			fseek(fp, 0, SEEK_END);


			int noWrites = 0;
			const long noWritesFilePos = ftell(fp);
			fwrite(&noWrites, sizeof(int), 1, fp);
			for (const auto &ref : pDataInfo->Writes)
			{
				const uint16_t refAddr = ref.first;
				if (refAddr >= startAddress && refAddr <= endAddress)
				{
					fwrite(&refAddr, sizeof(refAddr), 1, fp);
					noWrites++;
				}
			}

			// patch number of writes
			fseek(fp, noWritesFilePos, SEEK_SET);
			fwrite(&noWrites, sizeof(int), 1, fp);
			fseek(fp, 0, SEEK_END);
		}
	}
	
	
}

void LoadDataInfoBin(FCodeAnalysisState& state, FILE *fp, int versionNo, uint16_t startAddress, uint16_t endAddress)
{
	int recordCount;
	fread(&recordCount, sizeof(int), 1, fp);

	for (int i = 0; i < recordCount; i++)
	{
		std::string enumVal;
		ReadStringFromFile(enumVal, fp);
		uint16_t address = 0;
		fread(&address, sizeof(address), 1, fp);

		FDataInfo* pDataInfo = state.GetReadDataInfoForAddress(address);
		pDataInfo->Address = address;
		pDataInfo->DataType = magic_enum::enum_cast<DataType>(enumVal).value();
		fread(&pDataInfo->ByteSize, sizeof(pDataInfo->ByteSize), 1, fp);
		if (versionNo > 5)
		{
			fread(&pDataInfo->Flags, sizeof(pDataInfo->Flags), 1, fp);
			if (pDataInfo->bShowCharMap)
			{
				pDataInfo->DataType = DataType::CharacterMap;
				pDataInfo->bShowCharMap = false;
			}

			if (pDataInfo->bShowBinary)
			{
				pDataInfo->DataType = DataType::Bitmap;
				pDataInfo->bShowBinary = false;
			}
		}

		ReadStringFromFile(pDataInfo->Comment, fp);

		// References?
		if (versionNo > 1)
		{
			int noReads;
			fread(&noReads, sizeof(int), 1, fp);
			for (int i = 0; i < noReads; i++)
			{
				uint16_t dataAddr;
				fread(&dataAddr, sizeof(uint16_t), 1, fp);
				if (dataAddr >= startAddress && dataAddr <= endAddress)
					pDataInfo->Reads[dataAddr] = 1;
				else
					LOGWARNING("LoadDataInfoBin: Address %x outside of range", dataAddr);
			}
		}
		if (versionNo > 2)
		{
			int noWrites;
			fread(&noWrites, sizeof(int), 1, fp);
			for (int i = 0; i < noWrites; i++)
			{
				uint16_t dataAddr;
				fread(&dataAddr, sizeof(uint16_t), 1, fp);
				if (dataAddr >= startAddress && dataAddr <= endAddress)
					pDataInfo->Writes[dataAddr] = 1;
				else
					LOGWARNING("LoadDataInfoBin: Address %x outside of range", dataAddr);
			}
		}

		//state.SetReadDataInfoForAddress(pDataInfo->Address, pDataInfo);
		//state.SetWriteDataInfoForAddress(pDataInfo->Address, pDataInfo);
	}
}

void SaveCommentBlocksBin(const FCodeAnalysisState& state, FILE* fp, uint16_t startAddress, uint16_t endAddress)
{
	int recordCount = 0;
	for (int i = startAddress; i <= endAddress; i++)
	{
		if (state.GetCommentBlockForAddress(i) != nullptr)
			recordCount++;
	}

	fwrite(&recordCount, sizeof(int), 1, fp);

	for (int i = startAddress; i <= endAddress; i++)
	{
		const FCommentBlock* pCommentBlock = state.GetCommentBlockForAddress(i);
		if (pCommentBlock != nullptr)
		{
			fwrite(&pCommentBlock->Address, sizeof(pCommentBlock->Address), 1, fp);
			WriteStringToFile(pCommentBlock->Comment, fp);
		}
	}
}

void LoadCommentBlocksBin(FCodeAnalysisState& state, FILE* fp, int versionNo, uint16_t startAddress, uint16_t endAddress)
{
	int recordCount;
	fread(&recordCount, sizeof(int), 1, fp);

	for (int i = 0; i < recordCount; i++)
	{
		FCommentBlock* pCommentBlock = FCommentBlock::Allocate();
		fread(&pCommentBlock->Address, sizeof(pCommentBlock->Address), 1, fp);
		ReadStringFromFile(pCommentBlock->Comment, fp);
		state.SetCommentBlockForAddress(pCommentBlock->Address, pCommentBlock);
	}
}

static const int g_kBinaryFileVersionNo = 8;
static const int g_kBinaryFileMagic = 0xdeadface;

// Binary save
bool SaveGameDataBin(const FCodeAnalysisState& state, const char *fname, uint16_t addrStart, uint16_t addrEnd)
{
	FILE *fp = fopen(fname, "wb");
	if (fp == NULL)
		return false;

	fwrite(&g_kBinaryFileMagic, sizeof(int), 1, fp);
	fwrite(&g_kBinaryFileVersionNo, sizeof(int), 1, fp);

	fwrite(&addrStart, sizeof(uint16_t), 1, fp);	// write memory range of this block
	fwrite(&addrEnd, sizeof(uint16_t), 1, fp);

	for (int i = addrStart; i <= addrEnd; i++)
	{
		const uint16_t invalid = 0;
		const uint16_t addr = state.GetLastWriterForAddress(i);
		if (addr >= addrStart && addr <= addrEnd)
			fwrite(&addr, sizeof(uint16_t), 1, fp);
		else
			fwrite(&invalid, sizeof(uint16_t), 1, fp);
	}
	
	SaveLabelsBin(state, fp, addrStart, addrEnd);
	SaveCodeInfoBin(state, fp, addrStart, addrEnd);
	SaveDataInfoBin(state, fp, addrStart, addrEnd);
	SaveCommentBlocksBin(state, fp, addrStart, addrEnd);

	// Save Watches
	int noWatches = 0;// (int)state.GetWatches().size();
	const long noWatchPos = ftell(fp);
	fwrite(&noWatches, sizeof(noWatches), 1, fp);

	for (const auto& watch : state.GetWatches())
	{
		if (watch >= addrStart && watch <= addrEnd)
		{
			fwrite(&watch, sizeof(uint16_t), 1, fp);
			noWatches++;
		}
	}

	// patch up watch count
	fseek(fp, noWatchPos, SEEK_SET);
	fwrite(&noWatches, sizeof(noWatches), 1, fp);
	fseek(fp, 0, SEEK_END);

	fclose(fp);
	return true;
}

// Binary load
bool LoadGameDataBin(FCodeAnalysisState& state, const char *fname, uint16_t addrStart, uint16_t addrEnd)
{
	FILE *fp = fopen(fname, "rb");
	if (fp == NULL)
		return false;
	int magic, versionNo;
	fread(&magic, sizeof(int), 1, fp);
	if (magic != g_kBinaryFileMagic)
	{
		fclose(fp);
		return false;
	}

	fread(&versionNo, sizeof(int), 1, fp);

	if (versionNo >= 8)
	{
		fread(&addrStart, sizeof(uint16_t), 1, fp);
		fread(&addrEnd, sizeof(uint16_t), 1, fp);

		for (int i = addrStart; i <= addrEnd; i++)
		{
			uint16_t lastWriter;
			fread(&lastWriter, sizeof(uint16_t), 1, fp);
			state.SetLastWriterForAddress(i, lastWriter);
		}
	}
	else if (versionNo >= 4)
	{
		for (int i = 0; i < (1 << 16); i++)
		{
			uint16_t lastWriter;
			fread(&lastWriter, sizeof(uint16_t), 1, fp);
			state.SetLastWriterForAddress(i, lastWriter);
		}
	}

	LoadLabelsBin(state, fp, versionNo, addrStart, addrEnd);
	LoadCodeInfoBin(state, fp, versionNo, addrStart, addrEnd);
	LoadDataInfoBin(state, fp, versionNo, addrStart, addrEnd);
	if (versionNo >= 5)
		LoadCommentBlocksBin(state, fp, versionNo, addrStart, addrEnd);

	// watches
	if (versionNo >= 7)
	{
		int noWatches;
		fread(&noWatches, sizeof(noWatches), 1, fp);

		for (int i=0;i<noWatches;i++)
		{
			uint16_t watch;
			fread(&watch, sizeof(uint16_t), 1, fp);
			if (watch >= addrStart && watch <= addrEnd)
				state.AddWatch(watch);
		}
	}
	fclose(fp);
	return true;
}

bool SaveGameData(const FCodeAnalysisState& state, const char *fname)
{
	return SaveGameDataBin(state, fname,0x4000, 0xffff);
}

bool SaveROMData(const FCodeAnalysisState& state, const char *fname)
{
	return SaveGameDataBin(state, fname, 0x0000, 0x3fff);
}

bool LoadGameData(FCodeAnalysisState& state, const char *fname)
{
	return LoadGameDataBin(state,fname,0x4000,0xffff);
}

bool LoadROMData(FCodeAnalysisState& state, const char *fname)
{
	return LoadGameDataBin(state, fname,0x0000,0x3fff);
}

bool LoadGameConfigs(FSpectrumEmu *pEmu)
{
	FDirFileList listing;

	if (EnumerateDirectory("Configs", listing) == false)
		return false;

	for (const auto &file : listing)
	{
		const std::string &fn = "configs/" + file.FileName;
		if ((fn.substr(fn.find_last_of(".") + 1) == "json"))
		{
			FGameConfig *pNewConfig = new FGameConfig;
			if (LoadGameConfigFromFile(*pNewConfig, fn.c_str()))
			{
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