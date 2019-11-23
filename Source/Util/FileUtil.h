#pragma once
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
	EType			FileType;
};

typedef std::vector<FDirEntry>	FDirFileList;

bool DetermineDataDirectory(const char *pRouteIdentifier);
const std::string &GetDataDirectory();

void NormaliseFilePath(char* filePath);
void NormaliseFilePath(char* outFilePath, const char* inFilePath);

bool EnumerateDirectory(const char *dir, FDirFileList &outDirListing);
bool EnsureDirectoryExists(const char *pDirectory);	// Ensure a directory exists creating it if it doesn't, returns if it was created

bool FileExists(const char *pFilename);
char *LoadTextFile(const char *pFilename);
void *LoadBinaryFile(const char *pFilename, size_t &byteCount);

bool OpenURLInBrowser(const char *pURL);
bool OpenDirectory(const char *pDirName);
bool DownloadURLToTextFile(const char*site, const char *url, bool bHttps, const char *fname);
bool DownloadURLToBinaryFile(const char*site, const char *url, bool bHttps, const char *fname);
bool DownloadURLToString(const char*site, const char *url, bool bHttps, std::string &outputString);

bool OpenFileDialog(std::string &outFile, const char *pInitialDir);