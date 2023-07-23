#pragma once

#include <cstdint>
#include <string>
#include <vector>

struct FDirEntry
{
	enum EType
	{
		Directory,
		File
	};

	std::string	FileName;
	EType			FileType = EType::File;
};

typedef std::vector<FDirEntry>	FDirFileList;

bool DetermineDataDirectory(const char *pRouteIdentifier);
const std::string &GetDataDirectory();

std::string RemoveFileExtension(const char* fname);
std::string GetFileFromPath(const char* fname);

bool EnumerateDirectory(const char *dir, FDirFileList &outDirListing);
bool EnsureDirectoryExists(const char *pDirectory);	// Ensure a directory exists creating it if it doesn't, returns if it was created

bool FileExists(const char *pFilename);
char *LoadTextFile(const char *pFilename);
void *LoadBinaryFile(const char *pFilename, size_t &byteCount);
bool SaveBinaryFile(const char *pFilename, const void * pData, size_t byteCount);

void WriteStringToFile(const std::string& str, FILE* fp);
void ReadStringFromFile(std::string& str, FILE* fp);
std::string MakeHexString(uint16_t val);
uint8_t ParseHexString8bit(const std::string& string);
uint16_t ParseHexString16bit(const std::string& string);

bool CreateDir(const char* osDir);
char GetDirSep();