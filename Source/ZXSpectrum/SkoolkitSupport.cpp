#include "SpectrumEmu.h"

#include "Debug/DebugLog.h"
#include "Util/FileUtil.h"
#include <Misc/GameConfig.h>
#include "CodeAnalyser/CodeAnalysisJson.h"
#include "Exporters/SkoolFileInfo.h"
#include "Exporters/SkoolkitExporter.h"
#include "Importers/SkoolkitImporter.h"

bool BackupAnalysisJson(FSpectrumEmu* pEmu, FCodeAnalysisState& CodeAnalysis)
{
	const FProjectConfig* pCurrentGameConfig = pEmu->GetProjectConfig();
	const std::string root = pEmu->GetGlobalConfig()->WorkspaceRoot;

	if (pEmu->pActiveGame && pCurrentGameConfig)
	{
		std::string analysisBackupFname;
		const std::string gameRoot = pEmu->GetGameWorkspaceRoot();

		if (FileExists((gameRoot + "Config.json").c_str()))
		{
			// new folder structure: folder per game
			analysisBackupFname = gameRoot + "Analysis.json.bak";
		}
		else
		{
			// old method
			analysisBackupFname = root + "AnalysisJson/" + pCurrentGameConfig->Name + ".json.bak";
		}

		LOGINFO("Backing up analysis data to '%s'", analysisBackupFname.c_str());

		if (!ExportAnalysisJson(CodeAnalysis, analysisBackupFname.c_str()))
		{
			LOGERROR("Failed to import skool file. Could not save backup of analysis data to '%s'", analysisBackupFname.c_str());
			return false;
		}
	}
	return true;
}

bool ImportSkoolFile(FSpectrumEmu* pEmu, const char* pFilename, const char* pOutSkoolInfoName /* = nullptr*/, FSkoolFileInfo* pSkoolInfo /* = nullptr*/)
{
	// one of these must be set
	if (!pEmu->pActiveGame && !pOutSkoolInfoName)
		return false;

	LOGINFO("Importing skool file '%s'", pFilename);

	const std::string root = pEmu->GetGlobalConfig()->WorkspaceRoot;

	FCodeAnalysisState& CodeAnalysis = pEmu->GetCodeAnalysis();

	if (!BackupAnalysisJson(pEmu, CodeAnalysis))
	{
		return false;
	}

	// use FSkoolFileInfo pointer if it's passed in.
	FSkoolFileInfo skoolInfo;
	FSkoolFileInfo* pInfo = pSkoolInfo ? pSkoolInfo : &skoolInfo;
	if (!ImportSkoolKitFile(CodeAnalysis, pFilename, pSkoolInfo ? pSkoolInfo : pInfo))
	{
		LOGINFO("Failed to import '%s'", pFilename);
		return false;
	}

	const std::string gameName = pEmu->pActiveGame ? pEmu->pActiveGame->pConfig->Name.c_str() : pOutSkoolInfoName;
	const std::string outDir = pEmu->GetGameWorkspaceRoot();
	const std::string skoolInfoFname(outDir + gameName + std::string(".skoolinfo"));
	EnsureDirectoryExists(outDir.c_str());
	LOGINFO("Saving skoolinfo file '%s'", skoolInfoFname.c_str());
	if (!SaveSkoolFileInfo(*pInfo, skoolInfoFname.c_str()))
	{
		LOGINFO("Failed to save skoolinfo file '%s'", skoolInfoFname.c_str());
		return false;
	}

	LOGINFO("Imported skool file '%s' successfully for '%s'.", pFilename, gameName.c_str());
	LOGDEBUG("Disassembly range $%x-$%x. %d locations saved to skoolinfo file", pInfo->StartAddr, pInfo->EndAddr, pInfo->Locations.size());

	return true;
}

bool ExportSkoolFile(FSpectrumEmu* pEmu, bool bHexadecimal, const char* pName /* = nullptr*/)
{
	if (!pEmu->pActiveGame)
		return false;

	const std::string outputDir = pEmu->GetGameWorkspaceRoot();
	EnsureDirectoryExists(outputDir.c_str());

	const std::string name = pName ? std::string(pName) : pEmu->pActiveGame->pConfig->Name;
	FSkoolFileInfo skoolInfo;
	std::string skoolInfoFname = outputDir + name + ".skoolinfo";
	bool bLoadedSkoolFileInfo = LoadSkoolFileInfo(skoolInfo, skoolInfoFname.c_str());

	const std::string outFname = outputDir + name + ".skool";
	ExportSkoolFile(pEmu->GetCodeAnalysis(), outFname.c_str(), bHexadecimal ? FSkoolFile::Base::Hexadecimal : FSkoolFile::Base::Decimal, bLoadedSkoolFileInfo ? &skoolInfo : nullptr);

	return true;
}

void DoSkoolKitTest(FSpectrumEmu* pEmu, const char* pGameName, const char* pInSkoolFileName, bool bHexadecimal, const char* pOutSkoolName /* = nullptr*/)
{
	if (!pGameName && !pOutSkoolName)
		return;

	if (pGameName)
	{
		if (!pEmu->StartGameFromName(pGameName, true))
			return;
	}

	FSkoolFileInfo skoolInfo;
	const std::string inSkoolPath = std::string("InputSkoolKit/") + pInSkoolFileName;
	if (!ImportSkoolFile(pEmu, inSkoolPath.c_str(), pOutSkoolName, &skoolInfo))
		return;

	EnsureDirectoryExists("OutputSkoolKit/");
	const std::string outFname = "OutputSkoolKit/" + std::string(pGameName ? pGameName : pOutSkoolName) + ".skool";
	const FSkoolFile::Base base = bHexadecimal ? FSkoolFile::Base::Hexadecimal : FSkoolFile::Base::Decimal;
	ExportSkoolFile(pEmu->GetCodeAnalysis(), outFname.c_str(), base, &skoolInfo);
}