#include "CharacterMapViewer.h"
#include "../CodeAnalyser.h"
#include "../DataTypes.h"
#include "MemoryAccessGrid.h"
#include "Misc/EmuBase.h"
#include <imgui.h>
#include "misc/cpp/imgui_stdlib.h"

#include "CodeAnalyserUI.h"

#include <cmath>
#include <ImGuiSupport/ImGuiScaling.h>
#include "UIColours.h"
#include "ComboBoxes.h"

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

bool DrawIntInputXY(const char *pLabel,int &x,int &y)
{
	const float scale = ImGui_GetScaling();
	const float kNumSize = 80.0f * scale;	// size for number GUI widget
	bool bChanged = false;

	ImGui::Text("%s",pLabel);
	ImGui::PushID(pLabel);
	ImGui::SameLine();
	ImGui::SetNextItemWidth(kNumSize);
	bChanged |= ImGui::InputInt("X", &x);
	ImGui::SameLine();
	ImGui::SetNextItemWidth(kNumSize);
	bChanged |= ImGui::InputInt("Y", &y);
	ImGui::PopID();

	return bChanged;
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
	//const FLabelInfo* pLabel = pCharSet != nullptr ? state.GetLabelForAddress(addr) : nullptr;

	const char* pCharSetName = pCharSet != nullptr ? pCharSet->Params.Name.c_str() : "None";

	if (ImGui::BeginCombo("CharacterSet", pCharSetName))
	{
		if (ImGui::Selectable("None", addr.IsValid() == false))
		{
			addr = FAddressRef();
		}

		for (int i=0;i< GetNoCharacterSets();i++)
		{
			const FCharacterSet* pCharSet = GetCharacterSetFromIndex(i);
			//const FLabelInfo* pSetLabel = state.GetLabelForAddress(pCharSet->Params.Address);
			//if (pSetLabel == nullptr)
			//	continue;
			if (ImGui::Selectable(pCharSet->Params.Name.c_str(), addr == pCharSet->Params.Address))
			{
				addr = pCharSet->Params.Address;
			}
		}

		ImGui::EndCombo();
	}
}

void FCharacterMapViewer::DrawCharacterSetViewer()
{
	FCodeAnalysisState& state = pEmulator->GetCodeAnalysis();
	FCodeAnalysisViewState& viewState = state.GetFocussedViewState();

	if (ImGui::BeginChild("##charsetselect", ImVec2(ImGui::GetContentRegionAvail().x * 0.25f, 0), true))
	{
		int deleteIndex = -1;
		for (int i = 0; i < GetNoCharacterSets(); i++)
		{
			const FCharacterSet* pCharSet = GetCharacterSetFromIndex(i);
			//const FLabelInfo* pSetLabel = state.GetLabelForAddress(pCharSet->Params.Address);
			const bool bSelected = SelectedCharSetId == pCharSet->Id;

			//if (pSetLabel == nullptr)
			//	continue;

			ImGui::PushID(i);

			if (ImGui::Selectable(pCharSet->Params.Name.c_str(), bSelected))
			{
				if (SelectedCharSetId != pCharSet->Id)
				{
					CharSetParams = pCharSet->Params;
				}
				SelectedCharSetId = pCharSet->Id;
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
		FCharacterSet* pCharSet = GetCharacterSetFromId(SelectedCharSetId);
		if (pCharSet)
		{
			ImGui::InputText("Name", &CharSetParams.Name);
			if (DrawAddressInput(state, "Address", CharSetParams.Address))
			{
				//UpdateCharacterSet(state, *pCharSet, params);
			}
			DrawAddressLabel(state, viewState, CharSetParams.Address);
			DrawMaskInfoComboBox(&CharSetParams.MaskInfo);
			DrawBitmapFormatCombo(CharSetParams.BitmapFormat, state);
			if (CharSetParams.BitmapFormat == EBitmapFormat::Bitmap_1Bpp)
			{
				DrawColourInfoComboBox(&CharSetParams.ColourInfo);
				if (CharSetParams.ColourInfo == EColourInfo::MemoryLUT)
				{
					DrawAddressInput(state, "Attribs Address", CharSetParams.AttribsAddress);
				}
			}
			else
			{
				DrawPaletteCombo("Palette", "None", CharSetParams.PaletteNo, GetNumColoursForBitmapFormat(CharSetParams.BitmapFormat));
			}

			ImGui::Checkbox("Dynamic", &CharSetParams.bDynamic);
			if (ImGui::Button("Update Character Set"))
			{
				UpdateCharacterSet(state, *pCharSet, CharSetParams);
			}
			pCharSet->Image->Draw();
		}
	}
	ImGui::EndChild();
}

// this assumes the character map is in address space
void FCharacterMapViewer::DrawCharacterMap()
{
	FCodeAnalysisState& state = pEmulator->GetCodeAnalysis();
	FCodeAnalysisViewState& viewState = state.GetFocussedViewState();

	FCharacterMap* pCharMap = GetCharacterMapFromAddress(UIState.SelectedCharMapAddr);

	if (pCharMap == nullptr)
		return;
	
	const float scale = ImGui_GetScaling();
	const float kNumSize = 80.0f * scale;	// size for number GUI widget

	FCharMapCreateParams& params = UIState.Params;

	DrawAddressLabel(state, viewState, UIState.SelectedCharMapAddr);

	// Display and edit params
	DrawAddressInput(state, "Address", params.Address);
	DrawCharacterSetComboBox(state, params.CharacterSet);
	DrawIntInputXY("Size", params.Width, params.Height);
	ImGui::SameLine();
	ImGui::SetNextItemWidth(kNumSize);
	ImGui::InputInt("Stride", &params.Stride);
	DrawIntInputXY("Offset", UIState.OffsetX, UIState.OffsetY);


	ImGui::SetNextItemWidth(kNumSize);
	DrawU8Input("Null Character", &params.IgnoreCharacter);
	ImGui::SetNextItemWidth(kNumSize);
	ImGui::SliderFloat("Scale", &UIState.Scale, 0.1f, 2.0f);

	state.GetDataTypes()->DrawFlagsComboBox("Flags", params.FlagSet);


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
		formattingOptions.AddLabelAtStart = false;
		FormatData(state, formattingOptions);
		state.SetCodeAnalysisDirty(params.Address);
	}

	// Display Character Map
	ImGuiIO& io = ImGui::GetIO();
	ImDrawList* dl = ImGui::GetWindowDrawList();
	ImVec2 pos = ImGui::GetCursorScreenPos();
	//uint16_t byte = 0;
	const FCharacterSet* pCharSet = GetCharacterSetFromAddress(params.CharacterSet);
	static bool bShowReadWrites = true;
	const uint16_t physAddress = params.Address.Address;
	const float rectSize = 12.0f * scale * UIState.Scale;

	for (int y = 0; y < params.Height; y++)
	{
		for (int x = 0; x < params.Width; x++)
		{
			const int byte = (x + UIState.OffsetX) + ((y + UIState.OffsetY) * params.Stride);
			const uint8_t val = state.ReadByte(physAddress + byte);
			FDataInfo* pDataInfo = state.GetReadDataInfoForAddress(physAddress + byte);
			const int framesSinceWritten = pDataInfo->LastFrameWritten == -1 ? 255 : state.CurrentFrameNo - pDataInfo->LastFrameWritten;
			const int framesSinceRead = pDataInfo->LastFrameRead == -1 ? 255 : state.CurrentFrameNo - pDataInfo->LastFrameRead;
			const int wBrightVal = (255 - std::min(framesSinceWritten << 3, 255)) & 0xff;
			const int rBrightVal = (255 - std::min(framesSinceRead << 3, 255)) & 0xff;
			const float xp = pos.x + (x * rectSize);
			const float yp = pos.y + (y * rectSize);
			ImVec2 rectMin(xp, yp);
			ImVec2 rectMax(xp + rectSize, yp + rectSize);

			if (val != params.IgnoreCharacter || wBrightVal > 0 || rBrightVal > 0)	// skip empty chars
			{
				

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
	}

	// draw highlight rect
	const float mousePosX = io.MousePos.x - pos.x;
	const float mousePosY = io.MousePos.y - pos.y;
	if (mousePosX >= 0 && mousePosY >= 0 && mousePosX < (params.Width * rectSize) && mousePosY < (params.Height * rectSize))
	{
		const int xChar = (int)floor(mousePosX / rectSize);
		const int yChar = (int)floor(mousePosY / rectSize);
		const uint16_t charAddress = pCharMap->Params.Address.Address + (xChar + UIState.OffsetX + ((yChar + UIState.OffsetY)* pCharMap->Params.Stride));
		const uint8_t charVal = state.ReadByte(charAddress);

		const float xp = pos.x + (xChar * rectSize);
		const float yp = pos.y + (yChar * rectSize);
		const ImVec2 rectMin(xp, yp);
		const ImVec2 rectMax(xp + rectSize, yp + rectSize);
		dl->AddRect(rectMin, rectMax, 0xffffffff);

		if (ImGui::IsMouseClicked(0))
		{
			UIState.SelectedCharAddress = FAddressRef(pCharMap->Params.Address.BankId,charAddress);
			UIState.SelectedCharX = xChar;
			UIState.SelectedCharY = yChar;
		}

		// Tool Tip
		ImGui::BeginTooltip();
		ImGui::Text("Char Pos (%d,%d)", xChar, yChar);
		ImGui::Text("Value: %s %s", NumStr(charVal), NumStr(charVal,ENumberDisplayMode::Binary));
		FFlagSet* pFlagSet = state.GetDataTypes()->GetFlagsFromTypeId(params.FlagSet);
		if (pFlagSet)
		{
			ImGui::Text("Flags: %s",pFlagSet->GenerateFlagsString(charVal).c_str());
		}
		ImGui::EndTooltip();
	}

	if (UIState.SelectedCharX != -1 && UIState.SelectedCharY != -1)
	{
		const float xp = pos.x + (UIState.SelectedCharX * rectSize);
		const float yp = pos.y + (UIState.SelectedCharY * rectSize);
		const ImVec2 rectMin(xp, yp);
		const ImVec2 rectMax(xp + rectSize, yp + rectSize);
		dl->AddRect(rectMin, rectMax, Colours::GetFlashColour());
	}

	// draw hovered address
	if (viewState.HighlightAddress.IsValid())
	{
		//const uint16_t charMapStartAddr = params.Address;
		const uint16_t charMapEndAddr = params.Address.Address + (params.Stride * params.Height) - 1;
		// TODO: this needs to use banks
		if (viewState.HighlightAddress.Address >= params.Address.Address && viewState.HighlightAddress.Address <= charMapEndAddr)	// pixel
		{
			const uint16_t addrOffset = viewState.HighlightAddress.Address - params.Address.Address;
			const int charX = addrOffset % params.Stride;
			const int charY = addrOffset / params.Stride;
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
	if (UIState.SelectedCharAddress.IsValid())
	{
		// Show data reads & writes
		// 
		ImGui::Text("Address: %s", NumStr(UIState.SelectedCharAddress.Address));
		DrawAddressLabel(state, state.GetFocussedViewState(), UIState.SelectedCharAddress);
		FDataInfo* pDataInfo = state.GetDataInfoForAddress(UIState.SelectedCharAddress);

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

void FCharacterMapViewer::DrawCharacterMaps()
{
	FCodeAnalysisState& state = pEmulator->GetCodeAnalysis();
	FCodeAnalysisViewState& viewState = state.GetFocussedViewState();

	if (ImGui::BeginChild("##charmapselect", ImVec2(ImGui::GetContentRegionAvail().x * 0.25f, 0), true))
	{
		int deleteIndex = -1;

		// List character maps
		for (int i = 0; i < GetNoCharacterMaps(); i++)
		{
			const FCharacterMap* pCharMap = GetCharacterMapFromIndex(i);
			const FLabelInfo* pSetLabel = state.GetLabelForAddress(pCharMap->Params.Address);
			const bool bSelected = UIState.SelectedCharMapAddr == pCharMap->Params.Address;

			if (pSetLabel == nullptr)
				continue;

			ImGui::PushID(i);

			if (ImGui::Selectable(pSetLabel->GetName(), bSelected))
			{
				UIState.SelectedCharMapAddr = pCharMap->Params.Address;
				if (UIState.SelectedCharMapAddr != UIState.Params.Address)
				{
					UIState.Params = pCharMap->Params;	// copy params
					UIState.SelectedCharAddress.SetInvalid();
					UIState.SelectedCharX = -1;
					UIState.SelectedCharY = -1;
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
		if (UIState.SelectedCharMapAddr.IsValid())
		{
			FCodeAnalysisBank* pBank = state.GetBank(UIState.SelectedCharMapAddr.BankId);
			assert(pBank != nullptr);
			state.MapBankForAnalysis(*pBank);	// map bank in so it can be read ok
			DrawCharacterMap();
			state.UnMapAnalysisBanks();	// map bank out
		}
	}
	ImGui::EndChild();
}


class FCharacterMapGrid : public FMemoryAccessGrid
{
public:
	FCharacterMapGrid(FCodeAnalysisState* pCodeAnalysis, int xGridSize, int yGridSize):FMemoryAccessGrid(pCodeAnalysis,xGridSize,yGridSize)
	{
		bShowValues = true;
		bShowReadWrites = true;
		bOutlineAllSquares = true;	
	}

	FAddressRef GetGridSquareAddress(int x, int y) override
	{
		const int offset = x + OffsetX + ((y + OffsetY) * GridStride);
		FAddressRef squareAddress = Address;
		if(CodeAnalysis->AdvanceAddressRef(squareAddress,offset))
			return squareAddress;
		else
		return FAddressRef();
	}

	void OnDraw() override
	{
		const float scale = ImGui_GetScaling();
		const float kNumSize = 80.0f * scale;	// size for number GUI widget
		
		// Size
		DrawIntInputXY("Grid Size", GridSizeX, GridSizeY);
		/*ImGui::Text("Grid Size");
		ImGui::SameLine();
		ImGui::SetNextItemWidth(kNumSize);
		ImGui::InputInt("##GridSizeX", &GridSizeX);
		ImGui::SameLine();
		ImGui::SetNextItemWidth(kNumSize);
		ImGui::InputInt("##GridSizeY", &GridSizeY);*/

		// Stride
		ImGui::SetNextItemWidth(kNumSize);
		ImGui::Text("Grid Stride");
		ImGui::SameLine();
		ImGui::InputInt("##GridStride", &GridStride);

		GridSizeX = std::min(GridSizeX,GridStride);
			

		// Offset
		DrawIntInputXY("Offset", OffsetX, OffsetY);
		/*
		ImGui::Text("Offset");
		ImGui::SameLine();
		ImGui::SetNextItemWidth(kNumSize);
		ImGui::InputInt("##GridOffsetX", &OffsetX);
		ImGui::SameLine();
		ImGui::SetNextItemWidth(kNumSize);
		ImGui::InputInt("##GridOffsetY", &OffsetY);
		*/

		// Character Set
		DrawCharacterSetComboBox(*CodeAnalysis, CharacterSet);

		ImGui::SetNextItemWidth(120.0f * scale);
		bool bUpdated = false;
		if (GetNumberDisplayMode() == ENumberDisplayMode::Decimal)
			bUpdated |= ImGui::InputInt("##Address", &PhysicalAddress, 1, 8, ImGuiInputTextFlags_CharsDecimal);
		else
			bUpdated |= ImGui::InputInt("##Address", &PhysicalAddress, 1, 8, ImGuiInputTextFlags_CharsHexadecimal);
		if(bUpdated)
			Address = CodeAnalysis->AddressRefFromPhysicalAddress(PhysicalAddress);
		if(Address.IsValid())
			DrawAddressLabel(*CodeAnalysis, CodeAnalysis->GetFocussedViewState(), Address);
		ImGui::SameLine();
		ImGui::Checkbox("##UseIgnoreValue", &bUseIgnoreValue);
		ImGui::SameLine();
		ImGui::SetNextItemWidth(16.0f * scale);
		ImGui::InputScalar("Ignore Value", ImGuiDataType_U8,&IgnoreValue);
		ImGui::Checkbox("DrawGrid",&bDrawGrid);

		// create and format a character map
		if (ImGui::Button("Create"))
		{
			FDataFormattingOptions formattingOptions;
			formattingOptions.StartAddress = Address;
			formattingOptions.DataType = EDataType::CharacterMap;
			formattingOptions.ItemSize = GridSizeX;
			formattingOptions.NoItems = GridSizeY;
			formattingOptions.ClearCodeInfo = true;
			formattingOptions.ClearLabels = true;
			formattingOptions.AddLabelAtStart = true;
			formattingOptions.CharacterSet = CharacterSet;
			formattingOptions.RegisterItem = true;
			FormatData(*CodeAnalysis, formattingOptions);
			CodeAnalysis->SetCodeAnalysisDirty(formattingOptions.StartAddress);
		}

		GridSquareSize = 14.0f * scale;	// to fit an 8x8 square on a scaling screen image
	}

	void DrawBackground(float x, float y) override
	{
		const FCharacterSet* pCharSet = GetCharacterSetFromAddress(CharacterSet);
		if(pCharSet == nullptr)
			return;

		ImDrawList* dl = ImGui::GetWindowDrawList();
		ImVec2 pos(x,y);
		const float scale = ImGui_GetScaling();
		const float rectSize = GridSquareSize;

		for (int y = 0; y < GridSizeY; y++)
		{
			for (int x = 0; x < GridSizeX; x++)
			{
				const int byte = (x + OffsetX) + ((y + OffsetY) * GridStride);
				const uint8_t val = CodeAnalysis->ReadByte(PhysicalAddress + byte);

				const float xp = pos.x + (x * rectSize);
				const float yp = pos.y + (y * rectSize);
				ImVec2 rectMin(xp, yp);
				ImVec2 rectMax(xp + rectSize, yp + rectSize);
				const FCharUVS UVS = pCharSet->GetCharacterUVS(val);
				dl->AddImage((ImTextureID)pCharSet->Image->GetTexture(), rectMin, rectMax, ImVec2(UVS.U0, UVS.V0), ImVec2(UVS.U1, UVS.V1));

			}
		}
	}

	void SetAddress(FAddressRef addr) 
	{ 
		Address = addr; 
		PhysicalAddress = addr.Address;
	}

	void FixupAddressRefs() override
	{
		FMemoryAccessGrid::FixupAddressRefs();
		FixupAddressRef(*CodeAnalysis, Address);
		SetAddress(Address);
		FixupAddressRef(*CodeAnalysis, CharacterSet);
	}

	FAddressRef Address;
	int PhysicalAddress = 0;

	FAddressRef		CharacterSet;

};

bool	FCharacterMapViewer::Init(void) 
{ 
	ViewerGrid = new FCharacterMapGrid(&pEmulator->GetCodeAnalysis(), 16, 16);

	return true; 
}

void	FCharacterMapViewer::Shutdown() 
{

}

void FCharacterMapViewer::DrawCharacterMapViewer(void)
{
	ViewerGrid->Draw();
}

void FCharacterMapViewer::GoToAddress(FAddressRef addr)
{
	ViewerGrid->SetAddress(addr);
}

void FCharacterMapViewer::SetGridSize(int x, int y)
{
	ViewerGrid->SetGridSize(x,y);
}

void FCharacterMapViewer::SetGridStride(int stride)
{
	ViewerGrid->SetGridStride(stride);
}

void FCharacterMapViewer::DrawUI(void)
{
	if (ImGui::BeginTabBar("CharacterMapTabs"))
	{
		if (ImGui::BeginTabItem("Character Map Viewer"))
		{
			DrawCharacterMapViewer();
			ImGui::EndTabItem();
		}

		if (ImGui::BeginTabItem("Character Sets"))
		{
			DrawCharacterSetViewer();
			ImGui::EndTabItem();
		}

		if (ImGui::BeginTabItem("Character Maps"))
		{
			DrawCharacterMaps();
			ImGui::EndTabItem();
		}

		ImGui::EndTabBar();
	}
}

void FCharacterMapViewer::FixupAddressRefs()
{
	ViewerGrid->FixupAddressRefs();

	FCodeAnalysisState& state = pEmulator->GetCodeAnalysis();

	//FixupAddressRef(state, SelectedCharSetAddr);
	FixupAddressRef(state, CharSetParams.Address);
	FixupAddressRef(state, CharSetParams.AttribsAddress);

	FixupAddressRef(state, UIState.SelectedCharAddress);
	FixupAddressRef(state, UIState.SelectedCharMapAddr);
	FixupAddressRef(state, UIState.Params.Address);
	FixupAddressRef(state, UIState.Params.CharacterSet);
}
