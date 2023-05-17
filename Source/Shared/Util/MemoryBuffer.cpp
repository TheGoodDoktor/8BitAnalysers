#include "MemoryBuffer.h"
#include <stdlib.h>
#include <string.h>
#include <cassert>
#include <cstdint>

#include "FileUtil.h"

FMemoryBuffer::~FMemoryBuffer()
{
	if (AllocationSize != 0)
		free(BasePtr);
}


void FMemoryBuffer::Init(size_t initialSize)
{
	if (BasePtr != nullptr)	// free old buffer
		free(BasePtr);

	BasePtr = malloc(initialSize);
	AllocationSize = initialSize;
	CurrentSize = 0;
}

void FMemoryBuffer::Init(const void *pData, size_t dataSize)
{
	Init(dataSize);
	CurrentSize = dataSize;
	memcpy(BasePtr,pData, dataSize);
}

void	FMemoryBuffer::WriteBytes(const void* pData, size_t noBytes)
{
	assert(AllocationSize != 0);

	if (CurrentSize + noBytes > AllocationSize)
	{
		AllocationSize = AllocationSize * 2;	// double allocation
		BasePtr = realloc(BasePtr, AllocationSize);
	}

	memcpy((uint8_t*)BasePtr + CurrentSize, pData, noBytes);
	CurrentSize += noBytes;
}

bool FMemoryBuffer::ReadBytes(void* Dest, size_t noBytes)
{
	if (ReadPosition + noBytes <= CurrentSize)
	{
		memcpy(Dest, (uint8_t*)BasePtr + ReadPosition, noBytes);
		ReadPosition += noBytes;
		return true;
	}
	else
	{
		return false;
	}
}

bool FMemoryBuffer::LoadFromFile(const char* pFileName)
{
	size_t fileSize;
	void* pData = LoadBinaryFile(pFileName, fileSize);
	if (pData == nullptr)
		return false;

	Init(pData, fileSize);
	free(pData);
	return true;
}

bool FMemoryBuffer::SaveToFile(const char* pFileName) const
{
	return SaveBinaryFile(pFileName, BasePtr, CurrentSize);
}