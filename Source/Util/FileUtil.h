#pragma once
#include "EASTL/string.h"
#include "EASTL/vector.h"
#include "rapidjson/document.h"

struct FDirEntry
{
	enum EType
	{
		Directory,
		File
	};

	eastl::string	FileName;
	EType			FileType;
};

typedef eastl::vector<FDirEntry>	FDirFileList;

bool DetermineDataDirectory(const char *pRouteIdentifier);
const eastl::string &GetDataDirectory();

void NormaliseFilePath(char* filePath);
void NormaliseFilePath(char* outFilePath, const char* inFilePath);

bool EnumerateDirectory(const char *dir, FDirFileList &outDirListing);
bool EnsureDirectoryExists(const char *pDirectory);	// Ensure a directory exists creating it if it doesn't, returns if it was created

bool FileExists(const char *pFilename);
char *LoadTextFile(const char *pFilename);
bool LoadJSONFileIntoDocument(const char *pPath, rapidjson::Document &document);
bool WriteJsonDocumentToFile(rapidjson::Document &doc, const char *pFileName);

bool OpenURLInBrowser(const char *pURL);
bool OpenDirectory(const char *pDirName);
bool DownloadURLToTextFile(const char*site, const char *url, bool bHttps, const char *fname);
bool DownloadURLToBinaryFile(const char*site, const char *url, bool bHttps, const char *fname);
bool DownloadURLToString(const char*site, const char *url, bool bHttps, eastl::string &outputString);
bool DownloadURLToJSONDocument(const char*site, const char *url, bool bHttps, rapidjson::Document &document);

bool OpenFileDialog(eastl::string &outFile, const char *pInitialDir);