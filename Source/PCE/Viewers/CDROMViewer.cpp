#include "CDROMViewer.h"

//#include <string.h>
#include <algorithm>
#include <imgui.h>

#include "CodeAnalyser/UI/CodeAnalyserUI.h"
#include "Util/GraphicsView.h"
#include "ImGuiSupport/ImGuiScaling.h"
//#include <imgui_internal.h>
//#include "optick/optick.h"
#include "../PCEEmu.h"
#include "geargrafx_core.h"
#include "cdrom_media.h"
#include "cdrom_image.h"
#include "cdrom_common.h"
#include "cdrom_audio.h"
#include "scsi_controller.h"

static const int kMemoryViewImageWidth  = 128;
static const int kMemoryViewImageHeight = 256;

void DrawDataAccessIndicator(const ImVec2& pos, ImU32 fillCol, ImU32 brdCol, float lineHeight, float lh2);

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

void FCDROMViewer::Tick()
{
	if (pPCEEmu->IsCDROM())
		DetectDataReads();
}

void FCDROMViewer::DetectDataReads()
{
	GeargrafxCore* pCore = pPCEEmu->GetCore();
	CdRomMedia* pCdRomMedia = pCore->GetCDROMMedia();
	const int trackCount = (int)pCdRomMedia->GetTracks().size();
	const int currentFrame = pPCEEmu->GetCodeAnalysis().CurrentFrameNo;

	if ((int)TrackLastReadFrame.size() < trackCount)
		TrackLastReadFrame.resize(trackCount, -1);

	// Poll data sector reads via SCSI state.
	const ScsiController::Scsi_State* pScsiState = pCore->GetScsiController()->GetState();
	if (*pScsiState->LOAD_SECTOR_COUNT > 0)
	{
		const s32 trackIdx = pCdRomMedia->GetTrackFromLBA(*pScsiState->LOAD_SECTOR);
		if (trackIdx >= 0 && trackIdx < trackCount)
			TrackLastReadFrame[trackIdx] = currentFrame;
	}

	// Poll audio reads via CdRomAudio state.
	const CdRomAudio::CdRomAudio_State* pAudioState = pCore->GetCDROMAudio()->GetState();
	if (*pAudioState->CURRENT_STATE == CdRomAudio::CD_AUDIO_STATE_PLAYING)
	{
		const s32 trackIdx = pCdRomMedia->GetTrackFromLBA(*pAudioState->CURRENT_LBA);
		if (trackIdx >= 0 && trackIdx < trackCount)
			TrackLastReadFrame[trackIdx] = currentFrame;
	}
}

void FCDROMViewer::DrawUI(void)
{
	if (!pPCEEmu->IsCDROM())
	{
		ImGui::TextDisabled("No CD-ROM loaded.");
		return;
	}

	CdRomMedia* pCdRomMedia = pPCEEmu->GetCore()->GetCDROMMedia();
	const auto& tracks = pCdRomMedia->GetTracks();
	const int trackCount = (int)tracks.size();

	ImGui::Text("Tracks: %d   Sectors: %u   Current Sector: %u",
		trackCount, pCdRomMedia->GetSectorCount(), pCdRomMedia->GetCurrentSector());
	ImGui::Separator();

	const ImGuiTableFlags tableFlags =
		ImGuiTableFlags_SizingFixedFit |
		ImGuiTableFlags_RowBg |
		ImGuiTableFlags_BordersInnerV |
		ImGuiTableFlags_BordersOuter |
		ImGuiTableFlags_ScrollY;

	const int   currentFrame = pPCEEmu->GetCodeAnalysis().CurrentFrameNo;
	const float lineHeight   = ImGui::GetTextLineHeight();
	const float lh2          = (float)(int)(lineHeight / 2.0f);
	const ImU32 brdCol       = 0xFF000000;
	const float fontWidth    = ImGui_GetFontCharWidth();

	if (ImGui::BeginTable("##cdtracks", 5, tableFlags))
	{
		ImGui::TableSetupScrollFreeze(0, 1);
		ImGui::TableSetupColumn("R",         ImGuiTableColumnFlags_WidthFixed,   18.0f);
		ImGui::TableSetupColumn("",          ImGuiTableColumnFlags_WidthFixed,   fontWidth * 3.0f);
		ImGui::TableSetupColumn("Type",      ImGuiTableColumnFlags_WidthFixed,   fontWidth * 7.0f);
		ImGui::TableSetupColumn("Sectors",   ImGuiTableColumnFlags_WidthFixed,   fontWidth * 8.0f);
		ImGui::TableSetupColumn("Size (KB)", ImGuiTableColumnFlags_WidthStretch);
		ImGui::TableHeadersRow();

		for (int i = 0; i < trackCount; i++)
		{
			const CdRomImage::Track& track = tracks[i];

			ImGui::TableNextRow();
			ImGui::TableSetColumnIndex(0);

			const bool bSelected = (SelectedTrackIndex == i);
			const ImGuiSelectableFlags selFlags = ImGuiSelectableFlags_SpanAllColumns | ImGuiSelectableFlags_AllowOverlap;
			ImGui::PushID(i);
			if (ImGui::Selectable("##row", bSelected, selFlags, ImVec2(0, lineHeight)))
				SelectedTrackIndex = i;
			ImGui::PopID();

			// Draw read indicator in the "R" column.
			if (i < (int)TrackLastReadFrame.size())
			{
				const int framesSinceRead = TrackLastReadFrame[i] == -1 ? 255 : currentFrame - TrackLastReadFrame[i];
				const int brightness      = (255 - std::min(framesSinceRead << 2, 255)) & 0xff;
				if (brightness > 0)
				{
					ImGui::SameLine();
					const ImVec2 indPos = ImGui::GetCursorScreenPos();
					ImGui::Dummy(ImVec2(14.0f, lineHeight));
					DrawDataAccessIndicator(indPos, 0xff000000 | (brightness << 8), brdCol, lineHeight, lh2);
				}
			}

			ImGui::TableSetColumnIndex(1);
			ImGui::Text("%d", i + 1);

			ImGui::TableSetColumnIndex(2);
			ImGui::Text("%s", track.type == GG_CDROM_AUDIO_TRACK ? "Audio" : "Data");

			ImGui::TableSetColumnIndex(3);
			ImGui::Text("%u", track.sector_count);

			ImGui::TableSetColumnIndex(4);
			ImGui::Text("%u", (track.sector_count * track.sector_size) / 1024);
		}

		ImGui::EndTable();
	}
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
