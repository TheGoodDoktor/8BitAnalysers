#include "CodeAnalyserUI.h"
#include "../CodeAnalyser.h"

#include "Util/Misc.h"
#include "Util/GraphicsView.h"
#include "ImageViewer.h"

#include "imgui.h"




float DrawDataCharMapLine(FCodeAnalysisState& state, const FDataInfo* pDataInfo)
{
	const float line_height = ImGui::GetTextLineHeight();
	const float rectSize = line_height + 4;
	ImDrawList* dl = ImGui::GetWindowDrawList();
	ImVec2 pos = ImGui::GetCursorScreenPos();
	pos.x += 150.0f;
	const float startPos = pos.x;
	pos.y -= rectSize + 2;

	for (int byte = 0; byte < pDataInfo->ByteSize; byte++)
	{
		const uint8_t val = state.CPUInterface->ReadByte(pDataInfo->Address + byte);
		if (val != 0)	// skip char of value 0 - should probably be an option
		{
			const ImVec2 rectMin(pos.x, pos.y);
			const ImVec2 rectMax(pos.x + rectSize, pos.y + rectSize);
			char valTxt[8];
			sprintf_s(valTxt, "%02x", val);
			dl->AddRect(rectMin, rectMax, 0xffffffff);
			dl->AddText(ImVec2(pos.x + 1, pos.y + 1), 0xffffffff, valTxt);
		}
		pos.x += rectSize;
	}
	return pos.x;

}

// returns how much space it took
float DrawDataBitmapLine(FCodeAnalysisState& state, const FDataInfo* pDataInfo, bool bEditMode)
{
	const float line_height = ImGui::GetTextLineHeight();
	const float rectSize = line_height + 4;
	ImDrawList* dl = ImGui::GetWindowDrawList();
	ImVec2 pos = ImGui::GetCursorScreenPos();
	pos.x += 200.0f;
	pos.y -= rectSize + 2;
	const ImVec2 startPos = pos;

	for (int byte = 0; byte < pDataInfo->ByteSize; byte++)
	{
		const uint8_t val = state.CPUInterface->ReadByte(pDataInfo->Address + byte);

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
	}

	// Edit bits
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

			uint8_t val = state.CPUInterface->ReadByte(pDataInfo->Address + byteNo);
			val = val ^ (1 << (7-bitNo));
			state.CPUInterface->WriteByte(pDataInfo->Address + byteNo,val);
		}
	}
	//return pos.x - startPos;
	return pos.x;
}

// Display a colour attribute
// This is written for ZX Spectrum colour attributes which goes against the platform independent nature of this file
// Some kind of abstraction is needed

// speccy colour CLUT
static const uint32_t g_ColourLUT[8] =
{
	0xFF000000,     // 0 - black
	0xFFFF0000,     // 1 - blue
	0xFF0000FF,     // 2 - red
	0xFFFF00FF,     // 3 - magenta
	0xFF00FF00,     // 4 - green
	0xFFFFFF00,     // 5 - cyan
	0xFF00FFFF,     // 6 - yellow
	0xFFFFFFFF,     // 7 - white
};

static uint32_t GetColFromAttr(uint8_t colBits, bool bBright)
{
	const uint32_t outCol = g_ColourLUT[colBits & 7];
	if (bBright == false)
		return outCol & 0xFFD7D7D7;
	else
		return outCol;
}

// returns how much space it took
float DrawColAttr(FCodeAnalysisState& state, const FDataInfo* pDataInfo, bool bEditMode)
{
	const float line_height = ImGui::GetTextLineHeight();
	const float rectSize = line_height + 4;
	ImDrawList* dl = ImGui::GetWindowDrawList();
	ImVec2 pos = ImGui::GetCursorScreenPos();
	pos.x += 200.0f;
	pos.y -= rectSize + 2;
	const ImVec2 startPos = pos;
	
	for (int byte = 0; byte < pDataInfo->ByteSize; byte++)
	{
		uint8_t colAttr = state.CPUInterface->ReadByte(pDataInfo->Address + byte);
		const bool bBright = !!(colAttr & (1 << 6));
		const uint32_t inkCol = GetColFromAttr(colAttr & 7, bBright);
		const uint32_t paperCol = GetColFromAttr(colAttr >> 3, bBright);

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
	return pos.x;
}

int DataItemEditCallback(ImGuiInputTextCallbackData* pData)
{
	return 1;
}

void EditByteDataItem(FCodeAnalysisState& state, uint16_t address)
{
	const ENumberDisplayMode numMode = GetNumberDisplayMode();
	uint8_t val = state.CPUInterface->ReadByte(address);
	int flags = ImGuiInputTextFlags_EnterReturnsTrue;
	int width = 18;
	const char* format = "%02X";

	switch (numMode)
	{
	case ENumberDisplayMode::Decimal:
		width = 24;
		format = "%d";
		break;
	case ENumberDisplayMode::HexAitch:
	case ENumberDisplayMode::HexDollar:
		flags |= ImGuiInputTextFlags_CharsHexadecimal;
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
	int width = 30;
	const char* format = "%04X";

	switch (numMode)
	{
	case ENumberDisplayMode::Decimal:
		width = 50;
		format = "%d";
		break;
	case ENumberDisplayMode::HexAitch:
	case ENumberDisplayMode::HexDollar:
		flags |= ImGuiInputTextFlags_CharsHexadecimal;
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

void DrawDataInfo(FCodeAnalysisState& state, FCodeAnalysisViewState& viewState, const FDataInfo* pDataInfo, bool bDrawLabel, bool bEdit)
{
	const float line_height = ImGui::GetTextLineHeight();
	const float glyph_width = ImGui::CalcTextSize("F").x;
	const float cell_width = 3 * glyph_width;
	const int framesSinceWritten = pDataInfo->LastFrameWritten == -1 ? 255 : state.CurrentFrameNo - pDataInfo->LastFrameWritten;
	const int framesSinceRead = pDataInfo->LastFrameRead == -1 ? 255 : state.CurrentFrameNo - pDataInfo->LastFrameRead;
	const int wBrightVal = (255 - std::min(framesSinceWritten << 2, 255)) & 0xff;
	const int rBrightVal = (255 - std::min(framesSinceRead << 2, 255)) & 0xff;
	float offset = 0;

	if (rBrightVal > 0 || wBrightVal > 0)
	{
		const ImU32 pc_color = 0xFF00FFFF;
		const ImU32 brd_color = 0xFF000000;

		ImVec2 pos = ImGui::GetCursorScreenPos();
		ImDrawList* dl = ImGui::GetWindowDrawList();
		const float lh2 = (float)(int)(line_height / 2);

		if (wBrightVal > 0)
		{
			const ImVec2 a(pos.x + 2, pos.y);
			const ImVec2 b(pos.x + 12, pos.y + lh2);
			const ImVec2 c(pos.x + 2, pos.y + line_height);

			const ImU32 col = 0xff000000 | (wBrightVal << 0);
			dl->AddTriangleFilled(a, b, c, col);
			dl->AddTriangle(a, b, c, brd_color);
		}

		pos.x += 10;


		if (rBrightVal > 0)
		{
			const ImVec2 a(pos.x + 2, pos.y);
			const ImVec2 b(pos.x + 12, pos.y + lh2);
			const ImVec2 c(pos.x + 2, pos.y + line_height);

			const ImU32 col = 0xff000000 | (rBrightVal << 8);
			dl->AddTriangleFilled(a, b, c, col);
			dl->AddTriangle(a, b, c, brd_color);
		}
	}

	// show if breakpointed
	if (state.CPUInterface->IsAddressBreakpointed(pDataInfo->Address))
	{
		const ImU32 bp_enabled_color = 0xFF0000FF;
		const ImU32 bp_disabled_color = 0xFF000088;
		const ImU32 brd_color = 0xFF000000;
		ImDrawList* dl = ImGui::GetWindowDrawList();
		const ImVec2 pos = ImGui::GetCursorScreenPos();
		const float lh2 = (float)(int)(line_height / 2);
		const ImVec2 mid(pos.x + 7, pos.y + lh2);

		dl->AddCircleFilled(mid, 7, bp_enabled_color);
		dl->AddCircle(mid, 7, brd_color);
	}

	ImGui::Text("\t%s", NumStr(pDataInfo->Address));

	ENumberDisplayMode trueNumberDisplayMode = GetNumberDisplayMode();
	if (pDataInfo->OperandType != EOperandType::Unknown)
	{
		//SetNumberDisplayMode(pDataInfo->NumDispOverride);
	}

	const float line_start_x = ImGui::GetCursorPosX();
	if (bDrawLabel)
	{
		DrawAddressLabel(state, viewState, pDataInfo->Address);
		ImGui::SameLine(line_start_x + cell_width * 10 + glyph_width * 2);
		ImGui::Text(":");
		ImGui::SameLine();
	}
	else
	{
		ImGui::SameLine(line_start_x + cell_width * 4 + glyph_width * 2);
	}

	if (pDataInfo->bShowCharMap)
	{
		ImGui::Text("CM:");
		DrawDataCharMapLine(state, pDataInfo);
		return;
	}

	switch (pDataInfo->DataType)
	{
	case DataType::Byte:
	{
		const uint8_t val = state.CPUInterface->ReadByte(pDataInfo->Address);
		ImGui::Text("db");
		ImGui::SameLine();

		if (bEdit)
			EditByteDataItem(state, pDataInfo->Address);
		else
			ImGui::Text("%s", NumStr(val));

		ImGui::SameLine();
		if (val == '\n')// carriage return messes up list
			ImGui::Text("'<cr>'");
		else
			ImGui::Text("'%c'", val);
	}
	break;

	case DataType::ByteArray:
	{
		uint8_t val = state.CPUInterface->ReadByte(pDataInfo->Address);

		// TODO: add edit support

		ImGui::Text("db %s", NumStr(val));
		for (int i = 1; i < pDataInfo->ByteSize; i++)	// first word already written
		{
			val = state.CPUInterface->ReadByte(pDataInfo->Address + i);
			ImGui::SameLine();
			ImGui::Text(",%s", NumStr(val));
		}
	}
	break;

	case DataType::Word:
	{
		const uint16_t val = state.CPUInterface->ReadWord(pDataInfo->Address);
		ImGui::Text("dw");
		ImGui::SameLine();
		if (bEdit)
			EditWordDataItem(state, pDataInfo->Address);
		else
			ImGui::Text("%s", NumStr(val));
		ImGui::SameLine();
		DrawAddressLabel(state, viewState, val);
	}
	break;

	case DataType::WordArray:
	{
		uint16_t val = state.CPUInterface->ReadWord(pDataInfo->Address);
		const int wordCount = pDataInfo->ByteSize / 2;

		// TODO: add edit support

		ImGui::Text("dw %s", NumStr(val));
		for (int i = 1; i < wordCount; i++)	// first word already written
		{
			val = state.CPUInterface->ReadWord(pDataInfo->Address + (i * 2));
			ImGui::SameLine();
			ImGui::Text(",%s", NumStr(val));
		}
	}
	break;

	case DataType::Text:
	{
		std::string textString;
		for (int i = 0; i < pDataInfo->ByteSize; i++)
		{
			const char ch = state.CPUInterface->ReadByte(pDataInfo->Address + i);
			if (ch == '\n')
				textString += "<cr>";
			if (pDataInfo->bBit7Terminator && ch & (1 << 7))	// check bit 7 terminator flag
				textString += ch & ~(1 << 7);	// remove bit 7
			else
				textString += ch;
		}
		ImGui::Text("ascii '%s'", textString.c_str());
	}
	break;

	case DataType::Bitmap:
		ImGui::Text("Bitmap");
		offset = DrawDataBitmapLine(state, pDataInfo, state.bAllowEditing);
		break;
	case DataType::CharacterMap:
		ImGui::Text("Charmap");
		offset = DrawDataCharMapLine(state, pDataInfo);
		break;
	case DataType::ColAttr:
		ImGui::Text("ColAttr");
		offset = DrawColAttr(state, pDataInfo, state.bAllowEditing);
		break;
	case DataType::Graphics:
	case DataType::Blob:
	default:
		ImGui::Text("%d Bytes", pDataInfo->ByteSize);
		break;
	}

	/*if (pDataInfo->bShowBinary)
	{
		offset = DrawDataBitmapLine(state, pDataInfo);
	}*/

	if (state.CPUInterface->GetSP() == pDataInfo->Address)
	{
		ImGui::SameLine();
		ImGui::Text("<- SP");
	}

	SetNumberDisplayMode(trueNumberDisplayMode);

	DrawComment(pDataInfo, offset);
}

void DrawDataValueGraph(float val, bool bReset)
{
	// Create a dummy array of contiguous float values to plot
		// Tip: If your float aren't contiguous but part of a structure, you can pass a pointer to your first float and the sizeof() of your structure in the Stride parameter.
	static bool animate = true;
	static float values[90] = { 0 };
	static int values_offset = 0;
	static double refresh_time = 0.0;
	if (!animate || refresh_time == 0.0)
		refresh_time = ImGui::GetTime();

	while (refresh_time < ImGui::GetTime()) // Create dummy data at fixed 60 hz rate for the demo
	{
		static float phase = 0.0f;
		values[values_offset] = val;
		values_offset = (values_offset + 1) % IM_ARRAYSIZE(values);
		phase += 0.10f * values_offset;
		refresh_time += 1.0f / 60.0f;
	}

	// Plots can display overlay texts
	// (in this example, we will display an average value)
	{
		float average = 0.0f;
		for (int n = 0; n < IM_ARRAYSIZE(values); n++)
			average += values[n];
		average /= (float)IM_ARRAYSIZE(values);
		char overlay[32];
		sprintf_s(overlay, "avg %f", average);
		ImGui::PlotLines("Lines", values, IM_ARRAYSIZE(values), values_offset, overlay, 0.0f, 255.0f, ImVec2(0, 80));
	}
	//ImGui::PlotHistogram("Histogram", arr, IM_ARRAYSIZE(arr), 0, NULL, 0.0f, 1.0f, ImVec2(0, 80));
}

void DrawDataValueGraph(uint8_t val, bool bReset)
{
	DrawDataValueGraph(static_cast<float>(val), bReset);
}

void DrawDataValueGraph(uint16_t val, bool bReset)
{
	DrawDataValueGraph(static_cast<float>(val), bReset);
}



void DrawDataDetails(FCodeAnalysisState& state, FCodeAnalysisViewState& viewState, FDataInfo* pDataInfo)
{
	ImGui::Text("Number Mode Override");
	DrawOperandTypeCombo("##dataOperand",pDataInfo->OperandType);
	switch (pDataInfo->DataType)
	{
	case DataType::Byte:
		DrawDataValueGraph(state.CPUInterface->ReadByte(pDataInfo->Address), false);
		break;

	case DataType::Word:
		DrawDataValueGraph(state.CPUInterface->ReadWord(pDataInfo->Address), false);
		break;

	case DataType::Text:
	{
		std::string textString;
		for (int i = 0; i < pDataInfo->ByteSize; i++)
		{
			const char ch = state.CPUInterface->ReadByte(pDataInfo->Address + i);
			if (ch == '\n')
				textString += "<cr>";
			if (pDataInfo->bBit7Terminator && ch & (1 << 7))	// check bit 7 terminator flag
				textString += ch & ~(1 << 7);	// remove bit 7
			else
				textString += ch;
		}
		ImGui::Text("%s", textString.c_str());
		int length = pDataInfo->ByteSize;
		if (ImGui::InputInt("Length", &length))
		{
			pDataInfo->ByteSize = length;
			state.bCodeAnalysisDataDirty = true;
		}
	}
	break;

	case DataType::Graphics:
		// TODO: GFX as texture?
		//point gfx viewer?
		break;

	case DataType::Blob:
		// draw memory detail
		break;

	case DataType::Image:
		// This is currently disabled
		if (0)
		{
			FImageData* pImageData = pDataInfo->ImageData;
			bool bRebuildImage = false;
			int sz[] = { pImageData->XSizeChars,pImageData->YSizeChars };

			if (ImGui::InputInt2("Image Size (chars)", sz))
			{
				pDataInfo->ByteSize = pImageData->SetSizeChars(sz[0], sz[1]);
				state.bCodeAnalysisDataDirty = true;	// force redraw of items
				bRebuildImage = true;
			}

			// viewer drop down
			const auto& viewerList = GetImageViewerList();
			if (viewerList.empty() == false)
			{
				if (ImGui::BeginCombo("Viewer", viewerList[pImageData->ViewerId]->GetName()))
				{
					for (int i = 0; i < (int)viewerList.size(); i++)
					{
						const bool bSelected = i == pImageData->ViewerId;
						if (ImGui::Selectable(viewerList[i]->GetName(), bSelected))
						{
							pImageData->ViewerId = i;
							bRebuildImage = true;
						}

						if (bSelected)
							ImGui::SetItemDefaultFocus();
					}
					ImGui::EndCombo();
				}
			}

			// Draw image
			if (pDataInfo->ImageData->GraphicsView == nullptr)
				bRebuildImage = true;

			if (bRebuildImage)
			{
				delete pImageData->GraphicsView;

				pImageData->GraphicsView = new FGraphicsView(64, 64);// pDataInfo->ImgData.XSizeChars * 8, pDataInfo->ImgData.YSizeChars * 8);
				pImageData->GraphicsView->Clear();

				viewerList[pImageData->ViewerId]->DrawImageToView(
					pDataInfo->Address,
					pImageData->XSizeChars, pImageData->YSizeChars,
					pImageData->GraphicsView,
					state.CPUInterface);
			}
			pImageData->GraphicsView->Draw();
		}
		break;
	default:
		break;
	}

	// List Data accesses
	if (pDataInfo->Reads.empty() == false)
	{
		ImGui::Text("Reads:");
		for (const auto& caller : pDataInfo->Reads)
		{
			const uint16_t accessorCodeAddr = caller.first;
			ShowCodeAccessorActivity(state, accessorCodeAddr);

			ImGui::Text("   ");
			ImGui::SameLine();
			DrawCodeAddress(state, viewState, accessorCodeAddr);
		}
	}

	if (pDataInfo->Writes.empty() == false)
	{
		ImGui::Text("Writes:");
		for (const auto& caller : pDataInfo->Writes)
		{
			const uint16_t accessorCodeAddr = caller.first;
			ShowCodeAccessorActivity(state, accessorCodeAddr);

			ImGui::Text("   ");
			ImGui::SameLine();
			DrawCodeAddress(state, viewState, caller.first);
		}
	}

	// last writer to address
	const uint16_t lastWriter = state.GetLastWriterForAddress(pDataInfo->Address);
	if (lastWriter != 0)
	{
		ImGui::Text("Last Writer: ");
		ImGui::SameLine();
		DrawCodeAddress(state, viewState, lastWriter);
	}
}

