#include "Misc/GamesList.h"

class FCpcEmu;

class FCpcGameLoader : public IGameLoader
{
public:
	// IGameLoader
	bool LoadGame(const char* pFileName) override;
	ESnapshotType GetSnapshotTypeFromFileName(const std::string& fn) override;
	// ~IGameLoader

	void Init(FCpcEmu* pCpc)
	{
		pCpcEmu = pCpc;
	}

	// When caching is enabled, repeated loading of the same snapshot will not need to load the file from disk.
	void SetCachingEnabled(bool bEnabled);
	void ClearCache();

private:
	FCpcEmu* pCpcEmu = 0;

	bool bCachingEnabled = false;
	uint8_t* pDataCache = 0;
	size_t CachedDataSize = 0;
	std::string CachedFilename;
};