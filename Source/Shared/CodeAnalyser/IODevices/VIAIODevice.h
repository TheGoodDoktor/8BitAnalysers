#pragma once

#include "../IOAnalyser.h"

#include <chips/m6522.h>
#include <ui/ui_util.h>
#include <ui/ui_chip.h>

class FEmuBase;

struct ui_m6522_t
{
	const char* title;
	m6522_t* via;
	uint16_t regs_base;
	float init_x, init_y;
	float init_w, init_h;
	bool open;
	bool valid;
	ui_chip_t chip;
};

class FVIAIODevice : public FIODevice
{
public:
	FVIAIODevice();
	bool	Init(const char* pName, FEmuBase* pEmulator, m6522_t* pVIA);

	void	OnFrameTick() override;
	void	OnMachineFrameEnd() override;
	void	DrawDetailsUI() override;
	void	DrawVIAStateUI(void);

private:
	const m6522_t*	pViaState = nullptr;
	ui_m6522_t		UIState;
};