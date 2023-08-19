#include "CharacterMapViewer.h"
#include "../CodeAnalyser.h"

#include <imgui.h>
#include "CodeAnalyserUI.h"

#include <cmath>

static const char* g_MaskInfoTxt[] =
{
	"None",
	"InterleavedBytesPM",
	"InterleavedBytesMP",
};

static const char* g_ColourInfoTxt[] =
{
	"None",
	"InterleavedPost",
	"MemoryLUT",
	"InterleavedPre",
};

void RunEnumTests()
{
	assert(IM_ARRAYSIZE(g_MaskInfoTxt) == (int)EMaskInfo::Max);	// if this asserts then you need to look at how EColourInfo maps to g_ColourInfoTxt
	assert(IM_ARRAYSIZE(g_ColourInfoTxt) == (int)EColourInfo::Max);	// if this asserts then you need to look at how EColourInfo maps to g_ColourInfoTxt
}

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

void DrawCharacterSetComboBox(FCodeAnalysisState& state, FAddressRef& addr)
{
	const FCharacterSet* pCharSet = addr.IsValid() ? GetCharacterSetFromAddress(addr) : nullptr;
	const FLabelInfo* pLabel = pCharSet != nullptr ? state.GetLabelForAddress(addr) : nullptr;

	const char* pCharSetName = pLabel != nullptr ? pLabel->Name.c_str() : "None";

	if (ImGui::BeginCombo("CharacterSet", pCharSetName))
	{
		if (ImGui::Selectable("None", addr.IsValid() == false))
		{
			addr = FAddressRef();
		}

		for (int i=0;i< GetNoCharacterSets();i++)
		{
			const FCharacterSet* pCharSet = GetCharacterSetFromIndex(i);
			const FLabelInfo* pSetLabel = state.GetLabelForAddress(pCharSet->Params.Address);
			if (pSetLabel == nullptr)
				continue;
			if (ImGui::Selectable(pSetLabel->Name.c_str(), addr == pCharSet->Params.Address))
			{
				addr = pCharSet->Params.Address;
			}
		}

		ImGui::EndCombo();
	}
}

void DrawCharacterSetViewer(FCodeAnalysisState& state, FCodeAnalysisViewState& viewState)
{
	static FAddressRef selectedCharSetAddr; 
	static FCharSetCreateParams params;

	if (ImGui::BeginChild("##charsetselect", ImVec2(ImGui::GetWindowContentRegionWidth() * 0.25f, 0), true))
	{
		int deleteIndex = -1;
		for (int i = 0; i < GetNoCharacterSets(); i++)
		{
			const FCharacterSet* pCharSet = GetCharacterSetFromIndex(i);
			const FLabelInfo* pSetLabel = state.GetLabelForAddress(pCharSet->Params.Address);
			const bool bSelected = params.Address == pCharSet->Params.Address;

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
			if (DrawAddressInput(state, "Address", params.Address))
			{
				//UpdateCharacterSet(state, *pCharSet, params);
			}
			DrawAddressLabel(state, viewState, params.Address);
			DrawMaskInfoComboBox(&params.MaskInfo);
			DrawColourInfoComboBox(&params.ColourInfo);
			if (params.ColourInfo == EColourInfo::MemoryLUT)
			{
				DrawAddressInput(state, "Attribs Address", params.AttribsAddress);
			}
			ImGui::Checkbox("Dynamic", &params.bDynamic);
			if (ImGui::Button("Update Character Set"))
			{
				selectedCharSetAddr = params.Address;
				UpdateCharacterSet(state, *pCharSet, params);
			}
			pCharSet->Image->Draw();

		}
	}
	ImGui::EndChild();
}

struct FCharacterMapViewerUIState
{
	FAddressRef				SelectedCharMapAddr;
	FCharMapCreateParams	Params;

	FAddressRef				SelectedCharAddress;
	int						SelectedCharX = -1;
	int						SelectedCharY = -1;
};

// this assumes the character map is in address space
void DrawCharacterMap(FCharacterMapViewerUIState& uiState, FCodeAnalysisState& state, FCodeAnalysisViewState& viewState)
{
	FCharacterMap* pCharMap = GetCharacterMapFromAddress(uiState.SelectedCharMapAddr);

	if (pCharMap == nullptr)
		return;
	
	FCharMapCreateParams& params = uiState.Params;
	
	DrawAddressLabel(state, viewState, uiState.SelectedCharMapAddr);

	// Display and edit params
	DrawAddressInput(state, "Address", params.Address);
	DrawCharacterSetComboBox(state, params.CharacterSet);
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
		formattingOptions.DataType = EDataType::CharacterMap;
		formattingOptions.StartAddress = params.Address;	
		formattingOptions.ItemSize = params.Width;
		formattingOptions.NoItems = params.Height;
		formattingOptions.CharacterSet = params.CharacterSet;
		formattingOptions.EmptyCharNo = params.IgnoreCharacter;
		FormatData(state, formattingOptions);
		state.SetCodeAnalysisDirty(params.Address);
	}

	// Display Character Map
	ImGuiIO& io = ImGui::GetIO();
	ImDrawList* dl = ImGui::GetWindowDrawList();
	ImVec2 pos = ImGui::GetCursorScreenPos();
	const float rectSize = 12.0f;
	uint16_t byte = 0;
	const FCharacterSet* pCharSet = GetCharacterSetFromAddress(params.CharacterSet);
	static bool bShowReadWrites = true;
	const uint16_t physAddress = params.Address.Address;

	for (int y = 0; y < params.Height; y++)
	{
		for (int x = 0; x < params.Width; x++)
		{
			const uint8_t val = state.ReadByte(physAddress + byte);
			FDataInfo* pDataInfo = state.GetReadDataInfoForAddress(physAddress + byte);
			const int framesSinceWritten = pDataInfo->LastFrameWritten == -1 ? 255 : state.CurrentFrameNo - pDataInfo->LastFrameWritten;
			const int framesSinceRead = pDataInfo->LastFrameRead == -1 ? 255 : state.CurrentFrameNo - pDataInfo->LastFrameRead;
			const int wBrightVal = (255 - std::min(framesSinceWritten << 3, 255)) & 0xff;
			const int rBrightVal = (255 - std::min(framesSinceRead << 3, 255)) & 0xff;

			if (val != params.IgnoreCharacter || wBrightVal > 0 || rBrightVal > 0)	// skip empty chars
			{
				const float xp = pos.x + (x * rectSize);
				const float yp = pos.y + (y * rectSize);
				ImVec2 rectMin(xp, yp);
				ImVec2 rectMax(xp + rectSize, yp + rectSize);

				if (val != params.IgnoreCharacter)
				{
					if (pCharSet)
					{
						const FCharUVS UVS = pCharSet->GetCharacterUVS(val);
						dl->AddImage((ImTextureID)pCharSet->Image->GetTexture(), rectMin, rectMax, ImVec2(UVS.U0, UVS.V0), ImVec2(UVS.U1, UVS.V1));
					}
					else
					{
						char valTxt[8];
						snprintf(valTxt,8, "%02x", val);
						dl->AddRect(rectMin, rectMax, 0xffffffff);
						dl->AddText(ImVec2(xp + 1, yp + 1), 0xffffffff, valTxt);
						//dl->AddText(NULL, 8.0f, ImVec2(xp + 1, yp + 1), 0xffffffff, valTxt, NULL);
					}
				}

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
		const uint16_t charAddress = pCharMap->Params.Address.Address + (xChar + (yChar * pCharMap->Params.Width));
		const uint8_t charVal = state.ReadByte(charAddress);

		const float xp = pos.x + (xChar * rectSize);
		const float yp = pos.y + (yChar * rectSize);
		const ImVec2 rectMin(xp, yp);
		const ImVec2 rectMax(xp + rectSize, yp + rectSize);
		dl->AddRect(rectMin, rectMax, 0xffffffff);

		if (ImGui::IsMouseClicked(0))
		{
			uiState.SelectedCharAddress = FAddressRef(pCharMap->Params.Address.BankId,charAddress);
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
	if (viewState.HighlightAddress.IsValid())
	{
		//const uint16_t charMapStartAddr = params.Address;
		const uint16_t charMapEndAddr = params.Address.Address + (params.Width * params.Height) - 1;
		// TODO: this needs to use banks
		if (viewState.HighlightAddress.Address >= params.Address.Address && viewState.HighlightAddress.Address <= charMapEndAddr)	// pixel
		{
			const uint16_t addrOffset = viewState.HighlightAddress.Address - params.Address.Address;
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

	ImGui::Checkbox("Show Reads & Writes", &bShowReadWrites);
	if (uiState.SelectedCharAddress.IsValid())
	{
		// Show data reads & writes
		// 
		FDataInfo* pDataInfo = state.GetDataInfoForAddress(uiState.SelectedCharAddress);
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
					uiState.SelectedCharAddress.SetInvalid();
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
		if (uiState.SelectedCharMapAddr.IsValid())
		{
			FCodeAnalysisBank* pBank = state.GetBank(uiState.SelectedCharMapAddr.BankId);
			assert(pBank != nullptr);
			state.MapBankForAnalysis(*pBank);	// map bank in so it can be read ok
			DrawCharacterMap(uiState, state, viewState);
			state.UnMapAnalysisBanks();	// map bank out
		}
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
