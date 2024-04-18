#include "CodeAnalysisJson.h"
#include "CodeAnalyser.h"
#include "CodeAnalysisPage.h"
#include "DataTypes.h"

#include <stdint.h>
#include <iomanip>
#include <fstream>
#include <sstream>

#include "Util/GraphicsView.h"
#include "Debug/DebugLog.h"
#include <iostream>
#include <json.hpp>
using json = nlohmann::json;

void WritePageToJson(const FCodeAnalysisPage& page, json& jsonDoc);
void ReadPageFromJson(FCodeAnalysisState& state, FCodeAnalysisPage& page, const json& jsonDoc);
FCommentBlock* CreateCommentBlockFromJson(const json& commentBlockJson);
FCodeInfo* CreateCodeInfoFromJson(const json& codeInfoJson);
FLabelInfo* CreateLabelInfoFromJson(const json& labelInfoJson);
void LoadDataInfoFromJson(FCodeAnalysisState& state, FDataInfo* pDataInfo, const json& dataInfoJson);
void FixupPostLoad(FCodeAnalysisState& state);

bool ExportAnalysisJson(FCodeAnalysisState& state, const char* pJsonFileName, bool bROMS)
{
	json jsonGameData;

	int pagesWritten = 0;
	const auto& banks = state.GetBanks();

	// iterate through all registered banks
	for (int bankNo = 0; bankNo < banks.size(); bankNo++)
	{
		const FCodeAnalysisBank& bank = banks[bankNo];
		if (bank.bReadOnly != bROMS)	// skip read only banks - ROM
			continue;

		json bankJson;
		bankJson["Id"] = bank.Id;
		bankJson["Description"] = bank.Description;

		//bankJson["PrimaryMappedPage"] = bank.PrimaryMappedPage;
		jsonGameData["Banks"].push_back(bankJson);

		for (int pageNo = 0; pageNo < bank.NoPages; pageNo++)
		{
			const FCodeAnalysisPage& page = bank.Pages[pageNo];
			//if (page.bUsed)
			{
				json pageData;

				WritePageToJson(page, pageData);
				jsonGameData["Pages"].push_back(pageData);
				pagesWritten++;
			}
		}
	}
	//LOGINFO("%d pages written", pagesWritten);

	// Write character sets
	for (int i = 0; i < GetNoCharacterSets(); i++)
	{
		const FCharacterSet* pCharSet = GetCharacterSetFromIndex(i);
		json jsonCharacterSet;

		jsonCharacterSet["AddressRef"] = pCharSet->Params.Address.Val;
		jsonCharacterSet["AttribsAddressRef"] = pCharSet->Params.AttribsAddress.Val;
		jsonCharacterSet["MaskInfo"] = pCharSet->Params.MaskInfo;
		jsonCharacterSet["ColourInfo"] = pCharSet->Params.ColourInfo;
		jsonCharacterSet["Dynamic"] = pCharSet->Params.bDynamic;
		jsonCharacterSet["BitmapFormat"] = (int)pCharSet->Params.BitmapFormat;
		jsonCharacterSet["PaletteNo"] = pCharSet->Params.PaletteNo;

		jsonGameData["CharacterSets"].push_back(jsonCharacterSet);
	}

	// Write character maps
	for (int i = 0; i < GetNoCharacterMaps(); i++)
	{
		const FCharacterMap* pCharMap = GetCharacterMapFromIndex(i);
		json jsonCharacterMap;

		jsonCharacterMap["AddressRef"] = pCharMap->Params.Address.Val;
		jsonCharacterMap["Width"] = pCharMap->Params.Width;
		jsonCharacterMap["Height"] = pCharMap->Params.Height;
		jsonCharacterMap["Stride"] = pCharMap->Params.Stride;
		jsonCharacterMap["CharacterSetRef"] = pCharMap->Params.CharacterSet.Val;
		jsonCharacterMap["IgnoreCharacter"] = pCharMap->Params.IgnoreCharacter;

		jsonGameData["CharacterMaps"].push_back(jsonCharacterMap);
	}

	// Write out palettes
	SavePalettesToJson(jsonGameData);
    
    // Write out data types
    const FDataTypes* pDataTypes = state.GetDataTypes();
    
    if(pDataTypes != nullptr)
    {
        json dataTypesJson;
        pDataTypes->WriteToJson(dataTypesJson);
        jsonGameData["DataTypes"] = dataTypesJson;
    }
    
	// Write file out
	std::ofstream outFileStream(pJsonFileName);
	if (outFileStream.is_open())
	{
		outFileStream << std::setw(4) << jsonGameData << std::endl;
		return true;
	}

	return false;
}

bool ImportAnalysisJson(FCodeAnalysisState& state, const char* pJsonFileName)
{
	std::ifstream inFileStream(pJsonFileName);
	if (inFileStream.is_open() == false)
		return false;

	json jsonGameData;

	inFileStream >> jsonGameData;
	inFileStream.close();

	if (jsonGameData.contains("Banks"))
	{
		for (const auto& bankJson : jsonGameData["Banks"])
		{
			FCodeAnalysisBank* pBank = state.GetBank(bankJson["Id"]);
			if (pBank != nullptr)
			{
				if (bankJson.contains("Description"))
					pBank->Description = bankJson["Description"];
				//if (bankJson.contains("Used"))
				//	pBank-> = bankJson["Used"];
				//pBank->PrimaryMappedPage = bankJson["PrimaryMappedPage"];
			}
		}
	}
	if (jsonGameData.contains("Pages"))
	{
		for (const auto& pageJson : jsonGameData["Pages"])
		{
			const int pageId = pageJson["PageId"];
			if(state.IsValidPageId(pageId) == false)
				continue;
			FCodeAnalysisPage* pPage = state.GetPage(pageId);
			if (pPage != nullptr)
			{
				ReadPageFromJson(state,*pPage, pageJson);
				pPage->bUsed = true;
			}
		}
	}

	// Below is legacy and should be removed at some point

	// info on last writer
	if (jsonGameData.contains("LastWriterStart"))
	{
		const int lwStart = jsonGameData["LastWriterStart"];

		const json& lastWriterArray = jsonGameData["LastWriter"];
		const int noWriters = (int)lastWriterArray.size();
		for (int i = 0; i < noWriters; i++)
			state.SetLastWriterForAddress(lwStart + i, state.AddressRefFromPhysicalAddress(lastWriterArray[i]));
	}

	if (jsonGameData.contains("CommentBlocks"))
	{
		for (const auto& commentBlockJson : jsonGameData["CommentBlocks"])
		{
			const uint16_t addr = commentBlockJson["Address"];
			FCommentBlock* pCommentBlock = CreateCommentBlockFromJson(commentBlockJson);
			state.SetCommentBlockForAddress(state.AddressRefFromPhysicalAddress(addr), pCommentBlock);
		}
	}

	if (jsonGameData.contains("CodeInfo"))
	{
		for (const auto codeInfoJson : jsonGameData["CodeInfo"])
		{
			if(codeInfoJson.contains("Address") == false)
			{
				std::cout << codeInfoJson;
				continue;
			}
			const uint16_t addr = codeInfoJson["Address"];
			FCodeInfo* pCodeInfo = CreateCodeInfoFromJson(codeInfoJson);
			state.SetCodeInfoForAddress(addr, pCodeInfo);

			// set operand data items
			for (int codeByte = 1; codeByte < pCodeInfo->ByteSize; codeByte++)
			{
				FDataInfo* pDataInfo = state.GetReadDataInfoForAddress(addr + codeByte);
				pDataInfo->DataType = EDataType::InstructionOperand;
				pDataInfo->ByteSize = 1;
				pDataInfo->InstructionAddress = state.AddressRefFromPhysicalAddress(addr);
			}
		}
	}

	if (jsonGameData.contains("LabelInfo"))
	{
		for (const auto labelInfoJson : jsonGameData["LabelInfo"])
		{
			const uint16_t addr = labelInfoJson["Address"];
			FLabelInfo* pLabelInfo = CreateLabelInfoFromJson(labelInfoJson);
			state.SetLabelForPhysicalAddress(addr, pLabelInfo);
		}
	}

	if (jsonGameData.contains("DataInfo"))
	{
		for (const auto dataInfoJson : jsonGameData["DataInfo"])
		{
			const int address = dataInfoJson["Address"];
			FDataInfo* pDataInfo = state.GetReadDataInfoForAddress(address);
			LoadDataInfoFromJson(state, pDataInfo, dataInfoJson);
		}
	}

	// Read in palettes.
	// May be needed to create the character set.
	LoadPalettesFromJson(jsonGameData);

	if (jsonGameData.contains("CharacterSets"))
	{
		for (const auto& charSet : jsonGameData["CharacterSets"])
		{
			FCharSetCreateParams params;
			if(charSet.contains("Address"))	// legacy
				params.Address = state.AddressRefFromPhysicalAddress(charSet["Address"]);
			if (charSet.contains("AddressRef"))
				params.Address.Val = charSet["AddressRef"];
			FixupAddressRef(state, params.Address);

			if (charSet.contains("AttribsAddress"))	// legacy
				params.AttribsAddress = state.AddressRefFromPhysicalAddress(charSet["AttribsAddress"]);
			if (charSet.contains("AttribsAddressRef"))
				params.AttribsAddress.Val = charSet["AttribsAddressRef"];

			if (charSet.contains("BitmapFormat"))
				params.BitmapFormat = (EBitmapFormat)(int)charSet["BitmapFormat"];
			if (charSet.contains("PaletteNo"))
				params.PaletteNo = charSet["PaletteNo"];
			
			params.MaskInfo = charSet["MaskInfo"];
			params.ColourInfo = charSet["ColourInfo"];
			params.bDynamic = charSet["Dynamic"];
			params.ColourLUT = state.Config.CharacterColourLUT;
			CreateCharacterSetAt(state, params);
		}
	}

	if (jsonGameData.contains("CharacterMaps"))
	{
		for (const auto& charMap : jsonGameData["CharacterMaps"])
		{
			FCharMapCreateParams params;
			
			if (charMap.contains("Address"))	// legacy
				params.Address = state.AddressRefFromPhysicalAddress(charMap["Address"]);
			if (charMap.contains("AddressRef"))
				params.Address.Val = charMap["AddressRef"];
			FixupAddressRef(state, params.Address);

			params.Width = charMap["Width"];
			params.Height = charMap["Height"];
			if (charMap.contains("Stride"))
				params.Stride = charMap["Stride"];
			else
				params.Stride = params.Width;

			if (charMap.contains("CharacterSet"))	// legacy
				params.CharacterSet = state.AddressRefFromPhysicalAddress(charMap["CharacterSet"]);
			if (charMap.contains("CharacterSetRef"))	
				params.CharacterSet.Val = charMap["CharacterSetRef"];

			params.IgnoreCharacter = charMap["IgnoreCharacter"];
			CreateCharacterMap(state, params);
		}
	}
    
    FDataTypes* pDataTypes = state.GetDataTypes();
    if (pDataTypes != nullptr && jsonGameData.contains("DataTypes"))
    {
        pDataTypes->ReadFromJson(jsonGameData["DataTypes"]);
    }

	FixupPostLoad(state);

	return true;
}

bool WriteDataInfoToJson(uint16_t addr, const FDataInfo* pDataInfo, json& jsonDoc, int addressOverride = -1)
{
	json dataInfoJson;

	assert(pDataInfo != nullptr);
	if (pDataInfo->DataType != EDataType::Byte)
		dataInfoJson["DataType"] = (int)pDataInfo->DataType;
	if (pDataInfo->DataType == EDataType::InstructionOperand)
		dataInfoJson["InstructionAddressRef"] = pDataInfo->InstructionAddress.Val;
	if (pDataInfo->DisplayType != EDataItemDisplayType::Unknown)
		dataInfoJson["DisplayType"] = pDataInfo->DisplayType;
	if (pDataInfo->ByteSize != 1)
		dataInfoJson["ByteSize"] = pDataInfo->ByteSize;
	if (pDataInfo->Flags != 0)
		dataInfoJson["Flags"] = pDataInfo->Flags;
	if (pDataInfo->Comment.empty() == false)
		dataInfoJson["Comment"] = pDataInfo->Comment;
	if (pDataInfo->PaletteNo != -1)
		dataInfoJson["PaletteNo"] = pDataInfo->PaletteNo;
	if(pDataInfo->StructByteOffset!=0)
		dataInfoJson["StructByteOffset"] = pDataInfo->StructByteOffset;

	// Charmap specific
	if (pDataInfo->DataType == EDataType::CharacterMap)
	{
		dataInfoJson["CharSetAddressRef"] = pDataInfo->CharSetAddress.Val;
		dataInfoJson["EmptyCharNo"] = pDataInfo->EmptyCharNo;
	}
	else if (pDataInfo->DataType == EDataType::Bitmap)
	{
		dataInfoJson["GraphicsSetRef"] = pDataInfo->GraphicsSetRef.Val;
	}

	if (dataInfoJson.size() != 0)	// only write it if it deviates from the normal
	{
		dataInfoJson["Address"] = addressOverride == -1 ? addr : addressOverride;
		jsonDoc["DataInfo"].push_back(dataInfoJson);
		return true;
	}

	return false;
}

void WriteCodeInfoToJson(uint16_t addr, const FCodeInfo* pCodeInfoItem, json& jsonDoc, int addressOverride = -1)
{
	json codeInfoJson;
	codeInfoJson["Address"] = addressOverride == -1 ? addr : addressOverride;
	codeInfoJson["ByteSize"] = pCodeInfoItem->ByteSize;
	if (pCodeInfoItem->bSelfModifyingCode)
		codeInfoJson["SMC"] = true;
	if (pCodeInfoItem->OperandType != EOperandType::Unknown)
		codeInfoJson["OperandType"] = (int)pCodeInfoItem->OperandType;
	if (pCodeInfoItem->StructId != -1)
		codeInfoJson["OperandSubType"] = pCodeInfoItem->StructId;
	if (pCodeInfoItem->Flags != 0)
		codeInfoJson["Flags"] = pCodeInfoItem->Flags;
	if (pCodeInfoItem->Comment.empty() == false)
		codeInfoJson["Comment"] = pCodeInfoItem->Comment;

	jsonDoc["CodeInfo"].push_back(codeInfoJson);
}

void WriteLabelInfoToJson(uint16_t addr, const FLabelInfo* pLabelInfo, json& jsonDoc, int addressOverride = -1)
{
	json labelInfoJson;
	labelInfoJson["Address"] = addressOverride == -1 ? addr : addressOverride;
	labelInfoJson["Name"] = pLabelInfo->GetName();
	if (pLabelInfo->Global)
		labelInfoJson["Global"] = pLabelInfo->Global;
	labelInfoJson["LabelType"] = pLabelInfo->LabelType;
	if (pLabelInfo->Comment.empty() == false)
		labelInfoJson["Comment"] = pLabelInfo->Comment;

	// These have moved to a binary file
	//for (const auto& reference : pLabelInfo->References.GetReferences())
	//	labelInfoJson["References"].push_back(reference.InstructionAddress);

	jsonDoc["LabelInfo"].push_back(labelInfoJson);
}

void WriteCommentBlockToJson(uint16_t addr, const FCommentBlock* pCommentBlock, json& jsonDoc, int addressOverride = -1)
{
	if (pCommentBlock->Comment.empty())
		return;

	json commentBlockJson;
	commentBlockJson["Address"] = addressOverride == -1 ? addr : addressOverride;
	commentBlockJson["Comment"] = pCommentBlock->Comment;

	jsonDoc["CommentBlocks"].push_back(commentBlockJson);
}

FCommentBlock* CreateCommentBlockFromJson(const json& commentBlockJson)
{
	FCommentBlock* pCommentBlock = FCommentBlock::Allocate();
	//pCommentBlock->Address = commentBlockJson["Address"];
	pCommentBlock->Comment = commentBlockJson["Comment"];
	return pCommentBlock;
}

FCodeInfo* CreateCodeInfoFromJson(const json& codeInfoJson)
{
	FCodeInfo* pCodeInfo = FCodeInfo::Allocate();
	pCodeInfo->ByteSize = codeInfoJson["ByteSize"];

	if (codeInfoJson.contains("SMC"))
		pCodeInfo->bSelfModifyingCode = (bool)codeInfoJson["SMC"];

	if (codeInfoJson.contains("OperandType"))
		pCodeInfo->OperandType = codeInfoJson["OperandType"];

	// hack patch for previous mistake - remove
	if(pCodeInfo->OperandType == EOperandType::Struct)
		pCodeInfo->OperandType = EOperandType::Unknown;

	if (codeInfoJson.contains("StructId"))
		pCodeInfo->StructId = codeInfoJson["StructId"];

	if (codeInfoJson.contains("Flags"))
		pCodeInfo->Flags = codeInfoJson["Flags"];

	if (codeInfoJson.contains("Comment"))
		pCodeInfo->Comment = codeInfoJson["Comment"];

	return pCodeInfo;
}

FLabelInfo* CreateLabelInfoFromJson(const json& labelInfoJson)
{
	FLabelInfo* pLabelInfo = FLabelInfo::Allocate();

	pLabelInfo->InitialiseName(((std::string)labelInfoJson["Name"]).c_str());
	if (labelInfoJson.contains("Global"))
		pLabelInfo->Global = true;

	if (labelInfoJson.contains("LabelType"))
		pLabelInfo->LabelType = (ELabelType)(int)labelInfoJson["LabelType"];
	if (labelInfoJson.contains("Comment"))
		pLabelInfo->Comment = labelInfoJson["Comment"];

	// Moved to binary file
	/*if (labelInfoJson.contains("References"))
	{
		for (const auto& reference : labelInfoJson["References"])
		{
			pLabelInfo->References.RegisterAccess(reference);
		}
	}*/

	return pLabelInfo;
}

void LoadDataInfoFromJson(FCodeAnalysisState& state, FDataInfo* pDataInfo, const json& dataInfoJson)
{
	if (dataInfoJson.contains("DataType"))
		pDataInfo->DataType = (EDataType)(int)dataInfoJson["DataType"];
	if (dataInfoJson.contains("OperandType"))	// old field
		pDataInfo->DisplayType = (EDataItemDisplayType)(int)dataInfoJson["OperandType"];
	if (dataInfoJson.contains("DisplayType"))
		pDataInfo->DisplayType = (EDataItemDisplayType)(int)dataInfoJson["DisplayType"];
	if (dataInfoJson.contains("InstructionAddress"))
		pDataInfo->InstructionAddress = state.AddressRefFromPhysicalAddress(dataInfoJson["InstructionAddress"]);
	if (dataInfoJson.contains("InstructionAddressRef"))
		pDataInfo->InstructionAddress.Val = dataInfoJson["InstructionAddressRef"];
	if (dataInfoJson.contains("ByteSize"))
		pDataInfo->ByteSize = dataInfoJson["ByteSize"];
	if (dataInfoJson.contains("Flags"))
		pDataInfo->Flags = dataInfoJson["Flags"];
	if (dataInfoJson.contains("Comment"))
		pDataInfo->Comment = dataInfoJson["Comment"];
	if (dataInfoJson.contains("PaletteNo"))
		pDataInfo->PaletteNo = dataInfoJson["PaletteNo"];
	if(dataInfoJson.contains("StructByteOffset"))
		pDataInfo->StructByteOffset = dataInfoJson["StructByteOffset"];

// Charmap specific
	if (pDataInfo->DataType == EDataType::CharacterMap)
	{
		if (dataInfoJson.contains("CharSetAddress"))
			pDataInfo->CharSetAddress = state.AddressRefFromPhysicalAddress(dataInfoJson["CharSetAddress"]);	// legacy
		if (dataInfoJson.contains("CharSetAddressRef"))
			pDataInfo->CharSetAddress.Val = dataInfoJson["CharSetAddressRef"];
		if (dataInfoJson.contains("EmptyCharNo"))
			pDataInfo->EmptyCharNo = dataInfoJson["EmptyCharNo"];
	}
	else if (pDataInfo->DataType == EDataType::Bitmap)
	{
		if (dataInfoJson.contains("GraphicsSetRef"))
			pDataInfo->GraphicsSetRef.Val = dataInfoJson["GraphicsSetRef"];

		if (pDataInfo->DisplayType == EDataItemDisplayType::Unknown)	// load fixup
			pDataInfo->DisplayType = EDataItemDisplayType::Bitmap;
	}
}




// write a 1K page to Json
// the plan is to move to this so we can support 128K games
void WritePageToJson(const FCodeAnalysisPage& page, json& jsonDoc)
{
	jsonDoc["PageId"] = page.PageId;

	int pageAddr = 0;

	while (pageAddr < FCodeAnalysisPage::kPageSize)
	{
		const FCommentBlock* pCommentBlock = page.CommentBlocks[pageAddr];
		if (pCommentBlock != nullptr)
			WriteCommentBlockToJson(pageAddr, pCommentBlock, jsonDoc);

		const FLabelInfo* pLabelInfo = page.Labels[pageAddr];
		if (pLabelInfo)
			WriteLabelInfoToJson(pageAddr, pLabelInfo, jsonDoc);

		const FCodeInfo* pCodeInfoItem = page.CodeInfo[pageAddr];
		if (pCodeInfoItem)
		{
			WriteCodeInfoToJson(pageAddr, pCodeInfoItem, jsonDoc);
			if (pCodeInfoItem->bSelfModifyingCode == false)
			{
				pageAddr += pCodeInfoItem->ByteSize;
			}
		}

		// we do want data info for SMC operands
		if (pCodeInfoItem == nullptr || pCodeInfoItem->bSelfModifyingCode == true)
		{
			const FDataInfo* pDataInfo = &page.DataInfo[pageAddr];
			WriteDataInfoToJson(pageAddr, pDataInfo, jsonDoc);
			pageAddr += pDataInfo->ByteSize;
		}

	}
}


void ReadPageFromJson(FCodeAnalysisState &state, FCodeAnalysisPage& page, const json& jsonDoc)
{
	if (jsonDoc.contains("CommentBlocks"))
	{
		for (const auto commentBlockJson : jsonDoc["CommentBlocks"])
		{
			const uint16_t pageAddr = commentBlockJson["Address"];
			FCommentBlock* pCommentBlock = CreateCommentBlockFromJson(commentBlockJson);
			page.CommentBlocks[pageAddr] = pCommentBlock;
		}
	}

	if (jsonDoc.contains("LabelInfo"))
	{
		for (const auto labelInfoJson : jsonDoc["LabelInfo"])
		{
			const uint16_t pageAddr = labelInfoJson["Address"];
			FLabelInfo* pLabelInfo = CreateLabelInfoFromJson(labelInfoJson);
			page.Labels[pageAddr] = pLabelInfo;
		}
	}

	if (jsonDoc.contains("CodeInfo"))
	{
		for (const auto codeInfoJson : jsonDoc["CodeInfo"])
		{
			const uint16_t pageAddr = codeInfoJson["Address"];
			FCodeInfo* pCodeInfo = CreateCodeInfoFromJson(codeInfoJson);
			page.CodeInfo[pageAddr] = pCodeInfo;
		}
	}

	if (jsonDoc.contains("DataInfo"))
	{
		for (const auto dataInfoJson : jsonDoc["DataInfo"])
		{
			const uint16_t pageAddr = dataInfoJson["Address"];
			FDataInfo* pDataInfo = &page.DataInfo[pageAddr];
			LoadDataInfoFromJson(state, pDataInfo, dataInfoJson);
		}
	}
}

// This is called after all the json is loaded to fix up any data referencing
void FixupPostLoad(FCodeAnalysisState& state)
{
	auto& banks = state.GetBanks();
	for (FCodeAnalysisBank& bank : banks)
	{
		assert(bank.PrimaryMappedPage !=-1);
		for(int pageNo=0;pageNo < bank.NoPages;pageNo++)
		{
			FCodeAnalysisPage& page = bank.Pages[pageNo];

			for (int addr = 0; addr < FCodeAnalysisPage::kPageSize; addr++)
			{
				const FDataInfo& dataInfo = page.DataInfo[addr];
				FAddressRef dataRef = FAddressRef(bank.Id,addr + (bank.PrimaryMappedPage + pageNo) * FCodeAnalysisPage::kPageSize);
				
				for (int readRef = 0; readRef < dataInfo.Reads.NumReferences(); readRef++)
				{
					FAddressRef ref = dataInfo.Reads.GetReferences()[readRef];
					FCodeInfo*pCodeInfo = state.GetCodeInfoForAddress(ref);
					//assert(pCodeInfo!=nullptr);
					if (pCodeInfo != nullptr)
						pCodeInfo->Reads.RegisterAccess(dataRef);
					else
						LOGWARNING("Code at 0x%04X reading from 0x%04X not found", ref.Address, dataRef.Address);
				}

				for (int writeRef = 0; writeRef < dataInfo.Writes.NumReferences(); writeRef++)
				{
					const FAddressRef ref = dataInfo.Writes.GetReferences()[writeRef];
					FCodeInfo* pCodeInfo = state.GetCodeInfoForAddress(ref);
					//assert(pCodeInfo != nullptr);
					if(pCodeInfo != nullptr)
						pCodeInfo->Writes.RegisterAccess(dataRef);
					else
						LOGWARNING("Code at 0x%04X writing to 0x%04X not found",ref.Address,dataRef.Address);
				}
			}
		}
	}

}
