#include "TAPLoader.h"
#include "../SpectrumEmu.h"

#include <cstdint>
#include <Util/FileUtil.h>
#include <cassert>
#include <systems/zx.h>
#include "Util/MemoryBuffer.h"
#include <Debug/DebugLog.h>

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

	LOGINFO("TAP started:");

	// TODO: This is a basic POC reader, we could store it a much nicer way
	while (tapBuffer.Finished() == false)
	{
		const uint16_t blockLength = tapBuffer.Read<uint16_t>();
		const uint8_t flags = tapBuffer.Read<uint8_t>();
		
		LOGINFO("");	// line space
		if (flags == 0)	// 0 Indicates Header
		{
			LOGINFO("TAP Header");
			// Block Types
			// 0 : Program
			// 1 : Number Array
			// 2 : Character Array
			// 3 : Code
			const uint8_t blockType = tapBuffer.Read<uint8_t>();
			char fileName[11];
			memset(fileName,0,11);
			tapBuffer.ReadBytes(fileName, 10);
			const uint16_t dataBlockLength = tapBuffer.Read<uint16_t>();
			

			if(blockType == 0)
			{
				// For Programs:
				// Param 1 : Autostart line number (or >32768 if no line number was given)
				// Param 2 : Start of variable area relative to start of program
				LOGINFO("Program: %s", fileName);
				const uint16_t autoLineNumber = tapBuffer.Read<uint16_t>();
				const uint16_t startOfVariableArea = tapBuffer.Read<uint16_t>();
				LOGINFO("Autostart line number: %d", autoLineNumber);
				LOGINFO("Start of variable area: %d", startOfVariableArea);
			}
			else if(blockType == 3)
			{
				// For Code:
				// Param 1 : Start address of code block
				// Param 2 : 32768
				LOGINFO("Code: %s", fileName);
				const uint16_t startAddress = tapBuffer.Read<uint16_t>();
				const uint16_t param2 = tapBuffer.Read<uint16_t>();
				LOGINFO("Start address: 0x%04X", startAddress);
				LOGINFO("Param2: %d",param2);
			}

			const uint8_t checksum = tapBuffer.Read<uint8_t>();
			LOGINFO("checksum = %d",checksum);
		}
		else if (flags == 0xff)	// 0xff indicates data
		{
			uint16_t dataLength = blockLength - 2;
			uint8_t* pData = new uint8_t[dataLength];
			tapBuffer.ReadBytes(pData, dataLength);
			const uint8_t checksum = tapBuffer.Read<uint8_t>();
			delete[] pData;

			LOGINFO("TAP Data");
			LOGINFO("Loaded %d bytes",dataLength);
		}
		else
		{
			// TODO: error
		}
	}

	LOGINFO("TAP: Done");
	return false;
}