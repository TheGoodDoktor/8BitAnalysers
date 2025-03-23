#include "CodeAnalyserUI.h"
#include "../CodeAnalyser.h"
#include "CodeAnalyser/DataTypes.h"
#include "CharacterMapViewer.h"

#include "Util/Misc.h"
#include "Util/GraphicsView.h"
#include "ImageViewer.h"

#include "imgui.h"
#include "misc/cpp/imgui_stdlib.h"
#include "UIColours.h"
#include "ComboBoxes.h"
#include <ImGuiSupport/ImGuiScaling.h>
#include "Z80/DataItemZ80.h"

float DrawDataCharMapLine(FCodeAnalysisState& state, FCodeAnalysisViewState& viewState, FAddressRef addr, const FDataInfo* pDataInfo)
{
	const float line_height = ImGui::GetTextLineHeight();
	const float rectSize = line_height + 4;
	ImDrawList* dl = ImGui::GetWindowDrawList();
	ImVec2 pos = ImGui::GetCursorScreenPos();
	pos.x += state.Config.AddressPos + state.Config.AddressSpace + 100.0f;// 150.0f;
	const float startPos = pos.x;
	pos.y -= rectSize + 2;

	FAddressRef charAddress = addr;

	const FCharacterSet* pCharSet = GetCharacterSetFromAddress(pDataInfo->CharSetAddress);

	for (int byte = 0; byte < pDataInfo->ByteSize; byte++)
	{
		const uint8_t val = state.ReadByte(addr);
		if (val != pDataInfo->EmptyCharNo)	// skip empty chars
		{
			const ImVec2 rectMin(pos.x, pos.y);
			const ImVec2 rectMax(pos.x + rectSize, pos.y + rectSize);

			if(pCharSet)
			{ 
				// charmap rendering
				const FCharUVS UVS = pCharSet->GetCharacterUVS(val);
				dl->AddImage((ImTextureID)pCharSet->Image->GetTexture(), rectMin, rectMax, ImVec2(UVS.U0, UVS.V0), ImVec2(UVS.U1, UVS.V1));
			}
			else
			{
				char valTxt[8];
				snprintf(valTxt,8, "%02x", val);
				dl->AddRect(rectMin, rectMax, 0xffffffff);
				dl->AddText(ImVec2(pos.x + 1, pos.y + 1), 0xffffffff, valTxt);
			}
			
		}

		if(addr == viewState.HighlightAddress)
		{
			dl->AddRect(ImVec2(pos.x, pos.y), ImVec2(pos.x + rectSize, pos.y + rectSize) , Colours::GetFlashColour());
		}

		state.AdvanceAddressRef(addr, 1);	// advance by one byte
		pos.x += rectSize;
	}

	// convert from screen space to local space
	pos.x -= ImGui::GetWindowPos().x;

	ImGui::SameLine(pos.x);
	ImGui::Text(" ");
	return pos.x;

}



// returns how much space it took
float DrawDataBitmapLine(FCodeAnalysisState& state, uint16_t addr, const FDataInfo* pDataInfo, bool bEditMode)
{
	const float line_height = ImGui::GetTextLineHeight();
	float rectSize = line_height + 4;
	ImDrawList* dl = ImGui::GetWindowDrawList();
	ImVec2 pos = ImGui::GetCursorScreenPos();
	pos.x += state.Config.AddressPos + state.Config.AddressSpace + 100.0f;
	pos.y -= rectSize + 2;
	const ImVec2 startPos = pos;

	const float areaWidth = ImGui::GetWindowWidth() - startPos.x;
	const float itemWidth = pDataInfo->ByteSize * 8 * rectSize;

	/*if (itemWidth > areaWidth)
	{
		rectSize *= areaWidth / itemWidth;
	}*/

	switch (pDataInfo->DisplayType)
	{
	case EDataItemDisplayType::Bitmap:	// 1 bit per pixel
		for (int byte = 0; byte < pDataInfo->ByteSize; byte++)
		{
			const uint8_t val = state.ReadByte(addr + byte);

			for (int bit = 7; bit >= 0; bit--)
			{
				const ImVec2 rectMin(pos.x, pos.y);
				const ImVec2 rectMax(pos.x + rectSize, pos.y + rectSize);
				if (val & (1 << bit))
					dl->AddRectFilled(rectMin, rectMax, 0xffffffff);
				else if (rectSize > 4)
					dl->AddRect(rectMin, rectMax, 0xffffffff);

				pos.x += rectSize;
			}
		}
		break;
	case EDataItemDisplayType::ColMap2Bpp_CPC:
		// CPC Mode 1.
		// 2 bits per pixel. Each pixel value is a lookup into a palette of 4 colours.
		// http://www.cpcmania.com/Docs/Programming/Painting_pixels_introduction_to_video_memory.htm

		for (int byte = 0; byte < pDataInfo->ByteSize; byte++)
		{
			const uint8_t val = state.ReadByte(addr + byte);

			for (int pixel = 0; pixel < 4; pixel++)
			{
				const ImVec2 rectMin(pos.x, pos.y);
				const ImVec2 rectMax(pos.x + rectSize, pos.y + rectSize);

				int colourIndex = 0;

				switch (pixel)
				{
				case 0:
					colourIndex = (val & 0x8 ? 2 : 0) | (val & 0x80 ? 1 : 0);
					break;
				case 1:
					colourIndex = (val & 0x4 ? 2 : 0) | (val & 0x40 ? 1 : 0);
					break;
				case 2:
					colourIndex = (val & 0x2 ? 2 : 0) | (val & 0x20 ? 1 : 0);
					break;
				case 3:
					colourIndex = (val & 0x1 ? 2 : 0) | (val & 0x10 ? 1 : 0);
					break;
				}

				const uint32_t* pPalette = GetPaletteFromPaletteNo(pDataInfo->PaletteNo);
				const uint32_t pixelCol = pPalette ? pPalette[colourIndex] : colourIndex == 0 ? 0 : 0xffffffff;

				dl->AddRectFilled(rectMin, rectMax, pixelCol);
				dl->AddRect(rectMin, rectMax, 0xffffffff);

				pos.x += rectSize;
			}
		}
		break;
	case EDataItemDisplayType::ColMap4Bpp_CPC:
		// CPC Mode 0.
		// 4 bits per pixel. Each pixel value is a lookup into a palette of 16 colours.
		// Pixels are double width.

		for (int byte = 0; byte < pDataInfo->ByteSize; byte++)
		{
			const uint8_t val = state.ReadByte(addr + byte);

			for (int pixel = 0; pixel < 2; pixel++)
			{
				const ImVec2 rectMin(pos.x, pos.y);
				const ImVec2 rectMax(pos.x + rectSize * 2.0f, pos.y + rectSize);

				int colourIndex = 0;

				switch (pixel)
				{
				case 0:
					colourIndex = (val & 0x80 ? 1 : 0) | (val & 0x8 ? 2 : 0) | (val & 0x20 ? 4 : 0) | (val & 0x2 ? 8 : 0);
					break;
				case 1:
					colourIndex = (val & 0x40 ? 1 : 0) | (val & 0x4 ? 2 : 0) | (val & 0x10 ? 4 : 0) | (val & 0x1 ? 8 : 0);
					break;
				}

				const uint32_t* pPalette = GetPaletteFromPaletteNo(pDataInfo->PaletteNo);
				const uint32_t pixelCol = pPalette ? pPalette[colourIndex] : colourIndex == 0 ? 0 : 0xffffffff;

				dl->AddRectFilled(rectMin, rectMax, pixelCol);
				dl->AddRect(rectMin, rectMax, 0xffffffff);

				pos.x += rectSize * 2.0f;
			}
		}
		break;

	case EDataItemDisplayType::ColMapMulticolour_C64:	// 2 bits per pixel - wide pixels
		for (int byte = 0; byte < pDataInfo->ByteSize; byte++)
		{
			const uint8_t val = state.ReadByte(addr + byte);

			for (int xpix = 0; xpix < 4; xpix++)
			{
				const uint8_t colNo = (val >> (6 - (xpix * 2))) & 3;

				const ImVec2 rectMin(pos.x, pos.y);
				const ImVec2 rectMax(pos.x + rectSize * 2, pos.y + rectSize);

				const uint32_t* pPalette = GetPaletteFromPaletteNo(pDataInfo->PaletteNo);
				const uint32_t pixelCol = pPalette ? pPalette[colNo] : colNo == 0 ? 0: 0xffffffff;

				dl->AddRectFilled(rectMin, rectMax, pixelCol);
				dl->AddRect(rectMin, rectMax, 0xffffffff);	// outline - needed? option?

				pos.x += rectSize * 2;
			}
		}
		break;
    default:
        break;
	}

	// Edit bits
	if (pDataInfo->DisplayType == EDataItemDisplayType::Bitmap)
	{
		if (bEditMode && ImGui::IsMouseClicked(0))
		{
			ImGuiIO& io = ImGui::GetIO();
			const int xp = (int)(io.MousePos.x - startPos.x);
			const int yp = (int)(io.MousePos.y - startPos.y);
			const int itemWidth = pDataInfo->ByteSize * 8 * (int)rectSize;

			if (xp >= 0 && yp >= 0 && xp < itemWidth && yp < rectSize)
			{
				const int squareNo = xp / (int)rectSize;
				const int byteNo = squareNo / 8;
				const int bitNo = squareNo & 7;
				const ImVec2 rectMin(startPos.x + (squareNo * rectSize), startPos.y);
				const ImVec2 rectMax(startPos.x + rectSize, startPos.y + rectSize);
				dl->AddRect(rectMin, rectMax, 0xffff00ff);

				uint8_t val = state.ReadByte(addr + byteNo);
				val = val ^ (1 << (7 - bitNo));
				state.WriteByte(addr + byteNo, val);
			}
		}
	}

	// convert from screen space to local space
	pos.x -= ImGui::GetWindowPos().x;

	//ImGui::SetCursorScreenPos(pos);
	ImGui::SameLine(pos.x);
	ImGui::Text(" ");
	//return pos.x - startPos;
	return pos.x;
}

// Display a colour attribute
// This is written for ZX Spectrum colour attributes which goes against the platform independent nature of this file
// Some kind of abstraction is needed



// returns how much space it took
float DrawColAttr(FCodeAnalysisState& state, uint16_t addr,const FDataInfo* pDataInfo, bool bEditMode)
{
	const float scale = ImGui_GetScaling();
	const float line_height = ImGui::GetTextLineHeight();
	const float rectSize = line_height + 4;
	ImDrawList* dl = ImGui::GetWindowDrawList();
	ImVec2 pos = ImGui::GetCursorScreenPos();
	pos.x += 120.0f * scale;
	pos.y -= rectSize + 2;
	const ImVec2 startPos = pos;
	const uint32_t* colourLUT = state.Config.CharacterColourLUT;
	
	for (int byte = 0; byte < pDataInfo->ByteSize; byte++)
	{
		uint8_t colAttr = state.ReadByte(addr + byte);
		const bool bBright = !!(colAttr & (1 << 6));
		const uint32_t inkCol = GetColFromAttr(colAttr & 7, colourLUT,bBright);
		const uint32_t paperCol = GetColFromAttr((colAttr >> 3) & 7, colourLUT, bBright);

		// Ink
		{
			const ImVec2 rectMin(pos.x, pos.y);
			const ImVec2 rectMax(pos.x + rectSize, pos.y + rectSize / 2);
			dl->AddRectFilled(rectMin, rectMax, inkCol);
		}

		// Paper
		{
			const ImVec2 rectMin(pos.x, pos.y + rectSize / 2);
			const ImVec2 rectMax(pos.x + rectSize, pos.y + rectSize);
			dl->AddRectFilled(rectMin, rectMax, paperCol);
		}

		
		pos.x += rectSize;
	}

	// convert from screen space to local space
	pos.x -= ImGui::GetWindowPos().x;

	ImGui::SameLine(pos.x);
	ImGui::Text(" ");

	return pos.x;
}

int DataItemEditCallback(ImGuiInputTextCallbackData* pData)
{
	return 1;
}

void EditByteDataItem(FCodeAnalysisState& state, uint16_t address)
{
	const ENumberDisplayMode numMode = GetNumberDisplayMode();
	uint8_t val = state.ReadByte(address);
	int flags = ImGuiInputTextFlags_EnterReturnsTrue;
	const float glyphWidth = ImGui::CalcTextSize("0").x;
	float width = glyphWidth * 2.0f;
	const char* format = "%02X";

	switch (numMode)
	{
	case ENumberDisplayMode::Decimal:
		width += glyphWidth;
		format = "%d";
		break;
	case ENumberDisplayMode::HexAitch:
	case ENumberDisplayMode::HexDollar:
		flags |= ImGuiInputTextFlags_CharsHexadecimal;
		break;
    default:
        break;
	}

	ImGui::PushID(address);
	if (numMode == ENumberDisplayMode::HexDollar)
	{
		ImGui::Text("$");
		ImGui::SameLine(0,0);
	}
	ImGui::SetNextItemWidth((float)width);
	ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(0, 0));
	ImGui::SetItemAllowOverlap();	// allow controls
	
	if (ImGui::InputScalar("##dbinput", ImGuiDataType_U8, &val, NULL, NULL, format, flags))
	{
		// Write value
		state.CPUInterface->WriteByte(address, val);
	}
	if (numMode == ENumberDisplayMode::HexAitch)
	{
		ImGui::SameLine(0,0);
		ImGui::Text("h");
	}
	ImGui::PopStyleVar();	

	ImGui::PopID();
}

void EditWordDataItem(FCodeAnalysisState& state, uint16_t address)
{
	const ENumberDisplayMode numMode = GetNumberDisplayMode();
	uint16_t val = state.CPUInterface->ReadWord(address);
	int flags = ImGuiInputTextFlags_EnterReturnsTrue;
	const float glyphWidth = ImGui::CalcTextSize("0").x;
	float width = glyphWidth * 4.0f;
	const char* format = "%04X";

	switch (numMode)
	{
	case ENumberDisplayMode::Decimal:
		width += glyphWidth;
		format = "%d";
		break;
	case ENumberDisplayMode::HexAitch:
	case ENumberDisplayMode::HexDollar:
		flags |= ImGuiInputTextFlags_CharsHexadecimal;
		break;
    default:
        break;
    }

	ImGui::PushID(address);
	if (numMode == ENumberDisplayMode::HexDollar)
	{
		ImGui::Text("$");
		ImGui::SameLine(0, 0);
	}
	ImGui::SetNextItemWidth((float)width);
	ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(0, 0));
	ImGui::SetItemAllowOverlap();	// allow controls
	if (ImGui::InputScalar("##dwinput", ImGuiDataType_U16, &val, NULL, NULL, format, flags))
	{
		// Write value
		state.CPUInterface->WriteByte(address, val & 255);
		state.CPUInterface->WriteByte(address + 1, val >> 8);
	}
	if (numMode == ENumberDisplayMode::HexAitch)
	{
		ImGui::SameLine(0, 0);
		ImGui::Text("h");
	}
	ImGui::PopStyleVar();

	ImGui::PopID();
}

void ShowDataItemActivity(FCodeAnalysisState& state, FAddressRef addr)
{
	const FDataInfo* pDataInfo = state.GetDataInfoForAddress(addr);
	//const FDataInfo* pWriteDataInfo = state.GetWriteDataInfoForAddress(addr);
	const int framesSinceWritten = pDataInfo->LastFrameWritten == -1 ? 255 : state.CurrentFrameNo - pDataInfo->LastFrameWritten;
	const int framesSinceRead = pDataInfo->LastFrameRead == -1 ? 255 : state.CurrentFrameNo - pDataInfo->LastFrameRead;
	const int wBrightVal = (255 - std::min(framesSinceWritten << 2, 255)) & 0xff;
	const int rBrightVal = (255 - std::min(framesSinceRead << 2, 255)) & 0xff;
	float offset = 0;

	if (rBrightVal > 0 || wBrightVal > 0)
	{
		const float lineHeight = ImGui::GetTextLineHeight();
		const ImU32 pc_color = 0xFF00FFFF;
		const ImU32 brd_color = 0xFF000000;

		ImVec2 pos = ImGui::GetCursorScreenPos();
		ImDrawList* dl = ImGui::GetWindowDrawList();
		const float lh2 = (float)(int)(lineHeight / 2);

		if (wBrightVal > 0)
		{
			const ImVec2 a(pos.x + 2, pos.y);
			const ImVec2 b(pos.x + 12, pos.y + lh2);
			const ImVec2 c(pos.x + 2, pos.y + lineHeight);

			const ImU32 col = 0xff000000 | (wBrightVal << 0);
			dl->AddTriangleFilled(a, b, c, col);
			dl->AddTriangle(a, b, c, brd_color);
		}

		pos.x += 10;


		if (rBrightVal > 0)
		{
			const ImVec2 a(pos.x + 2, pos.y);
			const ImVec2 b(pos.x + 12, pos.y + lh2);
			const ImVec2 c(pos.x + 2, pos.y + lineHeight);

			const ImU32 col = 0xff000000 | (rBrightVal << 8);
			dl->AddTriangleFilled(a, b, c, col);
			dl->AddTriangle(a, b, c, brd_color);
		}
	}
}

void DrawDataInfo(FCodeAnalysisState& state, FCodeAnalysisViewState& viewState, const FCodeAnalysisItem& item, bool bDrawLabel, bool bEdit)
{
	const FDataInfo* const pDataInfo = static_cast<const FDataInfo*>(item.Item);
	const float line_height = ImGui::GetTextLineHeight();
	const float glyph_width = ImGui::CalcTextSize("F").x;
	const float cell_width = 3 * glyph_width;
	const uint16_t physAddr = item.AddressRef.Address;
	const bool bHighlight = (viewState.HighlightAddress.IsValid() && viewState.HighlightAddress.Address >= physAddr && viewState.HighlightAddress.Address < physAddr + item.Item->ByteSize);
	const FDebugger& debugger = state.Debugger;

	ShowDataItemActivity(state, item.AddressRef);
	ShowCodeAccessorActivity(state, item.AddressRef);	// only really for PC - shortcut?


	// show if breakpointed
	const FBreakpoint* pBP = debugger.GetBreakpointForAddress(item.AddressRef);
	if (pBP != nullptr)
	{
		const ImU32 bp_enabled_color = 0xFF0000FF;
		const ImU32 bp_disabled_color = 0xFF000088;
		const ImU32 brd_color = 0xFF000000;
		ImDrawList* dl = ImGui::GetWindowDrawList();
		const ImVec2 pos = ImGui::GetCursorScreenPos();
		const float lh2 = (float)(int)(line_height / 2);
		const ImVec2 mid(pos.x + 7, pos.y + lh2);

		dl->AddCircleFilled(mid, 7, pBP->bEnabled ? bp_enabled_color : bp_disabled_color);
		dl->AddCircle(mid, 7, brd_color);
	}

	const float lineStartX = ImGui::GetCursorPosX();
	ImGui::SameLine(lineStartX + state.Config.AddressPos);
	ImGui::PushStyleColor(ImGuiCol_Text, bHighlight ? Colours::highlight : Colours::address);
	ImGui::Text("%s", NumStr(item.AddressRef.Address));
	ImGui::PopStyleColor();

	ENumberDisplayMode trueNumberDisplayMode = GetNumberDisplayMode();
	bool bShowItemLabel = false;
	bool bShowASCII = false;

	uint32_t valueColour = Colours::defaultValue;

	switch (pDataInfo->DisplayType)
	{
	case EDataItemDisplayType::Pointer:
	case EDataItemDisplayType::JumpAddress:
		bShowItemLabel = true;
		break;
	case EDataItemDisplayType::Decimal:
		SetNumberDisplayMode(ENumberDisplayMode::Decimal);
		break;
	case EDataItemDisplayType::Binary:
		SetNumberDisplayMode(ENumberDisplayMode::Binary);
		break;
	case EDataItemDisplayType::Hex:
		SetNumberDisplayMode(GetHexNumberDisplayMode());
		break;
	case EDataItemDisplayType::SignedNumber:
	case EDataItemDisplayType::UnsignedNumber:	// these should use the number display mode
		break;
	case EDataItemDisplayType::Ascii:
		bShowASCII = true;
		break;
	case EDataItemDisplayType::Unknown:

		if (pDataInfo->Writes.IsEmpty() == false)
			valueColour = Colours::unknownDataWrite;
		else if(pDataInfo->Reads.IsEmpty() == false)
			valueColour = Colours::unknownDataRead;
		else
			valueColour = Colours::unknownValue;
		bShowASCII = true;
		break;
    default:
        break;
	}

	ImGui::PushStyleColor(ImGuiCol_Text, valueColour);

	const float line_start_x = ImGui::GetCursorPosX();
	float offset = 0;

	if (bDrawLabel)
	{
		DrawAddressLabel(state, viewState, item.AddressRef);
		ImGui::SameLine(line_start_x + cell_width * 12 + glyph_width * 2);	// TODO
		ImGui::Text(":");
		ImGui::SameLine();
	}
	else
	{
		ImGui::SameLine(lineStartX + state.Config.AddressPos + state.Config.AddressSpace);
	}

	switch (pDataInfo->DataType)
	{
	case EDataType::Byte:
	case EDataType::InstructionOperand:
	{
		const uint8_t val = state.ReadByte(item.AddressRef);
		ImGui::Text("db");
		ImGui::SameLine();

		if (bEdit)
			EditByteDataItem(state, physAddr);
		else
			ImGui::Text("%s", NumStr(val));

		if(bShowASCII)
		{
			ImGui::SameLine();
			if (val == '\n')// carriage return messes up list
				ImGui::Text("'<cr>'");
			else
				ImGui::Text("'%c'", val);
		}
	}
	break;

	case EDataType::ByteArray:
	{
		uint8_t val = state.ReadByte(item.AddressRef);

		// TODO: add edit support

		ImGui::Text("db", NumStr(val));
		ImGui::SameLine();
		if (bShowASCII)
		{
			if (val == '\n')// carriage return messes up list
				ImGui::Text("'<cr>'");
			else
				ImGui::Text("'%c'", val);
		}
		else
		{
			ImGui::Text("%s", NumStr(val));
		}

		for (int i = 1; i < pDataInfo->ByteSize; i++)	// first word already written
		{
			val = state.ReadByte(physAddr + i);
			ImGui::SameLine(0,0);
			ImGui::Text(",", NumStr(val));
			ImGui::SameLine(0,0);

			if (bShowASCII)
			{
				if (val == '\n')// carriage return messes up list
					ImGui::Text("'<cr>'");
				else
					ImGui::Text("'%c'", val);
			}
			else
			{
				ImGui::Text("%s", NumStr(val));
			}
		}
	}
	break;

	case EDataType::Word:
	{
		const uint16_t val = state.ReadWord(physAddr);
		ImGui::Text("dw");
		ImGui::SameLine();
		if (bEdit)
			EditWordDataItem(state, physAddr);
		else
			ImGui::Text("%s ", NumStr(val));

		if (bShowItemLabel)
		{
			ImGui::SameLine();
			DrawAddressLabel(state, viewState, val);
		}
	}
	break;

	case EDataType::WordArray:
	{
		uint16_t val = state.ReadWord(physAddr);
		const int wordCount = pDataInfo->ByteSize / 2;

		// TODO: add edit support

		ImGui::Text("dw %s", NumStr(val));
		for (int i = 1; i < wordCount; i++)	// first word already written
		{
			val = state.ReadWord(physAddr + (i * 2));
			ImGui::SameLine();
			ImGui::Text(",%s", NumStr(val));
		}
	}
	break;

	case EDataType::Text:
	{
		const std::string textString = GetItemText(state, item.AddressRef);
		ImGui::PushStyleColor(ImGuiCol_Text, Colours::defaultValue);
		ImGui::Text("ascii"); 
		ImGui::PopStyleColor();
		ImGui::SameLine();
		ImGui::PushStyleColor(ImGuiCol_Text, Colours::text);
		ImGui::Text("'%s'", textString.c_str());
		ImGui::PopStyleColor();
	}
	break;

	case EDataType::Bitmap:
		ImGui::PushStyleColor(ImGuiCol_Text, Colours::defaultValue);
		ImGui::Text("Bitmap");
		ImGui::PopStyleColor();
		offset = DrawDataBitmapLine(state, physAddr, pDataInfo, state.bAllowEditing);
		break;
	case EDataType::CharacterMap:
		ImGui::PushStyleColor(ImGuiCol_Text, Colours::defaultValue);
		ImGui::Text("Charmap");
		ImGui::PopStyleColor();
		offset = DrawDataCharMapLine(state,viewState, item.AddressRef, pDataInfo);
		break;
	case EDataType::ColAttr:
		ImGui::PushStyleColor(ImGuiCol_Text, Colours::defaultValue);
		ImGui::Text("ColAttr");
		ImGui::PopStyleColor();
		offset = DrawColAttr(state, physAddr, pDataInfo, state.bAllowEditing);
		break;
	case EDataType::ScreenPixels:
	case EDataType::Blob:
	default:
		ImGui::Text("%d Bytes", pDataInfo->ByteSize);
		break;
	}

	ImGui::PopStyleColor();

	// show struct info if struct member
	if (pDataInfo->bStructMember)
	{
		state.GetDataTypes()->DrawStructMember(pDataInfo->SubTypeId, pDataInfo->StructByteOffset, false);
	}

	if (state.CPUInterface->CPUType == ECPUType::Z80)
	{
		if (DrawDataItemRegisterPtrsZ80(state, physAddr, pDataInfo->ByteSize))
		{
			offset = 0.f;
		}
	}
	else
	{
		if (state.CPUInterface->GetSP() == physAddr)
		{
			ImGui::SameLine();
			ImGui::Text("<- SP");
			offset = 0.f;
		}
	}

	SetNumberDisplayMode(trueNumberDisplayMode);

	DrawComment(state, viewState, pDataInfo, offset);
}

struct FDataValueGraphState
{
	float	Min = FLT_MAX;
	float	Max = 0;
	float	Values[90] = { 0 };
};

std::map<uint32_t, FDataValueGraphState>	g_DataValueGraphs;

void DrawDataValueGraph(FCodeAnalysisState& state, FAddressRef addressRef, float val)
{
	FDataValueGraphState& graphState = g_DataValueGraphs[addressRef.Val];
	graphState.Min = std::min(graphState.Min, val);
	graphState.Max = std::max(graphState.Max, val);
	// Create a dummy array of contiguous float values to plot
		// Tip: If your float aren't contiguous but part of a structure, you can pass a pointer to your first float and the sizeof() of your structure in the Stride parameter.
	static bool animate = true;
	//static float values[90] = { 0 };
	static int values_offset = 0;
	static double refresh_time = 0.0;
	if (!animate || refresh_time == 0.0)
		refresh_time = ImGui::GetTime();

	while (refresh_time < ImGui::GetTime()) // Create dummy data at fixed 60 hz rate for the demo
	{
		static float phase = 0.0f;
		graphState.Values[values_offset] = val;
		values_offset = (values_offset + 1) % IM_ARRAYSIZE(graphState.Values);
		phase += 0.10f * values_offset;
		refresh_time += 1.0f / 60.0f;
	}

	// Plots can display overlay texts
	// (in this example, we will display an average value)
	{
		float average = 0.0f;
		for (int n = 0; n < IM_ARRAYSIZE(graphState.Values); n++)
			average += graphState.Values[n];
		average /= (float)IM_ARRAYSIZE(graphState.Values);
		char overlay[32];
		snprintf(overlay,32, "avg %f", average);
		ImGui::PlotLines("Lines", graphState.Values, IM_ARRAYSIZE(graphState.Values), values_offset, overlay, graphState.Min, graphState.Max, ImVec2(0, 80));
	}
	//ImGui::PlotHistogram("Histogram", graphState.Values, IM_ARRAYSIZE(graphState.Values), 0, NULL, graphState.Min, graphState.Max, ImVec2(0, 80));
}

void DrawDataValueGraphByte(FCodeAnalysisState& state, FAddressRef addressRef)
{
	const uint8_t val = state.ReadByte(addressRef);

	DrawDataValueGraph(state,addressRef,static_cast<float>(val));
}

void DrawDataValueGraphWord(FCodeAnalysisState& state, FAddressRef addressRef)
{
	const uint16_t val = state.ReadWord(addressRef);
	DrawDataValueGraph(state, addressRef, static_cast<float>(val));
}


void DrawDataAccesses(FCodeAnalysisState& state, FCodeAnalysisViewState& viewState, const FDataInfo* pDataInfo)
{
	// List Data accesses
	if (pDataInfo->Reads.IsEmpty() == false)
	{
		static std::string commentTxt;
		static bool bOverride = false;
		bool bWriteComment = false;

		ImGui::Separator();

		// comment all reads
		ImGui::InputText("Read Comment", &commentTxt);
		ImGui::Checkbox("Override Existing", &bOverride);
		ImGui::SameLine();
		if (ImGui::Button("Set all"))
		{
			bWriteComment = true;
		}

		ImGui::Text("Reads:");
		for (const auto& reader : pDataInfo->Reads.GetReferences())
		{
			ShowCodeAccessorActivity(state, reader);

			ImGui::Text("   ");
			ImGui::SameLine();
			DrawCodeAddress(state, viewState, reader);

			if (bWriteComment)
			{
				FCodeInfo* pCodeInfo = state.GetCodeInfoForAddress(reader);
				if (pCodeInfo)
				{
					if (pCodeInfo->Comment.empty() || bOverride)
						pCodeInfo->Comment = commentTxt;
				}
			}
		}
	}

	if (pDataInfo->Writes.IsEmpty() == false)
	{
		static std::string commentTxt;
		static bool bOverride = false;
		bool bWriteComment = false;

		ImGui::Separator();

		// comment all reads
		ImGui::InputText("Write Comment", &commentTxt);
		ImGui::Checkbox("Override Existing", &bOverride);
		ImGui::SameLine();
		if (ImGui::Button("Set all"))
		{
			bWriteComment = true;
		}

		ImGui::Text("Writes:");
		for (const auto& writer : pDataInfo->Writes.GetReferences())
		{
			ShowCodeAccessorActivity(state, writer);

			ImGui::Text("   ");
			ImGui::SameLine();
			DrawCodeAddress(state, viewState, writer);

			if (bWriteComment)
			{
				FCodeInfo* pCodeInfo = state.GetCodeInfoForAddress(writer);
				if (pCodeInfo)
				{
					if (pCodeInfo->Comment.empty() || bOverride)
						pCodeInfo->Comment = commentTxt;
				}
			}
		}
	}

	// last writer to address
	const FAddressRef lastWriter = pDataInfo->LastWriter;
	if (lastWriter.IsValid())
	{
		ImGui::Text("Last Writer: ");
		ImGui::SameLine();
		DrawCodeAddress(state, viewState, lastWriter);
	}
}


void DrawDataDetails(FCodeAnalysisState& state, FCodeAnalysisViewState& viewState, const FCodeAnalysisItem& item)
{
	FDataInfo* pDataInfo = static_cast<FDataInfo*>(item.Item);
	const uint16_t physAddr = item.AddressRef.Address;
	ImGui::Text("Display Mode:");
	ImGui::SameLine();
	ImGui::SetNextItemWidth(120.0f);
	DrawDataDisplayTypeCombo("##dataOperand",pDataInfo->DisplayType,state);
	switch (pDataInfo->DataType)
	{
	case EDataType::Byte:
		DrawDataValueGraphByte(state, item.AddressRef);
		break;

	case EDataType::Word:
		DrawDataValueGraphWord(state, item.AddressRef);
		break;

	case EDataType::Text:
	{
		std::string textString;
		for (int i = 0; i < pDataInfo->ByteSize; i++)
		{
			const char ch = state.ReadByte(physAddr + i);
			if (ch == '\n')
				textString += "<cr>";
			if (pDataInfo->bBit7Terminator && ch & (1 << 7))	// check bit 7 terminator flag
				textString += ch & ~(1 << 7);	// remove bit 7
			else
				textString += ch;
		}
		ImGui::TextWrapped("%s", textString.c_str());
		int length = pDataInfo->ByteSize;
		if (ImGui::InputInt("Length", &length))
		{
			pDataInfo->ByteSize = length;
			state.SetCodeAnalysisDirty(item.AddressRef);
		}
	}
	break;

	case EDataType::ScreenPixels:
		// TODO: GFX as texture?
		//point gfx viewer?
		break;

	case EDataType::Blob:
		// draw memory detail
		break;

	case EDataType::Bitmap:
	{
		static FCharSetCreateParams params;
		params.Address = item.AddressRef;
		DrawMaskInfoComboBox(&params.MaskInfo);
		DrawBitmapFormatCombo(params.BitmapFormat, state);
		if (params.BitmapFormat == EBitmapFormat::Bitmap_1Bpp)
		{
			DrawColourInfoComboBox(&params.ColourInfo);

			if (params.ColourInfo == EColourInfo::MemoryLUT)
			{
				DrawAddressInput(state, "Attribs Address", params.AttribsAddress);
			}
		}
		else
		{
			DrawPaletteCombo("Palette", "None", params.PaletteNo, GetNumColoursForBitmapFormat(params.BitmapFormat));
		}
		FCharacterSet *pCharSet = GetCharacterSetFromAddress(item.AddressRef);
		if (pCharSet != nullptr)
		{
			if (ImGui::Button("Update Character Set"))
			{
				UpdateCharacterSet(state, *pCharSet, params);
			}
			pCharSet->Image->Draw();
		}
		else
		{
			if (ImGui::Button("Create Character Set"))
			{
				FLabelInfo* pLabel = state.GetLabelForAddress(item.AddressRef);
				if (pLabel == nullptr)
					AddLabelAtAddress(state, item.AddressRef);
				params.Address = item.AddressRef;
				params.ColourLUT = state.Config.CharacterColourLUT;

				CreateCharacterSetAt(state, params);
			}
		}
	}
	break;

	case EDataType::CharacterMap:
	{
		DrawCharacterSetComboBox(state, pDataInfo->CharSetAddress);
		const char* format = "%02X";
		int flags = ImGuiInputTextFlags_EnterReturnsTrue | ImGuiInputTextFlags_CharsHexadecimal;
		ImGui::InputScalar("Null Character", ImGuiDataType_U8, &pDataInfo->EmptyCharNo,0,0,format,flags);
	}
	break;

	default:
		break;
	}

	DrawDataAccesses(state, viewState, pDataInfo);
}

