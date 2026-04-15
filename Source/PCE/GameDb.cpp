#include "GameDb.h"

#include "json.hpp"
#include <iomanip>
#include <fstream>
#include <sstream>

using json = nlohmann::json;

TGameDb gGameDb;

FGameDbEntry* GetGameDbEntry(const std::string& name)
{
	auto it = gGameDb.find(name);
	if (it == gGameDb.end())
		return nullptr;
	return &(it->second);
}

FGameDbEntry& CreateGameDbEntry(const std::string& name, int bankCount)
{
	assert(gGameDb.find(name) == gGameDb.end());

	FGameDbEntry& dbEntry = gGameDb[name];
	dbEntry.Banks.resize(bankCount);
	for (FGameDbBank& bank : dbEntry.Banks)
		bank.MprSlots.reserve(8);
	return dbEntry;
}

TGameDb& GetGameDb()
{
	return gGameDb;
}

bool SaveGameDbEntry(const std::string& gameName, const std::string& fname)
{
	json jsonFile;

	const auto it = gGameDb.find(gameName);
	if (it == gGameDb.end())
	{
		// Dont save if no entry exists
		return false;
	}

	FGameDbEntry& entry = it->second;

	jsonFile["Name"] = gameName;
	jsonFile["NumBanks"] = entry.Banks.size();
	jsonFile["MaxDupeBanks"] = entry.MaxDupeBanks;

	jsonFile["Validated"] = entry.bValidated;
	jsonFile["AssemblesOk"] = entry.bAssemblesOk;
	jsonFile["RomFilePartialMatch"] = entry.bRomFilePartialMatch;
	jsonFile["RomFileIdentical"] = entry.bRomFileIdentical;
	jsonFile["EmulatorTestOk"] = entry.bEmulatorTestOk;

	jsonFile["TestingMethodology"] = entry.TestingMethodology;

	entry.NumDynamicBanks = 0;

	for (auto dbBank : entry.Banks)
	{
		json mappingJson;
		mappingJson["MprSlots"] = dbBank.MprSlots;
		mappingJson["Fixed"] = dbBank.bFixed;
		jsonFile["Mappings"].push_back(mappingJson);

		if (!dbBank.MprSlots.empty() && !dbBank.bFixed)
			entry.NumDynamicBanks++;
	}

	std::ofstream outFileStream(fname);
	if (outFileStream.is_open())
	{
		outFileStream << std::setw(4) << jsonFile << std::endl;
		return true;
	}
	return false;
}

bool LoadGameDbEntry(const std::string& gameName, const std::string& fname)
{
	std::ifstream inFileStream(fname);
	if (inFileStream.is_open() == false)
		return false;

	json jsonFile;

	inFileStream >> jsonFile;
	inFileStream.close();

	const std::string name = jsonFile["Name"];
	const int numBanks = jsonFile["NumBanks"];

	FGameDbEntry& entry = gGameDb[name];
	entry.bValidated = jsonFile["Validated"];
	entry.bAssemblesOk = jsonFile["AssemblesOk"];
	entry.bRomFilePartialMatch = jsonFile["RomFilePartialMatch"];
	entry.bRomFileIdentical = jsonFile["RomFileIdentical"];
	entry.bEmulatorTestOk = jsonFile["EmulatorTestOk"];
	entry.TestingMethodology = jsonFile["TestingMethodology"];
	entry.MaxDupeBanks = jsonFile["MaxDupeBanks"];

	entry.Banks.clear();
	entry.Banks.resize(numBanks);
	for (FGameDbBank& bank : entry.Banks)
		bank.MprSlots.reserve(8);

	json& mappingsJson = jsonFile["Mappings"];

	entry.NumDynamicBanks = 0;

	for (int i = 0; i < numBanks; i++)
	{
		json& mappingJson = mappingsJson[i];
		entry.Banks[i].bFixed = mappingJson["Fixed"];

		// Support old files that stored a single MprSlot int
		if (mappingJson.contains("MprSlots"))
			entry.Banks[i].MprSlots = mappingJson["MprSlots"].get<std::vector<int>>();
		else if (mappingJson.contains("MprSlot") && mappingJson["MprSlot"] != -1)
			entry.Banks[i].MprSlots.push_back(mappingJson["MprSlot"].get<int>());

		if (!entry.Banks[i].MprSlots.empty() && !entry.Banks[i].bFixed)
			entry.NumDynamicBanks++;
	}
	
	return true;
}