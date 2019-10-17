#include "FileUtil.h"


static std::string g_DataDirFromRoot("Tools/MapDataTool/Data/");
static std::string g_DataDirectory;

const std::string &GetDataDirectory()
{
	return g_DataDirectory;
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
	FILE *fp = fopen(pFilename, "rt");
	if (fp == nullptr)
		return false;
	fclose(fp);

	return true;
}

char *LoadTextFile(const char *pFilename)
{
	FILE *fp = fopen(pFilename, "rt");
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
	FILE *fp = fopen(pFilename, "rb");
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
