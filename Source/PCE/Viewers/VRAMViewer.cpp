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

void FVRAMViewer::Reset(void)
{
	for (FVRAMAccess& access : Access)
	{
		access.FrameLastWritten = -1;
		access.LastWriter = FAddressRef();
	}
}

void FVRAMViewer::DrawUI(void)
{
	OPTICK_EVENT();

	DrawPhysicalMemoryOverview();
}

void	FVRAMViewer::DrawLegend()
{
	ImGui::BeginTooltip();

	ImGui::ColorButton("Unwritten", ImGui::ColorConvertU32ToFloat4(kUnwrittenCol), ImGuiColorEditFlags_NoTooltip);
	ImGui::SameLine();
	ImGui::Text("Unwritten");

	ImGui::ColorButton("Write", ImGui::ColorConvertU32ToFloat4(kUnknownWriteCol), ImGuiColorEditFlags_NoTooltip);
	ImGui::SameLine();
	ImGui::Text("Write");

	ImGui::ColorButton("Active Write", ImGui::ColorConvertU32ToFloat4(kUnknownWriteActiveCol), ImGuiColorEditFlags_NoTooltip);
	ImGui::SameLine();
	ImGui::Text("Active Write");

	ImGui::ColorButton("SpriteWrite", ImGui::ColorConvertU32ToFloat4(kSpriteWriteCol), ImGuiColorEditFlags_NoTooltip);
	ImGui::SameLine();
	ImGui::Text("Sprite Write");

	ImGui::ColorButton("Active Sprite Write", ImGui::ColorConvertU32ToFloat4(kSpriteWriteActiveCol), ImGuiColorEditFlags_NoTooltip);
	ImGui::SameLine();
	ImGui::Text("Active Sprite Write");

	ImGui::EndTooltip();
}

void	FVRAMViewer::DrawPhysicalMemoryOverview()
{
	FCodeAnalysisState& state = pEmulator->GetCodeAnalysis();

	MemoryViewImage->Clear(0xff808080);

	uint32_t* pViewImagePixels = MemoryViewImage->GetPixelBuffer();
	uint32_t* pPix = pViewImagePixels;

	DrawUtilisationMap(state,pPix);

	FGlobalConfig* pConfig = state.pGlobalConfig;

	ImGui::InputInt("Scale", &pConfig->VRAMViewerScale, 1, 1);
	pConfig->VRAMViewerScale = MAX(1, pConfig->VRAMViewerScale);	// clamp
	
	const float scale = ImGui_GetScaling() * (float)pConfig->VRAMViewerScale;

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
	SpriteHighlight = -1;

	ImGui::SameLine();
	ImGui::Button("?");

	if (ImGui::IsItemHovered())
	{
		DrawLegend();
	}

	ImDrawList* dl = ImGui::GetWindowDrawList();

	if (bMapIsHovered)
	{
		FCodeAnalysisViewState& viewState = state.GetFocussedViewState();

		const int xp = (int)((io.MousePos.x - pos.x) / scale);
		const int yp = (int)((io.MousePos.y - pos.y) / scale);

		const uint16_t addr = xp + yp * kMemoryViewImageWidth;
		ImGui::Text("VRAM Address: %s", NumStr(addr));
		if (Access[addr].FrameLastWritten != -1)
		{
			const FAddressRef& lastWriter = Access[addr].LastWriter;
			const int spriteIndex = GetSpriteIndexForAddress(addr);
			if (spriteIndex != -1)
			{
				ImGui::BeginTooltip();
				ImGui::Text("Sprite %d", spriteIndex);
				ImGui::Text("Last Writer ");
				DrawAddressLabel(state, viewState, lastWriter);
				ImGui::EndTooltip();
				SpriteHighlight = spriteIndex;
			}

			if (ImGui::IsMouseDoubleClicked(0))
			{
				viewState.GoToAddress(lastWriter, false);
			}
		}
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
		uint32_t drawCol = kUnwrittenCol;

		if (Access[addr].FrameLastWritten != -1)
		{
			const int framesSinceWritten = currentFrameNo - Access[addr].FrameLastWritten;

			// this will be slow.
			// todo: a better way
			const int spriteIndex = GetSpriteIndexForAddress(addr);
			const bool bIsSprite = spriteIndex != -1;

			if (bIsSprite)
			{
				if (SpriteHighlight == spriteIndex)
					drawCol = Colours::GetFlashColour();
				else
				{
					if (framesSinceWritten < frameThreshold)
					{
						drawCol = kSpriteWriteActiveCol;
					}
					else
					{
						drawCol = kSpriteWriteCol;
					}
				}
			}
			else
			{
				if (framesSinceWritten < frameThreshold)
				{
					drawCol = kUnknownWriteActiveCol;
				}
				else
				{
					drawCol = kUnknownWriteCol;
				}
			}
		}
		*pPix++ = drawCol;
	}
}

void FVRAMViewer::RegisterAccess(uint16_t vramAddress, FAddressRef writer)
{
	if (vramAddress < HUC6270_VRAM_SIZE)
	{
		FVRAMAccess& access = Access[vramAddress];
		access.FrameLastWritten = pPCEEmu->GetCodeAnalysis().CurrentFrameNo;
		access.LastWriter = writer;
	}
}

// Cache sprite info, such as dimensions and vram address. 
void FVRAMViewer::Tick()
{
	GeargrafxCore* pCore = pPCEEmu->GetCore();

	HuC6270* huc6270 = pCore->GetHuC6270_1();
	u16* vram = huc6270->GetVRAM();
	u16* sat = huc6270->GetSAT();

	for (int i = 0; i < HUC6270_SPRITES; i++)
	{
		const int sprite_offset = i << 2;
		const u16 flags = sat[sprite_offset + 3] & 0xB98F;
		const int cgx = (flags >> 8) & 0x01;
		const int cgy = (flags >> 12) & 0x03;
		SpriteInfo[i].Width = k_huc6270_sprite_width[cgx];
		SpriteInfo[i].Height = k_huc6270_sprite_height[cgy];
		SpriteInfo[i].SizeInBytes = (SpriteInfo[i].Width * SpriteInfo[i].Height) >> 2;

		u16 pattern = (sat[sprite_offset + 2] >> 1) & 0x3FF;
		pattern &= k_huc6270_sprite_mask_width[cgx];
		pattern &= k_huc6270_sprite_mask_height[cgy];
		SpriteInfo[i].Address = pattern << 6;
	}
}

// todo: deal with the fact there can be multiple sprites sharing the same adddress.
// they can have the same address but different palettes
int FVRAMViewer::GetSpriteIndexForAddress(uint16_t addr) const
{
	for (int i = 0; i < HUC6270_SPRITES; i++)
	{
		const uint16_t addrEnd = SpriteInfo[i].Address + SpriteInfo[i].SizeInBytes;
		if (addr > SpriteInfo[i].Address && addr < addrEnd)
		{
			return i;
		}
	}
	return -1;
}
