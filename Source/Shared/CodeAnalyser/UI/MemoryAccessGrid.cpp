#include "MemoryAccessGrid.h"

#include "CodeAnalyser/CodeAnalyser.h"
#include <imgui.h>
#include "ImGuiSupport/ImGuiScaling.h"
#include "CodeAnalyserUI.h"

#include <cmath>

FMemoryAccessGrid::FMemoryAccessGrid(FCodeAnalysisState* pCodeAnalysis, int xGridSize, int yGridSize)
	: CodeAnalysis(pCodeAnalysis)
	, GridSizeX(xGridSize)
	, GridSizeY(yGridSize)
	, GridStride(xGridSize)
{

}

bool FMemoryAccessGrid::GetAddressGridPosition(FAddressRef address, int& outX, int& outY)
{
	// draw hovered address
	if (address.IsValid())
	{
		const FAddressRef startAddr = GetGridSquareAddress(0, 0);
		const FAddressRef endAddr = GetGridSquareAddress(GridSizeX - 1, GridSizeY - 1);

		const uint16_t charMapEndAddr = endAddr.Address;//charMapAddress + (noCharsX * noCharsY) - 1;

		// is checking bank ID enough?
		if (address.BankId == endAddr.BankId &&
			address.Address >= startAddr.Address &&
			address.Address <= endAddr.Address)
		{
			const uint16_t addrOffset = address.Address - startAddr.Address;
			outX = addrOffset % GridStride;
			outY = addrOffset / GridStride;
			return true;
		}
	}
	return false;
}

void FMemoryAccessGrid::DrawGrid(float x, float y)
{
	// Display Character Map
	FCodeAnalysisState& state = *CodeAnalysis;
	FCodeAnalysisViewState& viewState = state.GetFocussedViewState();
	ImGuiIO& io = ImGui::GetIO();
	ImDrawList* dl = ImGui::GetWindowDrawList();
	ImVec2 pos(x, y);
	const float rectSize = GridSquareSize;
	const float imgScale = ImGui_GetScaling();
	const float fontSize = ImGui::GetFontSize();

	const float txtOffX = (rectSize / 2) - (fontSize/2);
	const float txtOffY = (rectSize / 2) - (fontSize/2);

	for (int y = 0; y < GridSizeY; y++)
	{
		for (int x = 0; x < GridSizeX; x++)
		{
			FAddressRef curCharAddress = GetGridSquareAddress(x, y);
			if(curCharAddress.IsValid() == false)
				continue;

			FDataInfo* pDataInfo = state.GetDataInfoForAddress(curCharAddress);
			const int framesSinceWritten = pDataInfo->LastFrameWritten == -1 ? 255 : state.CurrentFrameNo - pDataInfo->LastFrameWritten;
			const int framesSinceRead = pDataInfo->LastFrameRead == -1 ? 255 : state.CurrentFrameNo - pDataInfo->LastFrameRead;
			const int wBrightVal = (255 - std::min(framesSinceWritten << 3, 255)) & 0xff;
			const int rBrightVal = (255 - std::min(framesSinceRead << 3, 255)) & 0xff;
			const float xp = pos.x + (x * rectSize);
			const float yp = pos.y + (y * rectSize);
			ImVec2 rectMin(xp, yp);
			ImVec2 rectMax(xp + rectSize, yp + rectSize);

			bool bIgnoreThis = bUseIgnoreValue && IgnoreValue == state.ReadByte(curCharAddress);

			if (wBrightVal > 0 || rBrightVal > 0)	// skip empty chars
			{
				if (bShowReadWrites)
				{
					if (rBrightVal > 0)
					{
						const ImU32 col = 0xff000000 | (rBrightVal << 8);
						dl->AddRect(rectMin, rectMax, col);

						rectMin = ImVec2(rectMin.x + 1, rectMin.y + 1);
						rectMax = ImVec2(rectMax.x - 1, rectMax.y - 1);
					}
					if (wBrightVal > 0)
					{
						const ImU32 col = 0xff000000 | (wBrightVal << 0);
						dl->AddRect(rectMin, rectMax, col);
					}
				}
			}
			else if(bOutlineAllSquares && bIgnoreThis == false)
			{
				dl->AddRect(rectMin, rectMax, 0xffffffff);
			}

			if (bShowValues && bIgnoreThis == false)
			{
				const uint8_t val = state.ReadByte(curCharAddress);
				char valTxt[8];
				snprintf(valTxt, 8, "%02x", val);
				dl->AddText(ImVec2(xp + txtOffX, yp + txtOffY), 0xffffffff, valTxt);
				//dl->AddText(NULL, 8.0f, ImVec2(xp + 1, yp + 1), 0xffffffff, valTxt, NULL);
			}
		}
	}

	// draw highlight rect
	const float mousePosX = io.MousePos.x - pos.x;
	const float mousePosY = io.MousePos.y - pos.y;
	if (mousePosX >= 0 && mousePosY >= 0 && mousePosX < (GridSizeX * rectSize) && mousePosY < (GridSizeY * rectSize))
	{
		const int xChar = (int)floor(mousePosX / rectSize);
		const int yChar = (int)floor(mousePosY / rectSize);

		const FAddressRef charAddress = GetGridSquareAddress(xChar, yChar);
		if(charAddress.IsValid())
		{
			const uint8_t charVal = state.ReadByte(charAddress);

			const float xp = pos.x + (xChar * rectSize);
			const float yp = pos.y + (yChar * rectSize);
			const ImVec2 rectMin(xp, yp);
			const ImVec2 rectMax(xp + rectSize, yp + rectSize);
			dl->AddRect(rectMin, rectMax, 0xffffffff);

			if (ImGui::IsMouseClicked(0))
			{
				SelectedCharAddress = charAddress;
				SelectedCharX = xChar;
				SelectedCharY = yChar;
			}
			if (ImGui::IsMouseDoubleClicked(0))
			{
				viewState.GoToAddress(charAddress);
			}

			// Tool Tip
			ImGui::BeginTooltip();
			ImGui::Text("Char Pos (%d,%d)", xChar + OffsetX, yChar + OffsetY);
			ImGui::Text("Value: %s", NumStr(charVal));
			ImGui::EndTooltip();
		}
	}

	if (SelectedCharX != -1 && SelectedCharY != -1)
	{
		const float xp = pos.x + (SelectedCharX * rectSize);
		const float yp = pos.y + (SelectedCharY * rectSize);
		const ImVec2 rectMin(xp, yp);
		const ImVec2 rectMax(xp + rectSize, yp + rectSize);
		dl->AddRect(rectMin, rectMax, 0xffffffff);
	}

	// draw hovered address
	int hoverredX = -1;
	int hoverredY = -1;
	if (GetAddressGridPosition(viewState.HighlightAddress,hoverredX,hoverredY))
	{
		const float xp = pos.x + (hoverredX * rectSize);
		const float yp = pos.y + (hoverredY * rectSize);
		const ImVec2 rectMin(xp, yp);
		const ImVec2 rectMax(xp + rectSize, yp + rectSize);
		dl->AddRect(rectMin, rectMax, 0xffff00ff);
	}

	if(bDetailsToSide)
		pos.x += GridSizeX * rectSize;
	else
		pos.y += GridSizeY * rectSize;

	ImGui::SetCursorScreenPos(pos);
	ImGui::Checkbox("Show Reads & Writes", &bShowReadWrites);
	if (SelectedCharAddress.IsValid())
	{
		ImGui::Text("Address: %s", NumStr(SelectedCharAddress.Address));
		DrawAddressLabel(state,state.GetFocussedViewState(),SelectedCharAddress);
		// Show data reads & writes
		FDataInfo* pDataInfo = state.GetDataInfoForAddress(SelectedCharAddress);
		// List Data accesses
		if (pDataInfo->Reads.IsEmpty() == false)
		{
			ImGui::Text("Reads:");
			for (const auto& reader : pDataInfo->Reads.GetReferences())
			{
				ShowCodeAccessorActivity(state, reader);

				ImGui::Text("   ");
				ImGui::SameLine();
				DrawCodeAddress(state, viewState, reader);
			}
		}

		if (pDataInfo->Writes.IsEmpty() == false)
		{
			ImGui::Text("Writes:");
			for (const auto& writer : pDataInfo->Writes.GetReferences())
			{
				ShowCodeAccessorActivity(state, writer);

				ImGui::Text("   ");
				ImGui::SameLine();
				DrawCodeAddress(state, viewState, writer);
			}
		}
	}
}

void FMemoryAccessGrid::DrawAt(float x, float y)
{
	OnDraw();
	DrawBackground(x,y);
	if(bDrawGrid)
		DrawGrid(x, y);
}

void FMemoryAccessGrid::Draw()
{
	OnDraw();
	const ImVec2 pos = ImGui::GetCursorScreenPos();
	DrawBackground(pos.x, pos.y);
	if(bDrawGrid)
		DrawGrid(pos.x,pos.y);
}

void FMemoryAccessGrid::FixupAddressRefs()
{
	FixupAddressRef(*CodeAnalysis, SelectedCharAddress);
}