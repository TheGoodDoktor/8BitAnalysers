#include "GamesList.h"
#include "Util/FileUtil.h"
#include "Z80Loader.h"
#include "SNALoader.h"
#include "RZXLoader.h"
#include "TAPLoader.h"
#include "TZXLoader.h"

ESnapshotType GetSnapshotTypeFromFileName(const std::string& fn)
{
	if ((fn.substr(fn.find_last_of(".") + 1) == "z80") || (fn.substr(fn.find_last_of(".") + 1) == "Z80"))
		return ESnapshotType::Z80;
	else if ((fn.substr(fn.find_last_of(".") + 1) == "sna") || (fn.substr(fn.find_last_of(".") + 1) == "SNA"))
		return ESnapshotType::SNA;
	else if ((fn.substr(fn.find_last_of(".") + 1) == "rzx") || (fn.substr(fn.find_last_of(".") + 1) == "RZX"))
		return ESnapshotType::RZX;
	else if ((fn.substr(fn.find_last_of(".") + 1) == "tap") || (fn.substr(fn.find_last_of(".") + 1) == "TAP"))
		return ESnapshotType::TAP;
	else if ((fn.substr(fn.find_last_of(".") + 1) == "tzx") || (fn.substr(fn.find_last_of(".") + 1) == "TZX"))
		return ESnapshotType::TZX;
	else
		return ESnapshotType::Unknown;
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
			newGame.DisplayName = file.FileName;
			newGame.Type = type;
			GamesList.push_back(newGame);
		}
	}
	return true;
}

bool FGamesList::LoadGame(int index)
{
	if (index < 0 || index >= GamesList.size())
		return false;

	return LoadGame(GamesList[index].FileName.c_str());
}

bool FGamesList::LoadGame(const char* pFileName)
{
	const std::string fn(pFileName);

	switch (GetSnapshotTypeFromFileName(pFileName))
	{
	case ESnapshotType::Z80:
		return LoadZ80File(pSpectrumEmu, pFileName);
	case ESnapshotType::SNA:
		return LoadSNAFile(pSpectrumEmu, pFileName);
	case ESnapshotType::TAP:
		return LoadTAPFile(pSpectrumEmu, pFileName);
	case ESnapshotType::TZX:
		return LoadTZXFile(pSpectrumEmu, pFileName);
	default: return false;
	}
}
