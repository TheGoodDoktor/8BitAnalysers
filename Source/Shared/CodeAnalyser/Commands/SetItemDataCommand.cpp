#include "SetItemDataCommand.h"
#include "../CodeAnaysisPage.h"
#include "../CodeAnalyser.h"

void FSetItemDataCommand::Do(FCodeAnalysisState& state)
{

	if (pItem->Type == EItemType::Data)
	{
		FDataInfo* pDataItem = static_cast<FDataInfo*>(pItem);

		oldDataType = pDataItem->DataType;
		oldDataSize = pDataItem->ByteSize;

		if (pDataItem->DataType == EDataType::Byte)
		{
			pDataItem->DataType = EDataType::Word;
			pDataItem->ByteSize = 2;
			state.SetCodeAnalysisDirty();
		}
		else if (pDataItem->DataType == EDataType::Word)
		{
			pDataItem->DataType = EDataType::Byte;
			pDataItem->ByteSize = 1;
			state.SetCodeAnalysisDirty();
		}
		else if (pDataItem->DataType == EDataType::Text)
		{
			pDataItem->DataType = EDataType::Byte;
			pDataItem->ByteSize = 1;
			state.SetCodeAnalysisDirty();
		}
	}
	else if (pItem->Type == EItemType::Code)
	{
		FCodeInfo* pCodeItem = static_cast<FCodeInfo*>(pItem);
		if (pCodeItem->bDisabled == false)
		{
			pCodeItem->bDisabled = true;
			state.SetCodeAnalysisDirty();

			FLabelInfo* pLabelInfo = state.GetLabelForAddress(pItem->Address);
			if (pLabelInfo != nullptr)
				pLabelInfo->LabelType = ELabelType::Data;
		}
	}
}

void FSetItemDataCommand::Undo(FCodeAnalysisState& state)
{
	FDataInfo* pDataItem = static_cast<FDataInfo*>(pItem);
	pDataItem->DataType = oldDataType;
	pDataItem->ByteSize = oldDataSize;
}

// Set Item Code

void FSetItemCodeCommand::Do(FCodeAnalysisState& state)
{
	FCodeInfo* pCodeInfo = state.GetCodeInfoForAddress(Addr);
	if (pCodeInfo != nullptr && pCodeInfo->bDisabled == true)
	{
		pCodeInfo->bDisabled = false;
		WriteCodeInfoForAddress(state, Addr);
	}
	else
	{
		RunStaticCodeAnalysis(state, Addr);
		UpdateCodeInfoForAddress(state, Addr);
	}
	state.SetCodeAnalysisDirty();
}

void FSetItemCodeCommand::Undo(FCodeAnalysisState& state)
{
	// TODO:
}
