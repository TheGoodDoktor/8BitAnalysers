#include "SpectrumViewer.h"

#include <CodeAnalyser/CodeAnalyser.h>

#include <imgui.h>
#include "../SpectrumEmu.h"
#include "../SpectrumConstants.h"
#include <CodeAnalyser/UI/CodeAnalyserUI.h>
#include "../ZXSpectrumConfig.h"

#include <Util/Misc.h>
#include <ImGuiSupport/ImGuiTexture.h>
#include <ImGuiSupport/ImGuiScaling.h>
#include <ImGuiSupport/ImGuiDrawing.h>
#include <CodeAnalyser/UI/UIColours.h>
#include <LuaScripting/LuaSys.h>

static const int kBorderOffsetX = (320 - 256) / 2;
static const int kBorderOffsetY = (256 - 192) / 2;


void FSpectrumViewer::Init(FSpectrumEmu* pEmu)
{
	pSpectrumEmu = pEmu;

	// setup texture
	chips_display_info_t dispInfo = zx_display_info(&pEmu->ZXEmuState);

	// setup pixel buffer
	const size_t pixelBufferSize = dispInfo.frame.dim.width * dispInfo.frame.dim.height;
	FrameBuffer = new uint32_t[pixelBufferSize * 2];
	ScreenTexture = ImGui_CreateTextureRGBA(FrameBuffer, dispInfo.frame.dim.width, dispInfo.frame.dim.height);

	//SetInputEventHandler(this);
}

void FSpectrumViewer::Draw()
{
	const FZXSpectrumConfig& config = *pSpectrumEmu->GetZXSpectrumGlobalConfig();
	FCodeAnalysisState& codeAnalysis = pSpectrumEmu->GetCodeAnalysis();
	FDebugger& debugger = codeAnalysis.Debugger;
	FCodeAnalysisViewState& viewState = codeAnalysis.GetFocussedViewState();

	chips_display_info_t disp = zx_display_info(&pSpectrumEmu->ZXEmuState);
	
	// convert texture to RGBA
	const uint8_t* pix = (const uint8_t*)disp.frame.buffer.ptr;
	const uint32_t* pal = (const uint32_t*)disp.palette.ptr;
	for (int i = 0; i < disp.frame.buffer.size; i++)
		FrameBuffer[i] = pal[pix[i]];

	// update screen texture
	ImGui_UpdateTextureRGBA(ScreenTexture, FrameBuffer);

	const ImVec2 pos = ImGui::GetCursorScreenPos();
	const float scale = (float)config.ImageScale;//ImGui_GetScaling();
	//ImGui::Text("Instructions this frame: %d \t(max:%d)", instructionsThisFrame,maxInst);
	ImVec2 uv0(0, 0);
	ImVec2 uv1(320.0f / 512.0f, 1.0f);
	ImGui::Image(ScreenTexture, ImVec2(320 * scale, 256 * scale),uv0,uv1);

	ImDrawList* dl = ImGui::GetWindowDrawList();
	const int topScreenScanLine = pSpectrumEmu->ZXEmuState.top_border_scanlines - 32;

	const uint8_t* scanlineEvents = debugger.GetScanlineEvents();
	for (int scanlineNo = 0; scanlineNo < 320; scanlineNo++)
	{
		const int scanlineY = std::min(std::max(scanlineNo - topScreenScanLine, 0), 256);
		if (scanlineEvents[scanlineNo])// != (int)EEventType::None)
		{
			const uint32_t col = debugger.GetEventColour(scanlineEvents[scanlineNo]);
			dl->AddLine(ImVec2(pos.x + (320 * scale), pos.y + (scanlineY * scale)), ImVec2(pos.x + (320 + 32) * scale, pos.y + (scanlineY * scale)), col);
		}
	}
	// Draw an indicator to show which scanline is being drawn
	if (config.bShowScanLineIndicator && codeAnalysis.Debugger.IsStopped())
	{
		// Compensate for the fact the border area on a real spectrum is bigger than the emulated spectrum.
		int scanlineY = std::min(std::max(pSpectrumEmu->ZXEmuState.scanline_y - topScreenScanLine, 0), 256);
		dl->AddLine(ImVec2(pos.x + (4 * scale), pos.y + (scanlineY * scale)), ImVec2(pos.x + (320 - 8) * scale, pos.y + (scanlineY * scale)), 0x50ffffff);
		DrawArrow(dl, ImVec2(pos.x - 2, pos.y + (scanlineY * scale) - 6), false);
		DrawArrow(dl, ImVec2(pos.x + (320 - 11) * scale, pos.y + (scanlineY * scale) - 6), true);
	}

	// highlight scanline
	if (debugger.IsStopped())
	{
		if (viewState.HighlightScanline != -1)
		{
			const int scanlineY = std::min(std::max(viewState.HighlightScanline - topScreenScanLine, 0), 256);

			ImVec2 start = ImVec2(pos.x, pos.y + (scanlineY * scale));
			const ImVec2 end = ImVec2(pos.x + (320 + 32) * scale, pos.y + (scanlineY * scale));

			dl->AddLine(start, end, Colours::GetFlashColour(), 1 * scale);
		}
	}

	if (bShowCoordinates)
		DrawCoordinatePositions(codeAnalysis, pos);

	
	// draw hovered address
	if (viewState.HighlightAddress.IsValid())
	{
		ImDrawList* dl = ImGui::GetWindowDrawList();
		const ImU32 flashCol = Colours::GetFlashColour();
		if (viewState.HighlightAddress.Address >= kScreenPixMemStart && viewState.HighlightAddress.Address <= kScreenPixMemEnd)	// pixel
		{
			int xp, yp;
			GetScreenAddressCoords(viewState.HighlightAddress.Address, xp, yp);

			const int rx = static_cast<int>(pos.x + (kBorderOffsetX + xp) * scale);
			const int ry = static_cast<int>(pos.y + (kBorderOffsetY + yp) * scale);
			dl->AddRect(ImVec2((float)rx, (float)ry), ImVec2((float)rx + (8 * scale), (float)ry + (1 * scale)), flashCol);	// TODO: flash?
		}

		if (viewState.HighlightAddress.Address >= kScreenAttrMemStart && viewState.HighlightAddress.Address <= kScreenAttrMemEnd)	// attributes
		{
			int xp, yp;
			GetAttribAddressCoords(viewState.HighlightAddress.Address, xp, yp);

			const int rx = static_cast<int>(pos.x + (kBorderOffsetX + xp) * scale);
			const int ry = static_cast<int>(pos.y + (kBorderOffsetY + yp) * scale);
			dl->AddRect(ImVec2((float)rx, (float)ry), ImVec2((float)rx + (8 * scale), (float)ry + (8 * scale)), flashCol);	// TODO: flash?
		}
	}

	if (ImGui::IsItemHovered())
		OnHovered(pos, codeAnalysis, viewState);

	if (bScreenCharSelected == true)
		DrawSelectedCharUI(pos);

	// This is experimental
	if (ImGui::Button("Find and format all on-screen chars"))
	{
		uint8_t charData[8];

		for (int yChar = 0; yChar < 24; yChar++)
		{
			for (int xChar = 0; xChar < 32; xChar++)
			{
				const int xp = xChar * 8;
				const int yp = yChar * 8;
				bool bEmpty = true;

				// store pixel data for selected character
				for (int charLine = 0; charLine < 8; charLine++)
				{
					charData[charLine] = pSpectrumEmu->ReadByte(GetScreenPixMemoryAddress(xp, yp + charLine));
					if (charData[charLine] != 0)
						bEmpty = false;
				}

				if (bEmpty == false)
				{
					const auto foundCharDataAddresses = codeAnalysis.FindAllMemoryPatterns(charData, 8, false, false);

					if (foundCharDataAddresses.empty() == false)
					{
						const FAddressRef& foundAddress = foundCharDataAddresses[0];	// just do first address for now
						const FCodeInfo* pCodeInfo = codeAnalysis.GetCodeInfoForAddress(foundAddress);
						if (pCodeInfo == nullptr || pCodeInfo->bDisabled)
						{
							FDataFormattingOptions formattingOptions;
							formattingOptions.StartAddress = foundAddress;
							formattingOptions.ItemSize = 1;
							formattingOptions.NoItems = 8;
							formattingOptions.DataType = EDataType::Bitmap;

							FormatData(codeAnalysis, formattingOptions);
						}
					}
				}

			}
		}		
	}

	ImGui::SliderFloat("Speed Scale", &pSpectrumEmu->ExecSpeedScale, 0.0f, 2.0f);
	ImGui::SameLine();
	if (ImGui::Button("Reset"))
		pSpectrumEmu->ExecSpeedScale = 1.0f;
	ImGui::Text("Application average %.3f ms/frame (%.1f FPS)", 1000.0f / ImGui::GetIO().Framerate, ImGui::GetIO().Framerate);

	if (pSpectrumEmu->bHasInterruptHandler)
	{
		ImGui::Text("IM2 Interrupt Handler at: %s", NumStr(pSpectrumEmu->InterruptHandlerAddress));
		DrawAddressLabel(codeAnalysis, viewState, pSpectrumEmu->InterruptHandlerAddress);
	}

	if (ImGui::CollapsingHeader("Screen Coordinates"))
	{
		DrawAddressInput(codeAnalysis, "X Coord", XCoordAddress);
		ImGui::SameLine();
		ImGui::Checkbox("Invert X", &bInvertXCoord);

		//ImGui::SameLine();
		DrawAddressInput(codeAnalysis, "Y Coord", YCoordAddress);
		ImGui::SameLine();
		ImGui::Checkbox("Invert Y", &bInvertYCoord);
		ImGui::SameLine();
		if (ImGui::Button("Set to X+1"))
		{
			YCoordAddress = XCoordAddress;
			YCoordAddress.Address++;
		}
		bShowCoordinates = true;
	}
	else
	{
		bShowCoordinates = false;
	}
	
	LuaSys::OnEmulatorScreenDrawn(pos.x, pos.y, scale);	// Call Lua handler

	bWindowFocused = ImGui::IsWindowFocused();
}

void FSpectrumViewer::DrawCoordinatePositions(FCodeAnalysisState& codeAnalysis, const ImVec2& pos)
{
	const FZXSpectrumConfig& config = *pSpectrumEmu->GetZXSpectrumGlobalConfig();
	const float scale = (float)config.ImageScale;//ImGui_GetScaling();

	// draw coordinate position
	if (XCoordAddress.IsValid())
	{
		ImDrawList* dl = ImGui::GetWindowDrawList();
		const uint8_t xVal = codeAnalysis.ReadByte(XCoordAddress);
		const float xLinePos = (float)(bInvertXCoord ? 255 - xVal : xVal);
		const ImVec2 lineStart(pos.x + (kBorderOffsetX + xLinePos) * scale, pos.y + kBorderOffsetY * scale);
		const ImVec2 lineEnd(pos.x + (kBorderOffsetX + xLinePos) * scale, pos.y + (kBorderOffsetY + 192.0f) * scale);

		dl->AddLine(lineStart, lineEnd, 0xffffffff);
	}
	if (YCoordAddress.IsValid())
	{
		ImDrawList* dl = ImGui::GetWindowDrawList();
		const uint8_t yVal = codeAnalysis.ReadByte(YCoordAddress);
		const float yLinePos = (float)(bInvertYCoord ? 192 - yVal : yVal);
		const ImVec2 lineStart(pos.x + kBorderOffsetX * scale, pos.y + (kBorderOffsetY + yLinePos) * scale);
		const ImVec2 lineEnd(pos.x + (kBorderOffsetX + 256.0f) * scale, pos.y + (kBorderOffsetY + yLinePos) * scale);

		dl->AddLine(lineStart, lineEnd, 0xffffffff);
	}
}

void FSpectrumViewer::DrawSelectedCharUI(const ImVec2& pos)
{
	ImDrawList* dl = ImGui::GetWindowDrawList();	
	FCodeAnalysisState& codeAnalysis = pSpectrumEmu->GetCodeAnalysis();
	FCodeAnalysisViewState& viewState = codeAnalysis.GetFocussedViewState();
	FDebugger& debugger = codeAnalysis.Debugger;
	const FZXSpectrumConfig& config = *pSpectrumEmu->GetZXSpectrumGlobalConfig();
	const float scale = (float)config.ImageScale;//ImGui_GetScaling();

	const ImU32 col = Colours::GetFlashColour();
	dl->AddRect(ImVec2(pos.x + ((float)SelectedCharX * scale), pos.y + (float)SelectedCharY * scale), ImVec2(pos.x + ((float)SelectedCharX + 8) * scale, pos.y + ((float)SelectedCharY + 8) * scale), col);

	ImGui::Text("Pixel Char Address: %s", NumStr(SelectPixAddr.Address));
	//ImGui::SameLine();
	DrawAddressLabel(codeAnalysis, viewState, SelectPixAddr);
	ImGui::Text("Attribute Address: %s", NumStr(SelectAttrAddr.Address));
	//ImGui::SameLine();
	DrawAddressLabel(codeAnalysis, viewState, SelectAttrAddr);

	// character pixel breakpoint
	if (ImGui::Checkbox("Break on pixel write", &bBreakOnCharPixelWrite))
	{
		if (bBreakOnCharPixelWrite)
		{
			// Add data breakpoint
			debugger.AddDataBreakpoint(SelectPixAddr, 8);
			CharacterPixelBPAddress = SelectPixAddr;
		}
		else
		{
			debugger.RemoveBreakpoint(CharacterPixelBPAddress);
		}
	}

	if (bBreakOnCharPixelWrite && SelectPixAddr != CharacterPixelBPAddress)
	{
		debugger.ChangeBreakpointAddress(CharacterPixelBPAddress, SelectPixAddr);
		CharacterPixelBPAddress = SelectPixAddr;
	}

	// attribute breakpoint
	if (ImGui::Checkbox("Break on attr write", &bBreakOnCharAttrWrite))
	{
		if (bBreakOnCharAttrWrite)
		{
			// Add data breakpoint
			debugger.AddDataBreakpoint(SelectAttrAddr, 1);
			CharacterAttrBPAddress = SelectAttrAddr;
		}
		else
		{
			debugger.RemoveBreakpoint(CharacterAttrBPAddress);
		}
	}

	if (bBreakOnCharAttrWrite && SelectAttrAddr != CharacterAttrBPAddress)
	{
		debugger.ChangeBreakpointAddress(CharacterAttrBPAddress, SelectAttrAddr);
		CharacterAttrBPAddress = SelectAttrAddr;
	}

	if (FoundCharAddresses.empty() == false)
	{
		// list?
		const FAddressRef& foundCharAddress = FoundCharAddresses[FoundCharIndex];
		ImGui::Text("Found at: %s", NumStr(foundCharAddress.Address));
		DrawAddressLabel(codeAnalysis, viewState, foundCharAddress);
		if (FoundCharAddresses.size() > 1)
		{
			ImGui::SameLine();
			if (ImGui::Button("Next"))
				FoundCharIndex = (FoundCharIndex + 1) % FoundCharAddresses.size();
		}

		if (ImGui::Button("Format as Bitmap"))
		{
			FDataFormattingOptions formattingOptions;
			formattingOptions.StartAddress = foundCharAddress;
			formattingOptions.ItemSize = 1;
			formattingOptions.NoItems = 8;
			formattingOptions.DataType = EDataType::Bitmap;

			FormatData(codeAnalysis, formattingOptions);
			viewState.GoToAddress(foundCharAddress, false);
		}
		ImGui::SameLine();
		if (ImGui::Button("Show in GFX View"))
		{
			pSpectrumEmu->GraphicsViewerSetView(foundCharAddress);
		}
	}
}


bool FSpectrumViewer::OnHovered(const ImVec2& pos, FCodeAnalysisState& codeAnalysis, FCodeAnalysisViewState& viewState)
{
	const FZXSpectrumConfig& config = *pSpectrumEmu->GetZXSpectrumGlobalConfig();
	const float scale = (float)config.ImageScale;//ImGui_GetScaling();
	bool bJustSelectedChar = false;
	
	ImGuiIO& io = ImGui::GetIO();
	const int xp = std::min(std::max((int)((io.MousePos.x - pos.x) / scale) - kBorderOffsetX, 0), 255);
	const int yp = std::min(std::max((int)((io.MousePos.y - pos.y) / scale) - kBorderOffsetY, 0), 191);

	const uint16_t scrPixAddress = GetScreenPixMemoryAddress(xp, yp);
	const uint16_t scrAttrAddress = GetScreenAttrMemoryAddress(xp, yp);

	if (scrPixAddress != 0)
	{
		ImDrawList* dl = ImGui::GetWindowDrawList();
		const int rx = kBorderOffsetX + (xp & ~0x7);
		const int ry = kBorderOffsetY + (yp & ~0x7);
		dl->AddRect(ImVec2(pos.x + ((float)rx * scale), pos.y + ((float)ry * scale)), ImVec2(pos.x + (float)(rx + 8) * scale,pos.y + (float)(ry + 8) * scale), 0xffffffff);
		ImGui::BeginTooltip();
		ImGui::Text("Screen Pos (%d,%d)", xp, yp);
		ImGui::Text("Pixel: %s, Attr: %s", NumStr(scrPixAddress), NumStr(scrAttrAddress));

		const FAddressRef lastPixWriter = codeAnalysis.GetLastWriterForAddress(scrPixAddress);
		const FAddressRef lastAttrWriter = codeAnalysis.GetLastWriterForAddress(scrAttrAddress);
		if (lastPixWriter.IsValid())
		{
			ImGui::Text("Pixel Writer: ");
			ImGui::SameLine();
			DrawCodeAddress(codeAnalysis, viewState, lastPixWriter);
		}
		if (lastAttrWriter.IsValid())
		{
			ImGui::Text("Attribute Writer: ");
			ImGui::SameLine();
			DrawCodeAddress(codeAnalysis, viewState, lastAttrWriter);
		}
		{
			//ImGui::Text("Image: ");
			//const float line_height = ImGui::GetTextLineHeight();
			const float rectSize = 10;
			ImDrawList* dl = ImGui::GetWindowDrawList();
			ImVec2 pos = ImGui::GetCursorScreenPos();
			const float startPos = pos.x;
			//pos.y -= rectSize + 2;

			for (int byte = 0; byte < 8; byte++)
			{
				const uint8_t val = pSpectrumEmu->ReadByte(GetScreenPixMemoryAddress(xp & ~0x7, (yp & ~0x7) + byte));

				for (int bit = 7; bit >= 0; bit--)
				{
					const ImVec2 rectMin(pos.x, pos.y);
					const ImVec2 rectMax(pos.x + rectSize, pos.y + rectSize);
					if (val & (1 << bit))
						dl->AddRectFilled(rectMin, rectMax, 0xffffffff);
					else
						dl->AddRect(rectMin, rectMax, 0xffffffff);

					pos.x += rectSize;
				}

				pos.x = startPos;
				pos.y += rectSize;
			}

			ImGui::Text("");
			ImGui::Text("");
			ImGui::Text("");
			ImGui::Text("");
			ImGui::Text("");
		}
		ImGui::EndTooltip();
		//ImGui::Text("Pixel Writer: %04X, Attrib Writer: %04X", lastPixWriter, lastAttrWriter);


		if (ImGui::IsMouseClicked(0))
		{
			bScreenCharSelected = true;
			SelectedCharX = rx;
			SelectedCharY = ry;
			SelectPixAddr = codeAnalysis.AddressRefFromPhysicalAddress(scrPixAddress);
			SelectAttrAddr = codeAnalysis.AddressRefFromPhysicalAddress(scrAttrAddress);

			// store pixel data for selected character
			for (int charLine = 0; charLine < 8; charLine++)
				CharData[charLine] = pSpectrumEmu->ReadByte(GetScreenPixMemoryAddress(xp & ~0x7, (yp & ~0x7) + charLine));
			//CharDataFound = codeAnalysis.FindMemoryPatternInPhysicalMemory(CharData, 8, 0, FoundCharDataAddress);
			FoundCharAddresses = codeAnalysis.FindAllMemoryPatterns(CharData, 8, true, false);
			FoundCharIndex = 0;
			bJustSelectedChar = true;
		}

		if (ImGui::IsMouseClicked(1))
		{
			bScreenCharSelected = false;
			bJustSelectedChar = false;
		}

		if (ImGui::IsMouseDoubleClicked(0))
			viewState.GoToAddress(lastPixWriter);
		if (ImGui::IsMouseDoubleClicked(1))
			viewState.GoToAddress(lastAttrWriter);
	}	

	return bJustSelectedChar;
}

int SpectrumKeyFromImGuiKey(ImGuiKey key)
{
	int speccyKey = 0;

	if (key >= ImGuiKey_0 && key <= ImGuiKey_9)
	{
		speccyKey = '0' + (key - ImGuiKey_0);
	}
	else if (key >= ImGuiKey_A && key <= ImGuiKey_Z)
	{
		speccyKey = 'A' + (key - ImGuiKey_A) + 0x20;
	}
	else if (key == ImGuiKey_Space)
	{
		speccyKey = ' ';
	}
	else if (key == ImGuiKey_Enter)
	{
		speccyKey = 0xd;
	}
	else if (key == ImGuiKey_LeftCtrl)
	{
		// symbol-shift
		speccyKey = 0xf;
	}
	else if (key == ImGuiKey_LeftShift)
	{
		// caps-shift
		speccyKey = 0xe;
	}
	else if (key == ImGuiKey_Backspace)
	{
		// delete (shift and 0)
		speccyKey = 0xc;
	}
	return speccyKey;
}

void FSpectrumViewer::Tick(void)
{
	// Check keys - not event driven, hopefully perf isn't too bad
	for (int key = ImGuiKey_NamedKey_BEGIN; key < ImGuiKey_COUNT; key++)
	{
		if (ImGui::IsKeyPressed((ImGuiKey)key,false))
		{ 
			const int speccyKey = SpectrumKeyFromImGuiKey((ImGuiKey)key);
			if (speccyKey != 0 && bWindowFocused)
				zx_key_down(&pSpectrumEmu->ZXEmuState, speccyKey);
		}
		else if (ImGui::IsKeyReleased((ImGuiKey)key))
		{
			const int speccyKey = SpectrumKeyFromImGuiKey((ImGuiKey)key);
			if (speccyKey != 0)
				zx_key_up(&pSpectrumEmu->ZXEmuState, speccyKey);
		}
	}

	// Gamepad support, can use ImGuiKey values here
	if (zx_joystick_type(&pSpectrumEmu->ZXEmuState) != ZX_JOYSTICKTYPE_NONE)
	{
		int mask = 0;
		if (ImGui::IsKeyDown(ImGuiKey_GamepadDpadRight))
			mask |= 1 << 0;
		if (ImGui::IsKeyDown(ImGuiKey_GamepadDpadLeft))
			mask |= 1 << 1;
		if (ImGui::IsKeyDown(ImGuiKey_GamepadDpadDown))
			mask |= 1 << 2;
		if (ImGui::IsKeyDown(ImGuiKey_GamepadDpadUp))
			mask |= 1 << 3;
		if (ImGui::IsKeyDown(ImGuiKey_GamepadFaceDown))
			mask |= 1 << 4;

		zx_joystick(&pSpectrumEmu->ZXEmuState, mask);
	}
}
