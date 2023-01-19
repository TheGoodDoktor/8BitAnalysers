#include "FileUtil.h"
#include <string.h>

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
	char *pTextData = new char[fileSize];
	if (pTextData == nullptr)
		return nullptr;
	fread(pTextData, fileSize, 1, fp);
	fclose(fp);

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
	sprintf(hexStr, "0x%x", val);
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