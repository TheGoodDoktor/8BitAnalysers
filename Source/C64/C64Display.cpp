#include "C64Display.h"

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

#include <imgui.h>
#include <ImGuiSupport/ImGuiTexture.h>
#include <CodeAnalyser/CodeAnalyser.h>
#include <CodeAnalyser/UI/CodeAnalyserUI.h>
#include <algorithm>

#include "C64Emulator.h"

void FC64Display::Init(FCodeAnalysisState* pAnalysis, FC64Emulator* pC64Emu)
{
	C64Emu = pC64Emu;
	CodeAnalysis = pAnalysis;


	// setup texture
	chips_display_info_t dispInfo = c64_display_info(C64Emu->GetEmu());


	// setup pixel buffer
	const size_t pixelBufferSize = dispInfo.frame.dim.width * dispInfo.frame.dim.height;
	FrameBuffer = new uint32_t[pixelBufferSize * 2];
	ScreenTexture = ImGui_CreateTextureRGBA(FrameBuffer, dispInfo.frame.dim.width, dispInfo.frame.dim.height);


    // setup pixel buffer
    // FIXME: seems uncommitted changes
    /*
    FramePixelBufferSize = c64_max_display_size();
    FramePixelBuffer = new unsigned char[FramePixelBufferSize * 2];

    // setup texture
    FrameBufferTexture = ImGui_CreateTextureRGBA(static_cast<unsigned char*>(FramePixelBuffer), c64_std_display_width(), c64_std_display_height());
    //DebugFrameBufferTexture = ImGui_ImplDX11_CreateTextureRGBA(static_cast<unsigned char*>(FramePixelBuffer), _C64_DBG_DISPLAY_WIDTH, _C64_DBG_DISPLAY_HEIGHT);
    */

}

uint16_t FC64Display::GetScreenBitmapAddress(int pixelX, int pixelY)
{
    c64_t* pC64 = C64Emu->GetEmu();
    const int charX = pixelX >> 3;
    const int charY = pixelY >> 3;

    uint16_t vicMemBase = pC64->vic_bank_select;
    //uint16_t screenMem = (pC64->vic.reg.mem_ptrs >> 4) << 10;
    uint16_t bitmapMem = ((pC64->vic.reg.mem_ptrs >> 3) & 1) << 13;

    const uint16_t pixLocation = (charY * 320) + (charX * 8) + (pixelY & 7);
    return vicMemBase + bitmapMem + pixLocation;
}

uint16_t FC64Display::GetScreenCharAddress(int pixelX, int pixelY)
{
    c64_t* pC64 = C64Emu->GetEmu();
    const int charX = pixelX >> 3;
    const int charY = pixelY >> 3;

    uint16_t vicMemBase = pC64->vic_bank_select;
    uint16_t screenMem = ((pC64->vic.reg.mem_ptrs >> 4) & 7) << 10;

    const uint16_t charLocation = (charY * 40) + charX;
    return vicMemBase + screenMem + charLocation;
}

uint16_t FC64Display::GetColourRAMAddress(int pixelX, int pixelY)
{
    const int charX = pixelX >> 3;
    const int charY = pixelY >> 3;
    return 0xD800 + (charY * 40) + charX;
}

void FC64Display::DrawUI()
{
    c64_t* pC64 = C64Emu->GetEmu();
    FCodeAnalysisViewState& viewState = CodeAnalysis->GetFocussedViewState();

    const bool bDebugFrame = pC64->vic.debug_vis;
    //if (bDebugFrame)
    //else
      //  ImGui_ImplDX11_UpdateTextureRGBA(FrameBufferTexture, FramePixelBuffer, _C64_STD_DISPLAY_WIDTH, _C64_STD_DISPLAY_HEIGHT);
    const int xScrollOff = pC64->vic.reg.ctrl_2 & 7;
    const int yScrollOff = pC64->vic.reg.ctrl_1 & 7;
    const int screenWidthChars = 40;// pC64->vic.reg.ctrl_2& (1 << 3) ? 40 : 38;
    const int screenHeightChars = 25;// pC64->vic.reg.ctrl_1& (1 << 3) ? 25 : 24;
    const int graphicsScreenWidth = screenWidthChars * 8;
    const int graphicsScreenHeight = screenHeightChars * 8;

	chips_display_info_t disp = c64_display_info(C64Emu->GetEmu());
	const int dispFrameWidth = disp.screen.width;
    const int dispFrameHeight = disp.screen.height;

	// convert texture to RGBA
	const uint8_t* pix = (const uint8_t*)disp.frame.buffer.ptr;
	const uint32_t* pal = (const uint32_t*)disp.palette.ptr;
	for (int i = 0; i < disp.frame.buffer.size; i++)
		FrameBuffer[i] = pal[pix[i]];

	ImGui_UpdateTextureRGBA(ScreenTexture, FrameBuffer);


    ImGui::Text("Frame buffer size = %d x %d", dispFrameWidth, dispFrameHeight);

    const bool bBitmapMode = !!(pC64->vic.reg.ctrl_1 & (1 << 5));
    const bool bExtendedBackgroundMode = !!(pC64->vic.reg.ctrl_1 & (1 << 6));
    const bool bMultiColourMode = !!(pC64->vic.reg.ctrl_2 & (1<<4));
    uint16_t vicMemBase = pC64->vic_bank_select;
    uint16_t screenMem = (pC64->vic.reg.mem_ptrs >> 4) << 10;
    ImGui::Text("VIC Bank: $%04X, Screen Mem: $%04X", vicMemBase, vicMemBase + screenMem);
    if (bBitmapMode)
    {
        uint16_t bitmapMem = ((pC64->vic.reg.mem_ptrs >> 2) & 1) << 13;
        ImGui::SameLine();
        ImGui::Text(", Bitmap Address: $%04X", vicMemBase + bitmapMem);
    }
    else
    {
        uint16_t charDefs = ((pC64->vic.reg.mem_ptrs >> 1) & 7) << 11;
        ImGui::SameLine();
        ImGui::Text(", Character Defs: $%04X", vicMemBase + charDefs);
    }
    ImGui::Text("%s Mode, Multi Colour %s, ECBM:%s", bBitmapMode ? "Bitmap" : "Character", bMultiColourMode ? "Yes" : "No", bExtendedBackgroundMode ? "Yes" : "No");

    ImGuiIO& io = ImGui::GetIO();
    ImVec2 pos = ImGui::GetCursorScreenPos();
    ImDrawList* dl = ImGui::GetWindowDrawList();
    ImVec2 uv0(0, 0);
    ImVec2 uv1((float)disp.screen.width / (float)disp.frame.dim.width, (float)disp.screen.height / (float)disp.frame.dim.height);


    ImGui::Image(ScreenTexture, ImVec2((float)disp.screen.width, (float)disp.screen.height),uv0,uv1);

    if (ImGui::IsItemHovered())
    {
        const int borderOffsetX = ((dispFrameWidth - graphicsScreenWidth) / 2) & ~7;    // align to character size
        const int borderOffsetY = ((dispFrameHeight - graphicsScreenHeight) / 2);

        const int xp = std::min(std::max((int)(io.MousePos.x - pos.x - borderOffsetX), 0), graphicsScreenWidth - 1);
        const int yp = std::min(std::max((int)(io.MousePos.y - pos.y - borderOffsetY), 0), graphicsScreenHeight - 1);
        
        dl->AddRect(
            ImVec2((float)pos.x + borderOffsetX, (float)pos.y + borderOffsetY), 
            ImVec2((float)pos.x + borderOffsetX + graphicsScreenWidth, (float)pos.y + borderOffsetY + graphicsScreenHeight), 
            0xffffffff);

        dl->AddRect(
            ImVec2((float)pos.x, (float)pos.y),
            ImVec2((float)pos.x + dispFrameWidth, (float)pos.y + dispFrameHeight),
            0xffffffff);

        const uint16_t scrBitmapAddress = GetScreenBitmapAddress(xp, yp);
        const uint16_t scrCharAddress = GetScreenCharAddress(xp, yp);
        const uint16_t scrColourRamAddress = GetColourRAMAddress(xp, yp);

        if (scrCharAddress != 0)
        {
            const int rx = static_cast<int>(pos.x) + borderOffsetX + (xp & ~0x7);
            const int ry = static_cast<int>(pos.y) + borderOffsetY + (yp & ~0x7);
            dl->AddRect(ImVec2((float)rx, (float)ry), ImVec2((float)rx + 8, (float)ry + 8), 0xffffffff);
            ImGui::BeginTooltip();
            ImGui::Text("Screen Pos (%d,%d)", xp, yp);
            if(bBitmapMode)
                ImGui::Text("Pixel: $%04X, Colour Char: $%04X Colour Ram: $%04X", scrBitmapAddress, scrCharAddress, scrColourRamAddress);
            else
                ImGui::Text("Char: $%04X Colour Ram: $%04X", scrCharAddress, scrColourRamAddress);

            const FAddressRef lastBitmapWriter = CodeAnalysis->GetLastWriterForAddress(scrBitmapAddress);
            const FAddressRef lastCharWriter = CodeAnalysis->GetLastWriterForAddress(scrCharAddress);
            const FAddressRef lastColourRamWriter = CodeAnalysis->GetLastWriterForAddress(scrColourRamAddress);

            if (bBitmapMode)
            {
                ImGui::Text("Bitmap Writer: ");
                ImGui::SameLine();
                DrawCodeAddress(*CodeAnalysis, viewState, lastBitmapWriter);
            }
            ImGui::Text("Char Writer: ");
            ImGui::SameLine();
            if(lastCharWriter.IsValid())
                DrawCodeAddress(*CodeAnalysis, viewState, lastCharWriter);
            ImGui::Text("Colour RAM Writer: ");
            ImGui::SameLine();
            if (lastColourRamWriter.IsValid())
                DrawCodeAddress(*CodeAnalysis, viewState, lastColourRamWriter);
            ImGui::EndTooltip();
            //ImGui::Text("Pixel Writer: %04X, Attrib Writer: %04X", lastPixWriter, lastAttrWriter);

            if (ImGui::IsMouseClicked(0))
            {
                bScreenCharSelected = true;
                SelectedCharX = rx;
                SelectedCharY = ry;
                SelectBitmapAddr = scrBitmapAddress;
                SelectCharAddr = scrCharAddress;
                SelectColourRamAddr = scrColourRamAddress;
            }

            if (ImGui::IsMouseClicked(1))
                bScreenCharSelected = false;

            // FIXME: seems uncommitted changes
            /*
            if (ImGui::IsMouseDoubleClicked(0))
            {
                if(bBitmapMode)
                    CodeAnalyserGoToAddress(viewState, lastBitmapWriter);
                else
                    CodeAnalyserGoToAddress(viewState, lastCharWriter);
            }
            if (ImGui::IsMouseDoubleClicked(1))
                CodeAnalyserGoToAddress(viewState, lastColourRamWriter);
            */
        }

    }

    if (bScreenCharSelected == true)
    {
        ImDrawList* dl = ImGui::GetWindowDrawList();
        const ImU32 col = 0xffffffff;	// TODO: pulse
        dl->AddRect(ImVec2((float)SelectedCharX, (float)SelectedCharY), ImVec2((float)SelectedCharX + 8, (float)SelectedCharY + 8), col);

        if (bBitmapMode)
        {
            const FAddressRef lastBitmapWriter = CodeAnalysis->GetLastWriterForAddress(SelectBitmapAddr);
            ImGui::Text("Bitmap Address: $%X, Last Writer:", SelectBitmapAddr);
            DrawAddressLabel(*CodeAnalysis, viewState, lastBitmapWriter);
        }
        const FAddressRef lastCharWriter = CodeAnalysis->GetLastWriterForAddress(SelectCharAddr);
        ImGui::Text("Char Address: $%X, Last Writer:", SelectCharAddr);
        DrawAddressLabel(*CodeAnalysis, viewState, lastCharWriter);

        const FAddressRef lastColourRamWriter = CodeAnalysis->GetLastWriterForAddress(SelectColourRamAddr);
        ImGui::Text("Colour RAM Address: $%X, Last Writer:", SelectColourRamAddr);
        DrawAddressLabel(*CodeAnalysis, viewState, lastColourRamWriter);
    }
}

