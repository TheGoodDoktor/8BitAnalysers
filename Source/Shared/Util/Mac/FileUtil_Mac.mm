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



#if 1
const std::string GetApplicationDirectory()
{
    id info = [[NSBundle mainBundle] infoDictionary];
    NSString *bundleID = [info objectForKey: @"CFBundleIdentifier"];

    NSWorkspace *wp = [[NSWorkspace alloc] init];
    NSString *app   = [wp absolutePathForAppBundleWithIdentifier: bundleID];

    const char *str = [[app substringToIndex: [app length] - [[app lastPathComponent] length]] cStringUsingEncoding: NSASCIIStringEncoding];
    return std::string( str );
}
#endif

const char * GetBundlePath(const char *fileName)
{
    //std::string appDir = GetApplicationDirectory();
    
    //fileName =[[[NSBundle mainBundle] resourcePath] stringByAppendingPathComponent: @(fileName)].UTF8String;
    fileName =[[[NSBundle mainBundle] bundlePath] stringByAppendingPathComponent: @(fileName)].UTF8String;
    return fileName;
}

const char * GetDocumentsPath(const char *fileName)
{
    NSArray *docPaths = NSSearchPathForDirectoriesInDomains(NSDocumentDirectory, NSUserDomainMask, YES);
    NSString *documentsFolder = [docPaths firstObject];
    return [documentsFolder stringByAppendingPathComponent: @(fileName)].UTF8String;
 }

const char * GetAppSupportPath(const char *fileName)
{
    id info = [[NSBundle mainBundle] infoDictionary];
    NSArray *paths = NSSearchPathForDirectoriesInDomains(NSApplicationSupportDirectory, NSUserDomainMask, YES);
    NSString *bundleID = [info objectForKey: @"CFBundleIdentifier"];
    NSString *applicationSupportDirectory = [[paths firstObject] stringByAppendingPathComponent:bundleID];
    return [applicationSupportDirectory stringByAppendingPathComponent: @(fileName)].UTF8String;
    //NSString *applicationSupportDirectory = [paths firstObject];

}

#define PLATFORM_MAX_PATH 256
static char g_appSupportPath[PLATFORM_MAX_PATH];
static char g_bundlePath[PLATFORM_MAX_PATH];

void FileInit(void)
{
    id info = [[NSBundle mainBundle] infoDictionary];
    NSString *bundleID = [info objectForKey: @"CFBundleIdentifier"];

    // Setup Bundle Root
    const char* bundlePath = [[[NSBundle mainBundle] resourcePath] UTF8String];
    sprintf(g_bundlePath,"%s/",bundlePath);
    
    // Setup Application Support Dir
    //NSString* bundleID = [[NSBundle mainBundle] bundleIdentifier];
    NSFileManager*fm = [NSFileManager defaultManager];
    NSURL*    dirPath = nil;
    
    NSArray *docPaths = NSSearchPathForDirectoriesInDomains(NSDocumentDirectory, NSUserDomainMask, YES);
    NSString *documentsFolder = [docPaths firstObject];
    
    NSArray *paths = NSSearchPathForDirectoriesInDomains(NSApplicationSupportDirectory, NSUserDomainMask, YES);
    NSString *applicationSupportDirectory = [paths firstObject];
    //dirPath = [applicationSupportDirectory appendString:bundleID];

/*
    // Find the application support directory in the home directory.
    NSArray* appSupportDir = [fm URLsForDirectory:NSApplicationSupportDirectory
                                        inDomains:NSUserDomainMask];
    if ([appSupportDir count] > 0)
    {
        // Append the bundle ID to the URL for the
        // Application Support directory
        dirPath = [[appSupportDir objectAtIndex:0] URLByAppendingPathComponent:bundleID];
        
        // If the directory does not exist, this method creates it.
        // This method is only available in OS X v10.7 and iOS 5.0 or later.
        NSError*    theError = nil;
        if (![fm createDirectoryAtURL:dirPath withIntermediateDirectories:YES
                           attributes:nil error:&theError])
        {
            // Handle the error.
            
        }
    }
    */
    if(applicationSupportDirectory != nil)
    {
        const char* appSupportPath = applicationSupportDirectory.UTF8String;
        sprintf(g_appSupportPath, "%s/",appSupportPath);
    }
    
}
