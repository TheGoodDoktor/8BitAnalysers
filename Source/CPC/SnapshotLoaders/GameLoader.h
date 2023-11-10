#include "Misc/GamesList.h"

class FCPCEmu;

class FCPCGameLoader : public IGameLoader
{
public:
	// IGameLoader
	bool LoadSnapshot(const FGameSnapshot& snapshot) override;
	//ESnapshotType GetSnapshotTypeFromFileName(const std::string& fn) override;
	// ~IGameLoader

	void Init(FCPCEmu* pEmu)
	{
		pCPCEmu = pEmu;
	}

	// When caching is enabled, repeated loading of the same snapshot will not need to load the file from disk.
	void SetCachingEnabled(bool bEnabled);
	void ClearCache();

private:
	FCPCEmu* pCPCEmu = 0;

	bool bCachingEnabled = false;
	uint8_t* pDataCache = 0;
	size_t CachedDataSize = 0;
	std::string CachedFilename;
};