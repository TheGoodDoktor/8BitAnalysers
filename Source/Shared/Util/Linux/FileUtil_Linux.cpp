#include  "../FileUtil.h"

#include <string.h>

bool EnumerateDirectory(const char *dir, FDirFileList &outDirListing)
{

	return false;
}

bool CreateDir(const char* osDir)
{
	return false;
}

char GetDirSep()
{
	return '/';
}

bool EnsureDirectoryExists(const char *pDirectory)
{
	const char sep = '\\';

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

void NormaliseFilePath(char* outFilePath, const char* inFilePath)
{
	strcpy(outFilePath, inFilePath);	// TODO: fix this
	NormaliseFilePath(outFilePath);
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

std::string g_BrowserURL;

bool OpenURLInBrowser(const char *pURL)
{
	
	return false;
}

bool OpenDirectory(const char *pDirName)
{
	
	return false;
}

bool OpenFileDialog(std::string &outFile,const char *pInitialDir, const char *pFilter)
{
	
	return false;
}

