#pragma once

#include "CommandProcessor.h"
#include "../CodeAnalyserTypes.h"

// execution parameters
struct FDataFormattingOptions
{
	EDataType				DataType = EDataType::Byte;
	EDataItemDisplayType	DisplayType = EDataItemDisplayType::Unknown;

	//int			StartAddress = 0;	// TODO: use Address Ref
	FAddressRef				StartAddress;
	int						ItemSize = 1;
	int						NoItems = 1;
	FAddressRef				CharacterSet;
	FAddressRef				GraphicsSetRef;
	int						PaletteNo = -1;
	int						StructId = -1;
	uint8_t					EmptyCharNo = 0;
	bool					RegisterItem = false;
	bool					ClearCodeInfo = false;
	bool					ClearLabels = false;
	bool					AddLabelAtStart = false;
	std::string				LabelName;
	bool					AddCommentAtStart = false;
	std::string				CommentText;

	bool					IsValid() const { return NoItems > 0 && ItemSize > 0; }
	uint16_t				CalcEndAddress() const { return StartAddress.Address + (NoItems * ItemSize) - 1; }
	void					SetupForBitmap(FAddressRef address, int xSizePixels, int ySizePixels, int bpp)
	{
		DataType = EDataType::Bitmap;
		DisplayType = EDataItemDisplayType::Bitmap;
		StartAddress = address;
		ItemSize = (xSizePixels * bpp) / 8;
		NoItems = ySizePixels;
	}

	void		SetupForCharmap(FAddressRef address, int xSize, int ySize)
	{
		DataType = EDataType::CharacterMap;
		StartAddress = address;
		ItemSize = xSize;
		NoItems = ySize;
	}
};

// for undo operation
struct FFormatUndoData
{
	FAddressRef	CharacterMapLocation;
	std::vector<std::pair<FAddressRef, FLabelInfo*>>	Labels;
	std::vector<std::pair<FAddressRef, FCommentBlock*>>	CommentBlocks;
	std::vector<std::pair<FAddressRef, FDataInfo>>	DataItems;
	std::vector<std::pair<FAddressRef, FCodeInfo*>>	CodeItems;	// pointer might be a bad idea
};

// command
class FFormatDataCommand : public FCommand
{
public:
	FFormatDataCommand(const FDataFormattingOptions& options);
	void Do(FCodeAnalysisState& state) override;
	void Undo(FCodeAnalysisState& state) override;
	void FixupAddressRefs(FCodeAnalysisState& state) override;

private: 
	FDataFormattingOptions	FormatOptions;
	FFormatUndoData			UndoData;
};

// Batch formatting command
// This formats multiple items
struct FBatchDataFormattingOptions
{
	int				NoItems = 0;
	FDataFormattingOptions	FormatOptions;

	bool			AddLabel = false;
	bool			AddComment = false;
	std::string		Prefix;
};


class FBatchFormatDataCommand : public FCommand
{
public:
	FBatchFormatDataCommand(const FBatchDataFormattingOptions& options);
	void Do(FCodeAnalysisState& state) override;
	void Undo(FCodeAnalysisState& state) override;
	void FixupAddressRefs(FCodeAnalysisState& state) override;

private:
	FBatchDataFormattingOptions	BatchFormatOptions;

	std::vector<FFormatDataCommand>	SubCommands;
};