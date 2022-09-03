#pragma once

#include <cstdint>

class FSpectrumEmu;

class FSpectrumViewer
{
public:
	FSpectrumViewer() {}

	void	Init(FSpectrumEmu* pEmu) { pSpectrumEmu = pEmu; }

	void	Draw();
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
};