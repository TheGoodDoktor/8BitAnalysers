#include "C64GraphicsViewer.h"
#include <CodeAnalyser/CodeAnalyser.h>
#include <Util/GraphicsView.h>
#include <imgui.h>

#include <chips/chips_common.h>
#include <chips/m6502.h>
#include <chips/m6526.h>
#include <chips/m6569.h>
#include <chips/m6581.h>
#include <chips/beeper.h>
#include <chips/kbd.h>
#include <chips/mem.h>
#include <chips/clk.h>
#include <chips/m6522.h>
#include <systems/c1530.h>
#include <systems/c1541.h>
#include <systems/c64.h>
#include <systems/c64.h>

#include "../C64Emulator.h"
#include "CodeAnalyser/UI/CodeAnalyserUI.h"

void FC64GraphicsViewer::Init(FC64Emulator* pC64Emu)
{
	C64Emu = pC64Emu;
	CodeAnalysis = &pC64Emu->GetCodeAnalysis();
	CharacterView = new FGraphicsView(320, 408);	// 40 * 51 enough for a 16K Vic bank
	SpriteView = new FGraphicsView(384, 16 * 21); // 16x16 sprites for a VIC bank
	FoundSpritesView = new FGraphicsView(384, 16 * 21); // 16x16 sprites 
	SpriteCols[0] = 0x00000000;
	SpriteCols[1] = 0xffffffff;
	SpriteCols[2] = 0xff888888;
	SpriteCols[3] = 0xff444444;

	CharCols[0] = 0x00000000;
	CharCols[1] = 0xffffffff;
	CharCols[2] = 0xff888888;
	CharCols[3] = 0xff444444;
}

void FC64GraphicsViewer::Shutdown()
{
	delete CharacterView;
	delete SpriteView;
	delete FoundSpritesView;
}

/*
void DrawHiresImageAt(const uint8_t* pSrc, int xp, int yp, int widthChars, int heightPix, FGraphicsView* pGraphicsView, const uint32_t* cols)
{
	uint32_t* pBase = pGraphicsView->GetPixelBuffer() + (xp + (yp * pGraphicsView->GetWidth()));

	*pBase = 0;
	for (int y = 0; y < heightPix; y++)
	{
		for (int x = 0; x < widthChars; x++)
		{
			const uint8_t charLine = *pSrc++;

			for (int xpix = 0; xpix < 8; xpix++)
			{
				const bool bSet = (charLine & (1 << (7 - xpix))) != 0;
				const uint32_t col = bSet ? cols[1] : cols[0];
				// 0 check for sprites?
				*(pBase + xpix + (x * 8)) = col;
			}
		}

		pBase += pGraphicsView->GetWidth();
	}
}*/

void FC64GraphicsViewer::DrawHiResSpriteAt(uint16_t addr, int xp, int yp)
{ 
	const c64_t* pC64 = C64Emu->GetEmu();
	const uint8_t* pRAMAddr = &pC64->ram[addr];
	SpriteView->Draw1BppImageAt(pRAMAddr, xp, yp, 24, 21, SpriteCols);
}

void FC64GraphicsViewer::DrawMultiColourSpriteAt(uint16_t addr, int xp, int yp)
{
	const c64_t* pC64 = C64Emu->GetEmu();
	const uint8_t* pRAMAddr = &pC64->ram[addr];
	SpriteView->Draw2BppWideImageAt(pRAMAddr, xp, yp, 24, 21, SpriteCols);
}

void FC64GraphicsViewer::DrawUI()
{
	
	ImGui::Combo("VIC Bank", &VicBankNo, "Bank 0: $0000 - $3FFF\0Bank 1: $4000 - $7FFF\0Bank 2: $8000 - $BFFF\0Bank 3: $C000 - $FFFF");

	if (ImGui::BeginTabBar("Graphics Tab Bar"))
	{
		if (ImGui::BeginTabItem("Sprites"))
		{
			// Sprite Viewer
			ImGui::Checkbox("Multi-Colour", &SpriteMultiColour);
			const uint16_t vicBankAddr = VicBankNo * 16384;
			SpriteView->Clear(0);

			for (int y = 0; y < 16; y++)
			{
				for (int x = 0; x < 16; x++)
				{
					const int spriteNo = x + (y * 16);
					const uint16_t spriteAddress = vicBankAddr + (spriteNo * 64);
					if (SpriteMultiColour)
						DrawMultiColourSpriteAt(spriteAddress, x * 24, y * 21);
					else
						DrawHiResSpriteAt(spriteAddress, x * 24, y * 21);
				}
			}

			SpriteView->Draw();

			// Colour selection
			ImGui::Text("Sprite Colours");
			if (ImGui::Button("Get from VIC"))
			{
				c64_t* pC64 = C64Emu->GetEmu();
				SpriteCols[1] = m6569_color(pC64->vic.reg.mm[0]);
				SpriteCols[2] = m6569_color(pC64->vic.reg.mc[0]);	// sprite colour
				SpriteCols[3] = m6569_color(pC64->vic.reg.mm[1]);
			}
			
			for (int col = 0; col < 4; col++)
			{
				ImGui::PushID(col);
				const ImVec2 size(18, 18);
				ImVec4 c = ImColor(SpriteCols[col]);
				ImGui::Text("Sprite Col: %d", col);
				ImGui::SameLine();
				
				ImGui::ColorButton("##cur_col", c, ImGuiColorEditFlags_NoAlpha, size);
				for (int i = 0; i < 16; i++)
				{
					ImGui::PushID(i);
					c = ImColor(m6569_color(i));
					if (ImGui::ColorButton("##pick_col", c, ImGuiColorEditFlags_NoAlpha, size))
					{
						SpriteCols[col] = m6569_color(i);
					}
					if(i<15)
						ImGui::SameLine();
					ImGui::PopID();
				}
				ImGui::Separator();
				ImGui::PopID();
			}
			ImGui::EndTabItem();
		}

		if (ImGui::BeginTabItem("Found Sprites"))
		{
			const auto& foundSprites = C64Emu->GetC64IOAnalysis().GetVICAnalysis().GetFoundSprites();
		
			for (int spriteNo = 0; spriteNo < foundSprites.size(); spriteNo++)
			{
				const FSpriteDef& spriteDef = foundSprites[spriteNo];
				ImGui::PushID(spriteDef.Address.Val);
				ImGui::Text("Address: %s",NumStr(spriteDef.Address.Address));
				DrawAddressLabel(*CodeAnalysis, CodeAnalysis->GetFocussedViewState(), spriteDef.Address);
				//ImGui::SameLine();
				if (ImGui::Button("Format Memory"))
				{
					FDataFormattingOptions formatOptions;
					formatOptions.SetupForBitmap(spriteDef.Address, spriteDef.bMultiColour ? 12:24,21, spriteDef.bMultiColour ? 2 : 1);
					formatOptions.DisplayType = spriteDef.bMultiColour ? EDataItemDisplayType::ColMap2Bpp_C64 : EDataItemDisplayType::Bitmap;
					formatOptions.PaletteIndex = spriteDef.PaletteIndex;
					FormatData(*CodeAnalysis,formatOptions);
					CodeAnalysis->SetCodeAnalysisDirty(spriteDef.Address);
				}
				spriteDef.SpriteImage->Draw(24 * 2,21 * 2,false);	// magnifier not working currently - do we need it?
				ImGui::Separator();
				ImGui::PopID();
			}
	#if 0
			FoundSpritesView->Clear(0);

			c64_t* pC64 = C64Emu->GetEmu();


			for (int y = 0; y < 16; y++)
			{
				for (int x = 0; x < 16; x++)
				{
					const int spriteNo = x + (y * 16);

					if(spriteNo < foundSprites.size())
					{
						const FSpriteDef& spriteDef = foundSprites[spriteNo];
						const uint8_t* pRAMAddr = &pC64->ram[spriteDef.Address.Address];
						if (spriteDef.bMultiColour)
							FoundSpritesView->Draw2BppWideImageAt(pRAMAddr, x * 24, y * 21, 24, 21, spriteDef.SpriteCols);
						else
							FoundSpritesView->Draw1BppImageAt(pRAMAddr, x * 24, y * 21, 24, 21, spriteDef.SpriteCols);
					}
					
				}
			}

			FoundSpritesView->Draw();
			#endif
			ImGui::EndTabItem();
		}

		if (ImGui::BeginTabItem("Characters"))
		{
			// Character Viewer
			ImGui::Checkbox("Multi-Colour", &CharacterMultiColour);
			const uint16_t vicBankAddr = VicBankNo * 16384;
			uint16_t charAddr = vicBankAddr;
			CharacterView->Clear(0);
			for (int y = 0; y < 40; y++)
			{
				for (int x = 0; x < 51; x++)
				{
					c64_t* pC64 = C64Emu->GetEmu();
					const uint8_t* pRAMAddr = &pC64->ram[charAddr];

					if (CharacterMultiColour)
						CharacterView->Draw2BppWideImageAt(pRAMAddr, x * 8, y * 8, 8,8,CharCols);
					else
						CharacterView->Draw1BppImageAt(pRAMAddr, x * 8, y * 8, 8, 8,  CharCols);

					charAddr += 8;
				}
			}
			CharacterView->Draw();
			ImGui::EndTabItem();
		}

		ImGui::EndTabBar();
	}
}

	


