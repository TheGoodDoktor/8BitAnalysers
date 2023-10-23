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
#include "C64Config.h"
#include <ImGuiSupport/ImGuiScaling.h>
#include <ImGuiSupport/ImGuiDrawing.h>

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
    const FC64Config &config = *C64Emu->GetGlobalConfig();
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
    uint16_t spritePtrs = vicMemBase + screenMem + 1016;

    uint8_t sprite1 = mem_rd(&pC64->mem_vic,spritePtrs);
    // sprite address is sprite1 * 64

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
    const float scale = ImGui_GetScaling();
    ImDrawList* dl = ImGui::GetWindowDrawList();
    ImVec2 uv0(0, 0);
    ImVec2 uv1((float)disp.screen.width / (float)disp.frame.dim.width, (float)disp.screen.height / (float)disp.frame.dim.height);

    // Draw Screen
    ImGui::Image(ScreenTexture, ImVec2((float)disp.screen.width * scale, (float)disp.screen.height * scale),uv0,uv1);

    // Draw an indicator to show which scanline is being drawn
    if (config.bShowScanLineIndicator && CodeAnalysis->Debugger.IsStopped())
    {
        int topScreenScanLine = 0;
        int scanlineX = pC64->vic.rs.h_count * M6569_PIXELS_PER_TICK;  
        int scanlineY = std::min(std::max(pC64->vic.rs.v_count - topScreenScanLine, 0), disp.screen.height);
        int interruptScanline = pC64->vic.rs.v_irqline;
        dl->AddLine(ImVec2(pos.x + (4 * scale), pos.y + (scanlineY * scale)), ImVec2(pos.x + (disp.screen.width - 8) * scale, pos.y + (scanlineY * scale)), 0x50ffffff);
        DrawArrow(dl, ImVec2(pos.x - 2, pos.y + (scanlineY * scale) - 6), false);
        DrawArrow(dl, ImVec2(pos.x + (disp.screen.width - 11) * scale, pos.y + (scanlineY * scale) - 6), true);

        // interrupt scanline
        dl->AddLine(ImVec2(pos.x + (4 * scale), pos.y + (interruptScanline * scale)), ImVec2(pos.x + (disp.screen.width - 8) * scale, pos.y + (interruptScanline * scale)), 0x50ffffff);

        if(config.bShowHCounter)
            dl->AddLine(ImVec2(pos.x + (scanlineX * scale), pos.y), ImVec2(pos.x + (scanlineX * scale), pos.y + disp.screen.height * scale), 0x50ffffff);
    }

    if (ImGui::IsItemHovered())
    {
        //pC64->vic.crt.vis_x0 * M6569_PIXELS_PER_TICK;
        //pC64->vic.brd.left* M6569_PIXELS_PER_TICK;

        const int borderOffsetX = ((dispFrameWidth - graphicsScreenWidth) / 2);    // align to character size
        const int borderOffsetY = ((dispFrameHeight - graphicsScreenHeight) / 2);

        const ImVec2 mouseOffset((io.MousePos.x - pos.x)/scale, (io.MousePos.y - pos.y) / scale);
        const int xp = std::min(std::max((int)(mouseOffset.x - borderOffsetX), 0), graphicsScreenWidth - 1);
        const int yp = std::min(std::max((int)(mouseOffset.y - borderOffsetY), 0), graphicsScreenHeight - 1);
        
        dl->AddRect(
            ImVec2((float)pos.x + borderOffsetX * scale, (float)pos.y + borderOffsetY * scale), 
            ImVec2((float)pos.x + borderOffsetX + graphicsScreenWidth * scale, (float)pos.y + (borderOffsetY + graphicsScreenHeight) * scale), 
            0xffffffff);

        dl->AddRect(
            ImVec2((float)pos.x, (float)pos.y),
            ImVec2((float)pos.x + dispFrameWidth * scale, (float)pos.y + dispFrameHeight * scale),
            0xffffffff);

        const uint16_t scrBitmapAddress = GetScreenBitmapAddress(xp, yp);
        const uint16_t scrCharAddress = GetScreenCharAddress(xp, yp);
        const uint16_t scrColourRamAddress = GetColourRAMAddress(xp, yp);

        if (scrCharAddress != 0)
        {
            const float rx = static_cast<int>(pos.x) + (borderOffsetX + (xp & ~0x7)) * scale;
            const float ry = static_cast<int>(pos.y) + (borderOffsetY + (yp & ~0x7)) * scale;
            dl->AddRect(ImVec2(rx, ry), ImVec2(rx + 8 * scale, ry + 8 * scale), 0xffffffff);
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
                if(lastBitmapWriter.IsValid())
                {
                    ImGui::SameLine();
                    DrawCodeAddress(*CodeAnalysis, viewState, lastBitmapWriter);
                }
            }
            ImGui::Text("Char Writer: ");
            ImGui::SameLine();
            if(lastCharWriter.IsValid())
            { 
                ImGui::SameLine();
                DrawCodeAddress(*CodeAnalysis, viewState, lastCharWriter);
            }
            ImGui::Text("Colour RAM Writer: ");
            if (lastColourRamWriter.IsValid())
            {
                ImGui::SameLine();
                DrawCodeAddress(*CodeAnalysis, viewState, lastColourRamWriter);
            }
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
        dl->AddRect(ImVec2(SelectedCharX, SelectedCharY), ImVec2(SelectedCharX + 8 * scale, SelectedCharY + 8 * scale), col);

        if (bBitmapMode)
        {
            const FAddressRef lastBitmapWriter = CodeAnalysis->GetLastWriterForAddress(SelectBitmapAddr);
            ImGui::Text("Bitmap Address: $%X, Last Writer:", SelectBitmapAddr);
            if (lastBitmapWriter.IsValid())
                DrawAddressLabel(*CodeAnalysis, viewState, lastBitmapWriter);
        }
        const FAddressRef lastCharWriter = CodeAnalysis->GetLastWriterForAddress(SelectCharAddr);
        ImGui::Text("Char Address: $%X, Last Writer:", SelectCharAddr);
        if (lastCharWriter.IsValid())
            (*CodeAnalysis, viewState, lastCharWriter);

        const FAddressRef lastColourRamWriter = CodeAnalysis->GetLastWriterForAddress(SelectColourRamAddr);
        ImGui::Text("Colour RAM Address: $%X, Last Writer:", SelectColourRamAddr);
        if(lastColourRamWriter.IsValid())
            DrawAddressLabel(*CodeAnalysis, viewState, lastColourRamWriter);
    }

	bWindowFocused = ImGui::IsWindowFocused();

}


int C64KeyFromImGuiKey(ImGuiKey key)
{
    const bool bShift = ImGui::IsKeyDown(ImGuiKey_LeftShift) || ImGui::IsKeyDown(ImGuiKey_RightShift);

    switch (key)
    {
        case ImGuiKey_Space:
            return C64_KEY_SPACE;
		case ImGuiKey_Enter:
			return C64_KEY_RETURN;
		case ImGuiKey_LeftCtrl:
			return C64_KEY_CTRL;
		case ImGuiKey_Delete:
			return C64_KEY_DEL;
        case ImGuiKey_LeftArrow:
            return C64_KEY_CSRLEFT;
        case ImGuiKey_RightArrow:
            return C64_KEY_CSRRIGHT;
        case ImGuiKey_UpArrow:
            return C64_KEY_CSRUP;
        case ImGuiKey_DownArrow:
            return C64_KEY_CSRDOWN;

        // Alphanumeric range
        default:
        {
			if (key >= ImGuiKey_0 && key <= ImGuiKey_9)
			{
				return '0' + (key - ImGuiKey_0);
			}
			else if (key >= ImGuiKey_A && key <= ImGuiKey_Z)
			{
                char c = (bShift ? 'A' : 'a') + (key - ImGuiKey_A);

                if (isupper(c))
                    c = tolower(c);
                else if (islower(c)) 
                    c = toupper(c);
                    
                return c;
			}
            else if (key >= ImGuiKey_F1 && key <= ImGuiKey_F8)
            {
                return C64_KEY_F1 + (key - ImGuiKey_F1);
            }
        }
        break;
    }

    return 0;
}

uint8_t GetJoystickMaskFromImGui()
{
    uint8_t mask = 0;
	if (ImGui::IsKeyDown(ImGuiNavInput_DpadRight))
        mask |= C64_JOYSTICK_RIGHT;
	if (ImGui::IsKeyDown(ImGuiNavInput_DpadLeft))
        mask |= C64_JOYSTICK_LEFT;
	if (ImGui::IsKeyDown(ImGuiNavInput_DpadDown))
        mask |= C64_JOYSTICK_DOWN;
	if (ImGui::IsKeyDown(ImGuiNavInput_DpadUp))
        mask |= C64_JOYSTICK_UP;
	if (ImGui::IsKeyDown(ImGuiNavInput_Activate))
        mask |= C64_JOYSTICK_BTN;

    return mask;
}

void FC64Display::Tick()
{
	// Check keys - not event driven, hopefully perf isn't too bad
	for (ImGuiKey key = ImGuiKey_NamedKey_BEGIN; key < ImGuiKey_COUNT; key++)
	{
		if (ImGui::IsKeyPressed(key, false))
		{
			const int c64Key = C64KeyFromImGuiKey(key);
			if (c64Key != 0 && bWindowFocused)
				c64_key_down(C64Emu->GetEmu(), c64Key);
		}
		else if (ImGui::IsKeyReleased(key))
		{
			const int c64Key = C64KeyFromImGuiKey(key);
			if (c64Key != 0)
                c64_key_up(C64Emu->GetEmu(), c64Key);
		}
	}

	const uint8_t keyMask = GetJoystickMaskFromImGui();

	c64_joystick(C64Emu->GetEmu(), 0, keyMask);
}