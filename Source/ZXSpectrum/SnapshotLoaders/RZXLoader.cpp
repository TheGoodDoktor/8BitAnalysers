#include "RZXLoader.h"
#include <stdlib.h>
#include "Util/FileUtil.h"
#include "Debug/DebugLog.h"
#include "GamesList.h"
#include "Z80Loader.h"
#include "SNALoader.h"

#include <imgui.h>
#include <zlib.h>
#include <Util/MemoryBuffer.h>


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

	FRZXInputRecordingBlock		InputRecordingBlock;
};

class FRZXLoader
{
public:
    bool    Load(const char* fName, FRZXRZXData& rzxData);
private:
	ERZXError    ReadBlock(FMemoryBuffer& inputBuffer, FRZXRZXData& rzxData);


};

ERZXError FRZXLoader::ReadBlock(FMemoryBuffer& inputBuffer, FRZXRZXData& rzxData)
{
	bool bDone = false;

	while (bDone == false)
	{
		if (inputBuffer.Finished())
			return ERZXError::Finished;

		// get block Id & Length
		uint8_t blockId = 0;
		uint32_t blockLength = 0;
		
		inputBuffer.Read(blockId);
		inputBuffer.Read(blockLength);

		if (blockLength == 0)
			return ERZXError::Invalid;

		switch (blockId)
		{
			case kBlockId_CreatorInfo:
			{
				LOGINFO("RZXLoader: Creator Info Block");
				inputBuffer.ReadBytes(rzxData.CreatorIdentifier, 20);
				inputBuffer.Read(rzxData.CreateVersionMajor);
				inputBuffer.Read(rzxData.CreateVersionMinor);

				const uint32_t customDataSize = blockLength - 29;
				if (customDataSize > 0)
				{
					rzxData.CreatorCustomData = new uint8_t[customDataSize];
					inputBuffer.ReadBytes(rzxData.CreatorCustomData, customDataSize);
				}
			}
			break;

			case kBlockId_SecurityInfo:
			{
				LOGINFO("RZXLoader: Security Info Block");
				inputBuffer.Read(rzxData.SecurityKeyId);
				inputBuffer.Read(rzxData.SecurityWeekCode);
			}
			break;

			case kBlockId_SecuritySignature:
			{
				LOGINFO("RZXLoader: Security Signature Block");
				const uint32_t securitySigSize = blockLength - 5;
				rzxData.DSASignature = new uint8_t[securitySigSize];
				inputBuffer.ReadBytes(rzxData.DSASignature, securitySigSize);
			}
			break;

			case kBlockId_Snapshot:
			{
				LOGINFO("RZXLoader: Snapshot Block");
				uint32_t snaphotFlags = 0;
				inputBuffer.Read(snaphotFlags);
				const bool bExternalSnapshot = !!(snaphotFlags & 0x1);
				const bool bCompressed = !!(snaphotFlags & 0x2);

				inputBuffer.Read(rzxData.SnapshotExtension);
				inputBuffer.Read(rzxData.SnapshotLength);

				uint32_t snapshotDataLength = blockLength - 17;
				uint8_t* snapshotData = new uint8_t[snapshotDataLength];
				inputBuffer.ReadBytes(snapshotData, snapshotDataLength);

				if (bExternalSnapshot)
				{
					LOGINFO("RZXLoader: External snapshot");
				}
				else
				{
					if (bCompressed)
					{
						uint8_t* decompressedData = new uint8_t[rzxData.SnapshotLength];
						unsigned long nDataSize = rzxData.SnapshotLength;
						// Decompress
						LOGINFO("RZXLoader: Compressed snapshot");
						uncompress(decompressedData, &nDataSize, snapshotData, snapshotDataLength);
						delete[] snapshotData;
						rzxData.SnapshotData = decompressedData;
					}
					else
					{
						rzxData.SnapshotData = snapshotData;
					}
				}

			}
			break;

			case kBlockId_InputRecording:
			{
				LOGINFO("RZXLoader: Input Recording Block");

				FRZXInputRecordingBlock& irb = rzxData.InputRecordingBlock;

				inputBuffer.Read(irb.NoFrames);
				uint8_t reserved;
				inputBuffer.Read(reserved);
				inputBuffer.Read(irb.TStateCounterAtBeginning);
				uint32_t irbFlags = 0;
				inputBuffer.Read(irbFlags);

				const bool bProtected = !!(irbFlags & 0x1);
				const bool bCompressed = !!(irbFlags & 0x2);

				if (bCompressed)
				{
					const uint32_t compDataSize = blockLength - 18;
					uint8_t* pCompData = new uint8_t[compDataSize];
					inputBuffer.ReadBytes(pCompData, compDataSize);
					unsigned long nDataSize = compDataSize * 4;
					uint8_t* pDecompBuffer = new uint8_t[nDataSize];

					uncompress(pDecompBuffer, &nDataSize, pCompData, compDataSize);

					// TODO: process data
					FMemoryBuffer framesBuffer;
					framesBuffer.Init(pDecompBuffer, nDataSize);

					for (int frameNo = 0; frameNo < irb.NoFrames; frameNo++)
					{
						FRZXInputRecordingBlockFrame& frame = irb.Frames.emplace_back();

						framesBuffer.Read(frame.FetchCounter);
						framesBuffer.Read(frame.NoIOPortReads);

						if (frame.NoIOPortReads > 0)
						{
							frame.PortReadValues = new uint8_t[frame.NoIOPortReads];
							framesBuffer.ReadBytes(frame.PortReadValues, frame.NoIOPortReads);
						}
					}
				}
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


bool FRZXLoader::Load(const char* fName, FRZXRZXData& rzxData)
{
	FMemoryBuffer inputBuffer;
	if (inputBuffer.LoadFromFile(fName) == false)
		return false;

	// load & check signature
	char signature[4];
	inputBuffer.ReadBytes(signature, 4);
	if (strncmp(signature, "RZX!", 4) != 0)
	{
		return false;
	}

	// get version number
	uint8_t	verMajor = inputBuffer.Read<uint8_t>();
	uint8_t	verMinor = inputBuffer.Read<uint8_t>();

	// flags
	uint32_t	flags = inputBuffer.Read<uint32_t>();
	ReadBlock(inputBuffer, rzxData);

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
	FRZXRZXData		rzxData;

	loader.Load(fName, rzxData);

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

