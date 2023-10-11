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
	virtual void	OnMachineFrame() {}
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
	void	OnMachineFrame(void);
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

struct FAYRegisterWrite
{
	ay38910_t	EmuState;
	FAddressRef	PC;
	int			FrameNo;
	uint8_t		Register;
	uint8_t		Value;
};

class FAYAudioDevice : public FIODevice
{
public:
	FAYAudioDevice();

	bool	Init(ay38910_t* pAY);
	void	SelectAYRegister(FAddressRef pc, uint8_t regNo) { SelectPC = pc; SelectedAYRegister = regNo & 15; }
	void	WriteAYRegister(FAddressRef pc, uint8_t value);
	
	void	OnFrameTick() override;
	void	OnMachineFrame() override;
	void	DrawDetailsUI() override;
private:

	FAddressRef	SelectPC;
	uint8_t		SelectedAYRegister = 255;
	uint8_t		AYRegisters[16];

	int			FrameNo = 0;
	// state ring buffer
	static const int	kWriteBufferSize = 100;
	FAYRegisterWrite	WriteBuffer[kWriteBufferSize];
	int					WriteBufferWriteIndex = 0;
	int					WriteBufferDisplayIndex = 0;


	const ay38910_t*	pAYEmulator = nullptr;
};

extern const char* g_AYRegNames[];