#include "RZXLoader.h"
#include <stdlib.h>
#include "Util/FileUtil.h"
#include "Debug/DebugLog.h"
#include "GamesList.h"
#include "Z80Loader.h"
#include "SNALoader.h"

#include <imgui.h>
#include <zlib.h>


//#include "rzx.h"
// https://worldofspectrum.net/RZXformat.html

// block Ids
const uint8_t	kBlockId_CreatorInfo		= 0x10;
const uint8_t	kBlockId_SecurityInfo		= 0x20;
const uint8_t	kBlockId_SecuritySignature	= 0x21;
const uint8_t	kBlockId_Snapshot			= 0x30;
const uint8_t	kBlockId_InputRecording		= 0x80;

enum class ERZXStatus
{
	Init = 0x01,
	IRB = 0x02,
	Prot = 0x04,
	Pack = 0x08,
};

static const int kRZXBlockBufferSize = 512;

enum class ERZXError
{
	Ok,
	Finished,
	SyncLost,
	Invalid,
	UnSupported
};

// aka: IRB
struct FRZXInputRecordingBlockFrame
{
	uint16_t	FetchCounter = 0;
	uint16_t	NoIOPortReads = 0;
	uint8_t*	PortReadValues = nullptr;
};

struct FRZXInputRecordingBlock
{
	uint32_t	NoFrames = 0;
	uint32_t	TStateCounterAtBeginning = 0;
	std::vector<FRZXInputRecordingBlockFrame>	Frames;
};

struct FRZXRZXData
{
	uint8_t		VersionMajor = 0;
	uint8_t		VersionMinor = 0;

	// creator
	char		CreatorIdentifier[20];
	uint16_t	CreateVersionMajor = 0;
	uint16_t	CreateVersionMinor = 0;
	uint8_t*	CreatorCustomData = nullptr;

	// security
	uint32_t	SecurityKeyId = 0;
	uint32_t	SecurityWeekCode = 0;
	uint8_t*	DSASignature = nullptr;

	// snapshot info
	uint16_t	SnapshotFlags = 0;
	char		SnapshotExtension[4];
	uint32_t	SnapshotLength = 0;
	uint8_t*	SnapshotData = nullptr;

	std::vector<FRZXInputRecordingBlock>	InputRecordingBlocks;
};

class FRZXLoader
{
public:
    bool    Load(const char* fName);
private:
	ERZXError    ReadBlock(FILE* fp, bool bTestIRB);

	FRZXRZXData		RZXData;

};

ERZXError FRZXLoader::ReadBlock(FILE* fp, bool bTestIRB)
{
	bool bDone = false;

	while (bDone == false)
	{
		// get block Id & Length
		uint8_t blockId;
		uint32_t blockLength;
		if (fread(&blockId, sizeof(uint8_t), 1, fp) != 1)
			return ERZXError::Finished;

		fread(&blockLength, sizeof(uint32_t), 1, fp);

		if (blockLength == 0)
			return ERZXError::Invalid;

		switch (blockId)
		{
			case kBlockId_CreatorInfo:
			{
				LOGINFO("RZXLoader: Creator Info Block");
				fread(RZXData.CreatorIdentifier, 1, 20, fp);
				fread(&RZXData.CreateVersionMajor, sizeof(uint16_t), 1, fp);
				fread(&RZXData.CreateVersionMinor, sizeof(uint16_t), 1, fp);
				const uint32_t customDataSize = blockLength - 29;
				if (customDataSize > 0)
				{
					RZXData.CreatorCustomData = new uint8_t[customDataSize];
					fread(RZXData.CreatorCustomData, 1, customDataSize, fp);
				}
			}
			break;

			case kBlockId_SecurityInfo:
			{
				LOGINFO("RZXLoader: Security Info Block");
				fread(&RZXData.SecurityKeyId, sizeof(uint32_t), 1, fp);
				fread(&RZXData.SecurityWeekCode, sizeof(uint32_t), 1, fp);
			}
			break;

			case kBlockId_SecuritySignature:
			{
				LOGINFO("RZXLoader: Security Signature Block");
				const uint32_t securitySigSize = blockLength - 5;
				RZXData.DSASignature = new uint8_t[securitySigSize];
				fread(RZXData.DSASignature, 1, securitySigSize, fp);
			}
			break;

			case kBlockId_Snapshot:
			{
				LOGINFO("RZXLoader: Snapshot Block");
				uint16_t snaphotFlags = 0;
				fread(&snaphotFlags, sizeof(uint16_t), 1, fp);
				const bool bExternalSnapshot = !!(snaphotFlags & 0x1);
				const bool bCompressed = !!(snaphotFlags & 0x2);

				fread(&RZXData.SnapshotExtension, sizeof(char), 4, fp);
				fread(&RZXData.SnapshotLength, sizeof(uint32_t), 1, fp);

				uint32_t snapshotDataLength = blockLength - 17;
				uint8_t* snapshotData = new uint8_t[snapshotDataLength];
				fread(snapshotData, snapshotDataLength, 1, fp);

				if (bExternalSnapshot)
				{
					LOGINFO("RZXLoader: External snapshot");
				}
				else
				{
					if (bCompressed)
					{
						uint8_t* decompressedData = new uint8_t[RZXData.SnapshotLength];
						unsigned long nDataSize = RZXData.SnapshotLength;
						// Decompress
						LOGINFO("RZXLoader: Compressed snapshot");
						uncompress(decompressedData, &nDataSize, snapshotData, snapshotDataLength);
					}
					else
					{
						RZXData.SnapshotData = snapshotData;
					}
				}

			}
			break;

			case kBlockId_InputRecording:
			{
				LOGINFO("RZXLoader: Input Recording Block");

				FRZXInputRecordingBlock& irb = RZXData.InputRecordingBlocks.emplace_back();

				fread(&irb.NoFrames, sizeof(uint32_t), 1, fp);
				uint8_t reserved;
				fread(&reserved, sizeof(uint8_t), 1, fp);
				fread(&irb.TStateCounterAtBeginning, sizeof(uint32_t), 1, fp);
				uint32_t irbFlags = 0;
				fread(&irbFlags, sizeof(uint32_t), 1, fp);

				const bool bProtected = !!(irbFlags & 0x1);
				const bool bCompressed = !!(irbFlags & 0x2);
			}
			break;

			default:
				LOGERROR("RZXLoader: Unrecognised block %d", blockId);
				return ERZXError::Invalid;
				break;
		}
	}

	return ERZXError::Ok;
}


bool FRZXLoader::Load(const char* fName)
{
	FILE* fp = fopen(fName, "rb");
	if (fp == nullptr)
		return false;

	// load & check signature
	char signature[4];
	fread(signature, 1, 4, fp);
	if (strncmp(signature, "RZX!", 4) != 0)
	{
		fclose(fp);
		return false;
	}

	// get version number
	uint8_t	verMajor = 0;
	uint8_t	verMinor = 0;
	fread(&verMajor, sizeof(uint8_t), 1, fp);
	fread(&verMinor, sizeof(uint8_t), 1, fp);

	// flags
	uint32_t	flags;
	fread(&flags, sizeof(uint32_t), 1, fp);

	while (ReadBlock(fp, true) == ERZXError::Ok)
	{

	}

	fclose(fp);

	return true;
}


// Manager class

bool	FRZXManager::Init(FSpectrumEmu* pEmu) 
{ 
    
    return true;
}


bool FRZXManager::Load(const char* fName)
{
	FRZXLoader	loader;

	loader.Load(fName);

    return true;
}

void FRZXManager::DrawUI(void)
{
 
}

uint16_t FRZXManager::Update(void)
{
    return 0;
}

bool	FRZXManager::GetInput(uint8_t& outVal)
{
   
    return true;
}

