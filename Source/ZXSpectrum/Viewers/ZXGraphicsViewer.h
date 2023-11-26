#pragma once

//#include "imgui.h"
#include <cstdint>
#include <map>
#include <string>

#include <CodeAnalyser/CodeAnalyserTypes.h>

#include <CodeAnalyser/UI/GraphicsViewer.h>
#include "SpriteViewer.h"

class FSpectrumEmu;
struct FGame;



class FZXGraphicsViewer : public FGraphicsViewer
{
public:
	FZXGraphicsViewer(FEmuBase *pEmu) : FGraphicsViewer(pEmu)
	{
		ScreenWidth = 256;
		ScreenHeight = 192;
	}

	void	DrawScreenViewer(void) override;
private:
	void	UpdateScreenPixelImage(void);
public:
	std::map<std::string, FUISpriteList>	SpriteLists;
	std::string				SelectedSpriteList;
	bool					bShowScreenMemoryAccesses = true;
	bool					bShowScreenAttributes = false;
};

