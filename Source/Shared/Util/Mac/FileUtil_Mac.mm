#include  "../FileUtil.h"

#import <Foundation/Foundation.h>
#import <Appkit/AppKit.h>

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

#define PLATFORM_MAX_PATH 256
static char g_appSupportPath[PLATFORM_MAX_PATH];
static char g_documentPath[PLATFORM_MAX_PATH];
static char g_bundlePath[PLATFORM_MAX_PATH];

const char *GetBundlePath(const char *fileName)
{
	static char path[PLATFORM_MAX_PATH];

	// App resources are presently being installed alongside the executable in
	// Contents/MacOS which is unusual. They're normally kept in the Resources
	// directory.
	snprintf(path, sizeof(path), "%s/Contents/MacOS/%s", g_bundlePath, fileName);
	return path;
}

const char *GetDocumentsPath(const char *fileName)
{
	static char path[PLATFORM_MAX_PATH];

	snprintf(path, sizeof(path), "%s/%s", g_documentPath, fileName);
	return path;
}

const char *GetAppSupportPath(const char *fileName)
{
	static char path[PLATFORM_MAX_PATH];

	snprintf(path, sizeof(path), "%s/%s", g_appSupportPath, fileName);
	return path;
}

void FileInit(void)
{
	// Find bundle path
	NSString *bundlePath = [[NSBundle mainBundle] bundlePath];

	NSFileManager *defaultFileManager = [NSFileManager defaultManager];

	// Find document path
	NSArray *documentsPaths = [defaultFileManager URLsForDirectory:NSDocumentDirectory inDomains:NSUserDomainMask];
	NSString *documentsPath = [documentsPaths firstObject];

	// Find application support path
	NSArray *applicationSupportDirectory = [defaultFileManager URLsForDirectory:NSApplicationSupportDirectory inDomains:NSUserDomainMask];
	NSString *spectrumAnalyserDirectory = nil;
	if ([applicationSupportDirectory count] > 0)
	{
		NSString *bundleID = [[NSBundle mainBundle] bundleIdentifier];
		NSURL *spectrumAnalyserDirectoryURL = nil;
		NSError *theError = nil;

		spectrumAnalyserDirectoryURL = [[applicationSupportDirectory objectAtIndex:0] URLByAppendingPathComponent:bundleID];

		if (![defaultFileManager createDirectoryAtURL:spectrumAnalyserDirectoryURL
						  withIntermediateDirectories:YES
										   attributes:nil
												error:&theError])
		{
			NSLog(@"Create directory error: %@", theError);
			return;
		}

		spectrumAnalyserDirectory = [spectrumAnalyserDirectoryURL path];
	}

	snprintf(g_bundlePath, PLATFORM_MAX_PATH, "%s", [bundlePath fileSystemRepresentation]);
	snprintf(g_documentPath, PLATFORM_MAX_PATH, "%s", [documentsPath fileSystemRepresentation]);
	snprintf(g_appSupportPath, PLATFORM_MAX_PATH, "%s", [spectrumAnalyserDirectory fileSystemRepresentation]);
}
