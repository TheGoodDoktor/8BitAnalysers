#include "GameLoader.h"

#include "SNALoader.h"
#include <string>
#include <algorithm>

void FCpcGameLoader::SetCachingEnabled(bool bEnabled)
{
	ClearCache();
	bCachingEnabled = bEnabled;
}

void FCpcGameLoader::ClearCache()
{
	if (pDataCache) 
		free(pDataCache);
	pDataCache = nullptr;
	CachedFilename = "";
	CachedDataSize = 0;
}

bool FCpcGameLoader::LoadGame(const char* pFileName)
{
	const std::string fn(pFileName);

	switch (GetSnapshotTypeFromFileName(fn))
	{
	case ESnapshotType::SNA:
	{
		bool bOk = false;
		if (bCachingEnabled)
		{
			if (CachedFilename != pFileName)
				ClearCache();

			bOk = LoadSNAFileCached(pCpcEmu, pFileName, pDataCache, CachedDataSize);

			if (bOk)
				CachedFilename = pFileName;
			else
				ClearCache();
		}
		else
		{
			bOk = LoadSNAFile(pCpcEmu, pFileName);
		}
		return bOk;
	}

	default: return false;
	}
}

ESnapshotType FCpcGameLoader::GetSnapshotTypeFromFileName(const std::string& fn)
{
	std::string fnLower = fn;
	std::transform(fnLower.begin(), fnLower.end(), fnLower.begin(), [](unsigned char c){ return std::tolower(c); });

	if (fnLower.substr(fn.find_last_of(".") + 1) == "sna")
		return ESnapshotType::SNA;
	else
		return ESnapshotType::Unknown;
}
