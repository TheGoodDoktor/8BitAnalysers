#include "SaveGame.h"

#include <vector>

struct FSaveFile
{
	static const int kSaveGameSize = 256;

	char	Name[32] = {0};			// Save game name
	uint8_t	SaveData[kSaveGameSize] = {0};
};

static std::vector<FSaveFile> SaveFiles;

FSaveFile* FindSaveFile(const char* pFilename)
{
	for (auto& saveFile : SaveFiles)
	{
		if (strcmp(saveFile.Name, pFilename) == 0)
			return &saveFile;
	}
	return nullptr;
}

void InitSaveGames()
{
	// Initialize save game system
}

bool SaveGame(const char* pFilename, const uint8_t* pData)
{
	FSaveFile* pSaveFile = FindSaveFile(pFilename);
	if (!pSaveFile)
	{
		// Create a new save file
		FSaveFile newSaveFile;
		strncpy(newSaveFile.Name, pFilename, sizeof(newSaveFile.Name) - 1);
		newSaveFile.Name[sizeof(newSaveFile.Name) - 1] = '\0'; // Ensure null termination
		memcpy(newSaveFile.SaveData, pData, FSaveFile::kSaveGameSize);
		SaveFiles.push_back(newSaveFile);
		return true;
	}
	else
	{
		// Update existing save file
		memcpy(pSaveFile->SaveData, pData, FSaveFile::kSaveGameSize);
		return true;
	}

	return false;
}

bool LoadGame(const char* pFilename, uint8_t* pData)
{
	FSaveFile* pSaveFile = FindSaveFile(pFilename);
	if (pSaveFile)
	{
		memcpy(pData, pSaveFile->SaveData, FSaveFile::kSaveGameSize);
		return true;
	}
	return false;
}

void CatalogueSaveGames()
{
	// List all save games
	for (const auto& saveFile : SaveFiles)
	{
		// TODO: Send to VDU
		printf("Save Game: %s\n", saveFile.Name);
	}
}