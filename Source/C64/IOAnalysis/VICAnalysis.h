#pragma once

#include "IORegisterAnalysis.h"

#include <vector>

class FCodeAnalysisState;
struct FCodeAnalysisPage;
class FC64Emulator;

struct FSpriteDef
{
	uint16_t	Address = 0;
	uint32_t	SpriteCols[4];
	bool		bMultiColour = false;

	bool operator==(const FSpriteDef& other) const { return HashFunction() == other.HashFunction(); }

	std::size_t	HashFunction() const {	return Address + (SpriteCols[0] + SpriteCols[1] + SpriteCols[2] + SpriteCols[3]); }
};

// Hash function for FAddresRef so we can use unordered sets/maps
template <>
struct std::hash<FSpriteDef>
{
	std::size_t operator()(const FSpriteDef& spriteDef) const { return spriteDef.HashFunction();}
};

class FVICAnalysis : public FC64IODevice
{
public:
	void	Init(FC64Emulator* pEmulator);
	void	Reset();
	void	OnRegisterRead(uint8_t reg, FAddressRef pc);
	void	OnRegisterWrite(uint8_t reg, uint8_t val, FAddressRef pc);

	void	DrawDetailsUI(void) override;
	void	OnMachineFrame(void) override;

	const std::vector<FSpriteDef>&	GetFoundSprites() const { return SpriteDefs; }
private:
	static const int kNoRegisters = 64;
	FC64IORegisterInfo	VICRegisters[kNoRegisters];

	int		SelectedRegister = -1;

	std::vector<FSpriteDef>	SpriteDefs;
};

void AddVICRegisterLabels(FCodeAnalysisPage& IOPage);