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
#include "ImGuiSupport/ImGuiScaling.h"


// Useful VIC info:
// https://www.codebase64.org/doku.php?id=base:vic

void	DrawColourPicker(uint32_t colours[4]);

FC64GraphicsViewer::FC64GraphicsViewer(FC64Emulator* pEmu) : FGraphicsViewer(pEmu)
{
}
bool FC64GraphicsViewer::Init(void)
{
	FGraphicsViewer::Init();

	C64Emu = (FC64Emulator*)pEmulator;
	CodeAnalysis = &pEmulator->GetCodeAnalysis();
	CharacterView = new FGraphicsView(320, 408);	// 40 * 51 enough for a 16K Vic bank
	SpriteView = new FGraphicsView(384, 16 * 21); // 16x16 sprites for a VIC bank
	ScreenView = new FGraphicsView(320,200); 
	ScreenAccessGrid = new FC64ScreenAccessGrid(this);
	SpriteCols[0] = 0x00000000;
	SpriteCols[1] = 0xffffffff;
	SpriteCols[2] = 0xff888888;
	SpriteCols[3] = 0xff444444;

	CharCols[0] = 0x00000000;
	CharCols[1] = 0xffffffff;
	CharCols[2] = 0xff888888;
	CharCols[3] = 0xff444444;

	CurrentPalette[0] = 0x00000000;
	CurrentPalette[1] = 0xffffffff;
	CurrentPalette[2] = 0xff888888;
	CurrentPalette[3] = 0xff444444;

	return true;
}

void FC64GraphicsViewer::Shutdown()
{
	delete CharacterView;
	delete SpriteView;
	delete ScreenView;

	FGraphicsViewer::Shutdown();
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

void FC64GraphicsViewer::DrawSpritesViewer()
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
	DrawColourPicker(SpriteCols);
	c64_t* pC64 = C64Emu->GetEmu();
	if (ImGui::Button("Get from VIC"))
	{
		SpriteCols[1] = m6569_color(pC64->vic.reg.mm[0]);
		SpriteCols[2] = m6569_color(pC64->vic.reg.mc[0]);	// sprite colour
		SpriteCols[3] = m6569_color(pC64->vic.reg.mm[1]);
	}

	const auto& foundSprites = C64Emu->GetC64IOAnalysis().GetVICAnalysis().GetFoundSprites();
	if(foundSprites.size() > 0)
	{
		ImGui::Text("Sprite found by VIC Analyser");
		if(ImGui::BeginChild("foundSprites"))
		{

			for (int spriteNo = 0; spriteNo < foundSprites.size(); spriteNo++)
			{
				const FSpriteDef& spriteDef = foundSprites[spriteNo];
				ImGui::PushID(spriteDef.Address.Val);
				ImGui::Text("Address: %s", NumStr(spriteDef.Address.Address));
				DrawAddressLabel(*CodeAnalysis, CodeAnalysis->GetFocussedViewState(), spriteDef.Address);
				//ImGui::SameLine();
				if (ImGui::Button("Format Memory"))
				{
					FDataFormattingOptions formatOptions;
					formatOptions.SetupForBitmap(spriteDef.Address, 24, 21, 1);
					formatOptions.DisplayType = spriteDef.bMultiColour ? EDataItemDisplayType::ColMapMulticolour_C64 : EDataItemDisplayType::Bitmap;
					formatOptions.PaletteNo = spriteDef.PaletteNo;
					formatOptions.AddLabelAtStart = true;
					formatOptions.LabelName = std::string("sprite_") + NumStr(spriteDef.Address.Address);
					FormatData(*CodeAnalysis, formatOptions);
					CodeAnalysis->SetCodeAnalysisDirty(spriteDef.Address);
				}
				spriteDef.SpriteImage->Draw(24 * 2, 21 * 2, false);	// magnifier not working currently - do we need it?
				ImGui::Separator();
				ImGui::PopID();
			}
		}
		ImGui::EndChild();
	}
}



void FC64GraphicsViewer::DrawCharactersViewer()
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
				CharacterView->Draw2BppWideImageAt(pRAMAddr, x * 8, y * 8, 8, 8, CharCols);
			else
				CharacterView->Draw1BppImageAt(pRAMAddr, x * 8, y * 8, 8, 8, CharCols);

			charAddr += 8;
		}
	}
	CharacterView->Draw();

	// Colour selection
	ImGui::Text("Character Colours");
	DrawColourPicker(CharCols);
	const c64_t* pC64 = C64Emu->GetEmu();
	if (ImGui::Button("Get from VIC"))
	{
		CharCols[0] = m6569_color(pC64->vic.reg.bc[0]);
		CharCols[1] = m6569_color(pC64->vic.reg.bc[1]);
		CharCols[2] = m6569_color(pC64->vic.reg.bc[2]);	
		CharCols[3] = 0;//m6569_color(pC64->vic.reg.bc[3]);
	}

	// Found char sets
	const auto& foundCharSets = C64Emu->GetC64IOAnalysis().GetVICAnalysis().GetFoundCharSets();
	if (foundCharSets.size() > 0)
	{
		ImGui::Text("Char sets found by VIC Analyser");
		if (ImGui::BeginChild("foundChars"))
		{

			for (auto charSet : foundCharSets)
			{
				ImGui::PushID(charSet.Address.Val);
				ImGui::Text("%s",NumStr(charSet.Address.Address));
				DrawAddressLabel(*CodeAnalysis,CodeAnalysis->GetFocussedViewState(), charSet.Address);
				ImGui::SameLine();
				if (ImGui::Button("Create"))
				{
					FCharSetCreateParams params;
					params.Address = charSet.Address;
					params.BitmapFormat = charSet.bMultiColour ? EBitmapFormat::ColMapMulticolour_C64 : EBitmapFormat::Bitmap_1Bpp;	// TODO: handle multicolour
					params.PaletteNo = charSet.PaletteNo;
					CreateCharacterSetAt(*CodeAnalysis, params);
				}
				ImGui::PopID();
			}
		}
		ImGui::EndChild();
	}
}

EC64ScreenMode GetScreenModeFromVIC(c64_t* pC64)
{
	const bool bBitmapMode = !!(pC64->vic.reg.ctrl_1 & (1 << 5));
	const bool bExtendedBackgroundMode = !!(pC64->vic.reg.ctrl_1 & (1 << 6));
	const bool bMultiColourMode = !!(pC64->vic.reg.ctrl_2 & (1 << 4));

	if (bBitmapMode == true)
	{
		if (bMultiColourMode == true)
			return EC64ScreenMode::MulticolourBitmap;
		else
			return EC64ScreenMode::HiresBitmap;
	}
	else 
	{
		if(bMultiColourMode == true)
			return EC64ScreenMode::MulticolourText;
		else
			return bExtendedBackgroundMode ? EC64ScreenMode::ECMText : EC64ScreenMode::HiresText;
	}

	return EC64ScreenMode::Invalid;
}

// Draw a character matrix screen
void FC64GraphicsViewer::DrawCharacterScreen(bool bMulticolour, bool ECM)
{
	c64_t* pC64 = C64Emu->GetEmu();
	const uint16_t charDefs = CharacterBankNo << 11;
	const uint16_t vicMemBase = VicBankNo * 16384;
	const uint16_t screenMem = ScreenBankNo << 10;
	const uint16_t charMapAddress = vicMemBase + screenMem;
	const uint16_t charDefsAddress = vicMemBase + charDefs;
	uint32_t ScreenCols[4];

	ScreenView->Clear(0);

	// Setup Colours
	ScreenCols[0] = m6569_color(pC64->vic.reg.bc[0]);	// these 2 never change
	ScreenCols[2] = m6569_color(pC64->vic.reg.bc[2]);

	const uint32_t BackgroundCol1 = m6569_color(pC64->vic.reg.bc[1]);

	for (int yChar = 0; yChar < 25; yChar++)
	{
		for (int xChar = 0; xChar < 40; xChar++)
		{
			const uint8_t charCode = mem_rd(&pC64->mem_vic,charMapAddress + xChar + (yChar * 40));			
			const uint8_t* pCharDef = mem_readptr(&pC64->mem_vic, charDefsAddress + (charCode * 8));
			const uint8_t colRamVal = pC64->color_ram[xChar + (yChar * 40)];
			const bool bMultiColourChar = bMulticolour && (colRamVal & (1 << 3));

			if(bMultiColourChar)	// if this a multi-colour character
			{
				ScreenCols[1] = BackgroundCol1;
				ScreenCols[3] = m6569_color(colRamVal & 7);
				ScreenView->Draw2BppWideImageAt(pCharDef, xChar * 8, yChar * 8, 8, 8, ScreenCols);
			}
			else if(bMulticolour)	// if this is a hires character in multicolour mode
			{
				ScreenCols[1] = m6569_color(colRamVal & 7);
				ScreenView->Draw1BppImageAt(pCharDef, xChar * 8, yChar * 8, 8, 8, ScreenCols);
			}
			else	// if this is a hires character in hires mode
			{
				ScreenCols[1] = m6569_color(colRamVal & 15);
				ScreenView->Draw1BppImageAt(pCharDef, xChar * 8, yChar * 8, 8, 8, ScreenCols);
			}
		}
	}
}

void FC64GraphicsViewer::DrawBitmapScreen(bool bMulticolour)
{
	c64_t* pC64 = C64Emu->GetEmu();
	const uint16_t bitmapMem = BitmapBankNo << 13;
	const uint16_t vicMemBase = VicBankNo * 16384;
	const uint16_t bitmapAddress = vicMemBase + bitmapMem;
	const uint16_t screenMem = ScreenBankNo << 10;
	const uint16_t charMapAddress = vicMemBase + screenMem;

	const uint8_t* pBitMapMemory = mem_readptr(&pC64->mem_vic, bitmapAddress);
	uint32_t ScreenCols[4];

	ScreenCols[0] = m6569_color(pC64->vic.reg.bc[0]);

	ScreenView->Clear(0);

	int charNo = 0;
	for (int yChar = 0; yChar < 25; yChar++)
	{
		for (int xChar = 0; xChar < 40; xChar++)
		{
			const uint8_t colAttr = mem_rd(&pC64->mem_vic, charMapAddress + xChar + (yChar * 40));
			const uint8_t colRamVal = pC64->color_ram[xChar + (yChar * 40)];
			const uint8_t* pBitmapData = mem_readptr(&pC64->mem_vic, bitmapAddress + (charNo * 8));

			if(bMulticolour)
			{ 
				ScreenCols[1] = m6569_color(colAttr >> 4);
				ScreenCols[2] = m6569_color(colAttr & 15);
				ScreenCols[3] = m6569_color(colRamVal & 15);
				ScreenView->Draw2BppWideImageAt(pBitmapData, xChar * 8, yChar * 8, 8, 8, ScreenCols);
			}
			else
			{
				ScreenCols[0] = m6569_color(colAttr >> 4);
				ScreenCols[1] = m6569_color(colAttr & 15);
				ScreenView->Draw1BppImageAt(pBitmapData, xChar * 8, yChar * 8, 8, 8, ScreenCols);
			}

			charNo++;
		}
	}
}

FC64ScreenAccessGrid::FC64ScreenAccessGrid(FC64GraphicsViewer* pViewer):FMemoryAccessGrid(pViewer->CodeAnalysis,40,25)
{
	GraphicsViewer = pViewer;
}

void FC64ScreenAccessGrid::OnDraw()
{
	const float imgScale = ImGui_GetScaling();
	GridSquareSize = 8.0f * imgScale;	// to fit an 8x8 square on a scaling screen image
}

FAddressRef FC64ScreenAccessGrid::GetGridSquareAddress(int x, int y)
{
	const FC64Emulator* pC64Emu = GraphicsViewer->C64Emu;
	const uint16_t screenCharacterMem = GraphicsViewer->ScreenBankNo << 10;
	const uint16_t bitmapMem = GraphicsViewer->BitmapBankNo << 13;

	FAddressRef address;

	switch (GraphicsViewer->MemoryAccessDisplay)
	{
	case EGraphicsMemoryAccess::Characters:
	case EGraphicsMemoryAccess::MulticolourAttributes:
		address = pC64Emu->GetVICMemoryAddress(screenCharacterMem + x + (y * GridSizeX));
		break;

	case EGraphicsMemoryAccess::Pixels:	// for Bitmap modes
		address = pC64Emu->GetVICMemoryAddress(bitmapMem + (x + (y * GridSizeX)) * 8);
		break;
	case EGraphicsMemoryAccess::ColourRAM:
		address = pC64Emu->GetColourRAMAddress(x + (y * GridSizeX));
		break;
	}

	return address;
}

void FC64GraphicsViewer::DrawCharacterBankCombo()
{
	const uint16_t charDefs = CharacterBankNo << 11;
	const uint16_t vicMemBase = VicBankNo * 16384;
	const uint16_t charDefsAddress = vicMemBase + charDefs;

	if (ImGui::BeginCombo("Character Bank", NumStr(charDefsAddress), ImGuiComboFlags_None))
	{
		for (int i = 0; i < 8; i++)
		{
			const uint16_t bankAddr = vicMemBase + (uint16_t)(i << 11);
			if (ImGui::Selectable(NumStr(bankAddr), i == CharacterBankNo))
			{
				CharacterBankNo = i;
			}
		}
		ImGui::EndCombo();
	}
}

void FC64GraphicsViewer::DrawScreenBankCombo()
{
	const uint16_t vicMemBase = VicBankNo * 16384;
	const uint16_t screenMem = ScreenBankNo << 10;
	const uint16_t screenMemAddress = vicMemBase + screenMem;

	if (ImGui::BeginCombo("Screen Bank", NumStr(screenMemAddress), ImGuiComboFlags_None))
	{
		for (int i = 0; i < 16; i++)
		{
			const uint16_t bankAddr = vicMemBase + (uint16_t)(i << 10);
			if (ImGui::Selectable(NumStr(bankAddr), i == ScreenBankNo))
			{
				ScreenBankNo = i;
			}
		}
		ImGui::EndCombo();
	}
}

void FC64GraphicsViewer::DrawBitmapBankCombo()
{

}

const char* g_MemoryAccessEnum[]=
{
	"Characters",
	"Pixels",
	"MulticolourAttributes",
	"ColourRAM"
};

void FC64GraphicsViewer::DrawMemoryAccessCombo()
{
	if (ImGui::BeginCombo("Show Memory Access", g_MemoryAccessEnum[(int)MemoryAccessDisplay], ImGuiComboFlags_None))
	{
		if (ImGui::Selectable("Characters", MemoryAccessDisplay == EGraphicsMemoryAccess::Characters))
			MemoryAccessDisplay = EGraphicsMemoryAccess::Characters;
		if (ImGui::Selectable("Pixels", MemoryAccessDisplay == EGraphicsMemoryAccess::Pixels))
			MemoryAccessDisplay = EGraphicsMemoryAccess::Pixels;
		if (ImGui::Selectable("MulticolourAttributes", MemoryAccessDisplay == EGraphicsMemoryAccess::MulticolourAttributes))
			MemoryAccessDisplay = EGraphicsMemoryAccess::MulticolourAttributes;
		if (ImGui::Selectable("ColourRAM", MemoryAccessDisplay == EGraphicsMemoryAccess::ColourRAM))
			MemoryAccessDisplay = EGraphicsMemoryAccess::ColourRAM;

		ImGui::EndCombo();
	}
}


void FC64GraphicsViewer::DrawScreenViewer()
{
	c64_t* pC64 = C64Emu->GetEmu();
	ImGui::Combo("Screen Mode", (int*) &ScreenMode, "HiresText\0MulticolourText\0HiresBitmap\0MulticolourBitmap\0ECMText\0");
	DrawScreenBankCombo();
	DrawCharacterBankCombo();
	DrawBitmapBankCombo();
	
	if (ImGui::Button("Get from VIC"))
	{
		EC64ScreenMode mode = GetScreenModeFromVIC(pC64);
		if(mode != EC64ScreenMode::Invalid)
			ScreenMode = mode;

		VicBankNo = pC64->vic_bank_select >> 14;
		BitmapBankNo = ((pC64->vic.reg.mem_ptrs >> 3) & 1);
		ScreenBankNo = pC64->vic.reg.mem_ptrs >> 4;
		CharacterBankNo = (pC64->vic.reg.mem_ptrs >> 1) & 7;
	}

	// Memory locations
	//const uint16_t vicMemBase = pC64->vic_bank_select;
	//const uint16_t screenMem = (pC64->vic.reg.mem_ptrs >> 4) << 10;
	//const uint16_t charDefs = ((pC64->vic.reg.mem_ptrs >> 1) & 7) << 11;
	bool bBitmapMode = false;
	switch (ScreenMode)
	{
		case EC64ScreenMode::HiresText:
			DrawCharacterScreen(false,false);
			break;
		case EC64ScreenMode::MulticolourText:
			DrawCharacterScreen(true,false);
			break;
		case EC64ScreenMode::HiresBitmap:
			DrawBitmapScreen(false);
			bBitmapMode = true;
			break;
		case EC64ScreenMode::MulticolourBitmap:
			DrawBitmapScreen(true);
			bBitmapMode = true;
			break;
	}

	DrawMemoryAccessCombo();
	const ImVec2 pos = ImGui::GetCursorScreenPos();
	ScreenView->Draw(true);
	ScreenAccessGrid->DrawAt(pos.x,pos.y);
}


void FC64GraphicsViewer::DrawUI()
{
	ImGui::Combo("VIC Bank", &VicBankNo, "Bank 0: $0000 - $3FFF\0Bank 1: $4000 - $7FFF\0Bank 2: $8000 - $BFFF\0Bank 3: $C000 - $FFFF\0");
	ImGui::SameLine();
	if (ImGui::Button("Get from VIC"))
	{
		VicBankNo = C64Emu->GetEmu()->vic_bank_select >> 14;
	}

	if (ImGui::BeginTabBar("Graphics Tab Bar"))
	{
		if (ImGui::BeginTabItem("GFX"))
		{
			DrawCharacterGraphicsViewer();
			ImGui::EndTabItem();
		}

		if (ImGui::BeginTabItem("Sprites"))
		{
			DrawSpritesViewer();
			ImGui::EndTabItem();
		}

		if (ImGui::BeginTabItem("Characters"))
		{
			DrawCharactersViewer();
			ImGui::EndTabItem();
		}

		if (ImGui::BeginTabItem("Screen"))
		{
			DrawScreenViewer();
			ImGui::EndTabItem();
		}



		ImGui::EndTabBar();
	}
}

	
// Util Functions
void DrawColourPicker(uint32_t colours[4])
{
	for (int col = 0; col < 4; col++)
	{
		ImGui::PushID(col);
		const ImVec2 size(18, 18);
		ImVec4 c = ImColor(colours[col]);
		ImGui::Text("Colour: %d", col);
		ImGui::SameLine();

		ImGui::ColorButton("##cur_col", c, ImGuiColorEditFlags_NoAlpha, size);
		for (int i = 0; i < 16; i++)
		{
			ImGui::PushID(i);
			c = ImColor(m6569_color(i));
			if (ImGui::ColorButton("##pick_col", c, ImGuiColorEditFlags_NoAlpha, size))
			{
				colours[col] = m6569_color(i);
			}
			if (i < 15)
				ImGui::SameLine();
			ImGui::PopID();
		}
		ImGui::Separator();
		ImGui::PopID();
	}

	
}

