#pragma once

#include <cinttypes>
#include "CodeAnalyserTypes.h"

#include <unordered_set>
#include <unordered_map>
#include <string>

class FCodeAnalysisState;

/*struct FIOAccessInfo
{
	uint16_t						IOAddress;
	std::unordered_set<FAddressRef>	ReadLocations;
	std::unordered_set<FAddressRef>	WriteLocations;
};*/

class FIODevice
{
public:
	virtual		~FIODevice(){}

	const char*		GetName() const { return Name.c_str(); }

	virtual void	DrawDetailsUI() = 0;

protected:
	std::string		Name;
};





class FIOAnalyser
{
public:
	void	Init(FCodeAnalysisState* pCodeAnalysis);
	void	Shutdown();

	void	AddDevice(FIODevice* pDevice){Devices.push_back(pDevice);}

	//void	SetIOReadAddressName(uint16_t IOAddress, const char* pName){ IOReadAddressName[IOAddress] = pName; }
	//void	SetIOWriteAddressName(uint16_t IOAddress, const char* pName) { IOWriteAddressName[IOAddress] = pName; }

	void	RegisterIORead(FAddressRef pc, uint16_t IOAddress, uint8_t value);
	void	RegisterIOWrite(FAddressRef pc, uint16_t IOAddress, uint8_t value);

	void	FrameTick(void);
	void	DrawUI(void);
private:
	FCodeAnalysisState* pCodeAnalysis = nullptr;

	int							SelectedDeviceIndex = -1;
	std::vector< FIODevice *>	Devices;

	//uint16_t										SelectedIOAddress = 0;
	//std::unordered_map<uint16_t, std::string>		IOReadAddressName;
	//std::unordered_map<uint16_t, std::string>		IOWriteAddressName;
	//std::unordered_map<uint16_t, FIOAccessInfo>		IOAccesses;

};


// AY-3-8910 Audio Chip - move
#include <chips/ay38910.h>

class FAYAudioDevice : public FIODevice
{
public:
	FAYAudioDevice();

	void	SetEmulator(const ay38910_t* pAY){pAYEmulator = pAY;}
	void	SelectAYRegister(FAddressRef pc, uint8_t regNo) { SelectPC = pc; SelectedAYRegister = regNo & 15; }
	void	WriteAYRegister(FAddressRef pc, uint8_t value);
	void	DrawDetailsUI();
private:

	FAddressRef	SelectPC;
	uint8_t		SelectedAYRegister = 255;
	uint8_t		AYRegisters[16];

	const ay38910_t*	pAYEmulator = nullptr;
};

extern const char* g_AYRegNames[];