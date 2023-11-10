#include "GameLoader.h"

#include "SNALoader.h"
#include <string>
#include <algorithm>

void FCPCGameLoader::SetCachingEnabled(bool bEnabled)
{
	ClearCache();
	bCachingEnabled = bEnabled;
}

void FCPCGameLoader::ClearCache()
{
	if (pDataCache) 
		free(pDataCache);
	pDataCache = nullptr;
	CachedFilename = "";
	CachedDataSize = 0;
}

bool FCPCGameLoader::LoadSnapshot(const FGameSnapshot& snapshot)
{
	switch (snapshot.Type)
	{
	case ESnapshotType::SNA:
	{
		bool bOk = false;
		if (bCachingEnabled)
		{
			if (CachedFilename != snapshot.FileName)
				ClearCache();

			bOk = LoadSNAFileCached(pCPCEmu, snapshot.FileName.c_str(), pDataCache, CachedDataSize);

			if (bOk)
				CachedFilename = snapshot.FileName;
			else
				ClearCache();
		}
		else
		{
			bOk = LoadSNAFile(pCPCEmu, snapshot.FileName.c_str());
		}
		return bOk;
	}

	default: return false;
	}
}

