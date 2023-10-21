#pragma once

#include <string>
#include <vector>

struct FGameInfo
{
	std::string Name;
	std::string PRGFile;
};

class FC64GamesList
{
public:
	bool	EnumerateGames(const char* pDirectory);
	int		GetNoGames() const { return (int)GamesList.size(); }
	const FGameInfo* GetGameInfo(const char *pName);
	const FGameInfo& GetGameInfo(int index) { return GamesList[index]; }
	int		GetSelectedGame() const { return SelectedGame; }
	int		DrawGameSelect();
private:
	std::vector<FGameInfo>		GamesList;
	int							SelectedGame = -1;
};