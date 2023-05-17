#include "TZXLoader.h"


#include "../SpectrumEmu.h"

#include <cstdint>
#include <Util/FileUtil.h>
#include <cassert>
#include <systems/zx.h>
#include "Util/MemoryBuffer.h"
#include "Debug/DebugLog.h"

// http://k1.spdns.de/Develop/Projects/zasm/Info/TZX%20format.html

enum class ETZXBlockId
{
	StandardSpeed	= 0x10,
	TurboSpeed		= 0x11,

	ArchiveInfo		= 0x32,
};

struct FTZXBlockBase
{
	ETZXBlockId	Type;
};

struct FTZXArchiveBlockText
{
	int			Type = -1;	// use enum?
	std::string	String;
};

struct FTZXArchiveBlock : public FTZXBlockBase
{
	void ReadFromMemoryBuffer(FMemoryBuffer& buffer)
	{
		const uint16_t	blockLength = buffer.Read<uint16_t>();
		const uint8_t noTextStrings = buffer.Read<uint8_t>();

		for (int strNo = 0; strNo < noTextStrings; strNo++)
		{
			FTZXArchiveBlockText	textEntry;
			textEntry.Type = buffer.Read<uint8_t>();
			const int noChars = buffer.Read<uint8_t>();
			textEntry.String = buffer.ReadString(noChars);
			TextEntries.push_back(textEntry);
		}
	}

	std::vector<FTZXArchiveBlockText>	TextEntries;
};

struct FTZXFile
{

	std::vector<FTZXBlockBase*>	Blocks;
};

bool LoadTZXFile(FSpectrumEmu* pEmu, const char* fName)
{
	size_t byteCount = 0;
	uint8_t* pData = (uint8_t*)LoadBinaryFile(fName, byteCount);
	if (!pData)
		return false;
	const bool bSuccess = LoadTZXFromMemory(pEmu, pData, byteCount);
	free(pData);

	return bSuccess;
}

bool LoadTZXFromMemory(FSpectrumEmu* pEmu, const uint8_t* pData, size_t dataSize)
{
	FMemoryBuffer tzxBuffer;
	tzxBuffer.Init(pData, dataSize);

	FTZXFile	tzxFile;

	char tzxSignature[7];

	tzxBuffer.ReadBytes(tzxSignature, 7);
	const uint8_t endTextMarker = tzxBuffer.Read<uint8_t>();
	const uint8_t majorVersion = tzxBuffer.Read<uint8_t>();
	const uint8_t minorVersion = tzxBuffer.Read<uint8_t>();

	while (tzxBuffer.Finished() == false)
	{
		const ETZXBlockId blockId = (ETZXBlockId)tzxBuffer.Read<uint8_t>();

		switch (blockId)
		{
		case ETZXBlockId::StandardSpeed:
			{
				LOGINFO("TZX Loader: Standard Speed Block");
			}
			break;
		case ETZXBlockId::TurboSpeed:
			LOGINFO("TZX Loader: Turbo Speed Block");
			break;

		case ETZXBlockId::ArchiveInfo:
			{
				LOGINFO("TZX Loader: Archive Info");
				FTZXArchiveBlock* pArchiveBlock = new FTZXArchiveBlock;
				pArchiveBlock->ReadFromMemoryBuffer(tzxBuffer);
				tzxFile.Blocks.push_back(pArchiveBlock);
			}
			break;
		default:
			LOGWARNING("TZX Loader: Unrecognised block Id: 0x%0X", (uint8_t)blockId);
		}

	}

	return false;
}
