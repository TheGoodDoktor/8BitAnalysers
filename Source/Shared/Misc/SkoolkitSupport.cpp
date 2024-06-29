#include "Debug/DebugLog.h"
#include "Util/FileUtil.h"
#include <Misc/EmuBase.h>
#include <Misc/GameConfig.h>
#include "CodeAnalyser/CodeAnalysisJson.h"
#include "Exporters/SkoolFileInfo.h"
#include "Exporters/SkoolkitExporter.h"
#include "Importers/SkoolkitImporter.h"

bool BackupAnalysisJson(FEmuBase* pEmu)
{
	const FProjectConfig* pCurrentGameConfig = pEmu->GetProjectConfig();

	if (pCurrentGameConfig)
	{
		std::string analysisBackupFname = pEmu->GetGameWorkspaceRoot() + "Analysis.json.bak";

		LOGINFO("Backing up analysis data to '%s'", analysisBackupFname.c_str());

		if (!ExportAnalysisJson(pEmu->GetCodeAnalysis(), analysisBackupFname.c_str()))
		{
			LOGERROR("Could not export analysis data to '%s'", analysisBackupFname.c_str());
			return false;
		}
	}
	return true;
}

bool ImportSkoolFile(FEmuBase* pEmu, const char* pFilename, const char* pOutSkoolInfoName /* = nullptr*/, FSkoolFileInfo* pSkoolInfo /* = nullptr*/)
{
	LOGINFO("Importing skool file '%s'", pFilename);

	if (!BackupAnalysisJson(pEmu))
	{
		LOGERROR("Failed to import skool file. Could not backup analysis data");
		return false;
	}

	// use FSkoolFileInfo pointer if it's passed in.
	FSkoolFileInfo skoolInfo;
	FSkoolFileInfo* pInfo = pSkoolInfo ? pSkoolInfo : &skoolInfo;
	if (!ImportSkoolKitFile(pEmu->GetCodeAnalysis(), pFilename, pSkoolInfo ? pSkoolInfo : pInfo))
	{
		LOGINFO("Failed to import skool file '%s'", pFilename);
		return false;
	}

	const FProjectConfig* pConfig = pEmu->GetProjectConfig();
	const std::string outDir = pConfig ? pEmu->GetGameWorkspaceRoot() : pEmu->GetGlobalConfig()->WorkspaceRoot;
	const std::string outFname = pOutSkoolInfoName ? pOutSkoolInfoName : "Out";
	const std::string skoolInfoFname(outDir + outFname + std::string(".skoolinfo"));
	LOGINFO("Saving skoolinfo file '%s'", skoolInfoFname.c_str());
	if (!SaveSkoolFileInfo(*pInfo, skoolInfoFname.c_str()))
	{
		LOGINFO("Failed to save skoolinfo file '%s'", skoolInfoFname.c_str());
		return false;
	}

	LOGINFO("Imported skool file '%s' successfully.", pFilename);
	LOGDEBUG("Disassembly range $%x-$%x. %d locations saved to skoolinfo file", pInfo->StartAddr, pInfo->EndAddr, pInfo->Locations.size());

	return true;
}

bool ExportSkoolFile(FEmuBase* pEmu, bool bHexadecimal, uint16_t startAddr, uint16_t endAddr, const char* pOutName /* = nullptr*/)
{
	const FProjectConfig* pConfig = pEmu->GetProjectConfig();
	const std::string outDir = pConfig ? pEmu->GetGameWorkspaceRoot() : pEmu->GetGlobalConfig()->WorkspaceRoot;
	const std::string filename = pConfig ? pConfig->Name : pOutName ? pOutName : "Out";
	
	FSkoolFileInfo skoolInfo;
	std::string skoolInfoFname = outDir + filename + ".skoolinfo";
	bool bLoadedSkoolFileInfo = LoadSkoolFileInfo(skoolInfo, skoolInfoFname.c_str());
	
	const std::string outFname = outDir + filename + ".skool";
	ExportSkoolFile(pEmu->GetCodeAnalysis(), outFname.c_str(), bHexadecimal ? FSkoolFile::Base::Hexadecimal : FSkoolFile::Base::Decimal, bLoadedSkoolFileInfo ? &skoolInfo : nullptr, startAddr, endAddr);

	return true;
}
