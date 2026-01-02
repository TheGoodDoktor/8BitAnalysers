#include "FileUtil.h"
#include <string.h>

#undef UNICODE 
#undef _UNICODE 
#include <tinydir/tinydir.h>

static std::string g_DataDirFromRoot("Tools/MapDataTool/Data/");
static std::string g_DataDirectory;

const std::string &GetDataDirectory()
{
	return g_DataDirectory;
}

std::string RemoveFileExtension(const char* fname)
{
	std::string fullname(fname);
	size_t lastindex = fullname.find_last_of(".");
	return fullname.substr(0, lastindex);
}

std::string GetFileFromPath(const char* fname)
{
	std::string fullname(fname);
	return fullname.substr(fullname.find_last_of("/\\") + 1);
}
bool EnumerateDirectory(const char* dirName, FDirFileList& outDirListing)
{
	tinydir_dir dir;
	tinydir_open(&dir, dirName);

	while (dir.has_next)
	{
		tinydir_file file;
		tinydir_readfile(&dir, &file);

		FDirEntry entry;

		entry.FileName = file.name;
		if (file.is_dir)
			entry.FileType = FDirEntry::Directory;

		outDirListing.push_back(entry);
		tinydir_next(&dir);
	}

	tinydir_close(&dir);
	return true;
}

bool DetermineDataDirectory(const char *pRouteIdentifier)
{
	g_DataDirectory = "";
	bool bFound = false;

	while (bFound == false)
	{
		FDirFileList dirListing;

		if (EnumerateDirectory(g_DataDirectory.c_str(), dirListing) == false)
			return false;

		for (FDirFileList::iterator entry = dirListing.begin(); entry < dirListing.end(); ++entry)
		{
			if (entry->FileType == FDirEntry::EType::File)
			{
				if (strcmp(entry->FileName.c_str(), ".root.txt") == 0)
				{
					g_DataDirectory += g_DataDirFromRoot;
					return true;
				}
			}
		}

		g_DataDirectory += "../";
	}

	return false;
}

bool FileExists(const char *pFilename)
{
	FILE* fp = fopen(pFilename, "rt");
	if (fp == nullptr)
		return false;
	fclose(fp);

	return true;
}

char *LoadTextFile(const char *pFilename)
{
	FILE* fp = fopen(pFilename, "rt");
	if (fp == nullptr)
		return nullptr;

	fseek(fp, 0, SEEK_END);
	size_t fileSize = ftell(fp);
	fseek(fp, 0, SEEK_SET);
	char *pTextData = new char[fileSize + 1];
	if (pTextData == nullptr)
		return nullptr;
	const size_t bytesRead = fread(pTextData,1, fileSize, fp);
	fclose(fp);
	pTextData[bytesRead] = 0;	// null terminator
	return pTextData;
}

void *LoadBinaryFile(const char *pFilename, size_t &byteCount)
{
	FILE* fp = fopen(pFilename, "rb");
	if (fp == nullptr)
		return nullptr;

	fseek(fp, 0, SEEK_END);
	byteCount = ftell(fp);
	fseek(fp, 0, SEEK_SET);
	void *pFileData = malloc(byteCount);
	if (pFileData == nullptr)
		return nullptr;
	fread(pFileData, byteCount, 1, fp);
	fclose(fp);

	return pFileData;
}

bool LoadBinaryFileToMem(const char* pFilename, void* pMem, size_t size)
{
	FILE* fp = fopen(pFilename, "rb");
	if (fp == nullptr)
		return false;

	fread(pMem, size, 1, fp);
	fclose(fp);
	return true;
}

bool SaveTextFile(const char* pFilename, const char* pText)
{
    FILE* fp = fopen(pFilename, "wt");
    if (fp == nullptr)
        return false;
    const size_t length = strlen(pText);
    fwrite(pText, length, 1, fp);
    fclose(fp);
    
    return true;
}

bool SaveBinaryFile(const char *pFilename, const void * pData,size_t byteCount)
{
	FILE* fp = fopen(pFilename, "wb");
	if (fp == nullptr)
		return false;
	
	fwrite(pData, byteCount, 1, fp);
	fclose(fp);

	return true;
}

void WriteStringToFile(const std::string& str, FILE* fp)
{
	const int stringLength = (int)str.size();
	fwrite(&stringLength, sizeof(int), 1, fp);
	fwrite(str.c_str(), 1, stringLength, fp);
}

void ReadStringFromFile(std::string& str, FILE* fp)
{
	int stringLength = 0;
	fread(&stringLength, sizeof(int), 1, fp);
	str.resize(stringLength);
	fread(&str[0], 1, stringLength, fp);
}

std::string MakeHexString(uint16_t val)
{
	char hexStr[16];
	snprintf(hexStr,16, "0x%x", val);
	return std::string(hexStr);
}

uint8_t ParseHexString8bit(const std::string& string)
{
	unsigned int val;
	sscanf(string.c_str(), "0x%x", &val);
	return static_cast<uint8_t>(val);
}

uint16_t ParseHexString16bit(const std::string& string)
{
	unsigned int val;
	sscanf(string.c_str(), "0x%x", &val);
	return static_cast<uint16_t>(val);
}

// directory related helpers
void NormaliseFilePath(char* outFilePath, const char* inFilePath);


bool EnsureDirectoryExists(const char* pDirectory)
{
	const char sep = GetDirSep();

	//    Debug::Error("Creating dir chain %s", osDir_);

	char osDir[FILENAME_MAX] = { 0 };
	NormaliseFilePath(osDir, pDirectory);

	if (osDir[0] == 0)
		return false;

	const char* left = osDir;
	for (const char* right = osDir + 1; *right; ++right)
	{
		if (*right == sep)
		{
			*(char*)right = 0;
			const char* partialPath = left;

			if (!CreateDir(partialPath))
				return false;

			*(char*)right = sep;
		}
	}

	return CreateDir(osDir);
}


void NormaliseFilePath(char* filePath)
{
	bool preceedingSlash = false;

	for (char* cursor = filePath; *cursor; ++cursor)
	{
		char c = *cursor;
		//Normalise slashes
		if (c == '\\' || c == '/')
			c = GetDirSep();

		//Ignore duplicate slashes
		if (c == GetDirSep())
		{
			if (preceedingSlash)
				continue;
			preceedingSlash = true;
		}
		else
		{
			preceedingSlash = false;
		}

		*cursor = c;
	}
}

void NormaliseFilePath(char* outFilePath, const char* inFilePath)
{
	strncpy(outFilePath, inFilePath, 256);	// TODO: fix this
	NormaliseFilePath(outFilePath);
}
