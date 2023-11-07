#include "GameLoader.h"

#include "Z80Loader.h"
#include "SNALoader.h"
#include "RZXLoader.h"
#include "TAPLoader.h"
#include "TZXLoader.h"

#include <string>
#include "../SpectrumEmu.h"
#include "../ZXSpectrumConfig.h"

bool FZXGameLoader::LoadSnapshot(const FGameSnapshot& snapshot)
{
	//FZXSpectrumConfig*	pGlobalConfig = pSpectrumEmu->pGlobalConfig;

	const char* pFileName = snapshot.FileName.c_str();

	switch (snapshot.Type)
	{
	case ESnapshotType::Z80:
		return LoadZ80File(pSpectrumEmu, pFileName);
	case ESnapshotType::SNA:
		return LoadSNAFile(pSpectrumEmu, pFileName);
	case ESnapshotType::TAP:
		return LoadTAPFile(pSpectrumEmu, pFileName);
	case ESnapshotType::TZX:
		return LoadTZXFile(pSpectrumEmu, pFileName);
	default: 
		return false;
	}
}

#if 0
ESnapshotType FZXGameLoader::GetSnapshotTypeFromFileName(const std::string& fn)
{
	if ((fn.substr(fn.find_last_of(".") + 1) == "z80") || (fn.substr(fn.find_last_of(".") + 1) == "Z80"))
		return ESnapshotType::Z80;
	else if ((fn.substr(fn.find_last_of(".") + 1) == "sna") || (fn.substr(fn.find_last_of(".") + 1) == "SNA"))
		return ESnapshotType::SNA;
	else if ((fn.substr(fn.find_last_of(".") + 1) == "rzx") || (fn.substr(fn.find_last_of(".") + 1) == "RZX"))
		return ESnapshotType::RZX;
	else if ((fn.substr(fn.find_last_of(".") + 1) == "tap") || (fn.substr(fn.find_last_of(".") + 1) == "TAP"))
		return ESnapshotType::TAP;
	else if ((fn.substr(fn.find_last_of(".") + 1) == "tzx") || (fn.substr(fn.find_last_of(".") + 1) == "TZX"))
		return ESnapshotType::TZX;
	else
		return ESnapshotType::Unknown;
}
#endif