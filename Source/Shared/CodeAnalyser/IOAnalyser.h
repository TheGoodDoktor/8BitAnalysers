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

	void	SetAnalyser(FCodeAnalysisState* pAnalyser){ pCodeAnalyser = pAnalyser;}

	const char*		GetName() const { return Name.c_str(); }

	virtual void	DrawDetailsUI() = 0;
	virtual void	OnMachineFrameStart() {}
	virtual void	OnMachineFrameEnd() {}
	virtual void	OnFrameTick() {}

protected:
	std::string		Name;
	FCodeAnalysisState* pCodeAnalyser = nullptr;
};





class FIOAnalyser
{
public:
	void	Init(FCodeAnalysisState* pCodeAnalysis);
	void	Shutdown();

	void	AddDevice(FIODevice* pDevice){Devices.push_back(pDevice);pDevice->SetAnalyser(pCodeAnalysis);}

	//void	SetIOReadAddressName(uint16_t IOAddress, const char* pName){ IOReadAddressName[IOAddress] = pName; }
	//void	SetIOWriteAddressName(uint16_t IOAddress, const char* pName) { IOWriteAddressName[IOAddress] = pName; }

	void	RegisterIORead(FAddressRef pc, uint16_t IOAddress, uint8_t value);
	void	RegisterIOWrite(FAddressRef pc, uint16_t IOAddress, uint8_t value);

	void	FrameTick(void);
	void	OnMachineFrameStart(void);
	void	OnMachineFrameEnd(void);
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


