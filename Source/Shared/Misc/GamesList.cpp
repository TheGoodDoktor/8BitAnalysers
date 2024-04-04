#include "GamesList.h"
#include "Util/FileUtil.h"
#include <unordered_map>
#include <algorithm>

std::unordered_map<std::string, EEmuFileType> g_ExtensionEmuFileType = 
{
	{"z80", EEmuFileType::Z80},
	{"sna", EEmuFileType::SNA},
	{"tap", EEmuFileType::TAP},
	{"tzx", EEmuFileType::TZX},
	{"rzx", EEmuFileType::RZX},
	{"prg", EEmuFileType::PRG},
	{"crt", EEmuFileType::CRT},
};

EEmuFileType GetEmuFileTypeFromFileName(const std::string& filename)
{
	std::string extension = filename.substr(filename.find_last_of(".") + 1);;
	std::transform(extension.begin(), extension.end(), extension.begin(),
		[](unsigned char c) { return std::tolower(c); });

	const auto& extIt = g_ExtensionEmuFileType.find(extension);
	if(extIt == g_ExtensionEmuFileType.end())
		return EEmuFileType::Unknown;

	return extIt->second;
}

bool FGamesList::EnumerateGames(void)
{
	FDirFileList listing;

	//RootDir = std::string(pDir);

	GamesList.clear();

	if (EnumerateDirectory(RootDir.c_str(), listing) == false)
		return false;

	for (const auto& file : listing)
	{
		const EEmuFileType type = GetEmuFileTypeFromFileName(file.FileName);

		if (type != EEmuFileType::Unknown)
		{
			FEmulatorFile newGame;
			newGame.FileName = file.FileName;
			newGame.ListName = GetFileType();
			newGame.DisplayName = RemoveFileExtension(file.FileName.c_str());
			newGame.Type = type;
			GamesList.push_back(newGame);
		}
	}
	return true;
}

const FEmulatorFile* FGamesList::GetGame(const char* pName) const
{
	const std::string name(pName);

	for (const auto& snapshot : GamesList)
	{
		if(snapshot.DisplayName == name)
			return &snapshot;
	}
	return nullptr;
}

#if 0
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
#endif