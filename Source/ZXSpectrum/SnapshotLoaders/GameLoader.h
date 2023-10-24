#include "Misc/GamesList.h"

class FSpectrumEmu;

class FZXGameLoader : public IGameLoader
{
public:
	// IGameLoader
	bool LoadGame(const char* pFileName) override;
	ESnapshotType GetSnapshotTypeFromFileName(const std::string& fn) override;
	// ~IGameLoader

	void Init(FSpectrumEmu* pCpc)
	{
		pSpectrumEmu = pCpc;
	}

private:
	FSpectrumEmu* pSpectrumEmu = 0;
};