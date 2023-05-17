#include "TAPLoader.h"
#include "../SpectrumEmu.h"

#include <cstdint>
#include <Util/FileUtil.h>
#include <cassert>
#include <systems/zx.h>
#include "Util/MemoryBuffer.h"

// https://sinclair.wiki.zxnet.co.uk/wiki/TAP_format

bool LoadTAPFile(FSpectrumEmu* pEmu, const char* fName)
{
	size_t byteCount = 0;
	uint8_t* pData = (uint8_t*)LoadBinaryFile(fName, byteCount);
	if (!pData)
		return false;
	const bool bSuccess = LoadTAPFromMemory(pEmu, pData, byteCount);
	free(pData);

	return bSuccess;
}

bool LoadTAPFromMemory(FSpectrumEmu* pEmu, const uint8_t* pData, size_t dataSize)
{
	FMemoryBuffer tapBuffer;
	tapBuffer.Init(pData, dataSize);

	// TODO: This is a basic POC reader, we could store it a much nicer way
	while (tapBuffer.Finished() == false)
	{
		const uint16_t blockLength = tapBuffer.Read<uint16_t>();
		const uint8_t flags = tapBuffer.Read<uint8_t>();
		if (flags == 0)
		{
			const uint8_t blockType = tapBuffer.Read<uint8_t>();
			char fileName[10];
			tapBuffer.ReadBytes(fileName, 10);
			const uint16_t dataBlockLength = tapBuffer.Read<uint16_t>();
			const uint16_t Param1 = tapBuffer.Read<uint16_t>();
			const uint16_t Param2 = tapBuffer.Read<uint16_t>();
			const uint8_t checksum = tapBuffer.Read<uint8_t>();
		}
		else if (flags == 0xff)
		{
			uint16_t dataLength = blockLength - 2;
			uint8_t* pData = new uint8_t[dataLength];
			tapBuffer.ReadBytes(pData, dataLength);
			const uint8_t checksum = tapBuffer.Read<uint8_t>();
			delete[] pData;
		}
	}

	return false;
}