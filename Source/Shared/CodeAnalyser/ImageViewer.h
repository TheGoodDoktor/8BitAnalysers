#pragma once

#include <cstdint>
#include <vector>

// Image Viewer code - move ?
class FImageViewer
{
public:
	uint8_t		GetId() const { return Id; }
	void		SetId(uint8_t id) { Id = id; }

	virtual const char* GetName() const = 0;

	virtual void DrawImageToView(uint16_t addr, int widthChars, int heightChars, class FGraphicsView* pView, const class ICPUInterface* pCPUIF) const = 0;
private:
	uint8_t		Id = 0;
};

void InitImageViewers(void);
const std::vector<const FImageViewer*>& GetImageViewerList();
const FImageViewer* GetImageViewer(uint8_t id);
const FImageViewer* GetImageViewer(const char* name);
uint8_t RegisterImageViewer(FImageViewer* pViewer);