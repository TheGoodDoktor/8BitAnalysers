#pragma once

#include <cinttypes>
#include "CodeAnalyserTypes.h"

#include <unordered_set>
#include <unordered_map>
#include <string>

class FCodeAnalysisState;




struct FIOAccessInfo
{
	uint16_t						IOAddress;
	std::unordered_set<FAddressRef>	ReadLocations;
	std::unordered_set<FAddressRef>	WriteLocations;
};



class FIOAnalyser
{
public:
	void	Init(FCodeAnalysisState* pCodeAnalysis);
	void	Shutdown();

	void	SetIOReadAddressName(uint16_t IOAddress, const char* pName){ IOReadAddressName[IOAddress] = pName; }
	void	SetIOWriteAddressName(uint16_t IOAddress, const char* pName) { IOWriteAddressName[IOAddress] = pName; }

	void	RegisterIORead(FAddressRef pc, uint16_t IOAddress, uint8_t value);
	void	RegisterIOWrite(FAddressRef pc, uint16_t IOAddress, uint8_t value);

	void	FrameTick(void);
	void	DrawUI(void);
private:
	FCodeAnalysisState* pCodeAnalysis = nullptr;

	uint16_t										SelectedIOAddress = 0;
	std::unordered_map<uint16_t, std::string>		IOReadAddressName;
	std::unordered_map<uint16_t, std::string>		IOWriteAddressName;
	std::unordered_map<uint16_t, FIOAccessInfo>		IOAccesses;

};