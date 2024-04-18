#include "SetItemDataCommand.h"
#include "../CodeAnalysisPage.h"
#include "../CodeAnalyser.h"

void FSetItemDataCommand::Do(FCodeAnalysisState& state)
{
	if (Item.IsValid() == false)
		return;

	if (Item.Item->Type == EItemType::Data)
	{
		FDataInfo* pDataItem = static_cast<FDataInfo*>(Item.Item);

		oldDataType = pDataItem->DataType;
		oldDataSize = pDataItem->ByteSize;

		if (pDataItem->DataType == EDataType::Byte)
		{
			pDataItem->DataType = EDataType::Word;
			pDataItem->ByteSize = 2;
			state.SetCodeAnalysisDirty(Item.AddressRef);
		}
		else if (pDataItem->DataType == EDataType::Word)
		{
			pDataItem->DataType = EDataType::Byte;
			pDataItem->ByteSize = 1;
			state.SetCodeAnalysisDirty(Item.AddressRef);
		}
		else if (pDataItem->DataType == EDataType::Text)
		{
			pDataItem->DataType = EDataType::Byte;
			pDataItem->ByteSize = 1;
			state.SetCodeAnalysisDirty(Item.AddressRef);
		}
	}
	else if (Item.Item->Type == EItemType::Code)
	{
		FCodeInfo* pCodeItem = static_cast<FCodeInfo*>(Item.Item);
		if (pCodeItem->bDisabled == false)
		{
			pCodeItem->bDisabled = true;
			pCodeItem->Text.clear();

			// set all bytes to be data
			for (int i = 0; i < pCodeItem->ByteSize; i++)
			{
				FDataInfo* pOperandData = state.GetDataInfoForAddress(FAddressRef(Item.AddressRef.BankId,Item.AddressRef.Address + i));
				pOperandData->DataType = EDataType::Byte;
				pOperandData->ByteSize = 1;
			}

			state.SetCodeAnalysisDirty(Item.AddressRef);

			FLabelInfo* pLabelInfo = state.GetLabelForAddress(Item.AddressRef);
			if (pLabelInfo != nullptr)
				pLabelInfo->LabelType = ELabelType::Data;
		}
	}
}

void FSetItemDataCommand::Undo(FCodeAnalysisState& state)
{
	FDataInfo* pDataItem = static_cast<FDataInfo*>(Item.Item);
	pDataItem->DataType = oldDataType;
	pDataItem->ByteSize = oldDataSize;
	FCodeInfo* pCodeItem = static_cast<FCodeInfo*>(Item.Item);
	if(pCodeItem)
		pCodeItem->bDisabled = false;
	state.SetCodeAnalysisDirty(Item.AddressRef);
}

void FSetItemDataCommand::FixupAddressRefs(FCodeAnalysisState& state)
{
	FixupAddressRef(state, Item.AddressRef);
}


// Set Data Item Display Type 
void FSetDataItemDisplayTypeCommand::Do(FCodeAnalysisState& state)
{
	if (Item.Item->Type == EItemType::Data)
	{
		FDataInfo* pDataItem = static_cast<FDataInfo*>(Item.Item);
		OldDisplayType = pDataItem->DisplayType;
		pDataItem->DisplayType = DisplayType;
	}

}

void FSetDataItemDisplayTypeCommand::Undo(FCodeAnalysisState& state)
{
	if (Item.Item->Type == EItemType::Data)
	{
		FDataInfo* pDataItem = static_cast<FDataInfo*>(Item.Item);
		pDataItem->DisplayType = OldDisplayType;
	}
}

void FSetDataItemDisplayTypeCommand::FixupAddressRefs(FCodeAnalysisState& state)
{
	FixupAddressRef(state, Item.AddressRef);
}

// Set Item Code

void FSetItemCodeCommand::Do(FCodeAnalysisState& state)
{
	FCodeInfo* pCodeInfo = state.GetCodeInfoForAddress(Addr);
	if (pCodeInfo != nullptr && pCodeInfo->bDisabled == true)
	{
		pCodeInfo->bDisabled = false;
		WriteCodeInfoForAddress(state, Addr.Address);
	}
	else
	{
		RunStaticCodeAnalysis(state, Addr.Address);		// TODO: this needs to fill an undo buffer
		UpdateCodeInfoForAddress(state, Addr.Address);
	}
	state.SetCodeAnalysisDirty(Addr);
}

void FSetItemCodeCommand::Undo(FCodeAnalysisState& state)
{
	state.SetCodeInfoForAddress(Addr,nullptr);
	state.SetCodeAnalysisDirty(Addr);
}

void FSetItemCodeCommand::FixupAddressRefs(FCodeAnalysisState& state)
{
	FixupAddressRef(state, Addr);
}