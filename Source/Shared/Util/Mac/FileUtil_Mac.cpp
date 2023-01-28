#include  "../FileUtil.h"

#include <string.h>
#include <sys/stat.h>

bool CreateDir(const char* osDir)
{
	struct stat st = { 0 };

	if (stat(osDir, &st) == -1)
	{
		mkdir(osDir, 0700);
		return true;
	}

	return false;
}

char GetDirSep()
{
	return '/';
}
