#include "VICAnalysis.h"
#include <CodeAnalyser/CodeAnalyser.h>
#include <CodeAnalyser/UI/CodeAnalyserUI.h>
#include "../C64Emulator.h"

#include <chips/chips_common.h>
#include "Util/GraphicsView.h"
#include <ImGuiSupport/ImGuiScaling.h>

void VICWriteEventShowAddress(FCodeAnalysisState& state, const FEvent& event);
void VICWriteEventShowValue(FCodeAnalysisState& state, const FEvent& event);

FVICAnalysis* pVIC = nullptr; // hack

// Handler to describe VIC memory regions
class FVICMemDescGenerator : public FMemoryRegionDescGenerator
{
	public:
		FVICMemDescGenerator(FC64Emulator* pEmulator):pC64Emulator(pEmulator)
		{
		}

		const char* GenerateAddressString(FAddressRef addr) override
		{
			if(pC64Emulator->IsAddressedByVIC(addr) == false)
				return nullptr;

			const uint16_t spritePtrs = CharMapAddress + 1016;

			if (addr.Address >= CharMapAddress && addr.Address < CharMapAddress + 1000)
			{
				const int charNo = addr.Address - CharMapAddress;
				const int charX = charNo % 40;
				const int charY = charNo / 40;

				sprintf(DescStr, "<Screen Char: %d,%d>", charX, charY);
				return DescStr;
			}
			else if(addr.Address >= spritePtrs && addr.Address < spritePtrs + 8)
			{
				sprintf(DescStr, "<Sprite%dImageNo>", addr.Address - spritePtrs);
				return DescStr;
			}
			else if (bBitmapMode && addr.Address >= BitmapAddress && addr.Address < BitmapAddress + (1 << 13))
			{
				const uint16_t bitmapAddressOffset = addr.Address - BitmapAddress;
				//const uint16_t bitmapCharNo = bitmapAddressOffset / 8;
				const int bitmapX = bitmapAddressOffset % 40;
				const int bitmapY = bitmapAddressOffset / 40;

				sprintf(DescStr, "<Bitmap Screen: %d,%d>",bitmapX,bitmapY);
				return DescStr;
			}
			return nullptr;
		}

		void FrameTick() override 
		{
			c64_t* pC64 = pC64Emulator->GetEmu();
			const uint16_t vicMemBase = pC64->vic_bank_select;

			RegionMin = vicMemBase;
			RegionMax = vicMemBase + 16384 - 1;	// 16K after
			RegionBankId = -1;//pC64Emulator->GetVICMemoryAddress(0).BankId;

			const uint16_t bitmapBankNo = ((pC64->vic.reg.mem_ptrs >> 3) & 1);
			const uint16_t screenBankNo = pC64->vic.reg.mem_ptrs >> 4;
			const uint16_t characterBankNo = (pC64->vic.reg.mem_ptrs >> 1) & 7;
			const uint16_t screenMem = screenBankNo << 10;
			CharMapAddress = vicMemBase + screenMem;
			
			// bitmap mode
			bBitmapMode = !!(pC64->vic.reg.ctrl_1 & (1 << 5));
			BitmapAddress = vicMemBase + (bitmapBankNo << 13);
		}
	private:
		FC64Emulator* pC64Emulator = nullptr;
		uint16_t CharMapAddress = 0;
		bool bBitmapMode = false;
		uint16_t BitmapAddress = 0;
		char DescStr[32] = { 0 };
};

class FColourRAMMemDescGenerator : public FMemoryRegionDescGenerator
{
public:
	FColourRAMMemDescGenerator(FC64Emulator* pEmulator)
	{
		RegionMin = 0xD800;
		RegionMax = 0xDBE7;
		RegionBankId = pEmulator->GetIOAreaBankId();
	}

	const char* GenerateAddressString(FAddressRef addr) override
	{
		const int ColRAMAddr = addr.Address - RegionMin;
		const int colX = ColRAMAddr % 40;
		const int colY = ColRAMAddr / 40;
		sprintf(DescStr, "<Colour RAM: %d,%d>", colX, colY);
		return DescStr;
	}
private:
	char DescStr[32] = { 0 };
};


void FVICAnalysis::Init(FC64Emulator* pEmulator)
{
	Name = "VIC-II";
	SetAnalyser(&pEmulator->GetCodeAnalysis());
	pCodeAnalyser->IOAnalyser.AddDevice(this);
	pC64Emu = pEmulator;
	pVIC = this;

	AddMemoryRegionDescGenerator(new FVICMemDescGenerator(pEmulator));
	AddMemoryRegionDescGenerator(new FColourRAMMemDescGenerator(pEmulator));
    
    // Register Events
	pCodeAnalyser->Debugger.RegisterEventType((uint8_t)EC64Event::VICRegisterWrite, "VIC Write", 0xff0000ff, VICWriteEventShowAddress, VICWriteEventShowValue);
    
    pCodeAnalyser->Debugger.RegisterEventType((uint8_t)EC64Event::VICScreenModeChar, "VIC Screen Mode Char", 0xff0010ff);
    pCodeAnalyser->Debugger.RegisterEventType((uint8_t)EC64Event::VICScreenModeBmp, "VIC Screen Mode Bmp", 0xff0020ff);
    pCodeAnalyser->Debugger.RegisterEventType((uint8_t)EC64Event::VICScreenModeMultiColour, "VIC Screen Mode MultiCol", 0xff0030ff);
    pCodeAnalyser->Debugger.RegisterEventType((uint8_t)EC64Event::VICScreenModeHiRes, "VIC Screen Mode HiRes", 0xff0040ff);
}

void FVICAnalysis::Reset(void)
{
	for (int i = 0; i < kNoRegisters; i++)
		VICRegisters[i].Reset();
}

void FVICAnalysis::OnMachineFrameStart(void)
{
	FrameSprites.clear();

	// add new sprites
	for (int spriteNo = 0; spriteNo < 8; spriteNo++)
	{
		FSpriteInfo newSprite;
		newSprite.SpriteNo = spriteNo;
		FrameSprites.push_back(newSprite);
	}
}

void FVICAnalysis::OnMachineFrameEnd(void)
{
	c64_t* pC64 = pC64Emu->GetEmu();
	uint16_t vicMemBase = pC64->vic_bank_select;
	uint16_t screenMem = (pC64->vic.reg.mem_ptrs >> 4) << 10;

	// Analyse active sprites
	uint16_t spritePtrs = vicMemBase + screenMem + 1016;

	for (int spriteNo = 0; spriteNo < 8; spriteNo++)
	{
		if (pC64->vic.reg.me & (1 << spriteNo))
		{
			const uint8_t spriteDefNo = mem_rd(&pC64->mem_vic, spritePtrs + spriteNo);
			FAddressRef spriteDefAddress = pC64Emu->GetVICMemoryAddress(spriteDefNo * 64);

			bool bFound = false;
			for (const FSpriteDef& spriteDef : SpriteDefs)
			{
				if (spriteDef.Address == spriteDefAddress)
				{
					bFound = true;
					break;
				}
			}

			if (bFound == false)
			{
				FSpriteDef newSpriteDef;
				newSpriteDef.Address = spriteDefAddress;
				newSpriteDef.bMultiColour = pC64->vic.reg.mmc & (1 << spriteNo);

				uint32_t SpriteCols[4];
				SpriteCols[0] = 0;	// transparent
				SpriteCols[1] = m6569_color(pC64->vic.reg.mm[0]);
				SpriteCols[2] = m6569_color(pC64->vic.reg.mc[spriteNo]);
				SpriteCols[3] = m6569_color(pC64->vic.reg.mm[1]);

				newSpriteDef.PaletteNo = GetPaletteNo(SpriteCols, newSpriteDef.bMultiColour ? 4 : 2);

				// Create sprite texture
				newSpriteDef.SpriteImage = new FGraphicsView(24, 21);
				const uint8_t* pRAMAddr = &pC64->ram[newSpriteDef.Address.Address];
				if (newSpriteDef.bMultiColour)
					newSpriteDef.SpriteImage->Draw2BppWideImageAt(pRAMAddr, 0, 0, 24, 21, SpriteCols);
				else
					newSpriteDef.SpriteImage->Draw1BppImageAt(pRAMAddr, 0, 0, 24, 21, SpriteCols);

				newSpriteDef.SpriteImage->UpdateTexture();
				SpriteDefs.push_back(newSpriteDef);
			}
		}
	}

	LastFrameSprites = FrameSprites;
}
// Draw over main emulator screen
void FVICAnalysis::DrawScreenOverlay(float x,float y) const
{
	const float scale = ImGui_GetScaling();
	ImDrawList* dl = ImGui::GetWindowDrawList();
	const float fontSize = ImGui::GetFontSize();

	const float txtOffX = ((24*scale) / 2) - (fontSize / 2);
	const float txtOffY = ((21*scale) / 2) - (fontSize / 2);


	for (const FSpriteInfo& spriteInfo : LastFrameSprites)
	{
		ImVec2 spritePos(x + ((spriteInfo.XPosition+6) * scale), y + ((spriteInfo.YPosition - 15) * scale));

		dl->AddRect(spritePos,ImVec2(spritePos.x + (24 * scale),spritePos.y + (21 * scale)),0xffffffff);
		char valTxt[8];
		snprintf(valTxt, 8, "%d", spriteInfo.SpriteNo);

		dl->AddText(ImVec2(spritePos.x + txtOffX, spritePos.y + txtOffY), 0xffffffff, valTxt);
	}
}



int	 FVICAnalysis::GetFrameSprite(int scanline, int spriteNo)
{
	for (int i = (int)FrameSprites.size()-1; i >= 0; i--)	// iterate backwards because we put new ones on the end
	{
		if(FrameSprites[i].SpriteNo == spriteNo)
			return i;
	}

	return -1;
}

EC64Event FVICAnalysis::GetVICEvent(uint8_t reg, uint8_t val, FAddressRef pc)
{

	return EC64Event::VICRegisterWrite;
}

void FVICAnalysis::OnRegisterRead(uint8_t reg, FAddressRef pc)
{
    
}

void FVICAnalysis::OnRegisterWrite(uint8_t reg, uint8_t val, FAddressRef pc)
{
	c64_t* pC64 = pC64Emu->GetEmu();
	const int scanline = pC64->vic.rs.v_count;

	FC64IORegisterInfo& vicRegister = VICRegisters[reg];
	const uint8_t regChange = vicRegister.LastVal ^ val;	// which bits have changed

    // BMP <-> Char mode
    if(reg == (uint8_t)EVicRegister::ScreenControl1 && regChange & (1<<5))
    {
        if( val & (1<<5))
            pCodeAnalyser->Debugger.RegisterEvent((uint8_t)EC64Event::VICScreenModeBmp,pc,reg,val, scanline);
        else
            pCodeAnalyser->Debugger.RegisterEvent((uint8_t)EC64Event::VICScreenModeChar,pc,reg,val, scanline);
    }
    
    // Multicolour On/Off
    if(reg == (uint8_t)EVicRegister::ScreenControl2 && regChange & (1<<4))
    {
        if( val & (1<<4))
            pCodeAnalyser->Debugger.RegisterEvent((uint8_t)EC64Event::VICScreenModeMultiColour,pc,reg,val, scanline);
        else
            pCodeAnalyser->Debugger.RegisterEvent((uint8_t)EC64Event::VICScreenModeHiRes,pc,reg,val, scanline);
    }

	if(reg == (uint8_t)EVicRegister::MemorySetup)
	{
		if((regChange >> 1) & 7)	// character address changed
		{
			const uint16_t vicMemBase = pC64->vic_bank_select;
			const uint16_t charAddr = (((val >> 1) & 7) << 11);
			FAddressRef charAddress = pC64Emu->GetVICMemoryAddress(charAddr);
			bool bFound = false;
			for(auto charAddrRef : CharSets)
			{
				if (charAddress == charAddrRef.Address)
				{
					bFound = true;
					break;
				}
			}

			if(bFound == false)
			{
				uint32_t charCols[4];
				charCols[0] = m6569_color(pC64->vic.reg.bc[0]);
				charCols[1] = m6569_color(pC64->vic.reg.bc[1]);
				charCols[2] = m6569_color(pC64->vic.reg.bc[2]);
				charCols[3] = 0;//m6569_color(pC64->vic.reg.bc[3]);

				FCharSetDef newCharSet;
				newCharSet.Address = charAddress;
				newCharSet.bMultiColour = pC64->vic.reg.ctrl_1 & (1 << 4);
				newCharSet.PaletteNo = GetPaletteNo(charCols, newCharSet.bMultiColour ? 4 : 2);

				CharSets.push_back(newCharSet);
			}
		}
	}
    
	// Events
	pCodeAnalyser->Debugger.RegisterEvent((uint8_t)GetVICEvent(reg,val,pc),pc,reg,val, scanline);
	vicRegister.Accesses[pc].WriteVals.insert(val);

	vicRegister.LastVal = val;

	// check for sprite register updates
	if (reg <= (int)EVicRegister::Sprite7_Y)	// set sprite coordinate
	{
		const int spriteNo = reg >> 1;
		int frameSpriteIndex = GetFrameSprite(scanline,spriteNo);
		bool bNewSprite = frameSpriteIndex == -1;

		if (bNewSprite == false)
		{
			FSpriteInfo& sprite = FrameSprites[frameSpriteIndex];
			if(sprite.XPosition != -1 && sprite.YPosition != -1 && (sprite.XPosition & 0xff) != 0)	// both coords have been set - multiplexed
				bNewSprite = true;
		}

		if(bNewSprite)
		{
			frameSpriteIndex = (int)FrameSprites.size();
			FSpriteInfo newSprite;
			newSprite.SpriteNo = spriteNo;
			FrameSprites.push_back(newSprite);
		}

		FSpriteInfo& sprite = FrameSprites[frameSpriteIndex];
		if(sprite.ScanlineNo == -1)
			sprite.ScanlineNo = scanline;
		if(sprite.CodeAddress.IsValid() == false)
			sprite.CodeAddress = pc;

		const bool bSetXPos = ((reg & 1) == 0);

		if(bSetXPos)
		{ 
			if(sprite.XPosition == -1)
				sprite.XPosition = val;
			else
				sprite.XPosition = (sprite.XPosition & ~0xff) + val;
		}
		else
		{
			sprite.YPosition = val;
		}

	}

	if (reg == (int)EVicRegister::SpritesXMSB)
	{
		for (int i = 0; i < 8; i++)
		{
			if (val & (1 << i))
			{
				int frameSpriteIndex = GetFrameSprite(scanline, i);
				if (frameSpriteIndex != -1)
				{
					FSpriteInfo& sprite = FrameSprites[frameSpriteIndex];
					if (sprite.XPosition == -1)
						sprite.XPosition = 1 << 8;
					else
						sprite.XPosition |= 1 << 8;
				}

			}
		}
	}
}

#include <imgui.h>
#include <CodeAnalyser/UI/CodeAnalyserUI.h>
#include <chips/m6569.h>
#include <vector>
#include <CodeAnalyser/CodeAnalysisPage.h>

void DrawRegValueSpriteEnable(FC64IODevice* pDevice, uint8_t val)
{
	for (int i = 0; i < 8; i++)
	{
		ImGui::Text("%d:%s ", i, ((val >> i) & 1) ? "Y" : "N");
		if (i < 7)
			ImGui::SameLine();
	}
}

void DrawRegValueColour(FC64IODevice* pDevice, uint8_t val)
{
	ImVec4 c;
	const ImVec2 size(18, 18);
	c = ImColor(m6569_color(val & 15));
	ImGui::ColorButton("##hw_color", c, ImGuiColorEditFlags_NoAlpha, size);
}

void DrawRegValueXPos(FC64IODevice* pDevice, uint8_t val)
{
	ImGui::Text("%d", val);
	if (ImGui::IsItemHovered())
	{
		pDevice->GetC64()->SetXHighlight((int)val - 30);
	}
}

void DrawRegValueYPos(FC64IODevice* pDevice, uint8_t val)
{
	ImGui::Text("%d", val);
	if (ImGui::IsItemHovered())
	{
		pDevice->GetC64()->SetYHighlight((int)val - 50);
	}
}

void DrawRegValueScanline(FC64IODevice* pDevice, uint8_t val)
{
	ImGui::Text("%d", val);
	if (ImGui::IsItemHovered())
		pDevice->GetC64()->SetScanlineHighlight(val);
}


/*
Screen control register #1. Bits:

Bits #0-#2: Vertical raster scroll.

Bit #3: Screen height; 0 = 24 rows; 1 = 25 rows.

Bit #4: 0 = Screen off, complete screen is covered by border; 1 = Screen on, normal screen contents are visible.

Bit #5: 0 = Text mode; 1 = Bitmap mode.

Bit #6: 1 = Extended background mode on.

Bit #7: Read: Current raster line (bit #8).
Write: Raster line to generate interrupt at (bit #8).
*/

void DrawRegValueScreenControlReg1(FC64IODevice* pDevice, uint8_t val)
{
	ImGui::Text("$%X",val);
	if(ImGui::IsItemHovered())
	{
		ImGui::BeginTooltip();
		ImGui::Text("VScroll:%d, Height:%d, Scr:%s, %s, ExtBG:%s, RastMSB:%d",
			val & 7,
			val & (1 << 3) ? 25 : 24,
			val & (1 << 4) ? "ON" : "OFF",
			val & (1 << 5) ? "BMP" : "TXT",
			val & (1 << 6) ? "ON" : "OFF",
			val & (1 << 7) ? 1 : 0);
		ImGui::EndTooltip();
	}
}

/*
Screen control register #2. Bits:

Bits #0-#2: Horizontal raster scroll.

Bit #3: Screen width; 0 = 38 columns; 1 = 40 columns.

Bit #4: 1 = Multicolor mode on.
*/

void DrawRegValueScreenControlReg2(FC64IODevice* pDevice, uint8_t val)
{
	ImGui::Text("$%X", val);
	if (ImGui::IsItemHovered())
	{
		ImGui::BeginTooltip();
		ImGui::Text("HScroll:%d, Width:%d, MultiColour:%s",
			val & 7,
			val & (1 << 3) ? 40 : 38,
			val & (1 << 4) ? "ON" : "OFF");
		ImGui::EndTooltip();
	}

}

void DrawRegValueMemorySetup(FC64IODevice* pDevice, uint8_t val)
{
	ImGui::Text("$%X", val);
	if (ImGui::IsItemHovered())
	{
		const uint16_t vicMemBase = pDevice->GetC64()->GetEmu()->vic_bank_select;

		ImGui::BeginTooltip();
		ImGui::Text("Char Addr: $%04X, Bitmap Address: $%04X, Screen Address: $%04X",
			vicMemBase + (((val >> 1) & 7) << 11),
			vicMemBase + (((val >> 3) & 1) << 13),
			vicMemBase + ((val >> 4) << 10));
		ImGui::EndTooltip();
	}
}

static std::vector<FRegDisplayConfig>	g_VICRegDrawInfo = 
{
	{"Sprite 0 X",	DrawRegValueXPos},	// 0x00
	{"Sprite 0 Y",	DrawRegValueYPos}, 	// 0x01
	{"Sprite 1 X",	DrawRegValueXPos}, 	// 0x02
	{"Sprite 1 Y",	DrawRegValueYPos}, 	// 0x03
	{"Sprite 2 X",	DrawRegValueXPos}, 	// 0x04
	{"Sprite 2 Y",	DrawRegValueYPos}, 	// 0x05
	{"Sprite 3 X",	DrawRegValueXPos}, 	// 0x06
	{"Sprite 3 Y",	DrawRegValueYPos}, 	// 0x07
	{"Sprite 4 X",	DrawRegValueXPos}, 	// 0x08
	{"Sprite 4 Y",	DrawRegValueYPos}, 	// 0x09
	{"Sprite 5 X",	DrawRegValueXPos}, 	// 0x0a
	{"Sprite 5 Y",	DrawRegValueYPos}, 	// 0x0b
	{"Sprite 6 X",	DrawRegValueXPos}, 	// 0x0c
	{"Sprite 6 Y",	DrawRegValueYPos}, 	// 0x0d
	{"Sprite 7 X",	DrawRegValueXPos}, 	// 0x0e
	{"Sprite 7 Y",	DrawRegValueYPos}, 	// 0x0f
	{"Sprite X MSB",	DrawRegValueHex, EDataItemDisplayType::Binary},	// 0x10
	{"Screen Ctrl 1",	DrawRegValueScreenControlReg1, EDataItemDisplayType::Hex},	// 0x11
	{"Raster Line",	DrawRegValueScanline},	// 0x12
	{"Light Pen X",	DrawRegValueDecimal},	// 0x13
	{"Light Pen Y",	DrawRegValueDecimal},	// 0x14
	{"Sprite Enable",	DrawRegValueSpriteEnable, EDataItemDisplayType::Binary},	// 0x15
	{"Screen Ctrl 2",		DrawRegValueScreenControlReg2},	// 0x16
	{"Sprite Dbl Height",	DrawRegValueSpriteEnable, EDataItemDisplayType::Binary},	// 0x17
	{"Memory Setup",		DrawRegValueMemorySetup, EDataItemDisplayType::Hex},	// 0x18
	{"Interrupt Status",		DrawRegValueHex, EDataItemDisplayType::Hex},	// 0x19
	{"Interrupt Control",	DrawRegValueHex},// 0x1a
	{"Sprite Priority",		DrawRegValueHex, EDataItemDisplayType::Binary},// 0x1b
	{"Sprite Multi Col",		DrawRegValueSpriteEnable, EDataItemDisplayType::Binary},// 0x1c
	{"Sprite Dbl Width",		DrawRegValueSpriteEnable, EDataItemDisplayType::Binary},// 0x1d
	{"Sprite-SpriteCol",		DrawRegValueSpriteEnable, EDataItemDisplayType::Binary},// 0x1e
	{"Sprite-BackCol",	DrawRegValueSpriteEnable, EDataItemDisplayType::Binary},	// 0x1f
	{"Border Colour",		DrawRegValueColour},// 0x20
	{"Background Colour",		DrawRegValueColour},// 0x21
	{"Extra BackColour 1",		DrawRegValueColour},// 0x22
	{"Extra BackColour 2",		DrawRegValueColour},// 0x23
	{"Extra BackColour 3",		DrawRegValueColour},// 0x24
	{"Sprite Extra Colour 1",		DrawRegValueColour},// 0x25
	{"Sprite Extra Colour 2",		DrawRegValueColour},// 0x26
	{"Sprite 0 Colour",		DrawRegValueColour},// 0x27
	{"Sprite 1 Colour",		DrawRegValueColour},// 0x28
	{"Sprite 2 Colour",		DrawRegValueColour},// 0x29
	{"Sprite 3 Colour",		DrawRegValueColour},// 0x2a
	{"Sprite 4 Colour",		DrawRegValueColour},// 0x2b
	{"Sprite 5 Colour",		DrawRegValueColour},// 0x2c
	{"Sprite 6 Colour",		DrawRegValueColour},// 0x2d
	{"Sprite 7 Colour",		DrawRegValueColour}// 0x2e
};

void VICWriteEventShowAddress(FCodeAnalysisState& state, const FEvent& event)
{
	ImGui::Text("%s", g_VICRegDrawInfo[event.Address].Name);
}

void VICWriteEventShowValue(FCodeAnalysisState& state, const FEvent& event)
{
	g_VICRegDrawInfo[event.Address].UIDrawFunction(pVIC,event.Value);
	//ImGui::Text("VIC Value: %s", NumStr(event.Value));
}

void FVICAnalysis::DrawLastFrameSpriteInfo(void)
{
	for (const auto& frameSprite : LastFrameSprites)
	{
		ImGui::Text("Sprite No %d", frameSprite.SpriteNo);
		ImGui::SameLine();
		ImGui::Text("Pos: %d,%d", frameSprite.XPosition, frameSprite.YPosition);

		if (frameSprite.CodeAddress.IsValid())
		{
			ImGui::SameLine();
			ImGui::Text("Set by:");	ImGui::SameLine();
			DrawCodeAddress(*pCodeAnalyser, pCodeAnalyser->GetFocussedViewState(), frameSprite.CodeAddress);
		}
	}
}

void FVICAnalysis::DrawDetailsUI(void)
{
	DrawVICRegisterInfo();
}

void FVICAnalysis::DrawVICRegisterInfo(void)
{
	if (ImGui::BeginChild("VIC Reg Select", ImVec2(ImGui::GetContentRegionAvail().x * 0.5f, 0), true))
	{
		for (int i = 0; i < (int)g_VICRegDrawInfo.size(); i++)
		{
			char selectableTXT[32];
			snprintf(selectableTXT, sizeof(selectableTXT), "$%X %s", i, g_VICRegDrawInfo[i].Name);
			if (ImGui::Selectable(selectableTXT, SelectedRegister == i))
			//if (ImGui::Selectable(g_VICRegDrawInfo[i].Name, SelectedRegister == i))
			{
				SelectedRegister = i;
			}
		}
	}
	ImGui::EndChild();
	ImGui::SameLine();
	if (ImGui::BeginChild("VIC Reg Details"))
	{
		if (SelectedRegister != -1)
			DrawRegDetails(this, VICRegisters[SelectedRegister], g_VICRegDrawInfo[SelectedRegister], pCodeAnalyser);
#if 0
		{
			FC64IORegisterInfo& vicRegister = VICRegisters[SelectedRegister];
			const FRegDisplayConfig& regConfig = g_VICRegDrawInfo[SelectedRegister];


			if (ImGui::Button("Clear"))
			{
				vicRegister.LastVal = 0;
				vicRegister.Accesses.clear();
			}
			// move out into function?
			ImGui::Text("Last Val:");
			regConfig.UIDrawFunction(this, vicRegister.LastVal);
			ImGui::Text("Accesses:");
			for (auto& access : vicRegister.Accesses)
			{
				ImGui::Separator();

				ShowCodeAccessorActivity(*pCodeAnalyser, access.first);
				ImGui::Text("   ");
				ImGui::SameLine(); 
				DrawCodeAddress(*pCodeAnalyser, pCodeAnalyser->GetFocussedViewState(), access.first);

				if(ImGui::CollapsingHeader("Values"))
				{
					for (auto& val : access.second.WriteVals)
						regConfig.UIDrawFunction(this, val);
				}
			}
		}
#endif
	}
	ImGui::EndChild();
}

std::string GetVICLabelName(int reg)
{
	std::string displayName = g_VICRegDrawInfo[reg].Name;
	std::string labelName = "VIC_";

	for (auto ch : displayName)
	{
		if(ch != ' ')
			labelName += ch;
	}

	return labelName;
}

void AddVICRegisterLabels(FCodeAnalysisPage& IOPage)
{
	for(int reg=0;reg< (int)g_VICRegDrawInfo.size();reg++)
	{
		IOPage.SetLabelAtAddress(GetVICLabelName(reg).c_str(), ELabelType::Data, reg, true);
		IOPage.DataInfo[reg].DisplayType = g_VICRegDrawInfo[reg].DisplayType;
	}
}
