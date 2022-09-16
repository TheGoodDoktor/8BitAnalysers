#pragma once

#include <cstdint>

#include "imgui.h"
#include "Shared/ImGuiSupport/imgui_impl_win32.h"

class FSpectrumEmu;

class FSpectrumViewer : public IInputEventHandler
{
public:
	FSpectrumViewer() {}

	void	Init(FSpectrumEmu* pEmu);
	void	Draw();

	// IInputEventHandler Begin
	void	OnKeyUp(int keyCode) override;
	void	OnKeyDown(int keyCode) override;
	void	OnChar(int charCode) override;
	// IInputEventHandler End

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
	bool		bWindowFocused = false;
};