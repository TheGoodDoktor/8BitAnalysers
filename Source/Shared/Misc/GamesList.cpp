#include "GamesList.h"
#include "Util/FileUtil.h"
#include <unordered_map>
#include <algorithm>

std::unordered_map<std::string, ESnapshotType> g_ExtensionSnapshotType = 
{
	{"z80", ESnapshotType::Z80},
	{"sna", ESnapshotType::SNA},
	{"tap", ESnapshotType::TAP},
	{"tzx", ESnapshotType::TZX},
	{"rzx", ESnapshotType::RZX},
	{"prg", ESnapshotType::PRG},
};

ESnapshotType GetSnapshotTypeFromFileName(const std::string& filename)
{
	std::string extension = filename.substr(filename.find_last_of(".") + 1);;
	std::transform(extension.begin(), extension.end(), extension.begin(),
		[](unsigned char c) { return std::tolower(c); });

	const auto& extIt = g_ExtensionSnapshotType.find(extension);
	if(extIt == g_ExtensionSnapshotType.end())
		return ESnapshotType::Unknown;

	return extIt->second;
}

bool FGamesList::EnumerateGames(const char* pDir)
{
	FDirFileList listing;

	RootDir = std::string(pDir);

	GamesList.clear();

	if (EnumerateDirectory(pDir, listing) == false)
		return false;

	for (const auto& file : listing)
	{
		const ESnapshotType type = GetSnapshotTypeFromFileName(file.FileName);

		if (type != ESnapshotType::Unknown)
		{
			FGameSnapshot newGame;
			newGame.FileName = RootDir + file.FileName;
			newGame.DisplayName = RemoveFileExtension(file.FileName.c_str());
			newGame.Type = type;
			GamesList.push_back(newGame);
		}
	}
	return true;
}

const FGameSnapshot* FGamesList::GetGame(const char* pName) const
{
	const std::string name(pName);

	for (const auto& snapshot : GamesList)
	{
		if(snapshot.DisplayName == name)
			return &snapshot;
	}
	return nullptr;
}


bool FGamesList::LoadGame(int index) const
{
	if (index < 0 || index >= GamesList.size())
		return false;

	return pGameLoader->LoadSnapshot(GamesList[index]);
}

bool FGamesList::LoadGame(const char* pFileName) const
{
	const FGameSnapshot* pSnapshot = GetGame(pFileName);
	if(pSnapshot != nullptr)
		return pGameLoader->LoadSnapshot(*pSnapshot);

	return false;
}