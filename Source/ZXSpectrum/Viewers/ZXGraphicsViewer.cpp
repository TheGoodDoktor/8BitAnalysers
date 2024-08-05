#include "ZXGraphicsViewer.h"


#include <Util/GraphicsView.h>
#include <CodeAnalyser/CodeAnalyser.h>

#include "../SpectrumConstants.h"
#include "../SpectrumEmu.h"
#include <ImGuiSupport/ImGuiScaling.h>

// ZX Spectrum specific implementation
void FZXGraphicsViewer::DrawScreenViewer()
{
	UpdateScreenPixelImage();

	// View Scale
	ImGui::InputInt("Scale", &ScreenViewScale, 1, 1);
	ScreenViewScale = std::max(1, ScreenViewScale);	// clamp

	const float scale = ImGui_GetScaling() * ScreenViewScale;

	ImVec2 pos = ImGui::GetCursorScreenPos();
	pScreenView->Draw(pScreenView->GetWidth() * (float)ScreenViewScale, pScreenView->GetHeight() * (float)ScreenViewScale, true);
	if (ImGui::IsItemHovered())
	{
		ImGuiIO& io = ImGui::GetIO();
		const int xp = std::min(std::max((int)((io.MousePos.x - pos.x) / scale), 0), 255);
		const int yp = std::min(std::max((int)((io.MousePos.y - pos.y) / scale), 0), 191);

		const uint16_t scrPixAddress = GetScreenPixMemoryAddress(xp, yp);
		const uint16_t scrAttrAddress = GetScreenAttrMemoryAddress(xp, yp);
	}
	ImGui::Checkbox("Show Memory Accesses", &bShowScreenMemoryAccesses);
	ImGui::Checkbox("Show Attributes",&bShowScreenAttributes);
}

// Description of memory format here:
// http://www.breakintoprogram.co.uk/computers/zx-spectrum/screen-memory-layout
void FZXGraphicsViewer::UpdateScreenPixelImage(void)
{
	FGraphicsView* pGraphicsView = pScreenView;
	const FCodeAnalysisState& state = GetCodeAnalysis();
	const int16_t bankId = Bank == -1 ? state.GetBankFromAddress(kScreenPixMemStart) : Bank;
	const FCodeAnalysisBank* pBank = state.GetBank(bankId);

	uint16_t bankAddr = 0;
	for (int y = 0; y < ScreenHeight; y++)
	{
		const int y0to2 = ((bankAddr >> 8) & 7);
		const int y3to5 = ((bankAddr >> 5) & 7) << 3;
		const int y6to7 = ((bankAddr >> 11) & 3) << 6;
		const int yDestPos = y0to2 | y3to5 | y6to7;	// or offsets together

		// determine dest pointer for scanline
		uint32_t* pLineAddr = pGraphicsView->GetPixelBuffer() + (yDestPos * ScreenWidth);

		// pixel line
		for (int x = 0; x < ScreenWidth / 8; x++)
		{
			const uint8_t charLine = pBank->Memory[bankAddr];
			const FCodeAnalysisPage& page = pBank->Pages[bankAddr >> 10];
			uint32_t inkCol = 0xffffffff;
			uint32_t paperCol = 0xff000000;
			
			if(bShowScreenMemoryAccesses)
				inkCol = GetHeatmapColourForMemoryAddress(page, bankAddr, state.CurrentFrameNo, HeatmapThreshold);
			if (bShowScreenAttributes && inkCol == 0xffffffff)
			{
				uint8_t colAttr = pBank->Memory[kScreenPixMemSize + ((yDestPos >>3) * 32) + x];
				const uint32_t* colourLUT = state.Config.CharacterColourLUT;
				const bool bBright = !!(colAttr & (1 << 6));
				inkCol = GetColFromAttr(colAttr & 7, colourLUT, bBright);
				paperCol = GetColFromAttr((colAttr >> 3) & 7, colourLUT, bBright);
			}

			for (int xpix = 0; xpix < 8; xpix++)
			{
				const bool bSet = (charLine & (1 << (7 - xpix))) != 0;
				*(pLineAddr + xpix + (x * 8)) = bSet ? inkCol : paperCol;
			}

			bankAddr++;
		}
	}

}