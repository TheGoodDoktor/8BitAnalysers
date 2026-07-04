#include "CDROMAnalyser.h"
#include "PCEEmu.h"

#include <geargrafx_core.h>

FCDROMAnalyser::FCDROMAnalyser(FPCEEmu* pEmu)
	: pPCEEmu(pEmu)
{
}

const uint16_t zipBaseAddr = 0x2000;
const uint16_t _al = zipBaseAddr + 0xF8;
//const uint16_t _ah = zipBaseAddr + 0xF9;
//const uint16_t _bl = zipBaseAddr + 0xFA;
//const uint16_t _bh = zipBaseAddr + 0xFB;
const uint16_t _cl = zipBaseAddr + 0xFC;
const uint16_t _ch = zipBaseAddr + 0xFD;
const uint16_t _dl = zipBaseAddr + 0xFE;
const uint16_t _dh = zipBaseAddr + 0xFF;

//const uint16_t _ax = zipBaseAddr + 0xF8;
const uint16_t _bx = zipBaseAddr + 0xFA;
//const uint16_t _cx = zipBaseAddr + 0xFC;
//const uint16_t _dx = zipBaseAddr + 0xFE;

bool GetBiosCDReadArgs(FPCEEmu* pEmu, FBiosCDReadArgs& args)
{
	const uint8_t mode = pEmu->ReadByte(_dh);
	uint32_t numToRead = pEmu->ReadByte(_al);

	const uint8_t clByte = pEmu->ReadByte(_cl);
	const uint8_t chByte = pEmu->ReadByte(_ch);
	const uint8_t dlByte = pEmu->ReadByte(_dl);
	args.CDByteOffset =  (dlByte + (chByte << 8) + (clByte << 16)) * 2048;

	switch (mode)
	{
		case 0: // Physical memory: size in bytes
			// todo
			return false;
		case 1: // Physical memory: size in sectors
			args.Destination = EBiosCDReadDest::PhysicalMemory;
			args.PhysicalAddr = pEmu->ReadWord(_bx);
			numToRead *= 2048;
			return true;
		case 2: // MPR num
		case 3:
		case 4:
		case 5:
		case 6:
			// todo
			return false;
		case 0xfe: // VRAM
		case 0xff: // VRAM
			// todo
			return false;
	}

	return false;
	//const uint32_t cdOffset = (dlByte + (chByte << 8) + (clByte << 16)) * 2048;
	//LOGINFO("CD_READ mode %d addr 0x%x bytes to read %d. cl %x ch %x dl %x. cd byte offset %d", mode, addr, numToRead, clByte, chByte, dlByte, cdOffset);
}