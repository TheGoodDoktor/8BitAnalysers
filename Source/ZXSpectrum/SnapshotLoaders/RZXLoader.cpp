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

#include "../SpectrumEmu.h"


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

struct FRZXData
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
    bool    Load(const char* fName, FRZXData& rzxData);
private:
	ERZXError    ReadBlock(FMemoryBuffer& inputBuffer, FRZXData& rzxData);


};

bool DecompressToBuffer(void* pCompData, uint32_t compDataSize, FMemoryBuffer& outBuffer)
{
	z_stream stream;
	memset(&stream, 0, sizeof(stream));
	stream.avail_in = compDataSize;
	stream.next_in = (Bytef*)pCompData;
	if (inflateInit(&stream) != Z_OK)
	{
		LOGERROR("RZXLoader: Decompression Error!");
		return false;
	}

	// Decompress the data
	char buffer[1024];
	int ret;
	outBuffer.Init(1024);
	do 
	{
		stream.avail_out = sizeof(buffer);
		stream.next_out = (Bytef*)buffer;
		ret = inflate(&stream, Z_NO_FLUSH);
		switch (ret) 
		{
		case Z_NEED_DICT:
		case Z_DATA_ERROR:
		case Z_MEM_ERROR:
			LOGERROR("RZXLoader: Error while decompressing the data");
			inflateEnd(&stream);
			return false;
		}
		int have = sizeof(buffer) - stream.avail_out;
		if (have > 0) 
		{
			outBuffer.WriteBytes(buffer, have);
		}
	} while (ret != Z_STREAM_END);

	// Clean up the decompression stream
	inflateEnd(&stream);

	outBuffer.ResetPosition();	// reset to beginning
	return true;
}

ERZXError FRZXLoader::ReadBlock(FMemoryBuffer& inputBuffer, FRZXData& rzxData)
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

				const uint32_t framesDataSize = blockLength - 18;
				uint8_t* pFramesData = new uint8_t[framesDataSize];
				inputBuffer.ReadBytes(pFramesData, framesDataSize);

				FMemoryBuffer framesBuffer;

				if (bCompressed)
					DecompressToBuffer(pFramesData, framesDataSize, framesBuffer);
				else
					framesBuffer.Init(pFramesData, framesDataSize);

				for (uint32_t frameNo = 0; frameNo < irb.NoFrames; frameNo++)
				{
					FRZXInputRecordingBlockFrame& frame = irb.Frames.emplace_back();

					framesBuffer.Read(frame.FetchCounter);
					framesBuffer.Read(frame.NoIOPortReads);
					//assert(frame.FetchCounter != 0);
					//assert(frame.NoIOPortReads < frame.FetchCounter || frame.NoIOPortReads == 65535);

					if (frame.NoIOPortReads > 0 && frame.NoIOPortReads != 65535)
					{
						frame.PortReadValues = new uint8_t[frame.NoIOPortReads];
						framesBuffer.ReadBytes(frame.PortReadValues, frame.NoIOPortReads);
					}
				}

				delete[] pFramesData;

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


bool FRZXLoader::Load(const char* fName, FRZXData& rzxData)
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
	pZXEmulator = pEmu;
    return true;
}


bool FRZXManager::Load(const char* fName)
{
	FRZXLoader	loader;
	pData = new FRZXData;

	loader.Load(fName, *pData);

	// Load Snapshot
	bool bSnapLoaded = false;
	if(strncmp(pData->SnapshotExtension,"Z80",3) == 0 || strncmp(pData->SnapshotExtension, "z80", 3) == 0)
		bSnapLoaded = LoadZ80FromMemory(pZXEmulator, pData->SnapshotData, pData->SnapshotLength);
	else if (strncmp(pData->SnapshotExtension, "SNA",3) == 0 || strncmp(pData->SnapshotExtension, "sna", 3) == 0)
		bSnapLoaded = LoadSNAFromMemory(pZXEmulator, pData->SnapshotData, pData->SnapshotLength);

	if (bSnapLoaded)
	{
		ReplayMode = EReplayMode::Playback;
		FrameNo = -1;	// because if gets incremented at the start of the update
		return true;
	}
    return false;
}

void FRZXManager::DrawUI(void)
{
 
}

// this should update the number of 
uint32_t FRZXManager::Update(void)
{
	// check if we've read all the IO reads
	if (FrameNo != -1)
	{
		FRZXInputRecordingBlockFrame& oldFrame = pData->InputRecordingBlock.Frames[FrameNo];
		if (NoPortVals != NoInputAttempts)
		{
			LOGINFO("FRZXManager : [Frame:%d] %d input attempts, old frame had %d inputs", FrameNo, NoInputAttempts, NoPortVals);
		}
	}

	FrameNo++;
	if (FrameNo >= (int)pData->InputRecordingBlock.NoFrames)
		return 0;	// we've reached the end

	FRZXInputRecordingBlockFrame& frame = pData->InputRecordingBlock.Frames[FrameNo];

	// update if not a repeating stream
	if (frame.NoIOPortReads != 0xffff)
	{
		NoPortVals = frame.NoIOPortReads;
		PortVals = frame.PortReadValues;
	}
	
	InputCount = 0;
	NoInputAttempts = 0;
    return frame.FetchCounter;
}

static void OutputPortDebug(FSpectrumEmu* pEmu, uint16_t port, uint8_t val);

bool	FRZXManager::GetInput(uint16_t port, uint8_t& outVal)
{
	NoInputAttempts++;

	if (NoInputAttempts > NoPortVals)
		return false;

	assert(PortVals != nullptr);
	outVal = PortVals[InputCount];
	//OutputPortDebug(pZXEmulator, port, outVal);
	InputCount++;

    return true;
}

// Some debugging stuff

#include <map>

static std::map<uint16_t,std::vector<std::string>> g_KeyNames =
{
	{0xfefe, {"Shift","Z","X","C","V"}},
	{0xfdfe, {"A","S","D","F","G"}},
	{0xfbfe, {"Q","W","E","R","T"}},
	{0xf7fe, {"1","2","3","4","5"}},
	{0xeffe, {"0","9","8","7","6"}},
	{0xdffe, {"P","O","I","U","Y"}},
	{0xbffe, {"Enter","L","K","J","H"}},
	{0x7ffe, {"Space","Sym","M","N","B"}},
};

static void OutputPortDebug(FSpectrumEmu* pEmu,uint16_t port, uint8_t val)
{
	if ((val & 0x1f) == 0x1f)	// no key down
		return;

	/*const uint16_t columnMask = (~(port >> 8)) & 0x00FF;

	for (int colNo = 0; colNo < 8; colNo++)
	{
		if (columnMask & (1 << colNo))
		{
			for (int i = 0; i < 5; i++)
			{
				if ((val & (1 << i)) == 0)
				{
					LOGINFO("0x%04X : Column %d, key %d Down", pEmu->GetPC().Address, colNo, i);

				}
			}
		}
	}*/

	const auto findIt = g_KeyNames.find(port);
	if (findIt == g_KeyNames.end())
		return;

	auto& row = findIt->second;
	for (int i = 0; i < 5; i++)
	{
		if ((val & (1 << i)) == 0)
			LOGINFO("0x%04X : Key %s pressed", pEmu->GetPC().Address, row[i].c_str());
	}

}

