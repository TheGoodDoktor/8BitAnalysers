#include "CDROMViewer.h"

//#include <string.h>
#include <imgui.h>

#include "CodeAnalyser/UI/CodeAnalyserUI.h"
#include "Util/GraphicsView.h"
#include "ImGuiSupport/ImGuiScaling.h"
//#include <imgui_internal.h>
//#include "optick/optick.h"
#include "../PCEEmu.h"

//#include "Misc/EmuBase.h"

static const int kMemoryViewImageWidth  = 128;
static const int kMemoryViewImageHeight = 256;

//void DrawDataAccessIndicator(const ImVec2& pos, ImU32 fillCol, ImU32 brdCol, float lineHeight, float lh2);

FCDROMViewer::FCDROMViewer(FEmuBase* pEmu)
	: FViewerBase(pEmu)
{
	Name = "CDROM";
	pPCEEmu = static_cast<FPCEEmu*>(pEmu);
}

bool FCDROMViewer::Init(void)
{
	MemoryViewImage = new FGraphicsView(kMemoryViewImageWidth, kMemoryViewImageHeight);
	MemoryViewImage->Clear(0xff000000);

	return true;
}

void FCDROMViewer::ClearUsage()
{
}

void FCDROMViewer::DrawUI(void)
{
	
}




void FCDROMViewer::DrawLegend()
{
	auto LegendRow = [](const char* id, uint32_t col, const char* label)
	{
		ImGui::ColorButton(id, ImGui::ColorConvertU32ToFloat4(col), ImGuiColorEditFlags_NoTooltip);
		ImGui::SameLine();
		ImGui::Text("%s", label);
	};

	ImGui::BeginTooltip();
	LegendRow("Unwritten",    kUnwrittenCol,          "Never accessed");
	LegendRow("Read",         kDataReadCol,            "Read");
	LegendRow("ReadActive",   kDataReadActiveCol,      "Read (active)");
	ImGui::EndTooltip();
}

#if 0
void FCDROMViewer::DrawPhysicalMemoryOverview()
{
	FCodeAnalysisState& state         = pEmulator->GetCodeAnalysis();
	FCodeAnalysisViewState& viewState = state.GetFocussedViewState();

	MemoryViewImage->Clear(0xff808080);

	uint32_t* pViewImagePixels = MemoryViewImage->GetPixelBuffer();
	uint32_t* pPix = pViewImagePixels;

	DrawUtilisationMap(state, pPix);

	FGlobalConfig* pConfig = state.pGlobalConfig;

	//ImGui::InputInt("Scale", &pConfig->VRAMViewerScale, 1, 1);
	//pConfig->VRAMViewerScale = MAX(1, pConfig->VRAMViewerScale);
	//ImGui::SameLine();
	//if (ImGui::Button("Clear Usage"))
	//	pVRAMState->ClearUsage();

	const float scale = ImGui_GetScaling() * (float)pConfig->VRAMViewerScale;

	MemoryViewImage->UpdateTexture();

	ImGuiIO& io = ImGui::GetIO();
	ImVec2 pos = ImGui::GetCursorScreenPos();

	const int height = kMemoryViewImageHeight;
	const ImVec2 size((float)kMemoryViewImageWidth * scale, height * scale);
	const ImVec2 uv0(0, 0);
	const ImVec2 uv1(1.0f, 1.0f);
	ImGui::Image((void*)MemoryViewImage->GetTexture(), size, uv0, uv1);

	const bool bMapIsHovered = ImGui::IsItemHovered();

	ImGui::SameLine();
	ImGui::Button("?");

	if (ImGui::IsItemHovered())
		DrawLegend();

	if (bMapIsHovered)
	{
		/*
		const int xp = (int)((io.MousePos.x - pos.x) / scale);
		const int yp = (int)((io.MousePos.y - pos.y) / scale);
		const uint16_t addr = (uint16_t)(xp + yp * kMemoryViewImageWidth);

		const FVRAMAccess& access = pVRAMState->GetVRAMAccess(addr);

		ImGui::BeginTooltip();
		ImGui::Text("VRAM: $%04X", addr);
		if (access.LastWriter.IsValid())
		{
			ImGui::Text("Writer:");
			if (bShowWriterSnippet)
			{
				ImGui::NewLine();
				ImGui::Separator();
				DrawSnippetContent(state, viewState, access.LastWriter);
			}
			else
			{
				ImGui::SameLine();
				DrawAddressLabel(state, viewState, access.LastWriter);
			}
		}
		if (access.LastReader.IsValid())
		{
			ImGui::Text("Read by:"); ImGui::SameLine();
			DrawAddressLabel(state, viewState, access.LastReader);
		}
		ImGui::EndTooltip();

		if (ImGui::IsMouseDoubleClicked(0) && access.LastWriter.IsValid())
			viewState.GoToAddress(access.LastWriter, false);
			*/
	}

	//const float lineHeight = ImGui::GetTextLineHeight();
	//const float lh2        = (float)(int)(lineHeight / 2.0f);
	//const int   curFrame   = state.CurrentFrameNo;
	//const ImU32 brdCol     = 0xFF000000;

	
	/*{
		const int lastReadFrame = pVRAMState->GetLastVRAMReadFrame();
		const int framesSince = lastReadFrame == -1 ? 255 : curFrame - lastReadFrame;
		const int brightness  = (255 - MIN(framesSince << 2, 255)) & 0xff;
		const ImVec2 indPos   = ImGui::GetCursorScreenPos();
		ImGui::Dummy(ImVec2(14.0f, lineHeight));
		if (brightness > 0)
			DrawDataAccessIndicator(indPos, 0xff000000 | (brightness << 8), brdCol, lineHeight, lh2);
		ImGui::SameLine();
		ImGui::Text("Last Reader: ");
		ImGui::SameLine();
		const FAddressRef lastReader = pVRAMState->GetLastVRAMReader();
		if (lastReader.IsValid())
			DrawAddressLabel(state, viewState, lastReader);
		else
			ImGui::TextDisabled("None");
	}*/
}
#endif

void FCDROMViewer::DrawUtilisationMap(FCodeAnalysisState& state, uint32_t* pPix)
{
	/*
	const int frameThreshold = 4;
	const int currentFrameNo = state.CurrentFrameNo;
	const FVRAMAnalysisState* pVRAMState = pPCEEmu->GetVRAMAnalysisState();

	for (uint16_t addr = 0; addr < HUC6270_VRAM_SIZE; addr++)
	{
		const FVRAMAccess& access = pVRAMState->GetVRAMAccess(addr);
		const bool written        = access.FrameLastWritten != -1;
		const bool activeWrite    = written && (currentFrameNo - access.FrameLastWritten < frameThreshold);
		const bool read           = access.FrameLastRead != -1;
		const bool activeRead     = read    && (currentFrameNo - access.FrameLastRead    < frameThreshold);

		uint32_t drawCol = kUnwrittenCol;

		if (activeWrite)
			drawCol = kUnknownWriteActiveCol;
		else if (written)
			drawCol = kUnknownWriteCol;
		else if (activeRead)
			drawCol = kDataReadActiveCol;
		else if (read)
			drawCol = kDataReadCol;

		*pPix++ = drawCol;
	}
	*/
}
