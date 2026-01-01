#pragma once

#include <cstdint>

void InitSaveGames();
bool SaveGame(const char* pFilename, const uint8_t* pData);
bool LoadGame(const char* pFilename, uint8_t* pData);
void CatalogueSaveGames();