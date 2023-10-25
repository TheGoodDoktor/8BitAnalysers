#include "ImageViewer.h"
#include "../CodeAnalyser.h"
#include "Util/GraphicsView.h"

#include <map>
#include <string>

std::vector<const FImageViewer*>	g_ImageViewers;
std::map<std::string, const FImageViewer*>	g_ImageViewerNameLookup;

// Deafault viewers
class FBasicBitmapScanlineViewer : public FImageViewer
{
public:
	const char* GetName() const override { return "Basic Scanline"; }

	void DrawImageToView(uint16_t addr, int widthChars, int heightChars, FGraphicsView* pView, const ICPUInterface* pCPUIF) const override
	{
		const uint32_t cols[] = { 0, 0xffffffff };
		pView->Draw1BppImageAt(pCPUIF->GetMemPtr(addr), 0, 0, widthChars * 8, heightChars * 8, cols);
	}

};

class FBasicBitmapCharsViewer : public FImageViewer
{
public:
	const char* GetName() const override { return "Basic Chars"; }

	void DrawImageToView(uint16_t addr, int widthChars, int heightChars, FGraphicsView* pView, const ICPUInterface* pCPUIF) const override
	{
		const uint32_t cols[] = { 0, 0xffffffff };
		pView->Draw1BppImageFromCharsAt(pCPUIF->GetMemPtr(addr), 0, 0, widthChars, heightChars, cols);
	}

};

static FBasicBitmapScanlineViewer	g_BasicScanlineViewer;
static FBasicBitmapCharsViewer		g_BasicCharsViewer;

void InitImageViewers(void)
{
	g_ImageViewers.clear();
	g_ImageViewerNameLookup.clear();
	RegisterImageViewer(&g_BasicScanlineViewer);
	RegisterImageViewer(&g_BasicCharsViewer);
}

uint8_t RegisterImageViewer(FImageViewer* pViewer)
{
	const uint8_t id = (uint8_t)g_ImageViewers.size();	// increment by 1 each time - TODO: check for overflow

	if (GetImageViewer(pViewer->GetName()) != nullptr)	// check if viewer with that name has already been registered
		return pViewer->GetId();

	pViewer->SetId(id);
	g_ImageViewers.push_back(pViewer);
	g_ImageViewerNameLookup[pViewer->GetName()] = pViewer;
	return id;
}

const std::vector<const FImageViewer*>& GetImageViewerList()
{
	return g_ImageViewers;
}


const FImageViewer* GetImageViewer(uint8_t id)
{
	if (id >= g_ImageViewers.size())
		return nullptr;

	return g_ImageViewers[id];
}

const FImageViewer* GetImageViewer(const char* name)
{
	auto findIt = g_ImageViewerNameLookup.find(name);
	if (findIt == g_ImageViewerNameLookup.end())
		return nullptr;

	return findIt->second;
}