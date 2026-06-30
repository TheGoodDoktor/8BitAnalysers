#pragma once

#include <vector>

#include <CodeAnalyser/UI/GraphicsViewer.h>

class FPCEEmu;
class FGraphicsView;
class FCodeAnalysisState;

enum class EPCEGraphicsViewMode
{
	Sprites,
	BGTiles,
};

enum class EPCEMemorySource
{
	ROM,
	WRAM,
	VRAM,
};

class FPCENewGraphicsViewer : public FGraphicsViewerBase
{
public:
	FPCENewGraphicsViewer(FEmuBase* pEmu);

	bool	Init(void) override;
	void	Shutdown(void) override;
	void	DrawUI(void) override;

	void	GoToAddress(FAddressRef address) override;

private:
	void	UpdateGraphicView();
	void	PopulateBankList(const FCodeAnalysisState& state);
	void	DrawPaletteListBox();

	// Get the address corresponding to a pixel position in the graphic view (ROM/WRAM sources)
	FAddressRef	GetAddressFromPos(int xp, int yp) const;

	// Get the VRAM byte offset corresponding to a pixel position in the graphic view
	int		GetVRAMOffsetFromPos(int xp, int yp) const;

	void	DrawBlockThumbnail(int xp, int yp) const;

	FPCEEmu*		pPCEEmu = nullptr;

	FGraphicsView*	pGraphicView = nullptr;
	EPCEGraphicsViewMode	ViewMode = EPCEGraphicsViewMode::Sprites;
	EPCEMemorySource	MemorySource = EPCEMemorySource::ROM;
	int16_t			SelectedBankId = -1;
	int				SelectedBankIndex = -1;	// index into ComboBankIds
	int16_t			WRAMBankId = -1;
	bool				bGraphicViewDirty = false;
	int				GraphicViewScale = 1;
	int				ViewWidth = 128;
	int				ViewHeight = 128;
	int				SelectedPaletteIndex = 0;	// index into the palette store (see GetPaletteFromPaletteNo)
	bool				bGreyscale = false;
	int				DisplayAddress = 0;

	std::vector<int16_t>	ComboBankIds;

	uint32_t		GreyscalePalette[16] = {};
};
