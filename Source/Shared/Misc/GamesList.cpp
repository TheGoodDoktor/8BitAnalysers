#include "GamesList.h"
#include "Util/FileUtil.h"

bool FGamesList::EnumerateGames(const char* pDir)
{
	FDirFileList listing;

	RootDir = std::string(pDir);

	GamesList.clear();

	if (EnumerateDirectory(pDir, listing) == false)
		return false;

	for (const auto& file : listing)
	{
		const ESnapshotType type = pGameLoader->GetSnapshotTypeFromFileName(file.FileName);

		if (type != ESnapshotType::Unknown)
		{
			FGameSnapshot newGame;
			newGame.FileName = RootDir + file.FileName;
			newGame.DisplayName = file.FileName;
			newGame.Type = type;
			GamesList.push_back(newGame);
		}
	}
	return true;
}

bool FGamesList::LoadGame(int index) const
{
	if (index < 0 || index >= GamesList.size())
		return false;

	return pGameLoader->LoadGame(GamesList[index].FileName.c_str());
}

bool FGamesList::LoadGame(const char* pFileName) const
{
	return pGameLoader->LoadGame(pFileName);
}