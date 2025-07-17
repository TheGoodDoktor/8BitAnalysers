#pragma once

#include "../IOAnalyser.h"

#include <chips/mc6845.h>
#include <ui/ui_util.h>
#include <ui/ui_chip.h>

class FEmuBase;

typedef struct 
{
	const char* title;
	mc6845_t* mc6845;
	float init_x, init_y;
	float init_w, init_h;
	bool open;
	bool valid;
	ui_chip_t chip;
} ui_mc6845_t;

class FMC6845Device : public FIODevice
{
public:
	FMC6845Device();
	bool	Init(const char* pName, FEmuBase* pEmulator, mc6845_t* pVIA);

	void	OnFrameTick() override;
	void	OnMachineFrameEnd() override;
	void	DrawDetailsUI() override;
	void	DrawCRTCStateUI(void);

private:
	mc6845_t*		pCRTCState = nullptr;
	ui_mc6845_t		UIState;
};