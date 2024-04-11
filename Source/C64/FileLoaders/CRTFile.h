#pragma once

#include <cinttypes>

class FC64Emulator;

enum class ECartridgeType
{
	None,
	Generic,
	MagicDesk,
	EasyFlash,

	Unknown,
};

class FCartridgeHandler
{
public:
			FCartridgeHandler(FC64Emulator* pEmu):pEmulator(pEmu){}
			virtual ~FCartridgeHandler(){}

			virtual bool	HandleIOWrite(uint16_t address, uint8_t value) {return false;}
			virtual bool	HandleIORead(uint16_t address, uint8_t& value) { return false;};

protected:
	FC64Emulator*	pEmulator = nullptr; 
};

bool LoadCRTFile(const char* pFName, FC64Emulator* pEmulator);
bool CreateCartridgeHandler(ECartridgeType type, FC64Emulator* pEmulator);
