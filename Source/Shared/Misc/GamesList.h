#pragma once

#include <string>
#include <vector>

class FSystem;

enum class EEmuFileType
{
	Z80,
	SNA,
	TAP,
	TZX,
	RZX,
	PRG,
	D64,
	CRT,

	Unknown
};

// TODO: rename to emulator file
struct FEmulatorFile
{
	EEmuFileType	Type;
	std::string		DisplayName;
	std::string		FileName;
	std::string		ListName;
};
/*
class IGameLoader
{
public:
	virtual bool LoadSnapshot(const FGameSnapshot& snapshot) = 0;
};*/

class FGamesList
{
public:
	FGamesList() = default;
	FGamesList(const char* pListName, const char* pRootDir)
		: ListName(pListName)
		, RootDir(pRootDir)
		{}

	bool	EnumerateGames(void);
	//bool	LoadGame(int index) const;
	//bool	LoadGame(const char* pFileName) const;

	const char*		GetFileType() const { return ListName.c_str(); }
	const char*		GetRootDir() const { return RootDir.c_str(); }
	int		GetNoGames() const { return (int)GamesList.size(); }
	const FEmulatorFile& GetGame(int index) const { return GamesList[index]; }
	const FEmulatorFile* GetGame(const char* pSnapshotName) const;
	//const std::string& GetGameName(int index) const { return GamesList[index].DisplayName; }

private:
	std::string	ListName;
	std::vector< FEmulatorFile>	GamesList;
	std::string RootDir;
	//IGameLoader* pGameLoader = 0;
};

EEmuFileType GetEmuFileTypeFromFileName(const std::string& filename);