#include  "../FileUtil.h"

#include <string.h>
#include <sys/stat.h>

bool CreateDir(const char* osDir)
{
	struct stat st = { 0 };

    int status = stat(osDir, &st);
    if (status == 0)
        return true; // already exists
    
	if (status == -1)
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
