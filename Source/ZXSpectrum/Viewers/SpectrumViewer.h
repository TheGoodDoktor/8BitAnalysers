#pragma once

#include <cstdint>

#include "imgui.h"
#include "Misc/InputEventHandler.h"

class FSpectrumEmu;

class FSpectrumViewer //: public IInputEventHandler
{
public:
	FSpectrumViewer() {}

	void	Init(FSpectrumEmu* pEmu);
	void	Draw();
	void	Tick(void);
#if 0
	// IInputEventHandler Begin
	void	OnKeyUp(int keyCode) override;
	void	OnKeyDown(int keyCode) override;
	void	OnChar(int charCode) override;
	void	OnGamepadUpdated(int mask) override;
	// IInputEventHandler End
#endif
private:
	FSpectrumEmu* pSpectrumEmu = nullptr;

	// screen inspector
	bool		bScreenCharSelected = false;
	uint16_t	SelectPixAddr = 0;
	uint16_t	SelectAttrAddr = 0;
	int			SelectedCharX = 0;
	int			SelectedCharY = 0;
	bool		CharDataFound = false;
	uint16_t	FoundCharDataAddress = 0;
	uint8_t		CharData[8] = {0};
	bool		bCharSearchWrap = true;
	bool		bWindowFocused = false;
};