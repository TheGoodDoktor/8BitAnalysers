#include "CRTFile.h"
#include <stdio.h>
#include <cinttypes>

#include <climits>
#include <vector>
#include <cassert>
#include "../C64Emulator.h"
#include <Debug/DebugLog.h>

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

ECartridgeType GetCartridgeType(int typeNo);


bool LoadCRTFile(const char* pFName, FC64Emulator* pEmulator)
{
	FCodeAnalysisState& state = pEmulator->GetCodeAnalysis();

	FILE *fp = fopen(pFName,"rb");
	if(fp == nullptr)
		return false;

	FCRTHeader	header;
	fread(&header,sizeof(FCRTHeader),1,fp);
	header.HeaderLength = swap_endian<uint32_t>(header.HeaderLength);
	header.CartridgeType = swap_endian<uint16_t>(header.CartridgeType);
	const ECartridgeType cartridgeType = GetCartridgeType(header.CartridgeType);
	pEmulator->SetCartridgeType(cartridgeType);
	CreateCartridgeHandler(cartridgeType,pEmulator);

	LOGINFO("Loaded CRT file %s", pFName);
	LOGINFO("Name: %s",header.Name);
	LOGINFO("CRT Version %d.%d", header.VersionMajor, header.VersionMinor);
	LOGINFO("Cartridge Type: %d",header.CartridgeType);
	LOGINFO("GAME Line: %d",header.GAMELine);	// $A000
	LOGINFO("EXROM Line: %d",header.EXROMLine);	// $8000
	LOGINFO("Hardware Revision: %d", header.HardwareRevision);

	while(true)
	{
		FChipPacketHeader chipHeader;
		size_t noRead = fread(&chipHeader, sizeof(FChipPacketHeader), 1, fp);
		if(noRead == 0)
			break;

		assert(memcmp(chipHeader.Signature,"CHIP",4) == 0);

		chipHeader.PacketLength = swap_endian<uint32_t>(chipHeader.PacketLength);
		chipHeader.ChipType = swap_endian<uint16_t>(chipHeader.ChipType);
		chipHeader.BankNumber = swap_endian<uint16_t>(chipHeader.BankNumber);
		chipHeader.StartingLoadAddress = swap_endian<uint16_t>(chipHeader.StartingLoadAddress);
		chipHeader.ROMSizeBytes = swap_endian<uint16_t>(chipHeader.ROMSizeBytes);

		if(cartridgeType == ECartridgeType::EasyFlash && chipHeader.StartingLoadAddress == 0xA000)	//hack
			chipHeader.StartingLoadAddress = 0xE000;

		LOGINFO("--CHIP SECTION--");
		LOGINFO("Chip bank number: %d", chipHeader.BankNumber);
		LOGINFO("Type: %d",chipHeader.ChipType);
		LOGINFO("Address: $%04X",chipHeader.StartingLoadAddress);
		LOGINFO("Size: %d bytes", chipHeader.ROMSizeBytes);

		// Create bank & read in data
		FCartridgeBank& bank = pEmulator->AddCartridgeBank(chipHeader.BankNumber,chipHeader.StartingLoadAddress,chipHeader.ROMSizeBytes);
		fread(bank.Data,bank.DataSize,1,fp);
	}

	if(header.GAMELine == 0)
		pEmulator->GetCartridgeSlot(ECartridgeSlot::Addr_E000).bActive = true;
	if (header.EXROMLine == 0)
		pEmulator->GetCartridgeSlot(ECartridgeSlot::Addr_8000).bActive = true;

	fclose(fp);
	return true;
}

ECartridgeType GetCartridgeType(int typeNo)
{
	switch (typeNo)
	{
		case 0:
			return ECartridgeType::Generic;
		case 19:
			return ECartridgeType::MagicDesk;
		case 32:
			return ECartridgeType::EasyFlash;
		default:
			return ECartridgeType::Unknown;
	}
}

// Cartridge handlers - other file?

class FGenericCartridgeHandler : public FCartridgeHandler
{
public:
	FGenericCartridgeHandler(FC64Emulator* pEmu):FCartridgeHandler(pEmu){}

	bool	HandleIOWrite(uint16_t address, uint8_t value) override
	{
		if (address == 0xDE00)
		{
			if (value & (1 << 7))	// map RAM back in
				pEmulator->UnMapCartridge(ECartridgeSlot::Addr_8000);
			else
				pEmulator->MapCartridgeBank(ECartridgeSlot::Addr_8000, value & 0x7f);
		}

		return true;
	}
};

class FEasyFlashCartridgeHandler : public FCartridgeHandler
{
public:
	FEasyFlashCartridgeHandler(FC64Emulator* pEmu) :FCartridgeHandler(pEmu) {}

	bool	HandleIOWrite(uint16_t address, uint8_t value) override
	{
		if (address == 0xDE00)
		{
			if (value & (1 << 7))	// map RAM back in
				pEmulator->UnMapCartridge(ECartridgeSlot::Addr_8000);
			else
				pEmulator->MapCartridgeBank(ECartridgeSlot::Addr_8000, value & 0x7f);
		}

		if ((address & 0xFF00) == 0xDF00)	// RAM
		{
			RAM[address & 0xff] = value;
		}

		return true;
	}

	bool	HandleIORead(uint16_t address, uint8_t& value)	override
	{
		if ((address & 0xFF00) == 0xDF00)	// RAM
		{ 
			value =  RAM[address & 0xff];
			return true;
		}
		return false;
	}


	uint8_t	RAM[256];
};

bool CreateCartridgeHandler(ECartridgeType type, FC64Emulator* pEmulator)
{
	FCartridgeHandler* pHandler = nullptr;

	switch (type)
	{
	case ECartridgeType::Generic:
	case ECartridgeType::MagicDesk:
		pHandler = new FGenericCartridgeHandler(pEmulator);
		break;
	case ECartridgeType::EasyFlash:
		pHandler = new FEasyFlashCartridgeHandler(pEmulator);
		break;
	default:
		break;
	}

	pEmulator->SetCartridgeHandler(pHandler);

	return pHandler != nullptr;
}