#include "FormatDataCommand.h"
#include "Util/GraphicsView.h"
#include "../CodeAnalyser.h"
#include "CodeAnalyser/DataTypes.h"

void UndoFormatData(FCodeAnalysisState& state, const FFormatUndoData& undoData)
{
	
}

FFormatDataCommand::FFormatDataCommand(const FDataFormattingOptions& options)
	: FormatOptions(options)
{

}

void FFormatDataCommand::Do(FCodeAnalysisState& state)
{
	
	if(FormatOptions.StartAddress.IsValid() == false)
		return;

	const FAddressRef firstAddress = FormatOptions.StartAddress;

	// Optionally register character maps
	if (FormatOptions.DataType == EDataType::CharacterMap && FormatOptions.RegisterItem == true)
	{
		FCharMapCreateParams charMapParams;
		charMapParams.Address = firstAddress;
		charMapParams.CharacterSet = FormatOptions.CharacterSet;
		charMapParams.Width = FormatOptions.ItemSize;
		charMapParams.Height = FormatOptions.NoItems;
		charMapParams.IgnoreCharacter = FormatOptions.EmptyCharNo;
		charMapParams.bAddLabel = false;
		CreateCharacterMap(state, charMapParams);

		// Character map undo data
		UndoData.CharacterMapLocation = firstAddress;
	}

	

	{
		FAddressRef addressRef = firstAddress;
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
			else if (FormatOptions.DataType == EDataType::Struct)
			{
				const FStruct* pStruct = state.GetDataTypes()->GetStructFromTypeId(FormatOptions.StructId);
				if (pStruct)
				{
					// Format memory as struct
					for (const auto& member : pStruct->Members)
					{
						FAddressRef memberAddr = addressRef;
						state.AdvanceAddressRef(memberAddr,member.ByteOffset);
						FDataInfo* pMemberDataInfo = state.GetDataInfoForAddress(memberAddr);
						pMemberDataInfo->DataType = member.DataType;
						pMemberDataInfo->bStructMember = true;
						pMemberDataInfo->SubTypeId = FormatOptions.StructId;
						pMemberDataInfo->StructByteOffset = member.ByteOffset;

						// TODO: undo buffer
					}
				}
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

				if (FormatOptions.ClearLabels)// && addressRef != FormatOptions.StartAddress)	// don't remove first label
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

			snprintf(labelName, 16, "%s_%s", pPrefix, NumStr(firstAddress.Address));
			labelText = labelName;
		}

		// Add or rename label
		FLabelInfo* pLabel = state.GetLabelForAddress(firstAddress);

		// Undo
		UndoData.Labels.push_back({ firstAddress, FLabelInfo::Duplicate(pLabel) });	// duplicate return nullptr if passed nullptr

		if (pLabel == nullptr)
			pLabel = AddLabel(state, firstAddress, labelText.c_str(), ELabelType::Data);
		else
			pLabel->ChangeName(labelText.c_str());

		pLabel->Global = true;
	}

	if (FormatOptions.AddCommentAtStart)
	{
		FCommentBlock* pCommentBlock = state.GetCommentBlockForAddress(firstAddress);
		// Undo
		UndoData.CommentBlocks.push_back({ firstAddress, FCommentBlock::Duplicate(pCommentBlock) });	// duplicate return nullptr if passed nullptr

		if (pCommentBlock == nullptr)
		{
			pCommentBlock = AddCommentBlock(state, firstAddress);
			pCommentBlock->Comment = FormatOptions.CommentText;
		}
		else
		{
			pCommentBlock->Comment += '\n' + FormatOptions.CommentText;
		}
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

	for (auto& commentBlock : UndoData.CommentBlocks)
	{
		state.SetCommentBlockForAddress(commentBlock.first, commentBlock.second);
		state.SetCodeAnalysisDirty(commentBlock.first);
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

void FFormatDataCommand::FixupAddressRefs(FCodeAnalysisState& state)
{
	FixupAddressRef(state, FormatOptions.GraphicsSetRef);
	FixupAddressRef(state, FormatOptions.CharacterSet);
	FixupAddressRef(state, FormatOptions.StartAddress);

	FixupAddressRef(state, UndoData.CharacterMapLocation);

	for (auto& label : UndoData.Labels)
	{
		FixupAddressRef(state, label.first);
	}
	
	for (auto& dataItem : UndoData.DataItems)
	{
		FixupAddressRef(state, dataItem.first);
	}

	for (auto& commentBlock : UndoData.CommentBlocks)
	{
		FixupAddressRef(state, commentBlock.first);
	}

	for (auto& dataItem : UndoData.DataItems)
	{
		FixupAddressRef(state, dataItem.first);
	}

	for (auto& codeItem : UndoData.CodeItems)
	{
		FixupAddressRef(state, codeItem.first);
	}
}


FBatchFormatDataCommand::FBatchFormatDataCommand(const FBatchDataFormattingOptions& options)
	:BatchFormatOptions(options)
{

}


void FBatchFormatDataCommand::Do(FCodeAnalysisState& state)
{
	FDataFormattingOptions options = BatchFormatOptions.FormatOptions;

	for (int i = 0; i < BatchFormatOptions.NoItems; i++)
	{
		char prefixTxt[32];
		snprintf(prefixTxt, 32, "%s_%d", BatchFormatOptions.Prefix.c_str(), i);
		if (BatchFormatOptions.AddLabel)
		{
			options.AddLabelAtStart = true;
			options.LabelName = prefixTxt;
		}
		if (BatchFormatOptions.AddComment)
		{
			options.AddCommentAtStart = true;
			options.CommentText = prefixTxt;
		}

		FFormatDataCommand& cmd = SubCommands.emplace_back(options);
		cmd.Do(state);
		state.AdvanceAddressRef(options.StartAddress, options.ItemSize * options.NoItems);
		state.SetCodeAnalysisDirty(options.StartAddress);
	}
}

void FBatchFormatDataCommand::Undo(FCodeAnalysisState& state)
{
	for (FFormatDataCommand& cmd : SubCommands )
	{
		cmd.Undo(state);
	}
}

void FBatchFormatDataCommand::FixupAddressRefs(FCodeAnalysisState& state)
{
	FixupAddressRef(state, BatchFormatOptions.FormatOptions.GraphicsSetRef);
	FixupAddressRef(state, BatchFormatOptions.FormatOptions.CharacterSet);
	FixupAddressRef(state, BatchFormatOptions.FormatOptions.StartAddress);

	for (FFormatDataCommand& cmd : SubCommands)
	{
		cmd.FixupAddressRefs(state);
	}
}