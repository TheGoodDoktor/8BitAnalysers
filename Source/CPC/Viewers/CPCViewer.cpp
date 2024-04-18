#include "CPCViewer.h"

#include <CodeAnalyser/CodeAnalyser.h>

#include <imgui.h>
#include "../CPCEmu.h"
#include <CodeAnalyser/UI/CodeAnalyserUI.h>

#include "CPCGraphicsViewer.h"
#include "Debug/DebugLog.h"
#include "Util/Misc.h"
#include "Util/GraphicsView.h"

#include <ImGuiSupport/ImGuiTexture.h>
#include <algorithm>
#include <cassert>

#include <LuaScripting/LuaSys.h>

int CPCKeyFromImGuiKey(ImGuiKey key);
void DrawPalette(const FPalette& palette);
void DrawSnapLoadButtons(FCPCEmu* pCPCEmu);

template<typename T> static inline T Clamp(T v, T mn, T mx)
{ 
	return (v < mn) ? mn : (v > mx) ? mx : v; 
}

void FCPCViewer::Init(FCPCEmu* pEmu)
{
	pCPCEmu = pEmu;

	// setup texture
	const chips_display_info_t dispInfo = cpc_display_info(&pEmu->CPCEmuState);

	// setup pixel buffer
	const int w = dispInfo.frame.dim.width; // 1024
	const int h = dispInfo.frame.dim.height; // 312

	const size_t pixelBufferSize = w * h;
	FrameBuffer = new uint32_t[pixelBufferSize * 2];
	ScreenTexture = ImGui_CreateTextureRGBA(FrameBuffer, w, h);

	TextureWidth = AM40010_DISPLAY_WIDTH / 2;
	TextureHeight = AM40010_DISPLAY_HEIGHT;
}

void FCPCViewer::Draw()
{		
	FCodeAnalysisState& state = pCPCEmu->GetCodeAnalysis();
#ifdef CPCVIEWER_EXTRA_DEBUG
	DrawSnapLoadButtons(pCPCEmu);
#endif 

	CalculateScreenProperties();

#ifndef NDEBUG
	static bool bDrawScreenExtents = true;
#else
	static bool bDrawScreenExtents = false;
#endif
	ImGui::Checkbox("Draw screen extents", &bDrawScreenExtents);
	static bool bShowScreenmodeChanges = false;
	ImGui::Checkbox("Show screenmode changes", &bShowScreenmodeChanges);
#ifdef CPCVIEWER_EXTRA_DEBUG
	ImGui::Checkbox("Write to screen on click", &bClickWritesToScreen);
#endif

	const bool bHasScreen = pCPCEmu->Screen.HasBeenDrawn();
	const FGlobalConfig* pConfig = pCPCEmu->GetGlobalConfig();
	const float scale = (float)pConfig->ImageScale;

	// see if mixed screen modes are used
	int scrMode = pCPCEmu->CPCEmuState.ga.video.mode;
	if (bHasScreen)
	{
		for (int s = 0; s < AM40010_DISPLAY_HEIGHT; s++)
		{
			if (pCPCEmu->Screen.GetScreenModeForScanline(s) != scrMode)
			{
				scrMode = -1;
				break;
			}
		}
	}
	// display screen mode and resolution
	const mc6845_t& crtc = pCPCEmu->CPCEmuState.crtc;
	const int multiplier[4] = {4, 8, 16, 4};
	if(scrMode == -1)
	{
		ImGui::Text("Screen mode: mixed");
		ImGui::Text("Resolution: mixed (%d) x %d", ScreenWidth, ScreenHeight);
	}
	else
	{
		ImGui::Text("Screen mode: %d", scrMode);
		ImGui::Text("Resolution: %d x %d", crtc.h_displayed * multiplier[scrMode], ScreenHeight);
	}
	
	const uint16_t pageIndex = (crtc.start_addr_hi >> 4) & 0x3;
	const uint16_t size = (crtc.start_addr_hi >> 2) & 0x3;

	ImGui::Text("Screen RAM: %s. Page: %s. Size %s. Scrolled: %s", 
		NumStr(pCPCEmu->Screen.GetScreenAddrStart()), 
		NumStr(pageIndex),
		size == 3 ? "32k" : "16k",
		pCPCEmu->Screen.IsScrolled() ? "Yes" : "No");

	// see if palette changes occured during last frame
	int numPaletteChanges = 0;
	if (bHasScreen)
	{
		for (int p = 1; p < AM40010_DISPLAY_HEIGHT; p++)
		{
			if (pCPCEmu->Screen.GetPaletteForScanline(p - 1) != pCPCEmu->Screen.GetPaletteForScanline(p))
				numPaletteChanges++;
		}
	}
	ImGui::Text("Palette changes: %d", numPaletteChanges);

	// draw the cpc display
	chips_display_info_t disp = cpc_display_info(&pCPCEmu->CPCEmuState);

	// convert texture to RGBA
	const uint8_t* pix = (const uint8_t*)disp.frame.buffer.ptr;
	const uint32_t* pal = (const uint32_t*)disp.palette.ptr;
	for (int i = 0; i < disp.frame.buffer.size; i++)
		FrameBuffer[i] = pal[pix[i]];
	
	ImGui_UpdateTextureRGBA(ScreenTexture, FrameBuffer);

	const static float uv0w = 0.0f;
	const static float uv0h = 0.0f;
	const static float uv1w = (float)AM40010_DISPLAY_WIDTH / (float)AM40010_FRAMEBUFFER_WIDTH;
	const static float uv1h = (float)AM40010_DISPLAY_HEIGHT / (float)AM40010_FRAMEBUFFER_HEIGHT;

	const ImVec2 pos = ImGui::GetCursorScreenPos(); // get the position of the texture
	ImVec2 uv0(uv0w, uv0h);
	ImVec2 uv1(uv1w, uv1h);

	ImGui::Image(ScreenTexture, ImVec2(TextureWidth * scale, TextureHeight * scale), uv0, uv1);

	ImDrawList* dl = ImGui::GetWindowDrawList();

	if (bHasScreen && ImGui::IsItemHovered())
	{
		// draw line around the screen area.
		if (bDrawScreenExtents)
		{
			const float x_min = Clamp(pos.x + (ScreenEdgeL * scale), pos.x, pos.x + (TextureWidth * scale));
			const float x_max = Clamp(pos.x + (ScreenEdgeL * scale) + (ScreenWidth * scale), pos.x, pos.x + (TextureWidth * scale));
			const float y_min = Clamp(pos.y + (ScreenTop * scale), pos.y, pos.y + (TextureHeight * scale));
			const float y_max = Clamp(pos.y + (ScreenTop * scale) + (ScreenHeight * scale), pos.y, pos.y + (TextureHeight * scale));

			dl->AddRect(ImVec2(x_min, y_min), ImVec2(x_max, y_max), 0xffffffff, 0, 0, 1 * scale);
		}
	}

	// colourize scanlines depending on the screen mode
	if (bShowScreenmodeChanges)
	{
		for (int s=0; s< AM40010_DISPLAY_HEIGHT; s++)
		{
			const uint8_t scrMode = pCPCEmu->Screen.GetScreenModeForScanline(s);
			dl->AddLine(ImVec2(pos.x, pos.y + (s * scale)), ImVec2(pos.x + (TextureWidth * scale), pos.y + (s * scale)), scrMode == 0 ? 0x40ffff00 : 0x4000ffff, 1 * scale);
		}
	}
	
	// Draw a line at each scanline position for each debugger event
	static int scanlineStart = 32;
	const FCodeAnalysisViewState& viewState = state.GetFocussedViewState();
	FDebugger& debugger = state.Debugger;
	const uint8_t* scanlineEvents = debugger.GetScanlineEvents();
	for (int scanlineNo = 0; scanlineNo < 320; scanlineNo++)
	{
		const int scanlineY = std::min(std::max(scanlineNo - scanlineStart, 0), AM40010_DISPLAY_HEIGHT);
		if (scanlineEvents[scanlineNo] != (int)EEventType::None)
		{
			const uint32_t col = debugger.GetEventColour(scanlineEvents[scanlineNo]);
			const ImVec2 start = ImVec2(pos.x + (TextureWidth * scale), pos.y + (scanlineY * scale));
			const ImVec2 end = ImVec2(pos.x + (TextureWidth + 32) * scale, pos.y + (scanlineY * scale));
			dl->AddLine(start, end, col, 1 * scale);
		}
	}

	// highlight scanline
	if (debugger.IsStopped())
	{
		if (viewState.HighlightScanline != -1)
		{
			const int scanlineY = std::min(std::max(viewState.HighlightScanline - scanlineStart, 0), AM40010_DISPLAY_HEIGHT);

			ImVec2 start = ImVec2(pos.x, pos.y + (scanlineY * scale));
			const ImVec2 end = ImVec2(pos.x + (TextureWidth + 32) * scale, pos.y + (scanlineY * scale));
			
			dl->AddLine(start, end, GetFlashColour(), 1 * scale);
		}
	}

	// todo highlight hovered address in code analyser view
	
	// draw pixels that correspond with address hovered in code analysis view
	if (viewState.HighlightAddress.IsValid())
	{
		ImDrawList* dl = ImGui::GetWindowDrawList();
		if (viewState.HighlightAddress.Address >= pCPCEmu->Screen.GetScreenAddrStart() && viewState.HighlightAddress.Address <= pCPCEmu->Screen.GetScreenAddrEnd())
		{
			int xp=0, yp=0;
			if (pCPCEmu->Screen.GetScreenAddressCoords(viewState.HighlightAddress.Address, xp, yp))
			{
				const int rx = static_cast<int>(pos.x + (ScreenEdgeL + xp) * scale);
				const int ry = static_cast<int>(pos.y + (ScreenTop + yp) * scale);
				// in screen mode 0, 1 byte will be 2 pixels. mode 1 will be 4 pixels 
				int pixelsToHighlight = 4;
				const int scrMode = GetScreenModeForPixelLine(yp);
				pixelsToHighlight = scrMode == 0 ? 2 : 4;

				dl->AddRectFilled(ImVec2((float)rx, (float)ry), ImVec2((float)rx + ((pixelsToHighlight + 1) * scale), (float)ry + (1 * scale)), GetFlashColour());
			}
		}
	}

	if (ImGui::IsItemHovered())
	{
		OnHovered(pos);
	}

#ifdef CPCVIEWER_EXTRA_DEBUG
	static bool bDrawTestScreen = false;
	ImGui::Checkbox("Draw test screen (DEBUG. SLOW!)", &bDrawTestScreen);
	if (bDrawTestScreen)
		DrawTestScreen();
#endif

	ImGui::SliderFloat("Speed Scale", &pCPCEmu->ExecSpeedScale, 0.0f, 2.0f);
	ImGui::SameLine();
	if (ImGui::Button("Reset"))
		pCPCEmu->ExecSpeedScale = 1.0f;
	ImGui::Text("Application average %.3f ms/frame (%.1f FPS)", 1000.0f / ImGui::GetIO().Framerate, ImGui::GetIO().Framerate);

	LuaSys::OnEmulatorScreenDrawn(pos.x, pos.y, scale);	// Call Lua handler

	bWindowFocused = ImGui::IsWindowHovered() || ImGui::IsWindowFocused();
	FrameCounter++;
}

// todo tidy this whole function up
bool FCPCViewer::OnHovered(const ImVec2& pos)
{
	if (!pCPCEmu->Screen.HasBeenDrawn())
	{
		ImGui::SetTooltip("Screen information will be unavailable until emulator is running.");
		return false;
	}

	const FGlobalConfig* pConfig = pCPCEmu->GetGlobalConfig();
	const float scale = (float)pConfig->ImageScale;

	ImDrawList* dl = ImGui::GetWindowDrawList();
	ImGuiIO& io = ImGui::GetIO();

	// get mouse cursor coords in logical screen area space
	const int xp = Clamp((int)(((io.MousePos.x - pos.x) / scale) - ScreenEdgeL), 0, ScreenWidth - 1);
	const int yp = Clamp((int)(((io.MousePos.y - pos.y) / scale) - ScreenTop), 0, ScreenHeight - 1);

	// get the screen mode for the raster line the mouse is pointed at
	// note: the logic doesn't always work and we sometimes end up with a negative scanline.
	// for example, this can happen for demos that set the scanlines-per-character to 1.
	const int scrMode = GetScreenModeForPixelLine(yp);
	const int charWidth = scrMode == 0 ? 16 : 8;

	// note: for screen mode 0 this will be in coord space of 320 x 200.
	// not sure that is right?
	uint16_t scrAddress = 0;
	if (pCPCEmu->Screen.GetScreenMemoryAddress(xp, yp, scrAddress))
	{
		// position (in pixels) of the start of the character
		const int charStartX = xp & ~(charWidth - 1); 
		const int charStartY = (yp / CharacterHeight) * CharacterHeight;
		
		{
			const float rx = Clamp(pos.x + (ScreenEdgeL + charStartX) * scale, pos.x, pos.x + (TextureWidth * scale));
			const float ry = Clamp(pos.y + (ScreenTop + charStartY) * scale, pos.y, pos.y + (TextureHeight * scale));

			// highlight the current character "square" (could actually be a rectangle if the char height is not 8)
			dl->AddRect(ImVec2(rx, ry), ImVec2((float)rx + (charWidth * scale), (float)ry + (CharacterHeight * scale)), 0xffffffff, 0, 0, 1 * scale);
		}

		FCodeAnalysisState& codeAnalysis = pCPCEmu->GetCodeAnalysis();
		const FAddressRef lastPixWriter = codeAnalysis.GetLastWriterForAddress(scrAddress);

#ifdef CPCVIEWER_EXTRA_DEBUG
		if (ImGui::IsMouseClicked(0))
		{
			// this code tests GetScreenMemoryAddress() is working correctly.
			if (bClickWritesToScreen)
			{
				const uint8_t numBytes = GetBitsPerPixel(scrMode);
				uint16_t plotAddress = 0;
				for (int y = 0; y < CharacterHeight; y++)
				{
					if (pCPCEmu->Screen.GetScreenMemoryAddress(charStartX, charStartY + y, plotAddress))
					{
						for (int b = 0; b < numBytes; b++)
						{
							pCPCEmu->WriteByte(plotAddress + b, 1 << y);
						}
					}
				}
			}
		}
#endif			
		ImGui::BeginTooltip();

		// adjust the x position based on the screen mode for the scanline
		const int divisor[4] = { 4, 2, 1, 2 };
		const int x_adj = (xp * 2) / (scrMode == -1 ? 2 : divisor[scrMode]);

		const int charIndexX = charStartX / charWidth;
		const int charIndexY = charStartY / CharacterHeight;
#ifdef CPCVIEWER_EXTRA_DEBUG
		ImGui::Text("Character (%d, %d)", charIndexX, charIndexY);
		ImGui::Text("Scanline: %d", ScreenTop + yp);
#endif

		ImGui::Text("Screen Pos (%d, %d)", x_adj, yp);
		ImGui::Text("Addr: %s", NumStr(scrAddress));

		FCodeAnalysisViewState& viewState = codeAnalysis.GetFocussedViewState();
		if (lastPixWriter.IsValid())
		{
			ImGui::Text("Pixel Writer: ");
			ImGui::SameLine();
			DrawCodeAddress(codeAnalysis, viewState, lastPixWriter);
		}
		else
		{
			ImGui::Text("Pixel Writer: None");
		}

		if (scrMode == -1)
			ImGui::Text("Screen Mode: unknown");
		else
			ImGui::Text("Screen Mode: %d", scrMode);
		
		const ImVec2 pos = ImGui::GetCursorScreenPos();
		DrawScreenCharacter(charIndexX, charIndexY, pos.x, pos.y, 10.f);
		ImGui::SetCursorScreenPos(ImVec2(pos.x, pos.y + 10.f * CharacterHeight));

		bool bPaletteChanged = false;
		const FPalette& thisScanlinePalette = GetPaletteForPixelLine(yp);
		if (yp > 0)
		{ 
			const FPalette& lastScanlinePalette = GetPaletteForPixelLine(yp-1);

			if (lastScanlinePalette != thisScanlinePalette)
				bPaletteChanged = true;
		}

		ImGui::Text("Palette");
		DrawPalette(thisScanlinePalette);
		ImGui::EndTooltip();

		if (ImGui::IsMouseDoubleClicked(0))
			viewState.GoToAddress(lastPixWriter);
	}

	return false;
}

// returns how much horizontal space it took
float FCPCViewer::DrawScreenCharacter(int xChar, int yChar, float x, float y, float pixelHeight) const 
{
	// todo move this comment out of this function
	// the x coord will be in mode 1 coordinates. [320 pixels]
	// mode 0 will effectively use 2 mode 1 pixels.

	const int screenMode = GetScreenModeForPixelLine(yChar * CharacterHeight);
	
	const FCodeAnalysisState& codeAnalysis = pCPCEmu->GetCodeAnalysis();
	const int xMult = screenMode == 0 ? 2 : 1;
	ImVec2 pixelSize = ImVec2(pixelHeight * (float)xMult, pixelHeight);

	ImDrawList* dl = ImGui::GetWindowDrawList();
	ImVec2 pos = ImVec2((float)x, (float)y);
	const float startPos = pos.x;

	for (int pixline = 0; pixline < CharacterHeight; pixline++) 
	{
		// todo: check return from this
		// todo: deal with non contiguous screen memory bytes
		uint16_t pixLineAddress = 0;
		pCPCEmu->Screen.GetScreenMemoryAddress(xChar * xMult * 8, yChar * CharacterHeight + pixline, pixLineAddress);

		switch (screenMode)
		{
		case 0:
			// double width
			for (int byte = 0; byte < 4; byte++)
			{
				const uint8_t val = codeAnalysis.ReadByte(pixLineAddress + byte);

				for (int pixel = 0; pixel < 2; pixel++)
				{
					const ImVec2 rectMin(pos.x, pos.y);
					const ImVec2 rectMax(pos.x + pixelSize.x, pos.y + pixelSize.y);

					int colourIndex = GetHWColourIndexForPixel(val, pixel, 0);

					const ImU32 colour = GetPaletteForPixelLine(yChar * CharacterHeight).GetColour(colourIndex);
					dl->AddRectFilled(rectMin, rectMax, colour);
					pos.x += pixelSize.x;
				}
			}
			break;
		case 1:
			for (int byte = 0; byte < 2; byte++)
			{
				const uint8_t val = codeAnalysis.ReadByte(pixLineAddress + byte);

				for (int pixel = 0; pixel < 4; pixel++)
				{
					const ImVec2 rectMin(pos.x, pos.y);
					const ImVec2 rectMax(pos.x + pixelSize.x, pos.y + pixelSize.y);

					int colourIndex = GetHWColourIndexForPixel(val, pixel, 1);

					const ImU32 colour = GetPaletteForPixelLine(yChar * CharacterHeight).GetColour(colourIndex);
					dl->AddRectFilled(rectMin, rectMax, colour);
					pos.x += pixelSize.x;
				}
			}
			break;
		}

		pos.x = startPos;
		pos.y += pixelSize.y;
	}
	float rectWidth = pixelSize.x * 8.0f;
	dl->AddRect(ImVec2(x,y), ImVec2(x + rectWidth, y + pixelSize.y * CharacterHeight), 0xffffffff);
	return rectWidth;
}

void FCPCViewer::CalculateScreenProperties()
{
	// work out the position and size of the logical cpc screen based on the crtc registers.
	// note: these calculations will be wrong if the game sets crtc registers dynamically during the frame.
	// registers not hooked up: R3
	const mc6845_t& crtc = pCPCEmu->CPCEmuState.crtc;

	CharacterHeight = crtc.max_scanline_addr + 1;			// crtc register 9 defines how many scanlines in a character square
	
	// not sure I should be doing this, but values >8 cause problems.
	CharacterHeight = std::min(CharacterHeight, 8);

	ScreenWidth = crtc.h_displayed * 8; // note: this is always in mode 1 coords. 
	ScreenHeight = crtc.v_displayed * CharacterHeight;

	ScreenTop = pCPCEmu->Screen.GetTopPixelEdge();
	ScreenEdgeL = pCPCEmu->Screen.GetLeftPixelEdge();

	HorizCharCount = crtc.h_displayed;
}

int FCPCViewer::GetScreenModeForPixelLine(int yPos) const
{
	const int scanline = ScreenTop + yPos;
	return pCPCEmu->Screen.GetScreenModeForScanline(scanline);
}

const FPalette& FCPCViewer::GetPaletteForPixelLine(int yPos) const
{
	const int scanline = ScreenTop + yPos;
	return pCPCEmu->Screen.GetPaletteForScanline(scanline);
}

ImU32 FCPCViewer::GetFlashColour() const
{
	// generate flash colour
	ImU32 flashCol = 0xff000000;
	const int flashCounter = FrameCounter >> 2;
	if (flashCounter & 1) flashCol |= 0xff << 0;
	if (flashCounter & 2) flashCol |= 0xff << 8;
	if (flashCounter & 4) flashCol |= 0xff << 16;
	return flashCol;
}

void FCPCViewer::Tick(void)
{
	// Check keys - not event driven, hopefully perf isn't too bad
	for (int key = ImGuiKey_NamedKey_BEGIN; key < ImGuiKey_COUNT; key++)
	{
		if (ImGui::IsKeyPressed((ImGuiKey)key,false))
		{ 
			if (bWindowFocused)
			{
				int cpcKey = CPCKeyFromImGuiKey((ImGuiKey)key);
				if (cpcKey != 0)
					cpc_key_down(&pCPCEmu->CPCEmuState, cpcKey);
			}
		}
		else if (ImGui::IsKeyReleased((ImGuiKey)key))
		{
			const int cpcKey = CPCKeyFromImGuiKey((ImGuiKey)key);
			if (cpcKey != 0)
				cpc_key_up(&pCPCEmu->CPCEmuState, cpcKey);
		}
	}
}


int CPCKeyFromImGuiKey(ImGuiKey key)
{
	int cpcKey = 0;

	if (key >= ImGuiKey_0 && key <= ImGuiKey_9)
	{
		cpcKey = '0' + (key - ImGuiKey_0);
	}
	else if (key >= ImGuiKey_A && key <= ImGuiKey_Z)
	{
		cpcKey = 'A' + (key - ImGuiKey_A) + 0x20;
	}
	else if (key >= ImGuiKey_Keypad1 && key <= ImGuiKey_Keypad9)
	{
		cpcKey = 0xf1 + (key - ImGuiKey_Keypad1);
	}
	else if (key == ImGuiKey_Keypad0)
	{
		cpcKey = 0xfa;
	}
	else if (key == ImGuiKey_Space)
	{
		cpcKey = ' ';
	}
	else if (key == ImGuiKey_Enter)
	{
		cpcKey = 0xd;
	}
	else if (key == ImGuiKey_Backspace)
	{	
		cpcKey = 0x1;
	}
	else if (key == ImGuiKey_Comma)
	{
		cpcKey = 0x2c;
	}
	else if (key == ImGuiKey_Tab)
	{
		cpcKey = 0x6;
	}
	else if (key == ImGuiKey_Period)
	{
		cpcKey = 0x2e;
	}
	else if (key == ImGuiKey_Semicolon)
	{
		cpcKey = 0x3a;
	}
	else if (key == ImGuiKey_CapsLock)
	{
		cpcKey = 0x7;
	}
	else if (key == ImGuiKey_Escape)
	{
		cpcKey = 0x3;
	}
	else if (key == ImGuiKey_Minus)
	{
		cpcKey = 0x2d;
	}
	else if (key == ImGuiKey_Apostrophe)
	{
		// ; semicolon
		cpcKey = 0x3b;
	}
	else if (key == ImGuiKey_Equal)
	{
		// up arrow with pound sign
		cpcKey = 0x5e;
	}
	else if (key == ImGuiKey_Delete)
	{
		// CLR
		cpcKey = 0xc;
	}
	else if (key == ImGuiKey_Insert)
	{
		// Copy
		cpcKey = 0x5;
	}
	else if (key == ImGuiKey_Slash)
	{
		// forward slash /
		cpcKey = 0x2f;
	}
	else if (key == ImGuiKey_LeftBracket)
	{
		// [
		cpcKey = 0x5b;
	}
	else if (key == ImGuiKey_RightBracket)
	{
		// ]
		cpcKey = 0x5d;
	}
	else if (key == ImGuiKey_Backslash)
	{
		// backslash '\'
		cpcKey = 0x5c;
	}
	else if (key == ImGuiKey_GraveAccent) // `
	{
		// @
		cpcKey = 0x40;
	}
	else if (key == ImGuiKey_LeftArrow)
	{
		cpcKey = 0x8;
	}
	else if (key == ImGuiKey_RightArrow)
	{
		cpcKey = 0x9;
	}
	else if (key == ImGuiKey_UpArrow)
	{
		cpcKey = 0xb;
	}
	else if (key == ImGuiKey_DownArrow)
	{
		cpcKey = 0xa;
	}
	else if (key == ImGuiKey_LeftShift || key == ImGuiKey_RightShift)
	{
		cpcKey = 0xe;
	}
	else if (key == ImGuiKey_LeftCtrl || key == ImGuiKey_RightCtrl)
	{
		cpcKey = 0xf;
	}

	return cpcKey;
}

void DrawPalette(const FPalette& palette)
{
	// todo scale this
	const float length = ImGui::GetFontSize() * 1.5f;
	const ImVec2 size(length, length);
	for (int i = 0; i < palette.GetColourCount(); i++) 
	{
		ImGui::PushID(128 + i);
		ImGui::ColorButton("##ink_color", ImColor(palette.GetColour(i)), ImGuiColorEditFlags_NoAlpha, size);
		ImGui::PopID();
		if (((i + 1) % 8) != 0) 
		{
			ImGui::SameLine();
		}
	}	
}

#ifdef CPCVIEWER_EXTRA_DEBUG
// Draw an entire screen out of characters - to test the code for drawing a screen character.
void FCPCViewer::DrawTestScreen()
{
	const mc6845_t& crtc = pCPCEmu->CPCEmuState.crtc;
	ImVec2 curPos = ImGui::GetCursorScreenPos();
	const float xStart = curPos.x;

	int scanLine = ScreenTop;

	const float pixelHeight = 5.0f;
	for (int y = 0; y < crtc.v_displayed; y++)
	{
		const int scrMode = pCPCEmu->Screen.GetScreenModeForScanline(scanLine);
		const int charCount = scrMode == 0 ? HorizCharCount / 2 : HorizCharCount;
		for (int x = 0; x < charCount; x++)
		{
			curPos.x += DrawScreenCharacter(x, y, curPos.x, curPos.y, pixelHeight);
		}
		scanLine += CharacterHeight;
		curPos.x = xStart;
		curPos.y += pixelHeight * CharacterHeight;
	}
	ImGui::SetCursorScreenPos(ImVec2(curPos.x, curPos.y));
}

// Debug code to manually iterate through all snaps in a directory.
void DrawSnapLoadButtons(FCPCEmu* pCPCEmu)
{
	if (pCPCEmu->GetGamesList().GetNoGames())
	{
		static int gGameIndex = 0;
		bool bLoadSnap = false;
		if (ImGui::Button("Prev snap") || ImGui::IsKeyPressed(ImGuiKey_F1))
		{
			if (gGameIndex > 0)
				gGameIndex--;
			bLoadSnap = true;
		}
		ImGui::SameLine();
		if (ImGui::Button("Next snap") || ImGui::IsKeyPressed(ImGuiKey_F2))
		{
			if (gGameIndex < pCPCEmu->GetGamesList().GetNoGames() - 1)
				gGameIndex++;
			bLoadSnap = true;
		}
		ImGui::SameLine();
		const FGameSnapshot& game = pCPCEmu->GetGamesList().GetGame(gGameIndex);
		ImGui::Text("(%d/%d) %s", gGameIndex + 1, pCPCEmu->GetGamesList().GetNoGames(), game.DisplayName.c_str());
		if (ImGui::IsKeyPressed(ImGuiKey_F3))
			bLoadSnap = true;

		if (bLoadSnap)
		{
			LOGINFO("Load game '%s'", game.DisplayName.c_str());
			const FGameSnapshot& game = pCPCEmu->GetGamesList().GetGame(gGameIndex);
			if (!pCPCEmu->NewGameFromSnapshot(game))
			{
				pCPCEmu->Reset();
				pCPCEmu->DisplayErrorMessage("Could not create game '%s'", game.DisplayName.c_str());
			}

			/*if (!pCPCEmu->GetGamesList().LoadGame(gGameIndex))
			{
				pCPCEmu->DisplayErrorMessage("Could not load '%s'", game.DisplayName.c_str());
			}*/
		}
	}
}
#endif