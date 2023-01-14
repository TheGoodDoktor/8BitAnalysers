#include "CharacterMapViewer.h"
#include "../CodeAnalyser.h"

#include <imgui.h>
#include "CodeAnalyserUI.h"

static const char* g_MaskInfoTxt[] =
{
	"None",
	"InterleavedBytesPM",
	"InterleavedBytesMP",
};

static const char* g_ColourInfoTxt[] =
{
	"None",
	"Interleaved",
	"MemoryLUT"
};

void DrawMaskInfoComboBox(EMaskInfo* pValue)
{
	if (ImGui::BeginCombo("Mask Info", g_MaskInfoTxt[(int)*pValue]))
	{
		for (int i = 0; i < IM_ARRAYSIZE(g_MaskInfoTxt); i++)
		{
			if (ImGui::Selectable(g_MaskInfoTxt[i], (int)*pValue == i))
			{
				*pValue = (EMaskInfo)i;
			}
		}
		ImGui::EndCombo();
	}
}

void DrawColourInfoComboBox(EColourInfo* pValue)
{
	if (ImGui::BeginCombo("Colour Info", g_ColourInfoTxt[(int)*pValue]))
	{
		for (int i = 0; i < IM_ARRAYSIZE(g_ColourInfoTxt); i++)
		{
			if (ImGui::Selectable(g_ColourInfoTxt[i], (int)*pValue == i))
			{
				*pValue = (EColourInfo)i;
			}
		}
		ImGui::EndCombo();
	}
}

void DrawCharacterSetComboBox(FCodeAnalysisState& state, uint16_t* pAddr)
{
	const FCharacterSet* pCharSet = *pAddr != 0 ? GetCharacterSetFromAddress(*pAddr) : nullptr;
	const FLabelInfo* pLabel = pCharSet != nullptr ? state.GetLabelForAddress(*pAddr) : nullptr;

	const char* pCharSetName = pLabel != nullptr ? pLabel->Name.c_str() : "None";

	if (ImGui::BeginCombo("CharacterSet", pCharSetName))
	{
		if (ImGui::Selectable("None", *pAddr == 0))
		{
			*pAddr = 0;
		}

		for (int i=0;i< GetNoCharacterSets();i++)
		{
			const FCharacterSet* pCharSet = GetCharacterSetFromIndex(i);
			const FLabelInfo* pSetLabel = state.GetLabelForAddress(pCharSet->Params.Address);
			if (pSetLabel == nullptr)
				continue;
			if (ImGui::Selectable(pSetLabel->Name.c_str(), *pAddr == pCharSet->Params.Address))
			{
				*pAddr = pCharSet->Params.Address;
			}
		}

		ImGui::EndCombo();
	}
}

void DrawCharacterSetViewer(FCodeAnalysisState& state, FCodeAnalysisViewState& viewState)
{
	static uint16_t selectedCharSetAddr = 0; 
	static FCharSetCreateParams params;

	if (ImGui::BeginChild("##charsetselect", ImVec2(ImGui::GetWindowContentRegionWidth() * 0.25f, 0), true))
	{
		int deleteIndex = -1;
		for (int i = 0; i < GetNoCharacterSets(); i++)
		{
			const FCharacterSet* pCharSet = GetCharacterSetFromIndex(i);
			const FLabelInfo* pSetLabel = state.GetLabelForAddress(pCharSet->Params.Address);
			const bool bSelected = selectedCharSetAddr == pCharSet->Params.Address;

			if (pSetLabel == nullptr)
				continue;

			ImGui::PushID(i);

			if (ImGui::Selectable(pSetLabel->Name.c_str(), bSelected))
			{
				selectedCharSetAddr = pCharSet->Params.Address;
				if (params.Address != pCharSet->Params.Address)
				{
					params = pCharSet->Params;
				}
			}

			if (ImGui::BeginPopupContextItem("char set context menu"))
			{
				if (ImGui::Selectable("Delete"))
				{
					deleteIndex = i;
				}
				ImGui::EndPopup();
			}

			ImGui::PopID();
		}

		if(deleteIndex != -1)
			DeleteCharacterSet(deleteIndex);
	}

	ImGui::EndChild();
	ImGui::SameLine();
	if (ImGui::BeginChild("##charsetdetails", ImVec2(0, 0), true))
	{
		FCharacterSet* pCharSet = GetCharacterSetFromAddress(selectedCharSetAddr);
		if (pCharSet)
		{
			params.Address = selectedCharSetAddr;
			DrawAddressInput("Address",&params.Address);
			DrawAddressLabel(state, viewState, selectedCharSetAddr);
			DrawMaskInfoComboBox(&params.MaskInfo);
			DrawColourInfoComboBox(&params.ColourInfo);
			if (params.ColourInfo == EColourInfo::MemoryLUT)
			{
				DrawAddressInput("Attribs Address", &params.AttribsAddress);
			}
			ImGui::Checkbox("Dynamic", &params.bDynamic);
			if (ImGui::Button("Update Character Set"))
			{
				UpdateCharacterSet(state, *pCharSet, params);
			}
			pCharSet->Image->Draw();

		}
	}
	ImGui::EndChild();
}

struct FCharacterMapViewerUIState
{
	uint16_t				SelectedCharMapAddr = 0;
	FCharMapCreateParams	Params;

	uint16_t				SelectedCharAddress = 0;
	int						SelectedCharX = -1;
	int						SelectedCharY = -1;
};

void DrawCharacterMap(FCharacterMapViewerUIState& uiState, FCodeAnalysisState& state, FCodeAnalysisViewState& viewState)
{
	FCharacterMap* pCharMap = GetCharacterMapFromAddress(uiState.SelectedCharMapAddr);

	if (pCharMap == nullptr)
		return;
	
	FCharMapCreateParams& params = uiState.Params;
	DrawAddressLabel(state, viewState, uiState.SelectedCharMapAddr);

	// Display and edit params
	DrawAddressInput("Address", &params.Address);
	DrawCharacterSetComboBox(state, &params.CharacterSet);
	int sz[2] = { params.Width, params.Height };
	if (ImGui::InputInt2("Size (X,Y)", sz))
	{
		params.Width = sz[0];
		params.Height = sz[1];
	}
	DrawU8Input("Null Character", &params.IgnoreCharacter);

	if (ImGui::Button("Apply"))
	{
		pCharMap->Params = params;

		// Reformat Memory
		FDataFormattingOptions formattingOptions;
		formattingOptions.DataType = DataType::CharacterMap;
		formattingOptions.StartAddress = params.Address;
		formattingOptions.ItemSize = params.Width;
		formattingOptions.NoItems = params.Height;
		formattingOptions.CharacterSet = params.CharacterSet;
		formattingOptions.EmptyCharNo = params.IgnoreCharacter;
		FormatData(state, formattingOptions);
		state.bCodeAnalysisDataDirty = true;
	}

	// Display Character Map
	ImGuiIO& io = ImGui::GetIO();
	ImDrawList* dl = ImGui::GetWindowDrawList();
	ImVec2 pos = ImGui::GetCursorScreenPos();
	const float rectSize = 12.0f;
	uint16_t byte = 0;
	const FCharacterSet* pCharSet = GetCharacterSetFromAddress(params.CharacterSet);

	for (int y = 0; y < params.Height; y++)
	{
		for (int x = 0; x < params.Width; x++)
		{
			const uint8_t val = state.CPUInterface->ReadByte(params.Address + byte);

			if (val != params.IgnoreCharacter)	// skip empty chars
			{
				const float xp = pos.x + (x * rectSize);
				const float yp = pos.y + (y * rectSize);
				const ImVec2 rectMin(xp, yp);
				const ImVec2 rectMax(xp + rectSize, yp + rectSize);

				if (pCharSet)
				{
					const FCharUVS UVS = pCharSet->GetCharacterUVS(val);
					dl->AddImage((ImTextureID)pCharSet->Image->GetTexture(), rectMin, rectMax, ImVec2(UVS.U0, UVS.V0), ImVec2(UVS.U1, UVS.V1));
				}
				else
				{
					char valTxt[8];
					sprintf_s(valTxt, "%02x", val);
					dl->AddRect(rectMin, rectMax, 0xffffffff);
					dl->AddText(ImVec2(xp + 1, yp + 1), 0xffffffff, valTxt);
					//dl->AddText(NULL, 8.0f, ImVec2(xp + 1, yp + 1), 0xffffffff, valTxt, NULL);
				}
			}

			byte++;	// go to next byte
		}
	}

	// draw highlight rect
	const float mousePosX = io.MousePos.x - pos.x;
	const float mousePosY = io.MousePos.y - pos.y;
	if (mousePosX >= 0 && mousePosY >= 0 && mousePosX < (params.Width * rectSize) && mousePosY < (params.Height * rectSize))
	{
		const int xChar = (int)floor(mousePosX / rectSize);
		const int yChar = (int)floor(mousePosY / rectSize);
		const uint16_t charAddress = pCharMap->Params.Address + (xChar + (yChar * pCharMap->Params.Width));
		const uint8_t charVal = state.CPUInterface->ReadByte(charAddress);

		const float xp = pos.x + (xChar * rectSize);
		const float yp = pos.y + (yChar * rectSize);
		const ImVec2 rectMin(xp, yp);
		const ImVec2 rectMax(xp + rectSize, yp + rectSize);
		dl->AddRect(rectMin, rectMax, 0xffffffff);

		if (ImGui::IsMouseClicked(0))
		{
			uiState.SelectedCharAddress = charAddress;
			uiState.SelectedCharX = xChar;
			uiState.SelectedCharY = yChar;
		}

		// Tool Tip
		ImGui::BeginTooltip();
		ImGui::Text("Char Pos (%d,%d)", xChar, yChar);
		ImGui::Text("Value: %s", NumStr(charVal));
		ImGui::EndTooltip();
	}

	if (uiState.SelectedCharX != -1 && uiState.SelectedCharY != -1)
	{
		const float xp = pos.x + (uiState.SelectedCharX * rectSize);
		const float yp = pos.y + (uiState.SelectedCharY * rectSize);
		const ImVec2 rectMin(xp, yp);
		const ImVec2 rectMax(xp + rectSize, yp + rectSize);
		dl->AddRect(rectMin, rectMax, 0xffffffff);
	}

	// draw hovered address
	if (viewState.HighlightAddress != -1)
	{
		const uint16_t charMapStartAddr = params.Address;
		const uint16_t charMapEndAddr = params.Address + (params.Width * params.Height) - 1;
		if (viewState.HighlightAddress >= params.Address && viewState.HighlightAddress <= charMapEndAddr)	// pixel
		{
			const uint16_t addrOffset = viewState.HighlightAddress - params.Address;
			const int charX = addrOffset % params.Width;
			const int charY = addrOffset / params.Width;
			const float xp = pos.x + (charX * rectSize);
			const float yp = pos.y + (charY * rectSize);
			const ImVec2 rectMin(xp, yp);
			const ImVec2 rectMax(xp + rectSize, yp + rectSize);
			dl->AddRect(rectMin, rectMax, 0xffff00ff);
		}
	}

	pos.y += params.Height * rectSize;
	ImGui::SetCursorScreenPos(pos);

	if (uiState.SelectedCharAddress != 0)
	{
		// TODO: show data reads & writes
		// 
		FDataInfo* pDataInfo = state.GetReadDataInfoForAddress(uiState.SelectedCharAddress);
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
	}

	
	
}

void DrawCharacterMaps(FCodeAnalysisState& state, FCodeAnalysisViewState& viewState)
{
	static FCharacterMapViewerUIState uiState;

	if (ImGui::BeginChild("##charmapselect", ImVec2(ImGui::GetWindowContentRegionWidth() * 0.25f, 0), true))
	{
		int deleteIndex = -1;

		// List character maps
		for (int i = 0; i < GetNoCharacterMaps(); i++)
		{
			const FCharacterMap* pCharMap = GetCharacterMapFromIndex(i);
			const FLabelInfo* pSetLabel = state.GetLabelForAddress(pCharMap->Params.Address);
			const bool bSelected = uiState.SelectedCharMapAddr == pCharMap->Params.Address;

			if (pSetLabel == nullptr)
				continue;

			ImGui::PushID(i);

			if (ImGui::Selectable(pSetLabel->Name.c_str(), bSelected))
			{
				uiState.SelectedCharMapAddr = pCharMap->Params.Address;
				if (uiState.SelectedCharMapAddr != uiState.Params.Address)
				{
					uiState.Params = pCharMap->Params;	// copy params
					uiState.SelectedCharAddress = 0;
					uiState.SelectedCharX = -1;
					uiState.SelectedCharY = -1;
				}
			}			

			if (ImGui::BeginPopupContextItem("char map context menu"))
			{
				if (ImGui::Selectable("Delete"))
				{
					deleteIndex = i;
				}
				ImGui::EndPopup();
			}

			ImGui::PopID();
		}

		if(deleteIndex != -1)
			DeleteCharacterMap(deleteIndex);

		
	}
	ImGui::EndChild();
	ImGui::SameLine();
	if (ImGui::BeginChild("##charmapdetails", ImVec2(0, 0), true))
	{
		DrawCharacterMap(uiState, state, viewState);
	}
	ImGui::EndChild();
}

void DrawCharacterMapViewer(FCodeAnalysisState& state, FCodeAnalysisViewState& viewState)
{
	if (ImGui::BeginTabBar("CharacterMapTabs"))
	{
		if (ImGui::BeginTabItem("Character Sets"))
		{
			DrawCharacterSetViewer(state, viewState);
			ImGui::EndTabItem();
		}

		if (ImGui::BeginTabItem("Character Maps"))
		{
			DrawCharacterMaps(state, viewState);
			ImGui::EndTabItem();
		}
		ImGui::EndTabBar();
	}
}
