#pragma once

#include "IORegisterAnalysis.h"

#include <vector>

class FCodeAnalysisState;
struct FCodeAnalysisPage;
class FC64Emulator;
class FGraphicsView;

struct FSpriteDef
{
	FAddressRef	Address;
	//uint32_t	SpriteCols[4];
	int			PaletteIndex = -1;
	FGraphicsView*	SpriteImage;
	bool		bMultiColour = false;
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