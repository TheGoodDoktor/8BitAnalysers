#pragma once

#include "CodeAnalyser/IOAnalyser.h"
#include "CodeAnalyser/UI/CodeAnalyserUI.h"
#include "SpectrumConstants.h"
#include <chips/kbd.h>
#include <chips/beeper.h>

#include <unordered_set>

class FSpectrumKeyboard : public FIODevice
{
public:
			FSpectrumKeyboard();

	bool	Init(kbd_t *kbd);
	//void	OnFrameTick() override;
	//void	OnMachineFrame() override;
	void	DrawDetailsUI() override;

	void	RegisterKeyboardRead(FAddressRef pc,uint16_t ioAddress,uint8_t value);
private:
	kbd_t*	pKeyboard = nullptr;
	std::unordered_set<FAddressRef>	AccessLocations;

};

class FSpectrumBeeper : public FIODevice
{
public:
	FSpectrumBeeper();

	bool	Init(beeper_t* beep);
	void	DrawDetailsUI() override;

	void	RegisterBeeperWrite(FAddressRef pc, uint8_t value);
private:
	beeper_t* pBeeper = nullptr;

	std::unordered_set<FAddressRef>	AccessLocations;
};


class FSpectrum128MemoryCtrl : public FIODevice
{
public:
			FSpectrum128MemoryCtrl();

	void	DrawDetailsUI() override;

	void	RegisterMemoryConfigWrite(FAddressRef pc, uint8_t value);
private:
	uint8_t		CurrentConfig = 0;
	std::unordered_set<FAddressRef>	AccessLocations;
};


class FScreenPixMemDescGenerator : public FMemoryRegionDescGenerator
{
public:
    FScreenPixMemDescGenerator():FScreenPixMemDescGenerator(-1){}
    FScreenPixMemDescGenerator(int16_t bankId)
    {
        RegionMin = kScreenPixMemStart;
        RegionMax = kScreenPixMemEnd;
        RegionBankId = bankId;
    }

    const char* GenerateAddressString(FAddressRef addr) override;
private:
    char DescStr[32] = { 0 };
};


class FScreenAttrMemDescGenerator : public FMemoryRegionDescGenerator
{
public:
    FScreenAttrMemDescGenerator():FScreenAttrMemDescGenerator(-1){}
    FScreenAttrMemDescGenerator(int16_t bankId)
    {
        RegionMin = kScreenAttrMemStart;
        RegionMax = kScreenAttrMemEnd;
        RegionBankId = bankId;
    }

    const char* GenerateAddressString(FAddressRef addr) override;
private:
    char DescStr[32] = { 0 };
};
