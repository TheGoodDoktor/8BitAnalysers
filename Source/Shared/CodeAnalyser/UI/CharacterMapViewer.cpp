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
			const FLabelInfo* pSetLabel = state.GetLabelForAddress(pCharSet->Address);
			if (pSetLabel == nullptr)
				continue;
			if (ImGui::Selectable(pSetLabel->Name.c_str(), *pAddr == pCharSet->Address))
			{
				*pAddr = pCharSet->Address;
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
		for (int i = 0; i < GetNoCharacterSets(); i++)
		{
			const FCharacterSet* pCharSet = GetCharacterSetFromIndex(i);
			const FLabelInfo* pSetLabel = state.GetLabelForAddress(pCharSet->Address);
			if (pSetLabel == nullptr)
				continue;

			if (ImGui::Selectable(pSetLabel->Name.c_str(), selectedCharSetAddr == pCharSet->Address))
			{
				selectedCharSetAddr = pCharSet->Address;
				if (params.Address != pCharSet->Address)
				{
					params.Address = pCharSet->Address;
					params.AttribsAddress = pCharSet->AttribAddress;
					params.ColourInfo = pCharSet->ColourInfo;
					params.MaskInfo = pCharSet->MaskInfo;
				}
			}
		}
	}
	ImGui::EndChild();
	ImGui::SameLine();
	if (ImGui::BeginChild("##charsetdetails", ImVec2(0, 0), true))
	{
		FCharacterSet* pCharSet = GetCharacterSetFromAddress(selectedCharSetAddr);
		if (pCharSet)
		{
			params.Address = selectedCharSetAddr;
			DrawAddressLabel(state, viewState, selectedCharSetAddr);
			DrawMaskInfoComboBox(&params.MaskInfo);
			DrawColourInfoComboBox(&params.ColourInfo);
			if (ImGui::Button("Update Character Set"))
			{
				UpdateCharacterSet(state, *pCharSet, params);
			}
			pCharSet->Image->Draw();

		}
	}
	ImGui::EndChild();
}

void DrawCharacterMaps(FCodeAnalysisState& state, FCodeAnalysisViewState& viewState)
{
	static uint16_t selectedCharMapAddr = 0;
	static FCharMapCreateParams params;

	if (ImGui::BeginChild("##charmapselect", ImVec2(ImGui::GetWindowContentRegionWidth() * 0.25f, 0), true))
	{
		// List character maps
		for (int i = 0; i < GetNoCharacterMaps(); i++)
		{
			const FCharacterMap* pCharMap = GetCharacterMapFromIndex(i);
			const FLabelInfo* pSetLabel = state.GetLabelForAddress(pCharMap->Params.Address);
			if (pSetLabel == nullptr)
				continue;

			if (ImGui::Selectable(pSetLabel->Name.c_str(), selectedCharMapAddr == pCharMap->Params.Address))
			{
				selectedCharMapAddr = pCharMap->Params.Address;
				if(selectedCharMapAddr != params.Address)
					params = pCharMap->Params;	// copy params
			}
		}
	}
	ImGui::EndChild();
	ImGui::SameLine();
	if (ImGui::BeginChild("##charmapdetails", ImVec2(0, 0), true))
	{
		FCharacterMap* pCharMap = GetCharacterMapFromAddress(selectedCharMapAddr);

		if (pCharMap)
		{
			DrawAddressLabel(state, viewState, selectedCharMapAddr);

			// Display and edit params
			DrawCharacterSetComboBox(state, &params.CharacterSet);
			int sz[2] = { params.Width, params.Height };
			if (ImGui::InputInt2("Size (X,Y)", sz))
			{
				params.Width = sz[0];
				params.Height = sz[1];
			}
			const char* format = "%02X";
			int flags = ImGuiInputTextFlags_CharsHexadecimal;
			ImGui::InputScalar("Null Character", ImGuiDataType_U8, &params.IgnoreCharacter, 0, 0, format, flags);

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
			ImDrawList* dl = ImGui::GetWindowDrawList();
			ImVec2 pos = ImGui::GetCursorScreenPos();
			const float rectSize = 10.0f;
			uint16_t byte = 0;
			const FCharacterSet* pCharSet = GetCharacterSetFromAddress(params.CharacterSet);

			for (int y = 0; y < params.Height; y++)
			{
				for (int x = 0; x < params.Width;x++)
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
							dl->AddText(ImVec2(pos.x + 1, pos.y + 1), 0xffffffff, valTxt);
						}
					}

					byte++;	// go to next byte
				}
			}
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
