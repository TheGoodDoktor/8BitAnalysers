#include "CRTFile.h"
#include <stdio.h>
#include <cinttypes>

#include <climits>

template <typename T>
T swap_endian(T u)
{
	static_assert (CHAR_BIT == 8, "CHAR_BIT != 8");

	union
	{
		T u;
		unsigned char u8[sizeof(T)];
	} source, dest;

	source.u = u;

	for (size_t k = 0; k < sizeof(T); k++)
		dest.u8[k] = source.u8[sizeof(T) - k - 1];

	return dest.u;
}

struct FCRTHeader
{
	char Signature[16];
	uint32_t	HeaderLength;
	uint8_t		VersionMajor;
	uint8_t		VersionMinor;
	uint16_t	CartridgeType;
	uint8_t		EXROMLine;
	uint8_t		GAMELine;
	uint8_t		HardwareRevision;
	char		Reserved[5];
	char		Name[32];
};

struct FChipPacketHeader
{
	char		Signature[4];
	uint32_t	PacketLength;
	uint16_t	ChipType;
	uint16_t	BankNumber;
	uint16_t	StartingLoadAddress;
	uint16_t	ROMSizeBytes;
};

bool LoadCRTFile(const char* pFName)
{
	FILE *fp = fopen(pFName,"rb");
	if(fp == nullptr)
		return false;

	FCRTHeader	header;
	fread(&header,sizeof(FCRTHeader),1,fp);
	header.HeaderLength = swap_endian<uint32_t>(header.HeaderLength);
	header.CartridgeType = swap_endian<uint16_t>(header.CartridgeType);

	while(true)
	{
		FChipPacketHeader chipHeader;
		size_t noRead = fread(&chipHeader, sizeof(FChipPacketHeader), 1, fp);
		if(noRead == 0)
			break;

		chipHeader.PacketLength = swap_endian<uint32_t>(chipHeader.PacketLength);
		chipHeader.ChipType = swap_endian<uint16_t>(chipHeader.ChipType);
		chipHeader.BankNumber = swap_endian<uint16_t>(chipHeader.BankNumber);
		chipHeader.StartingLoadAddress = swap_endian<uint16_t>(chipHeader.StartingLoadAddress);
		chipHeader.ROMSizeBytes = swap_endian<uint16_t>(chipHeader.ROMSizeBytes);

		// TODO: read in bank data
	}
	fclose(fp);
	return true;
}