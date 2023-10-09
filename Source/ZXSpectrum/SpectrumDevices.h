#pragma once

#include "CodeAnalyser/IOAnalyser.h"
#include <chips/kbd.h>
#include <chips/beeper.h>

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
};


class FSpectrum128MemoryCtrl : public FIODevice
{
public:
			FSpectrum128MemoryCtrl();

	void	DrawDetailsUI() override;

	void	RegisterMemoryConfigWrite(FAddressRef pc, uint8_t value);
private:
	uint8_t		CurrentConfig = 0;
};