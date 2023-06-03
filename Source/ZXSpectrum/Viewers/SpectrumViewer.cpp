#include "SpectrumViewer.h"

#include <CodeAnalyser/CodeAnalyser.h>

#include <imgui.h>
#include "../SpectrumEmu.h"
#include "../SpectrumConstants.h"
#include <CodeAnalyser/UI/CodeAnalyserUI.h>
#include "GraphicsViewer.h"
#include "../GlobalConfig.h"

#include <Util/Misc.h>
#include <ImGuiSupport/ImGuiTexture.h>

void DrawArrow(ImDrawList* dl, ImVec2 pos, bool bLeftDirection);

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
	const FGlobalConfig& config = GetGlobalConfig();
	FCodeAnalysisState& codeAnalysis = pSpectrumEmu->CodeAnalysis;
	FDebugger& debugger = codeAnalysis.Debugger;
	FCodeAnalysisViewState& viewState = codeAnalysis.GetFocussedViewState();

	chips_display_info_t disp = zx_display_info(&pSpectrumEmu->ZXEmuState);

	// convert texture to RGBA
	const uint8_t* pix = (const uint8_t*)disp.frame.buffer.ptr;
	const uint32_t* pal = (const uint32_t*)disp.palette.ptr;
	for (int i = 0; i < disp.frame.buffer.size; i++)
		FrameBuffer[i] = pal[pix[i]];

	ImGui_UpdateTextureRGBA(ScreenTexture, FrameBuffer);

	const ImVec2 pos = ImGui::GetCursorScreenPos();
	//ImGui::Text("Instructions this frame: %d \t(max:%d)", instructionsThisFrame,maxInst);
	ImVec2 uv0(0, 0);
	ImVec2 uv1(320.0f / 512.0f, 1.0f);
	ImGui::Image(ScreenTexture, ImVec2(320, 256),uv0,uv1);

	ImDrawList* dl = ImGui::GetWindowDrawList();
	const int topScreenScanLine = pSpectrumEmu->ZXEmuState.top_border_scanlines - 32;

	const uint8_t* scanlineEvents = debugger.GetScanlineEvents();
	for (int scanlineNo = 0; scanlineNo < 320; scanlineNo++)
	{
		const int scanlineY = std::min(std::max(scanlineNo - topScreenScanLine, 0), 256);
		if (scanlineEvents[scanlineNo] != (int)EEventType::None)
		{
			const uint32_t col = debugger.GetEventColour(scanlineEvents[scanlineNo]);
			dl->AddLine(ImVec2(pos.x + 320, pos.y + scanlineY), ImVec2(pos.x + 320 + 32, pos.y + scanlineY), col);
		}
	}
	// Draw an indicator to show which scanline is being drawn
	if (config.bShowScanLineIndicator && pSpectrumEmu->UIZX.dbg.dbg.stopped)
	{
		// Compensate for the fact the border area on a real spectrum is bigger than the emulated spectrum.
		int scanlineY = std::min(std::max(pSpectrumEmu->ZXEmuState.scanline_y - topScreenScanLine, 0), 256);
		dl->AddLine(ImVec2(pos.x + 4, pos.y + scanlineY), ImVec2(pos.x + 320 - 8, pos.y + scanlineY), 0x50ffffff);
		DrawArrow(dl, ImVec2(pos.x - 2, pos.y + scanlineY - 6), false);
		DrawArrow(dl, ImVec2(pos.x + 320 - 11, pos.y + scanlineY - 6), true);
	}

	if (bShowCoordinates)
		DrawCoordinatePositions(codeAnalysis, pos);

	
	// draw hovered address
	if (viewState.HighlightAddress.IsValid())
	{
		ImDrawList* dl = ImGui::GetWindowDrawList();
		if (viewState.HighlightAddress.Address >= kScreenPixMemStart && viewState.HighlightAddress.Address <= kScreenPixMemEnd)	// pixel
		{
			int xp, yp;
			GetScreenAddressCoords(viewState.HighlightAddress.Address, xp, yp);

			const int rx = static_cast<int>(pos.x) + kBorderOffsetX + xp;
			const int ry = static_cast<int>(pos.y) + kBorderOffsetY + yp;
			dl->AddRect(ImVec2((float)rx, (float)ry), ImVec2((float)rx + 8, (float)ry + 1), 0xffffffff);	// TODO: flash?
		}

		if (viewState.HighlightAddress.Address >= kScreenAttrMemStart && viewState.HighlightAddress.Address <= kScreenAttrMemEnd)	// attributes
		{
			int xp, yp;
			GetAttribAddressCoords(viewState.HighlightAddress.Address, xp, yp);

			const int rx = static_cast<int>(pos.x) + kBorderOffsetX + xp;
			const int ry = static_cast<int>(pos.y) + kBorderOffsetY + yp;
			dl->AddRect(ImVec2((float)rx, (float)ry), ImVec2((float)rx + 8, (float)ry + 8), 0xffffffff);	// TODO: flash?
		}
	}

	bool bJustSelectedChar = false;

	if (ImGui::IsItemHovered())
	{
		bJustSelectedChar = OnHovered(pos, codeAnalysis, viewState);
	}

	if (bScreenCharSelected == true)
	{
		ImDrawList* dl = ImGui::GetWindowDrawList();
		const ImU32 col = 0xffffffff;	// TODO: pulse
		dl->AddRect(ImVec2((float)SelectedCharX, (float)SelectedCharY), ImVec2((float)SelectedCharX + 8, (float)SelectedCharY + 8), col);

		ImGui::Text("Pixel Char Address: %s", NumStr(SelectPixAddr));
		//ImGui::SameLine();
		DrawAddressLabel(codeAnalysis, viewState, SelectPixAddr);
		ImGui::Text("Attribute Address: %s", NumStr(SelectAttrAddr));
		//ImGui::SameLine();
		DrawAddressLabel(codeAnalysis, viewState, SelectAttrAddr);

		if (CharDataFound)
		{
			ImGui::Text("Found at: %s", NumStr(FoundCharDataAddress));
			DrawAddressLabel(codeAnalysis, viewState, FoundCharDataAddress);
			//ImGui::SameLine();
			bool bShowInGfxView = ImGui::Button("Show in GFX View");
			ImGui::SameLine();
			ImGui::Checkbox("Wrap", &bCharSearchWrap);
			ImGui::SameLine();
			if (ImGui::Button("Format as Bitmap"))
			{
				FDataFormattingOptions formattingOptions;
				formattingOptions.StartAddress = FoundCharDataAddress;
				formattingOptions.ItemSize = 1;
				formattingOptions.NoItems = 8;
				formattingOptions.DataType = EDataType::Bitmap;

				FormatData(codeAnalysis, formattingOptions);
				viewState.GoToAddress({ codeAnalysis.GetBankFromAddress(FoundCharDataAddress), FoundCharDataAddress }, false);
			}

			if (bShowInGfxView)
			{
				if (!bJustSelectedChar)
				{
					bool bFound = codeAnalysis.FindMemoryPattern(CharData, 8, FoundCharDataAddress+8, FoundCharDataAddress);
					
					// If we didn't find anything, then wraparound and look from the start of ram.
					if (!bFound && bCharSearchWrap)
						CharDataFound = codeAnalysis.FindMemoryPattern(CharData, 8, 0, FoundCharDataAddress);
				}

				pSpectrumEmu->GraphicsViewerGoToAddress(codeAnalysis.AddressRefFromPhysicalAddress(FoundCharDataAddress));
			}
		}
	}

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
				bool bContainsBits = false;

				// store pixel data for selected character
				for (int charLine = 0; charLine < 8; charLine++)
				{
					charData[charLine] = pSpectrumEmu->ReadByte(GetScreenPixMemoryAddress(xp, yp + charLine));
					if (charData[charLine] != 0)
						bContainsBits = true;
				}

				if (bContainsBits)
				{
					uint16_t foundCharDataAddress;

					if (codeAnalysis.FindMemoryPattern(charData, 8, 0x5800, foundCharDataAddress))
					{
						const FCodeInfo* pCodeInfo = codeAnalysis.GetCodeInfoForAddress(foundCharDataAddress);
						if (pCodeInfo == nullptr || pCodeInfo->bDisabled)
						{
							FDataFormattingOptions formattingOptions;
							formattingOptions.StartAddress = foundCharDataAddress;
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
	
	bWindowFocused = ImGui::IsWindowFocused();
}

void FSpectrumViewer::DrawCoordinatePositions(FCodeAnalysisState& codeAnalysis, const ImVec2& pos)
{
	// draw coordinate position
	if (XCoordAddress.IsValid())
	{
		ImDrawList* dl = ImGui::GetWindowDrawList();
		const uint8_t xVal = codeAnalysis.ReadByte(XCoordAddress);
		const float xLinePos = (float)(bInvertXCoord ? 255 - xVal : xVal);
		const ImVec2 lineStart(pos.x + kBorderOffsetX + xLinePos, pos.y + kBorderOffsetY);
		const ImVec2 lineEnd(pos.x + kBorderOffsetX + xLinePos, pos.y + kBorderOffsetY + 192.0f);

		dl->AddLine(lineStart, lineEnd, 0xffffffff);
	}
	if (YCoordAddress.IsValid())
	{
		ImDrawList* dl = ImGui::GetWindowDrawList();
		const uint8_t yVal = codeAnalysis.ReadByte(YCoordAddress);
		const float yLinePos = (float)(bInvertYCoord ? 192 - yVal : yVal);
		const ImVec2 lineStart(pos.x + kBorderOffsetX, pos.y + kBorderOffsetY + yLinePos);
		const ImVec2 lineEnd(pos.x + kBorderOffsetX + 256.0f, pos.y + kBorderOffsetY + yLinePos);

		dl->AddLine(lineStart, lineEnd, 0xffffffff);
	}
}

bool FSpectrumViewer::OnHovered(const ImVec2& pos, FCodeAnalysisState& codeAnalysis, FCodeAnalysisViewState& viewState)
{
	bool bJustSelectedChar = false;
	
	ImGuiIO& io = ImGui::GetIO();
	const int xp = std::min(std::max((int)(io.MousePos.x - pos.x - kBorderOffsetX), 0), 255);
	const int yp = std::min(std::max((int)(io.MousePos.y - pos.y - kBorderOffsetY), 0), 191);

	const uint16_t scrPixAddress = GetScreenPixMemoryAddress(xp, yp);
	const uint16_t scrAttrAddress = GetScreenAttrMemoryAddress(xp, yp);

	if (scrPixAddress != 0)
	{
		ImDrawList* dl = ImGui::GetWindowDrawList();
		const int rx = static_cast<int>(pos.x) + kBorderOffsetX + (xp & ~0x7);
		const int ry = static_cast<int>(pos.y) + kBorderOffsetY + (yp & ~0x7);
		dl->AddRect(ImVec2((float)rx, (float)ry), ImVec2((float)rx + 8, (float)ry + 8), 0xffffffff);
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
			SelectPixAddr = scrPixAddress;
			SelectAttrAddr = scrAttrAddress;

			// store pixel data for selected character
			for (int charLine = 0; charLine < 8; charLine++)
				CharData[charLine] = pSpectrumEmu->ReadByte(GetScreenPixMemoryAddress(xp & ~0x7, (yp & ~0x7) + charLine));
			CharDataFound = codeAnalysis.FindMemoryPattern(CharData, 8, 0, FoundCharDataAddress);
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
	for (ImGuiKey key = ImGuiKey_NamedKey_BEGIN; key < ImGuiKey_COUNT; key++)
	{
		if (ImGui::IsKeyPressed(key,false))
		{ 
			const int speccyKey = SpectrumKeyFromImGuiKey(key);
			if (speccyKey != 0 && bWindowFocused)
				zx_key_down(&pSpectrumEmu->ZXEmuState, speccyKey);
		}
		else if (ImGui::IsKeyReleased(key))
		{
			const int speccyKey = SpectrumKeyFromImGuiKey(key);
			if (speccyKey != 0)
				zx_key_up(&pSpectrumEmu->ZXEmuState, speccyKey);
		}
	}

	// Gamepad support, can use ImGuiKey values here
	if (zx_joystick_type(&pSpectrumEmu->ZXEmuState) != ZX_JOYSTICKTYPE_NONE)
	{
		int mask = 0;
		if (ImGui::IsKeyDown(ImGuiNavInput_DpadRight))
			mask |= 1 << 0;
		if (ImGui::IsKeyDown(ImGuiNavInput_DpadLeft))
			mask |= 1 << 1;
		if (ImGui::IsKeyDown(ImGuiNavInput_DpadDown))
			mask |= 1 << 2;
		if (ImGui::IsKeyDown(ImGuiNavInput_DpadUp))
			mask |= 1 << 3;
		if (ImGui::IsKeyDown(ImGuiNavInput_Activate))
			mask |= 1 << 4;

		zx_joystick(&pSpectrumEmu->ZXEmuState, mask);
	}
}

void DrawArrow(ImDrawList* dl, ImVec2 pos, bool bLeftDirection)
{
	ImVec2 arrowPos = ImVec2(pos.x, pos.y);
	const float h = 13.f;
	float r = h * 0.40f;
	ImVec2 center = ImVec2(arrowPos.x + h * 0.5f, arrowPos.y + h * 0.5f);
	ImVec2 a, b, c;
	if (bLeftDirection) r = -r;
	a = ImVec2(+0.750f * r, +0.000f * r);
	b = ImVec2(-0.750f * r, +0.866f * r);
	c = ImVec2(-0.750f * r, -0.866f * r);
	dl->AddTriangleFilled(ImVec2(center.x + a.x, center.y + a.y), ImVec2(center.x + b.x, center.y + b.y), ImVec2(center.x + c.x, center.y + c.y), 0xffffffff);
}