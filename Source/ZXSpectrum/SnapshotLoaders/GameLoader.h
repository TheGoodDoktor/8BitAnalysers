#pragma once

#include "Misc/GamesList.h"

class FSpectrumEmu;
struct FGameSnapshot;

class FZXGameLoader : public IGameLoader
{
public:
	// IGameLoader
	bool LoadSnapshot(const FGameSnapshot& snapshot) override;
	//ESnapshotType GetSnapshotTypeFromFileName(const std::string& fn) override;
	// ~IGameLoader

	void Init(FSpectrumEmu* pCpc) {	pSpectrumEmu = pCpc; }

private:
	FSpectrumEmu* pSpectrumEmu = nullptr;
};