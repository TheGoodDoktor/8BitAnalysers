#include "GamesList.h"
#include "Util/FileUtil.h"
#include "Z80Loader.h"
#include "SNALoader.h"

bool FGamesList::EnumerateGames(void)
{
	FDirFileList listing;

	if (EnumerateDirectory("./Games", listing) == false)
		return false;

	for (const auto& file : listing)
	{
		const std::string& fn = file.FileName;
		if ((fn.substr(fn.find_last_of(".") + 1) == "z80") || (fn.substr(fn.find_last_of(".") + 1) == "Z80"))
		{
			FGameSnapshot newGame;
			newGame.FileName = fn;
			newGame.DisplayName = fn;
			newGame.Type = ESnapshotType::Z80;
			GamesList.push_back(newGame);
		}
		else if ((fn.substr(fn.find_last_of(".") + 1) == "sna") || (fn.substr(fn.find_last_of(".") + 1) == "SNA"))
		{
			FGameSnapshot newGame;
			newGame.FileName = fn;
			newGame.DisplayName = fn;
			newGame.Type = ESnapshotType::SNA;
			GamesList.push_back(newGame);
		}
	}
	return true;
}

bool FGamesList::LoadGame(int index)
{
	if (index < 0 || index >= GamesList.size())
		return false;

	const std::string gameFile = "Games/" + GamesList[index].FileName;
	return LoadGame(gameFile.c_str());
}

bool FGamesList::LoadGame(const char* pFileName)
{
	const std::string fn(pFileName);
	if ((fn.substr(fn.find_last_of(".") + 1) == "z80") || (fn.substr(fn.find_last_of(".") + 1) == "Z80"))
		return LoadZ80File(pSpectrumEmu, pFileName);
	else if ((fn.substr(fn.find_last_of(".") + 1) == "sna") || (fn.substr(fn.find_last_of(".") + 1) == "SNA"))
		return LoadSNAFile(pSpectrumEmu, pFileName);
	else
		return false;
}
