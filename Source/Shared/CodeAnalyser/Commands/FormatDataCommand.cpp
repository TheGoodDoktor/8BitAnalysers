#include "FormatDataCommand.h"
#include "Util/GraphicsView.h"
#include "../CodeAnalyser.h"

void UndoFormatData(FCodeAnalysisState& state, const FFormatUndoData& undoData)
{
	
}

FFormatDataCommand::FFormatDataCommand(const FDataFormattingOptions& options)
	: FormatOptions(options)
{

}

void FFormatDataCommand::Do(FCodeAnalysisState& state)
{
	FAddressRef addressRef = FormatOptions.StartAddress;
	if(addressRef.IsValid() == false)
		return;

	const FAddressRef firstAddres = addressRef;

	// TODO: Register Character Maps here?
	if (FormatOptions.DataType == EDataType::CharacterMap)
	{
		FCharMapCreateParams charMapParams;
		charMapParams.Address = addressRef;
		charMapParams.CharacterSet = FormatOptions.CharacterSet;
		charMapParams.Width = FormatOptions.ItemSize;
		charMapParams.Height = FormatOptions.NoItems;
		charMapParams.IgnoreCharacter = FormatOptions.EmptyCharNo;
		CreateCharacterMap(state, charMapParams);

		// Character map undo data
		UndoData.CharacterMapLocation = addressRef;
	}

	if (FormatOptions.AddLabelAtStart)
	{
		std::string labelText = FormatOptions.LabelName;

		// generate label if none is supplied
		if (labelText.empty())
		{
			char labelName[16];
			const char* pPrefix = "data";

			if (FormatOptions.DataType == EDataType::Bitmap)
				pPrefix = "bitmap";
			else if (FormatOptions.DataType == EDataType::CharacterMap)
				pPrefix = "charmap";
			else if (FormatOptions.DataType == EDataType::Text)
				pPrefix = "text";

			snprintf(labelName, 16, "%s_%s", pPrefix, NumStr(addressRef.Address));
			labelText = labelName;
		}

		// Add or rename label
		FLabelInfo* pLabel = state.GetLabelForAddress(addressRef);

		// Undo
		UndoData.Labels.push_back({ addressRef, FLabelInfo::Duplicate(pLabel) });	// duplicate return nullptr if passed nullptr

		if (pLabel == nullptr)
			pLabel = AddLabel(state, addressRef, labelText.c_str(), ELabelType::Data);
		else
			pLabel->ChangeName(labelText.c_str());

		pLabel->Global = true;
	}	

	for (int itemNo = 0; itemNo < FormatOptions.NoItems; itemNo++)
	{
		FDataInfo* pDataInfo = state.GetDataInfoForAddress(addressRef);

		UndoData.DataItems.push_back({ addressRef,*pDataInfo });

		pDataInfo->ByteSize = FormatOptions.ItemSize;
		pDataInfo->DataType = FormatOptions.DataType;
		pDataInfo->DisplayType = FormatOptions.DisplayType;

		if (FormatOptions.DataType == EDataType::CharacterMap)
		{
			pDataInfo->CharSetAddress = FormatOptions.CharacterSet;
			pDataInfo->EmptyCharNo = FormatOptions.EmptyCharNo;
		}
		else if (FormatOptions.DataType == EDataType::Bitmap)
		{
			pDataInfo->GraphicsSetRef = FormatOptions.GraphicsSetRef;
			pDataInfo->PaletteNo = FormatOptions.PaletteNo;
		}

		// iterate through each memory location
		for (int i = 0; i < FormatOptions.ItemSize; i++)
		{
			if (FormatOptions.ClearCodeInfo)
			{
				FCodeInfo* pCodeInfo = state.GetCodeInfoForAddress(addressRef);

				if (pDataInfo != nullptr)
					UndoData.CodeItems.push_back({ addressRef,pCodeInfo });

				state.SetCodeInfoForAddress(addressRef, nullptr);
			}

			if (FormatOptions.ClearLabels && addressRef != FormatOptions.StartAddress)	// don't remove first label
			{
				FLabelInfo* pLabel = state.GetLabelForAddress(addressRef);
				if (pLabel != nullptr)
					UndoData.Labels.push_back({ addressRef,pLabel });

				RemoveLabelAtAddress(state, addressRef);
			}

			if (state.AdvanceAddressRef(addressRef, 1) == false)
			{
				// TODO: report?
				break;
			}
			//dataAddress++;
		}
	}

	if (FormatOptions.AddCommentAtStart)
	{
		FDataInfo* pDataInfo = state.GetDataInfoForAddress(firstAddres);
		pDataInfo->Comment = FormatOptions.CommentText;
	}
}

void FFormatDataCommand::Undo(FCodeAnalysisState& state)
{
	if (UndoData.CharacterMapLocation.IsValid())
	{
		DeleteCharacterMap(UndoData.CharacterMapLocation);
		state.SetCodeAnalysisDirty(UndoData.CharacterMapLocation);
	}

	for (auto& label : UndoData.Labels)
	{
		state.SetLabelForAddress(label.first, label.second);
		state.SetCodeAnalysisDirty(label.first);
	}

	for (auto& dataItem : UndoData.DataItems)
	{
		*state.GetDataInfoForAddress(dataItem.first) = dataItem.second;
		state.SetCodeAnalysisDirty(dataItem.first);
	}

	for (auto& codeItem : UndoData.CodeItems)
	{
		state.SetCodeInfoForAddress(codeItem.first, codeItem.second);
		state.SetCodeAnalysisDirty(codeItem.first);
	}

}