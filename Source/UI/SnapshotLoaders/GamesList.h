#pragma once

#include <string>
#include <vector>

class FSpectrumEmu;

enum class ESnapshotType
{
	Z80,
	SNA,
	RZX,

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
	bool	EnumerateGames(const char* pDir);
	bool	LoadGame(int index);
	bool	LoadGame(const char* pFileName);

	int		GetNoGames() const { return (int)GamesList.size(); }
	const FGameSnapshot& GetGame(int index) const { return GamesList[index]; }
	//const std::string& GetGameName(int index) const { return GamesList[index].DisplayName; }

private:
	FSpectrumEmu* pSpectrumEmu = nullptr;
	std::vector< FGameSnapshot>	GamesList;
};