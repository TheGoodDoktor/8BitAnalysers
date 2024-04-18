#pragma once

#include <cstdint>
#include <map>
#include <string>
#include <CodeAnalyser/CodeAnalyserTypes.h>
#include <Misc/EmuBase.h>

class FGraphicsView;
class FCodeAnalysisState;
struct FCodeAnalysisPage;

enum class EGraphicsViewMode : int
{
	Bitmap,				// bitmap - byte granularity in X
	BitmapChars,		// 8x8 character bitmap graphics

	BitmapWinding,	// winding bitmap (0,0) (1,0) (1,1) (0,1)

	Count
};

struct FGraphicsSet
{
	std::string	Name;
	FAddressRef	Address;	// start address of images
	int			XSizePixels;	// width in pixels
	int			YSizePixels;	// height in pixels
	int			Count;	// number of images
};

// Graphics Viewer
class FGraphicsViewer : public FViewerBase
{
public:
					FGraphicsViewer(FEmuBase* pEmu) : FViewerBase(pEmu) { Name = "Graphics View";}
	bool			Init(void) override;
	void			Shutdown(void) override;
	void			Reset();

	void			GoToAddress(FAddressRef address);

	void			DrawUI() override;
	
	void			SetImagesRoot(const char* pImagesRoot) { ImagesRoot = pImagesRoot; }
	bool			SaveGraphicsSets(const char* pFName);
	bool			LoadGraphicsSets(const char* pFName);
	bool			ExportImages(void);
	bool			ExportGraphicSet(const FGraphicsSet& set);
	void			DrawGraphicToView(const FGraphicsSet& set, FGraphicsView* pView, int imageNo, int x, int y);

	void			FixupAddressRefs() {}

	// protected methods
protected:
	FCodeAnalysisState& GetCodeAnalysis() { return *pCodeAnalysis; }
	const FCodeAnalysisState& GetCodeAnalysis() const { return *pCodeAnalysis; }
	void			DrawCharacterGraphicsViewer(void);
	virtual void	DrawScreenViewer(void) = 0;

	uint16_t		GetAddressOffsetFromPositionInView(int x, int y) const;

	void			DrawPhysicalMemoryAsGraphicsColumn(uint16_t memAddr, int xPos, int columnWidth);
	void			DrawPhysicalMemoryAsGraphicsColumnChars(uint16_t memAddr, int xPos, int columnWidth);
	void			DrawMemoryBankAsGraphicsColumn(int16_t bankId, uint16_t memAddr, int xPos, int columnWidth);
	void			DrawMemoryBankAsGraphicsColumnChars(int16_t bankId, uint16_t memAddr, int xPos, int columnWidth);
	void			UpdateCharacterGraphicsViewerImage(void); // make virtual for other platforms?

	virtual			const uint32_t* GetCurrentPalette() const { return nullptr; }

	// protected Members
protected:
	int				ScreenWidth = 0;
	int				ScreenHeight = 0;
	bool			bShowPhysicalMemory = true;
	int16_t			Bank = -1;
	uint16_t		AddressOffset = 0;	// offset to view from the start of the region (bank or physical address space)
	uint32_t		MemorySize = 0x10000;	// size of area being viewed
	FAddressRef		ClickedAddress;
	EGraphicsViewMode	ViewMode = EGraphicsViewMode::Bitmap;
	int				ViewScale = 1;
	int				HeatmapThreshold = 4;

	int				XSizePixels = 8;			// Image X Size in pixels
	int				YSizePixels = 8;			// Image Y Size in pixels
	int				ImageCount = 0;	// how many images?
	bool			bYSizePixelsFineCtrl = false;
	bool			bVSliderFineControl = false;
	int				GraphicColumnSizeBytes = 0;

	std::string		ImageSetName;

	std::map<FAddressRef, FGraphicsSet>		GraphicsSets;
	FAddressRef		SelectedGraphicSet;

	EBitmapFormat	BitmapFormat = EBitmapFormat::Bitmap_1Bpp;
	int				PaletteNo = -1;

	// housekeeping
	std::string			ImagesRoot;
	FCodeAnalysisState* pCodeAnalysis = nullptr;
	FGraphicsView* pGraphicsView = nullptr;
	FGraphicsView* pScreenView = nullptr;

	int				ItemNo = 0;
	FAddressRef		ImageGraphicSet;
	FGraphicsView* pItemView = nullptr;
};

uint32_t GetHeatmapColourForMemoryAddress(const FCodeAnalysisPage& page, uint16_t addr, int currentFrameNo, int frameThreshold);