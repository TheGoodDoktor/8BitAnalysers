#include "DataTypes.h"

#include <imgui.h>
#include <misc/cpp/imgui_stdlib.h>
#include "CodeAnalyser/UI/CodeAnalyserUI.h"
#include "CodeAnalyser/UI/UIColours.h"

#include <json.hpp>
using json = nlohmann::json;


int32_t FDataType::NextTypeId = 0;

void FDataTypes::Reset()
{
	EnumTypes.Reset();
	FlagTypes.Reset();
	StructTypes.Reset();
}

bool FDataTypeList::AddType(FDataType* pType)
{
	if(GetTypeFromName(pType->Name.c_str()) != nullptr) // don't allow dupes
		return false;
	
	Items.push_back(pType);
	return true;
}

FDataType*  FDataTypeList::GetTypeFromId(int32_t typeId)
{
	for(auto pItem : Items)
	{
		if(pItem->TypeId == typeId)
			return pItem;
	}
	
	return nullptr;
}

FDataType*  FDataTypeList::GetTypeFromName(const char* pName)
{
	for(auto pItem : Items)
	{
		if(pItem->Name == pName)
			return pItem;
	}
	
	return nullptr;
}

bool        FDataTypeList::RemoveType(int32_t typeId)
{
	for(auto itemIt = Items.begin();itemIt!=Items.end();++itemIt)
	{
		FDataType* pDataType = *itemIt;
		if(pDataType->TypeId == typeId)
		{
			Items.erase(itemIt);
			delete pDataType;
			return true;
		}
	}
	
	return false;
}

// Enums

bool FEnumTypeList::Add(const char* pName)
{
	if(strlen(pName) == 0)
		return false;
	
	FEnum* pNewEnum = new FEnum(pName);
	
	if(AddType(pNewEnum) == false)
	{
		delete pNewEnum;
		pNewEnum = nullptr;
		return false;
	}
	
	//SelectedType = pNewEnum->TypeId;
	//TypeName = pNewEnum->Name;
	
	return true;
}

bool FEnum::AddEntry(const char* name, int val)
{
	// check if value already exists
	for(auto& entry : Entries)
	{
		if(entry.Name == name)
			return false;
	}
	
	Entries.push_back(FEnumEntry(name,val));
	return true;
}

void FEnum::DrawDetailsUI()
{
	// Add new enum value
	if(ImGui::Button("Add Value"))
	{
		const int kNameTextSize = 16;
		char enumName[kNameTextSize];
		const int enumVal = (int)Entries.size();
		snprintf(enumName,kNameTextSize,"%s_%d",Name.c_str(),enumVal);
		AddEntry(enumName,enumVal);
	}

	// Table
	//static ImGuiTableFlags flags = ImGuiTableFlags_SizingFixedFit | ImGuiTableFlags_RowBg | ImGuiTableFlags_Borders | ImGuiTableFlags_Resizable | ImGuiTableFlags_Reorderable | ImGuiTableFlags_Hideable;
	static ImGuiTableFlags flags = ImGuiTableFlags_Borders | ImGuiTableFlags_RowBg;
	
	if (ImGui::BeginTable("enumtable", 2, flags))
	{
		ImGui::TableSetupColumn("Name");
		ImGui::TableSetupColumn("Value");
		ImGui::TableHeadersRow();
		
		bool bEdited = false;
		int id=0;
		for(FEnumEntry& enumEntry : Entries)
		{
			ImGui::PushID(++id);
			ImGui::TableNextRow();
			
			ImGui::TableSetColumnIndex(0);
			bEdited |= ImGui::InputText("##name",&enumEntry.Name);
			
			ImGui::TableSetColumnIndex(1);
			bEdited |= ImGui::InputInt("##value", &enumEntry.Value,0,0);
			ImGui::PopID();
		}
		
		ImGui::EndTable();
	}
}

void    FEnum::ReadJson(const json& jsonIn)
{
	Name = jsonIn["Name"];
	TypeId = jsonIn["TypeId"];
	Entries.clear();
	
	for(const auto& enumEntryJson : jsonIn["Entries"])
	{
		Entries.emplace_back(enumEntryJson["Name"],enumEntryJson["Value"]);
	}
}

void    FEnum:: WriteJson(json& jsonOut) const
{
	jsonOut["Name"] = Name;
	jsonOut["TypeId"] = TypeId;
	
	json enumEntriesJson;
	
	for(const FEnumEntry& enumEntry : Entries)
	{
		json enumValJson;
		
		enumValJson["Name"] = enumEntry.Name;
		enumValJson["Value"] = enumEntry.Value;
		
		enumEntriesJson.push_back(enumValJson);
	}
	
	jsonOut["Entries"] = enumEntriesJson;
}

// Flags

bool FFlagTypeList::Add(const char* pName)
{
	if(strlen(pName) == 0)
		return false;
	
	FFlagSet* pNewFlagSet = new FFlagSet(pName);
	
	if(AddType(pNewFlagSet) == false)
	{
		delete pNewFlagSet;
		pNewFlagSet = nullptr;
		return false;
	}
	
	return true;
};



FFlagSet::FFlagSet(const char *pName):FDataType(pName)
{
	for(int i=0;i<8;i++)
	{
		Bits.emplace_back(pName,i);
	}
}

void FFlagSet::DrawDetailsUI()
{
	static ImGuiTableFlags flags = ImGuiTableFlags_Borders | ImGuiTableFlags_RowBg;
	
	if (ImGui::BeginTable("flagtable", 2, flags))
	{
		ImGui::TableSetupColumn("Bit");
		ImGui::TableSetupColumn("Name");
		ImGui::TableHeadersRow();
		
		bool bEdited = false;
		int bitNo=0;
		for(FFlagBit& flagBit : Bits)
		{
			ImGui::PushID(bitNo);
			ImGui::TableNextRow();
			
			ImGui::TableSetColumnIndex(0);
			ImGui::Text("%d",bitNo);
			ImGui::TableSetColumnIndex(1);
			bEdited |= ImGui::InputText("##name",&flagBit.Name);
			
			bitNo++;
			ImGui::PopID();
		}
		
		ImGui::EndTable();
	}
}

void    FFlagSet::ReadJson(const json& jsonIn) 
{
	Name = jsonIn["Name"];
	TypeId = jsonIn["TypeId"];
	Bits.clear();
	
	for(const auto& bitValJson : jsonIn["FlagBits"])
	{
		Bits.emplace_back(bitValJson["Name"]);
	}
 
}
void    FFlagSet:: WriteJson(json& jsonOut) const
{
	jsonOut["Name"] = Name;
	jsonOut["TypeId"] = TypeId;
	
	json flagBitsJson;
	for(const FFlagBit& flagBit : Bits)
	{
		json bitValJson;
		
		bitValJson["Name"] = flagBit.Name;
		
		flagBitsJson.push_back(bitValJson);
	}
	
	jsonOut["FlagBits"] = flagBitsJson;
}


FFlagSet*   FDataTypes::CreateNewFlagSet(const char* name)
{
	FFlagSet* pNewFlagSet = new FFlagSet(name);
	
	if(FlagTypes.AddType(pNewFlagSet) == false)
	{
		delete pNewFlagSet;
		pNewFlagSet = nullptr;
	}
	
	return pNewFlagSet;
}

// Structs

bool FStructTypeList::Add(const char* pName)
{
	if(strlen(pName) == 0)
		return false;
	
	FStruct* pNewStruct = new FStruct(pName);
	
	if(AddType(pNewStruct) == false)
	{
		delete pNewStruct;
		pNewStruct = nullptr;
		return false;
	}
	
	return true;
};

int DataTypeSize(EDataType dataType)
{
	switch (dataType)
	{
		case EDataType::Word:
			return 2;
		default:
			return 1;
	}
}

void FStruct::CalcByteOffsets()
{
	int byteNo = 0;
	for (FStructMember& structMember : Members)
	{
		structMember.ByteOffset = byteNo;
		byteNo += DataTypeSize(structMember.DataType);
	}
}

FStructMember* FStruct::GetStructMemberFromByteOffset(int byteOffset)
{
	for (FStructMember& structMember : Members)
	{
		if(structMember.ByteOffset == byteOffset)
			return &structMember;
	}

	return nullptr;
}


void FStruct::DrawDetailsUI()
{
	if(ImGui::Button("Add Member"))
	{
		FStructMember newMember;
		const int kNameTextSize = 16;
		char memberName[kNameTextSize];
		snprintf(memberName,kNameTextSize,"%s_%d",Name.c_str(),(int)Members.size());
		newMember.Name = memberName;
		newMember.DataType = EDataType::Byte;
		Members.push_back(newMember);
		CalcByteOffsets();
	}
	
	static ImGuiTableFlags flags = ImGuiTableFlags_Borders | ImGuiTableFlags_RowBg;
	
	if (ImGui::BeginTable("structtable", 3, flags))
	{
		ImGui::TableSetupColumn("Byte");
		ImGui::TableSetupColumn("Name");
		ImGui::TableSetupColumn("Type");
		ImGui::TableHeadersRow();
		
		bool bEdited = false;
		int memberIndex = 0;
		for(FStructMember& structMember : Members)
		{
			ImGui::PushID(memberIndex);
			ImGui::TableNextRow();
			
			ImGui::TableSetColumnIndex(0);
			ImGui::Text("%d", structMember.ByteOffset);
			ImGui::TableSetColumnIndex(1);
			bEdited |= ImGui::InputText("##name",&structMember.Name);
			ImGui::TableSetColumnIndex(2);
			bEdited |= DrawDataTypeCombo("##Type", structMember.DataType);
			ImGui::PopID();
			memberIndex++;
		}

		if(bEdited)
			CalcByteOffsets();

		ImGui::EndTable();
	}
}



void    FStruct::ReadJson(const json& jsonIn)
{
	Name = jsonIn["Name"];
	TypeId = jsonIn["TypeId"];
	Members.clear();
	
	for(const auto& jsonMember : jsonIn["Members"])
	{
		FStructMember member;
		member.Name = jsonMember["Name"];
		member.DataType = (EDataType)jsonMember["Type"];
		Members.push_back(member);
	}

	CalcByteOffsets();
}

void    FStruct:: WriteJson(json& jsonOut) const
{
	jsonOut["Name"] = Name;
	jsonOut["TypeId"] = TypeId;
 
	json membersJson;
	for(const auto& member : Members)
	{
		json memberJson;
		memberJson["Name"] = member.Name;
		memberJson["Type"] = (int)member.DataType;
		membersJson.push_back(memberJson);
	}
	jsonOut["Members"] = membersJson;
}


bool FDataTypes::WriteToJson(json& jsonDataTypes) const
{
	// Enums
	json enumsJson;
	for(auto enumIt : EnumTypes.Items)
	{
		json jsonOut;
		enumIt->WriteJson(jsonOut);
		enumsJson.push_back(jsonOut);
	}
	jsonDataTypes["Enums"] = enumsJson;
	
	// Flags
	json flagsJson;
	for(auto flagsIt : FlagTypes.Items)
	{
		json jsonOut;
		flagsIt->WriteJson(jsonOut);
		flagsJson.push_back(jsonOut);
	}
	jsonDataTypes["Flags"] = flagsJson;
	
	// Structs
	json structsJson;
	for(auto structsIt : StructTypes.Items)
	{
		json jsonOut;
		structsIt->WriteJson(jsonOut);
		structsJson.push_back(jsonOut);
	}
	jsonDataTypes["Structs"] = structsJson;
	
	// Housekeeping
	jsonDataTypes["NextItemId"] = FDataType::NextTypeId;
	
	return false;
}

bool FDataTypes::ReadFromJson(const json& jsonDataTypes)
{
	EnumTypes.Reset();
	
	for(const auto& enumJson : jsonDataTypes["Enums"])
	{
		FEnum* pNewEnum = new FEnum;
		pNewEnum->ReadJson(enumJson);
		EnumTypes.AddType(pNewEnum);
	}
	
	for(const auto& flagJson : jsonDataTypes["Flags"])
	{
		FFlagSet* pNewFlagSet = new FFlagSet;
		pNewFlagSet->ReadJson(flagJson);
		FlagTypes.AddType(pNewFlagSet);
	}
	
	for(const auto& structJson : jsonDataTypes["Structs"])
	{
		FStruct* pNewStruct = new FStruct;
		pNewStruct->ReadJson(structJson);
		StructTypes.AddType(pNewStruct);
	}
	
	FDataType::NextTypeId = jsonDataTypes["NextItemId"];
	
	return false;
}


bool DrawTypeListCombo(const char* pLabel, int& selection, const FDataTypeList& typeList)
{
	bool bChanged = false;
	const char* pPreviewStr = nullptr;
	for (const auto& type : typeList.Items)
	{
		if (type->TypeId == selection)
		{
			pPreviewStr = type->Name.c_str();
			break;
		}
	}

	if (ImGui::BeginCombo(pLabel, pPreviewStr))
	{
		for (const auto type : typeList.Items)
		{
			const bool isSelected = (selection == type->TypeId);
			if (ImGui::Selectable(type->Name.c_str(), isSelected))
			{
				selection = type->TypeId;
				bChanged = true;
			}
		}

		ImGui::EndCombo();
	}

	return bChanged;
}

bool FDataTypes::DrawStructComboBox(const char* pLabel, int& selection)
{
	return DrawTypeListCombo(pLabel,selection, StructTypes);
}

void FDataTypes::DrawStructMember(int structId, int byteOffset, bool bDrawStructName)
{
	FStruct* pStruct = GetStructFromTypeId(structId);
	if (pStruct)
	{
		// Get member from byte offset and display
		FStructMember* pStructMember = pStruct->GetStructMemberFromByteOffset(byteOffset);
		if (pStructMember != nullptr)
		{
			ImGui::SameLine();
			ImGui::PushStyleColor(ImGuiCol_Text, Colours::comment);
			if(bDrawStructName)
				ImGui::Text("(%s::%s)", pStruct->Name.c_str(), pStructMember->Name.c_str());
			else
				ImGui::Text("(%s)",pStructMember->Name.c_str());
			ImGui::PopStyleColor();
		}
	}
}

// Viewer

bool FDataTypesViewer::Init(void)
{
	FCodeAnalysisState& state = pEmulator->GetCodeAnalysis();

	assert(state.GetDataTypes() != nullptr);
	DataTypes = state.GetDataTypes();
	
	return true;
}

void FDataTypesViewer::Shutdown(void)
{
}



void FDataTypesViewer::DrawTypeList(FDataTypeList& typeList)
{
	assert(DataTypes != nullptr);
	const float lineHeight = ImGui::GetTextLineHeight();
	
	// TODO: This is type specific
	if(ImGui::Button("Add"))
	{
		typeList.Add(NameText.c_str());
	}
	ImGui::SameLine();
	
	// TODO: Make red if type exists?
	const bool bDupe = typeList.GetTypeFromName(NameText.c_str()) != nullptr;
	
	// Draw red if dupe
	if(bDupe)
		ImGui::PushStyleColor(ImGuiCol_Text, Colours::error);
	
	if(ImGui::InputText("Name", &NameText))
	{
		
	}
	if(bDupe)
		ImGui::PopStyleColor();
	
	if (ImGui::BeginChild("##typesselect", ImVec2(ImGui::GetContentRegionAvail().x * 0.25f, 0), true))
	{
		if (ImGui::BeginChild("##typeslist", ImVec2(0, -lineHeight * 2.0f), false))
		{
			// list enums as selectables
			for(const auto pType : typeList.Items)
			{
				if(ImGui::Selectable(pType->Name.c_str(),pType->TypeId == SelectedType))
				{
					SelectedType = pType->TypeId;
					TypeName = pType->Name;
				}
			}
		}
		ImGui::EndChild();
	}
	if(ImGui::Button("Remove"))
	{
		typeList.RemoveType(SelectedType);
	}
	ImGui::EndChild();
	
	ImGui::SameLine();
	if (ImGui::BeginChild("##typedetails", ImVec2(0, 0), true))
	{
		// Show details of selected data type
		FDataType* pDataType = typeList.GetTypeFromId(SelectedType);
		if(pDataType != nullptr)
		{
			// stop duplicate enums
			const bool bDupe = typeList.GetTypeFromName(TypeName.c_str()) != nullptr && TypeName != pDataType->Name;
			
			// Draw red if dupe
			if(bDupe)
				ImGui::PushStyleColor(ImGuiCol_Text, Colours::error);
			if(ImGui::InputText("Name",&TypeName, ImGuiInputTextFlags_EnterReturnsTrue))
			{
				if(bDupe == false)
					pDataType->Name = TypeName;
			}
			if(bDupe)
				ImGui::PopStyleColor();
			
			pDataType->DrawDetailsUI();
			
		}
	}
	ImGui::EndChild();
	
	
}

void FDataTypesViewer::DrawUI()
{
	FCodeAnalysisState& state = pEmulator->GetCodeAnalysis();
	FCodeAnalysisViewState& viewState = state.GetFocussedViewState();

	if (ImGui::BeginTabBar("DataTypesTabs"))
	{
		if (ImGui::BeginTabItem("Enums"))
		{
			DrawTypeList(DataTypes->GetEnums());
			ImGui::EndTabItem();
		}

		if (ImGui::BeginTabItem("Flags"))
		{
			DrawTypeList(DataTypes->GetFlagSets());
			ImGui::EndTabItem();
		}
		if (ImGui::BeginTabItem("Structs"))
		{
			DrawTypeList(DataTypes->GetStructs());
			ImGui::EndTabItem();
		}

		ImGui::EndTabBar();
	}
}


