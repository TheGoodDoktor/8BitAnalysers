#pragma once

#include "CodeAnalyser/CodeAnalyserTypes.h"
#include "CodeAnalyser/IOAnalyser.h"

#include <cstdint>
#include <unordered_map>
#include <unordered_set>
#include <vector>

class FCodeAnalysisState;
class FBBCEmulator;



struct FBBCIORegisterAccessInfo
{
	std::unordered_set<uint8_t>	WriteVals;
};

struct FBBCIORegisterInfo
{
	void Reset() { Accesses.clear(); LastVal = 0; }
	std::unordered_map<FAddressRef, FBBCIORegisterAccessInfo>	Accesses;
	uint8_t			LastVal = 0;
};

void DrawRegValueHex(class FBBCIODevice* pDevice, uint8_t val);
void DrawRegValueDecimal(class FBBCIODevice* pDevice, uint8_t val);

struct FRegDisplayConfig
{
	uint16_t	Address;
	const char* Name;
	void		(*UIDrawFunction)(FBBCIODevice* pDevice, uint8_t val) = DrawRegValueHex;
	EDataItemDisplayType	DisplayType = EDataItemDisplayType::Hex;
};

class FBBCIODevice : public FIODevice
{
public:
	FBBCEmulator* GetBBC() { return pBBCEmu; }

	void	OnRegisterRead(uint8_t reg, FAddressRef pc);
	void	OnRegisterWrite(uint8_t reg, uint8_t val, FAddressRef pc);
	void	DrawDetailsUI() override;

protected:
	FBBCEmulator* pBBCEmu = nullptr;

	int					SelectedRegister = -1;
	FBBCIORegisterInfo	RegisterInfo[256];

	std::vector<FRegDisplayConfig>* pRegConfig = nullptr;

};

int DrawRegSelectList(std::vector<FRegDisplayConfig>& regList, int selection);
void DrawRegDetails(FBBCIODevice* pDevice, FBBCIORegisterInfo& reg, const FRegDisplayConfig& regConfig, FCodeAnalysisState* pCodeAnalysis);
