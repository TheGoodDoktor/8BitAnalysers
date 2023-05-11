#pragma once

#include <string>
#include <vector>

class FSpectrumEmu;

enum class ESnapshotType
{
	Z80,
	SNA,
	RZX,
	TAP,
	TZX,

	Unknown
};

struct FGameSnapshot
{
	ESnapshotType	Type;
	std::string		DisplayName;
	std::string		FileName;
};

class FGamesList
{
public:
	void	Init(FSpectrumEmu* pEmu) { pSpectrumEmu = pEmu; }
	bool	EnumerateGames(const char* pRootDir);
	bool	LoadGame(int index);
	bool	LoadGame(const char* pFileName);

	int		GetNoGames() const { return (int)GamesList.size(); }
	const FGameSnapshot& GetGame(int index) const { return GamesList[index]; }
	//const std::string& GetGameName(int index) const { return GamesList[index].DisplayName; }

private:
	FSpectrumEmu* pSpectrumEmu = nullptr;
	std::vector< FGameSnapshot>	GamesList;
	std::string RootDir;
};

ESnapshotType GetSnapshotTypeFromFileName(const std::string& fn);