#include "JsonExport.h"
#include "CodeAnalyser/CodeAnalyser.h"
#include "../SpectrumConstants.h"

#include <json.hpp>
#include <iomanip>
#include <fstream>
#include <sstream>

using json = nlohmann::json;

// We want to eventually move to using Json as it will allow merging 


bool ExportJson(FCodeAnalysisState& state, const char* pJsonFileName)
{
	json jsonGameData;

	int startAddress = kScreenAttrMemEnd + 1;
	int address = startAddress;

	while(address < 0x10000)
	{
		FLabelInfo* pLabelInfo = state.GetLabelForAddress(address);
		if (pLabelInfo)
		{
			json labelInfoJson;
			labelInfoJson["Address"] = pLabelInfo->Address;
			labelInfoJson["Name"] = pLabelInfo->Name;
			labelInfoJson["Global"] = pLabelInfo->Global;
			labelInfoJson["LabelType"] = pLabelInfo->LabelType;
			if (pLabelInfo->Comment.empty() == false)
				labelInfoJson["Comment"] = pLabelInfo->Comment;

			jsonGameData["LabelInfo"].push_back(labelInfoJson);
		}

		FCodeInfo* pCodeInfoItem = state.GetCodeInfoForAddress(address);
		if (pCodeInfoItem)
		{
			json codeInfoJson;
			codeInfoJson["Address"] = pCodeInfoItem->Address;
			codeInfoJson["ByteSize"] = pCodeInfoItem->ByteSize;
			if(pCodeInfoItem->OperandType != EOperandType::Unknown)
				codeInfoJson["OperandType"] = (int)pCodeInfoItem->OperandType;
			if(pCodeInfoItem->Flags != 0)
				codeInfoJson["Flags"] = pCodeInfoItem->Flags;
			if(pCodeInfoItem->Comment.empty() == false)
				codeInfoJson["Comment"] = pCodeInfoItem->Comment;

			jsonGameData["CodeInfo"].push_back(codeInfoJson);

			address += pCodeInfoItem->ByteSize;
		}
		else 
		{
			FDataInfo* pDataInfo = state.GetReadDataInfoForAddress(address);
			assert(pDataInfo != nullptr);
			json dataInfoJson;

			if (pDataInfo->DataType != DataType::Byte)
				dataInfoJson["Type"] = (int)pDataInfo->Type;
			if (pDataInfo->ByteSize != 1)
				dataInfoJson["ByteSize"] = pDataInfo->ByteSize;
			if (pDataInfo->Flags != 0)
				dataInfoJson["Flags"] = pDataInfo->Flags;
			if (pDataInfo->Comment.empty() == false)
				dataInfoJson["Comment"] = pDataInfo->Comment;

			if (dataInfoJson.size() != 0)
			{
				dataInfoJson["Address"] = pDataInfo->Address;
				jsonGameData["DataInfo"].push_back(dataInfoJson);
			}

			address += pDataInfo->ByteSize;
		}

	}


	std::ofstream outFileStream(pJsonFileName);
	if (outFileStream.is_open())
	{
		outFileStream << std::setw(4) << jsonGameData << std::endl;
		return true;
	}

	return false;
}


bool ImportJson(FCodeAnalysisState& state, const char* pJsonFileName)
{

	std::ifstream inFileStream(pJsonFileName);
	if (inFileStream.is_open() == false)
		return false;

	json jsonGameData;

	inFileStream >> jsonGameData;
	inFileStream.close();

	if (jsonGameData["CodeInfo"])
	{
		for (const auto codeInfoJson : jsonGameData["CodeInfo"])
		{
			FCodeInfo* pCodeInfo = FCodeInfo::Allocate();
			pCodeInfo->Address = codeInfoJson["Address"];
			pCodeInfo->ByteSize = codeInfoJson["ByteSize"];

			if (codeInfoJson["OperandType"])
				pCodeInfo->OperandType = codeInfoJson["OperandType"];

			if (codeInfoJson["Flags"])
				pCodeInfo->Flags = codeInfoJson["Flags"];

			if (codeInfoJson["Comment"])
				pCodeInfo->Comment = codeInfoJson["Comment"];


			for (int codeByte = 0; codeByte < pCodeInfo->ByteSize; codeByte++)	// set for whole instruction address range
				state.SetCodeInfoForAddress(pCodeInfo->Address + codeByte, pCodeInfo);
		}
	}

	if (jsonGameData["LabelInfo"])
	{
		for (const auto labelInfoJson : jsonGameData["LabelInfo"])
		{
			// TODO:
		}
	}

	if (jsonGameData["DataInfo"])
	{
		for (const auto dataInfoJson : jsonGameData["DataInfo"])
		{
			// TODO:
		}
	}

	return true;
}