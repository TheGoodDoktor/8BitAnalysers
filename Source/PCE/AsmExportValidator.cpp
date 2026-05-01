#include <imgui.h>

#include "AsmExportValidator.h"

#include "crc.h"

#include "PCEEmu.h"

//#include <chrono>
#include "PCEConfig.h"
#include "Util/FileUtil.h"
#include "Debug/DebugLog.h"
#include "PCEGameConfig.h"

// I couldn't get this to work. The framebuffer CRCs would not match for certain games even though I wrote pngs
// to disk and verified they matched. Might be better to calculate CRCs on the contents of VRAM and SAT combined?
// 
// Ended up coming to the conclusion that running an emulator test is a waste of time.
// Better to pursue the goal of creating identical pce rom files. If we have identical pce
// file we don't need to run an emulator test on it.
#define ASM_EXPORT_VALIDATOR_USE_EMU_TEST 0

#if ASM_EXPORT_VALIDATOR_USE_EMU_TEST
static constexpr int kMaxFramebufferCRCs = 1800;
static constexpr int kNumIgnoredCRCs = 30;
#endif

bool FAsmExportValidator::FResults::DidPass()
{
#if ASM_EXPORT_VALIDATOR_USE_EMU_TEST
		return bAssembledOk && bRomFileIdentical && bEmulatorTestOk;
#else
	return bAssembledOk && bRomFileIdentical;
#endif
}

bool FAsmExportValidator::Validate(const std::vector<int16_t>& banksExported, const std::string& asmFname, bool bOutputListing/* = false*/)
{
#ifdef _WIN32
	LOGINFO("Assembling: %s. [%d/%d banks]", pPCEEmu->GetProjectConfig()->Name.c_str(), (int)banksExported.size(), pPCEEmu->GetBankCount());

	if (!Assemble(asmFname, bOutputListing))
	{
		return false;
	}

	CompareRomFiles(banksExported, asmFname);

#if ASM_EXPORT_VALIDATOR_USE_EMU_TEST
	RunEmulatorTest(asmFname);
#endif

	bIsValidating = false;

	return Results.DidPass();
#else
	return false;
#endif
}

bool FAsmExportValidator::Assemble(const std::string& asmFname, bool bOutputListing)
{
	printf("--------------------------------------------------------------------------------------------------------\n");

	// todo: export to temp directory?

	// create tmp.txt and output which file we are assembling
	std::string echoCmd = "echo Assembling " + pPCEEmu->GetProjectConfig()->Name + " > tmp.txt";
	std::system(echoCmd.c_str());

	// echo blank line
	std::system("echo[ >> tmp.txt");

	// This presumes pceas.exe is in your windows path.
	char cmdTxt[256];

	// append the results to out.txt
	snprintf(cmdTxt, 256, "pceas.exe --raw %s\"%s\" >> tmp.txt", bOutputListing ? "-l3 " : "", asmFname.c_str());

	const int errorCode = std::system(cmdTxt);

	// append to the batch log file
	std::system("type tmp.txt >> AssembleLog.txt");

	// print the contents to std output so we can see the result in the PCEAnalyser command window
	std::system("type tmp.txt");

	LOGINFO("Assembled '%s' : %s", pPCEEmu->GetProjectConfig()->Name.c_str(), errorCode ? "FAILURE" : "SUCESS");

	std::system("echo -------------------------------------------------------------------------------------------------------- >> AssembleLog.txt");
	printf("--------------------------------------------------------------------------------------------------------\n");

	Results.bAssembledOk = errorCode == 0 ? true : false;

	return Results.bAssembledOk;
}

bool FAsmExportValidator::CompareRomFiles(const std::vector<int16_t>& banksExported, const std::string& asmFname)
{
	const std::string outputPceFname = RemoveFileExtension(asmFname.c_str()) + ".pce";

	size_t assembledFileSize = 0;
	uint8_t* pAssembledData = (uint8_t*)LoadBinaryFile(outputPceFname.c_str(), assembledFileSize);
	if (pAssembledData == nullptr)
	{
		LOGINFO("Could not load '%s' to verify contents.", outputPceFname.c_str());
		return false;
	}

	size_t originalFileSize = 0;
	uint8_t* pOriginalData = nullptr;

	auto findIt = pPCEEmu->GetGamesLists().find(pPCEEmu->GetProjectConfig()->EmulatorFile.ListName);
	if (findIt != pPCEEmu->GetGamesLists().end())
	{
		const int bankCount = pPCEEmu->GetBankCount();
		const std::string originalFname = findIt->second.GetRootDir() + pPCEEmu->GetProjectConfig()->EmulatorFile.FileName;

		pOriginalData = (uint8_t*)LoadBinaryFile(originalFname.c_str(), originalFileSize);
		if (pOriginalData != nullptr)
		{
			size_t originalDataOffset = 0;
			size_t adjustedOriginalSize = originalFileSize;

			if ((originalFileSize % 0x2000) != 0)
			{
				if ((originalFileSize - 512) % 0x2000 == 0)
				{
					LOGINFO("Detected 512-byte header in original ROM. Skipping header for comparison.");
					originalDataOffset = 512;
					adjustedOriginalSize -= 512;
				}
			}

			LOGINFO("Produced ROM is %d bytes, %.1fKB. Original ROM is %d bytes, %.1fKB. [%s]", assembledFileSize, (float)assembledFileSize / 1024.f, originalFileSize, (float)originalFileSize / 1024.f, assembledFileSize == adjustedOriginalSize ? "MATCH" : "DIFF");

			if (assembledFileSize != adjustedOriginalSize)
			{
				const long diffBytes = abs((long)(assembledFileSize - adjustedOriginalSize));
				LOGINFO("ROM files size do not match. Difference is %d bytes (0x%x) [%.1fKB]", diffBytes, diffBytes, (float)diffBytes / 1024.0f);
			}

			int numDiffs = 0;

			for (auto bankId : banksExported)
			{
				const uint8_t bankIndex = pPCEEmu->GetBankIndexForBankId(bankId);

				uint8_t* pAssembledBankData = pAssembledData + 0x2000 * bankIndex;
				uint8_t* pOriginalBankData = pOriginalData + originalDataOffset + 0x2000 * bankIndex;

				const FCodeAnalysisBank* pBank = pPCEEmu->GetCodeAnalysis().GetBank(bankId);

				if (bankIndex < bankCount)
				{
					int numBankDiffs = 0;
					int numDiffsLogged = 0;

					for (int i = 0; i < 0x2000; i++)
					{
						if (pAssembledBankData[i] != pOriginalBankData[i])
						{
							numBankDiffs++;

							if (numDiffsLogged < 4)
							{
								LOGINFO("[%s][%04x] %02x -> %02x", pBank->Name.c_str(), pBank->GetMappedAddress() + i, pOriginalBankData[i], pAssembledBankData[i]);
								numDiffsLogged++;
							}
						}
					}

					if (numBankDiffs)
						LOGINFO("Found %04d diffs in %s (bank index %d).", numBankDiffs, pBank->Name.c_str(), bankIndex);

					numDiffs += numBankDiffs;
				}
				else
				{
					LOGINFO("Invalid bank index %d for %s", bankIndex, pBank->Name.c_str());
				}
			}

			if (!numDiffs)
			{
				const bool bExportedAllBanks = bankCount == banksExported.size();
				if (bExportedAllBanks && assembledFileSize == adjustedOriginalSize)
				{
					LOGINFO("Files are identical! Exported all banks.");
					Results.bRomFileIdentical = true;
				}
				else
				{
					Results.bRomFilePartialMatch = true;
					LOGINFO("Exported banks match the originals. Did not export all banks.");
				}
			}
			else
			{
				LOGINFO("Found %d bytes that are different overall.", numDiffs);
			}

			free(pOriginalData);
		}
		else
		{
			LOGINFO("Could not load '%s' to verify contents.", originalFname.c_str());
		}
	}

	free(pAssembledData);

	return Results.bRomFileIdentical;
}

void NormaliseFilePath(char* outFilePath, const char* inFilePath);

bool FAsmExportValidator::RunEmulatorTest(const std::string& asmFname)
{
#if ASM_EXPORT_VALIDATOR_USE_EMU_TEST
	const std::string outputPceFname = RemoveFileExtension(asmFname.c_str()) + ".pce";

	// turn off callback to improve performance
	pPCEEmu->EnableGeargrafxCallbacks(false);

	// do I need to reset anything here?
	// do I need to worry about tidying up the analyser state if this fails?

	LOGINFO("Running emulator frame buffer test on '%s'...", outputPceFname.c_str());

	if (pPCEEmu->GetCore()->LoadMedia(outputPceFname.c_str()) == false)
		return false;

	if (pPCEEmu->GetMedia()->IsReady() == false)
		return false;

	FDebugger& debugger = pPCEEmu->GetCodeAnalysis().Debugger;
	const bool bWasStopped = debugger.IsStopped();
	
	debugger.Continue();

	LOGINFO("Checking %d frames...", GameFrameNo);
	
	int numMatches = 0;
	int numCountedDiffs = 0;
	int numInitialDiffs = 0;
	bool bInInitialDiffs = true;
	for (int i = 0; i < GameFrameNo; i++)
	{
		int audioSampleCount = 0;
		pPCEEmu->GetCore()->RunToVBlank(pPCEEmu->GetFrameBuffer(), pPCEEmu->GetAudioBuffer(), &audioSampleCount);

		const int width = pPCEEmu->GetCore()->GetHuC6260()->GetCurrentWidth();
		const int height = pPCEEmu->GetCore()->GetHuC6260()->GetCurrentHeight();
		const u32 framebufCRC = CalculateCRC32(0, pPCEEmu->GetFrameBuffer(), width * height * 4);
		//LOGINFO("%03d CRC %8x [%s]%s (%d bytes)", i, framebufCRC, framebufCRC == FramebufferCRCs[i] ? "MATCH" : "DIFF", i < kNumIgnoredCRCs ? "[IGNORED]" : "", width * height * 4);

		const bool bCRCMatch = framebufCRC == FramebufferCRCs[i];
		if (bCRCMatch)
		{
			bInInitialDiffs = false;
			numMatches++;
		}
		if (bInInitialDiffs)
		{
			if (!bCRCMatch)
				numInitialDiffs++;
		}

		// Skip the first few frames because the frame CRCs often don't match - for some unknown reason
		if (i < kNumIgnoredCRCs)
			continue;

		if (!bCRCMatch)
			numCountedDiffs++;
	}

	pPCEEmu->EnableGeargrafxCallbacks(true);
	
	if (bWasStopped)
		debugger.Break();
	
	if (numInitialDiffs)
		LOGINFO("First %d frames were different.", numInitialDiffs);

	if (numCountedDiffs)
	{
		LOGINFO("Test Failed. %d frames differ.", numCountedDiffs);
	}
	else
	{
		if (numMatches == GameFrameNo)
			LOGINFO("Test passed. All frames are identical.");
		else
			LOGINFO("Test passed. %d frames are identical. Ignored %d frames", numMatches, kNumIgnoredCRCs);
		Results.bEmulatorTestOk = true;
	}

	// copy pce to specific directory based on if it passed or not
	const std::string validatorPath = pPCEEmu->GetPCEGlobalConfig()->ValidatorPath;
	const std::string passedPath = validatorPath + "EmuTestPassed";
	const std::string failedPath = validatorPath + "EmuTestFailed";
	EnsureDirectoryExists(passedPath.c_str());
	EnsureDirectoryExists(failedPath.c_str());

	char cmdTxt[256];
	snprintf(cmdTxt, 256, "copy \"%s\" \"%s\"", outputPceFname.c_str(), Results.bEmulatorTestOk ? passedPath.c_str() : failedPath.c_str());

	char cmdTxtNormalised[256];
	NormaliseFilePath(cmdTxtNormalised, cmdTxt);
	std::system(cmdTxtNormalised);
#endif
	return true;
}

void FAsmExportValidator::Reset(bool bStartValidating)
{
	bIsValidating = bStartValidating;

	GameFrameNo = 0;

#if ASM_EXPORT_VALIDATOR_USE_EMU_TEST
	FramebufferCRCs.clear();
	FramebufferCRCs.resize(kMaxFramebufferCRCs);
#endif

	Results.bAssembledOk = false;
	Results.bEmulatorTestOk = false;
	Results.bRomFileIdentical = false;
	Results.bRomFilePartialMatch = false;
}

void FAsmExportValidator::Tick()
{
#if ASM_EXPORT_VALIDATOR_USE_EMU_TEST
	if (!bIsValidating)
		return;

	if (GameFrameNo < kMaxFramebufferCRCs)
	{
		const int width = pPCEEmu->GetCore()->GetHuC6260()->GetCurrentWidth();
		const int height = pPCEEmu->GetCore()->GetHuC6260()->GetCurrentHeight();
		const u32 framebufCRC = CalculateCRC32(0, pPCEEmu->GetFrameBuffer(), width * height * 4);

		//LOGINFO("%03d CRC %x (%d bytes)", GameFrameNo, framebufCRC, width * height * 4);

		FramebufferCRCs[GameFrameNo] = framebufCRC;
		GameFrameNo++;
	}
#endif
}
