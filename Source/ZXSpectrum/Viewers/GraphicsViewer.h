#pragma once

#include "imgui.h"
#include <cstdint>
#include "SpriteViewer.h"
#include <map>
#include <string>

#include <CodeAnalyser/CodeAnalyserTypes.h>

class FSpectrumEmu;
struct FGame;
class FZXGraphicsView;


enum class GraphicsViewMode : int
{
	Character,	// 8x8 bitmap graphics
	CharacterWinding,	// winding bitmap (0,0) (1,0) (1,1) (0,1)

	Count
};

struct FGraphicsSet
{
	FAddressRef	Address;	// start address of images
	int			XSizePixels;	// width in pixels
	int			YSizePixels;	// height in pixels
	int			Count;	// number of images
};

// Graphics Viewer
class FGraphicsViewerState
{
public:
	bool			Init(FSpectrumEmu* emuPtr);
	void			Shutdown(void);

	void			GoToAddress(FAddressRef address);

	void			Draw();

	bool			SaveGraphicsSets(const char* pFName);
	bool			LoadGraphicsSets(const char* pFName);
private:
	void			DrawCharacterGraphicsViewer(void);
	void			DrawScreenViewer(void);

	uint16_t		GetAddressOffsetFromPositionInView(int x, int y) const;

	void			DrawMemoryBankAsGraphicsColumn(int16_t bankId, uint16_t memAddr, int xPos, int columnWidth);
	void			UpdateCharacterGraphicsViewerImage(void); // make virtual for other platforms?
protected:
	bool			bShowPhysicalMemory = true;
	int32_t			Bank = -1;
	uint16_t		AddressOffset = 0;	// offset to view from the start of the region (bank or physical address space)
	uint32_t		MemorySize = 0x10000;	// size of area being viewed
	FAddressRef		ClickedAddress;
	GraphicsViewMode	ViewMode = GraphicsViewMode::Character;
	int				ViewScale = 1;
	int				HeatmapThreshold = 4;

	int				XSizePixels = 8;			// Image X Size in pixels
	int				YSizePixels = 8;			// Image Y Size in pixels
	int				ImageCount = 0;	// how many images?
	bool			YSizePixelsFineCtrl = false;

	std::string		ImageSetName;


	std::map<FAddressRef, FGraphicsSet>		GraphicSets;

	// housekeeping
	FZXGraphicsView* pGraphicsView = nullptr;
	FZXGraphicsView* pScreenView = nullptr;
public:
	FSpectrumEmu*			pEmu = nullptr;		// can we phase this out?
	std::map<std::string, FUISpriteList>	SpriteLists;
	std::string				SelectedSpriteList;
};

