#include  "../FileUtil.h"

//#include "Debug/Debug.h"
#include <assert.h>
#include <windows.h>
#include <winhttp.h>
#include <strsafe.h>
#include <shobjidl.h> 
#include <functional>
//#pragma comment(lib, "User32.lib")
//#pragma comment(lib, "WinHttp.lib")


#if 0
bool EnumerateDirectory_Win32(const char *dir, FDirFileList &outDirListing)
{
	WIN32_FIND_DATA ffd;
	TCHAR szDir[MAX_PATH];

	StringCchCopy(szDir, MAX_PATH, dir);
	StringCchCat(szDir, MAX_PATH, TEXT("\\*"));

	HANDLE hFind = FindFirstFile(szDir, &ffd);

	if (hFind == INVALID_HANDLE_VALUE)
	{
		return false;
	}

	do
	{
		FDirEntry entry;

		entry.FileName = std::string(ffd.cFileName);

		if (ffd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)
		{
			//_tprintf(TEXT("  %s   <DIR>\n"), ffd.cFileName);
			entry.FileType = FDirEntry::Directory;
		}
		else
		{
			entry.FileType = FDirEntry::File;

			//filesize.LowPart = ffd.nFileSizeLow;
			//filesize.HighPart = ffd.nFileSizeHigh;
			//_tprintf(TEXT("  %s   %ld bytes\n"), ffd.cFileName, filesize.QuadPart);
		}

		outDirListing.push_back(entry);

	} while (FindNextFile(hFind, &ffd) != 0);

	return true;
}
#endif

bool CreateDir(const char* osDir)
{
	DWORD ftyp = GetFileAttributesA(osDir);
	if (ftyp != INVALID_FILE_ATTRIBUTES)
	{
		if ((ftyp & FILE_ATTRIBUTE_DIRECTORY) != 0)
			return true;

		//LOGERROR("File obstructing dir creation %s", osDir);
		return false;
	}

	if (!CreateDirectoryA(osDir, NULL) && GetLastError() != ERROR_PATH_NOT_FOUND)
	{
		//LOGERROR("Mkdir failed for %s", osDir);
		return false;
	}

	return true;
}

char GetDirSep()
{
	return '\\';
}


#if 0
std::string g_BrowserURL;

bool OpenURLInBrowser(const char *pURL)
{
	g_BrowserURL = pURL;
	ShellExecute(NULL, "open", g_BrowserURL.c_str(), nullptr, nullptr, SW_SHOWDEFAULT);
	return true;
}

bool OpenDirectory(const char *pDirName)
{
	char tempDir[256];
	assert(strlen(pDirName) < 256);
	NormaliseFilePath(tempDir, pDirName);
	ShellExecute(NULL, "open", tempDir, nullptr, nullptr, SW_SHOWDEFAULT);
	return true;
}

// download file
bool DownloadURLToFile(const char*site, const char *url, bool bHttps, std::function<void(const void *, size_t)> readFunc)
{
	DWORD dwSize = 0;
	DWORD dwDownloaded = 0;
	const char *kHTTPS = "https:";
	//const bool bHttps = strncmp(site, kHTTPS, strlen(kHTTPS)) == 0;

	wchar_t siteStr[32];
	wchar_t urlStr[128];
	size_t noCharsConverted;
	size_t size1 = mbstowcs_s(&noCharsConverted,siteStr, site, 32);
	size_t size2 = mbstowcs_s(&noCharsConverted,urlStr, url, 128);
	
	// Use WinHttpOpen to obtain a session handle.
	HINTERNET hSession = WinHttpOpen(L"MapDataTool/1.0", WINHTTP_ACCESS_TYPE_DEFAULT_PROXY, WINHTTP_NO_PROXY_NAME, WINHTTP_NO_PROXY_BYPASS, 0);
	if (!hSession)
	{
		//LOGERROR("DownloadFileFromURL: couldn't create session");
		return false;
	}
	
	HINTERNET hConnect = WinHttpConnect(hSession, siteStr, bHttps ? INTERNET_DEFAULT_HTTPS_PORT : INTERNET_DEFAULT_HTTP_PORT, 0);
	if (!hConnect)
	{
		const DWORD errorCode = GetLastError();
		switch (errorCode)
		{
		case ERROR_WINHTTP_INCORRECT_HANDLE_TYPE:
			//LOGERROR("DownloadFileFromURL: couldn't create connection - Incorrect Handle Type");
			break;
		case ERROR_WINHTTP_INVALID_URL:
			//LOGERROR("DownloadFileFromURL: couldn't create connection - Invalid URL: %S", siteStr);
			break;
		default:
			//LOGERROR("DownloadFileFromURL: couldn't create connection - Error Code %d", errorCode);
			break;

		}
		WinHttpCloseHandle(hSession);
		return false;
	}

	HINTERNET hRequest = WinHttpOpenRequest(hConnect, L"GET",urlStr,NULL, WINHTTP_NO_REFERER,NULL, bHttps ? WINHTTP_FLAG_SECURE : NULL);
	if (!hRequest)
	{
		//LOGERROR("DownloadFileFromURL: couldn't create request- %d", GetLastError());
		//fclose(fp);
		WinHttpCloseHandle(hConnect);
		WinHttpCloseHandle(hSession);
		return false;
	}

	BOOL bResults = WinHttpSendRequest(hRequest,
			WINHTTP_NO_ADDITIONAL_HEADERS,
			0, WINHTTP_NO_REQUEST_DATA, 0,
			0, 0);

	bResults = WinHttpReceiveResponse(hRequest, NULL);
	if (bResults == FALSE)
	{
		DWORD errorCode = GetLastError();
		//fclose(fp);
		WinHttpCloseHandle(hRequest);
		WinHttpCloseHandle(hConnect);
		WinHttpCloseHandle(hSession);

		switch(errorCode)
		{
		case ERROR_WINHTTP_TIMEOUT:
			//LOGERROR("DownloadFileFromURL: WinHttp request timed out");
			break;
		default:
			//LOGERROR("DownloadFileFromURL: couldn't receive response. Error %u", errorCode);
			break;
		}
		return false;
	}

	do
	{
		// Check for available data.
		dwSize = 0;
		if (!WinHttpQueryDataAvailable(hRequest, &dwSize))
		{
			//LOGERROR("Error %u in WinHttpQueryDataAvailable.\n", GetLastError());
		}

		// Allocate space for the buffer.
		char *pszOutBuffer = new char[dwSize + 1];
		if (!pszOutBuffer)
		{
			//LOGERROR("Out of memory\n");
			dwSize = 0;
		}
		else
		{
			// Read the Data.
			ZeroMemory(pszOutBuffer, dwSize + 1);

			if (!WinHttpReadData(hRequest, (LPVOID)pszOutBuffer, dwSize, &dwDownloaded))
			{
				//LOGERROR("Error %u in WinHttpReadData.\n", GetLastError());
			}
			else
			{
				readFunc(pszOutBuffer, dwSize);
				//fwrite(pszOutBuffer, 1, dwSize, fp);
				//LOGINFO("%s", pszOutBuffer);
			}

			// Free the memory allocated to the buffer.
			delete[] pszOutBuffer;
		}

	} while (dwSize > 0);

	//fclose(fp);
	if (hRequest)
		WinHttpCloseHandle(hRequest);
	if (hConnect) 
		WinHttpCloseHandle(hConnect);
	if (hSession) 
		WinHttpCloseHandle(hSession);

	return true;
}

bool DownloadURLToTextFile(const char*site, const char *url,bool bHttps, const char *fname)
{
	FILE* fp; 
	fopen_s(&fp,fname, "wt");
	if (fp == NULL)
	{
		//LOGERROR("Couldn't open file %s for writing", fname);
		return false;
	}
	const bool bSuccess =  DownloadURLToFile(site, url, bHttps,
		[fp](const void *dataPtr, size_t dataSize)
		{
			fwrite(dataPtr, 1, dataSize, fp);
		});

	fclose(fp);
	return bSuccess;
}

bool DownloadURLToBinaryFile(const char*site, const char *url, bool bHttps, const char *fname)
{
	FILE* fp;
	fopen_s(&fp, fname, "wb");
	if (fp == NULL)
	{
		//LOGERROR("Couldn't open file %s for writing", fname);
		return false;
	}
	
	const bool bSuccess = DownloadURLToFile(site, url, bHttps,
		[fp](const void *dataPtr, size_t dataSize)
	{
		fwrite(dataPtr, 1, dataSize, fp);
	});

	fclose(fp);
	return bSuccess;
}

bool DownloadURLToString(const char*site, const char *url, bool bHttps, std::string &outputString)
{
	const bool bSuccess = DownloadURLToFile(site, url, bHttps,
		[&outputString](const void *dataPtr, size_t dataSize)
		{
			outputString += std::string((const char *)dataPtr, dataSize);
		}
	);

	return bSuccess;
}


bool OpenFileDialog(std::string &outFile,const char *pInitialDir, const char *pFilter)
{
	OPENFILENAMEA openFileName = {};
	char szFile[128];

	char tempDir[256];
	assert(strlen(pInitialDir) < 256);
	NormaliseFilePath(tempDir, pInitialDir);

	// Initialize OPENFILENAME
	ZeroMemory(&openFileName, sizeof(openFileName));
	openFileName.lStructSize = sizeof(openFileName);
	openFileName.hwndOwner = NULL;
	openFileName.lpstrFile = szFile;
	// Set lpstrFile[0] to '\0' so that GetOpenFileName does not 
	// use the contents of szFile to initialize itself.
	openFileName.lpstrFile[0] = '\0';
	openFileName.nMaxFile = sizeof(szFile);
	openFileName.lpstrFilter = pFilter;
	openFileName.nFilterIndex = 1;
	openFileName.lpstrFileTitle = NULL;
	openFileName.nMaxFileTitle = 0;
	openFileName.lpstrInitialDir = tempDir;
	openFileName.Flags = OFN_PATHMUSTEXIST | OFN_FILEMUSTEXIST;
	
	BOOL ret = GetOpenFileNameA(&openFileName);

	outFile = openFileName.lpstrFile;
	
	/*
	HRESULT hr = CoInitializeEx(NULL, COINIT_APARTMENTTHREADED |
		COINIT_DISABLE_OLE1DDE);
	if (SUCCEEDED(hr))
	{
		IFileOpenDialog *pFileOpen;

		// Create the FileOpenDialog object.
		hr = CoCreateInstance(CLSID_FileOpenDialog, NULL, CLSCTX_ALL,
			IID_IFileOpenDialog, reinterpret_cast<void**>(&pFileOpen));

		if (SUCCEEDED(hr))
		{
			// Show the Open dialog box.
			hr = pFileOpen->Show(NULL);

			// Get the file name from the dialog box.
			if (SUCCEEDED(hr))
			{
				IShellItem *pItem;
				hr = pFileOpen->GetResult(&pItem);
				if (SUCCEEDED(hr))
				{
					PWSTR pszFilePath;
					hr = pItem->GetDisplayName(SIGDN_FILESYSPATH, &pszFilePath);

					// Display the file name to the user.
					if (SUCCEEDED(hr))
					{
						file.sprintf("%S", pszFilePath);
						//MessageBoxW(NULL, pszFilePath, L"File Path", MB_OK);
						CoTaskMemFree(pszFilePath);
					}
					pItem->Release();
				}
			}
			pFileOpen->Release();
		}
		CoUninitialize();
	}
	*/
	return true;
}

#endif