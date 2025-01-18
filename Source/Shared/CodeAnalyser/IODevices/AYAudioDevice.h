#pragma once

#include "../IOAnalyser.h"

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
	void	OnMachineFrameEnd() override;
	void	DrawDetailsUI() override;

	void	DrawAYStateUI(void);

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

	static const int kNoValues = 100;
	float	ChanAValues[kNoValues];
	float	ChanBValues[kNoValues];
	float	ChanCValues[kNoValues];
	int		GraphOffset = 0;

	const ay38910_t* pAYEmulator = nullptr;
};

extern const char* g_AYRegNames[];