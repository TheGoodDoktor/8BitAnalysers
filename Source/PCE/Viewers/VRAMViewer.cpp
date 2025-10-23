#include "VRAMViewer.h"

#include <imgui.h>
#include "CodeAnalyser/UI/CodeAnalyserUI.h"
#include "Util/GraphicsView.h"
#include "ImGuiSupport/ImGuiScaling.h"
#include "CodeAnalyser/UI/UIColours.h"
#include "optick/optick.h"
#include "../PCEEmu.h"

#include "Misc/EmuBase.h"

static const int kMemoryViewImageWidth = 128;
static const int kMemoryViewImageHeight = 256;

FVRAMViewer::FVRAMViewer(FEmuBase* pEmu)
	: FViewerBase(pEmu)
{
	Name = "VRAM";
	pPCEEmu = static_cast<FPCEEmu*>(pEmu);
}

bool FVRAMViewer::Init(void)
{
	MemoryViewImage = new FGraphicsView(kMemoryViewImageWidth, kMemoryViewImageHeight);
	MemoryViewImage->Clear(0xff000000);	

	return true;
}

void FVRAMViewer::DrawUI(void)
{
	OPTICK_EVENT();

	DrawPhysicalMemoryOverview();
}

void	FVRAMViewer::DrawLegend()
{
	ImGui::BeginTooltip();

	ImGui::ColorButton("Data Read", ImGui::ColorConvertU32ToFloat4(kDataReadCol), ImGuiColorEditFlags_NoTooltip);
	ImGui::SameLine();
	ImGui::Text("Data Read");

	ImGui::ColorButton("Data Read Active", ImGui::ColorConvertU32ToFloat4(kDataReadActiveCol), ImGuiColorEditFlags_NoTooltip);
	ImGui::SameLine();
	ImGui::Text("Data Read Active");

	ImGui::ColorButton("Data Write", ImGui::ColorConvertU32ToFloat4(kDataWriteCol), ImGuiColorEditFlags_NoTooltip);
	ImGui::SameLine();
	ImGui::Text("Data Write");

	ImGui::ColorButton("Data Write Active", ImGui::ColorConvertU32ToFloat4(kDataWriteActiveCol), ImGuiColorEditFlags_NoTooltip);
	ImGui::SameLine();
	ImGui::Text("Data Write Active");

	ImGui::EndTooltip();
}


void	FVRAMViewer::DrawPhysicalMemoryOverview()
{
	FCodeAnalysisState& state = pEmulator->GetCodeAnalysis();

	MemoryViewImage->Clear(0xff808080);

	uint32_t* pViewImagePixels = MemoryViewImage->GetPixelBuffer();
	uint32_t* pPix = pViewImagePixels;

	DrawUtilisationMap(state,pPix);

	//FCodeAnalysisViewState& viewState = state.GetFocussedViewState();

	FGlobalConfig* pConfig = state.pGlobalConfig;

	ImGui::InputInt("Scale", &pConfig->OverviewScale, 1, 1);
	pConfig->OverviewScale = MAX(1, pConfig->OverviewScale);	// clamp
	
	const float scale = ImGui_GetScaling() * (float)pConfig->OverviewScale;

	MemoryViewImage->UpdateTexture();

	ImGuiIO& io = ImGui::GetIO();
	ImVec2 pos = ImGui::GetCursorScreenPos();

	// Draw Image
	const int height = kMemoryViewImageHeight;
	const ImVec2 size((float)kMemoryViewImageWidth * scale, height * scale);
	const ImVec2 uv0(0, 0);
	const ImVec2 uv1(1.0f, 1.0f);
	ImGui::Image((void*)MemoryViewImage->GetTexture(), size,uv0,uv1);

	const bool bMapIsHovered = ImGui::IsItemHovered();

	ImGui::SameLine();
	ImGui::Button("?");

	if (ImGui::IsItemHovered())
	{
		DrawLegend();
	}

	ImDrawList* dl = ImGui::GetWindowDrawList();

	if (bMapIsHovered)
	{
		/*const int xp = (int)((io.MousePos.x - pos.x) / scale);
		const int yp = (int)((io.MousePos.y - pos.y) / scale);

		const uint16_t addr = (xp + yp * kMemoryViewImageWidth) + (bShowROM ? 0 : 0x4000);	// add offset if we're not showing ROM
		const FAddressRef addrRef = state.AddressRefFromPhysicalAddress(addr);
		ImGui::Text("Location:");
		DrawAddressLabel(state, viewState, addrRef);
		DrawSnippetToolTip(state, viewState, addrRef);

		if (ImGui::IsMouseDoubleClicked(0))
		{
			viewState.GoToAddress(addrRef, false);
		}*/
	}
	else
	{
		ImGui::NewLine();
	}
}


void FVRAMViewer::DrawUtilisationMap(FCodeAnalysisState& state, uint32_t* pPix)
{
	const int frameThreshold = 4;
	const int currentFrameNo = state.CurrentFrameNo;

	int selectedItemAddr = -1;
	for (uint16_t addr = 0; addr < HUC6270_VRAM_SIZE; addr++)
	{
		uint32_t drawCol = kDefaultDataCol;

		if (Access[addr].FrameLastWritten != -1)
		{
			const int framesSinceWritten = currentFrameNo - Access[addr].FrameLastWritten;
			if (framesSinceWritten < frameThreshold)
				drawCol = kDataWriteActiveCol;
			else
				drawCol = kDataWriteCol;
		}
		*pPix++ = drawCol;
	}
}

void FVRAMViewer::RegisterAccess(uint16_t address)
{
	Access[address].FrameLastWritten = pPCEEmu->GetCodeAnalysis().CurrentFrameNo;
}