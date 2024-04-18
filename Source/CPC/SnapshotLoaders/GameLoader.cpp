#if 0

#include "GameLoader.h"

#include "SNALoader.h"
#include "../CPCEmu.h"

#include <string>
#include <algorithm>

void FCPCGameLoader::Init(FCPCEmu* pEmu, ECPCModel fallbackCPCModel)
{
	pCPCEmu = pEmu;
	FallbackCPCModel = fallbackCPCModel;
}

bool FCPCGameLoader::LoadSnapshot(const FGameSnapshot& snapshot)
{
	switch (snapshot.Type)
	{
	case ESnapshotType::SNA:
	{
		return LoadSNAFile(pCPCEmu, snapshot.FileName.c_str(), FallbackCPCModel);;
	}

	default: return false;
	}
}

void FCPCGameLoader::SetFallbackCPCModel(ECPCModel cpcModel)
{
	FallbackCPCModel = cpcModel;
}
#endif

