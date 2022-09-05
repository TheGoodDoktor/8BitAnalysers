
#include "SkoolkitExporter.h"
#include "CodeAnalyser/CodeAnalyser.h"

// See here for a description of Skoolkit control files
// https://skoolkit.ca/docs/skoolkit/control-files.html
bool ExportSkoolKitControlFile(FCodeAnalysisState& state, const char* pTextFileName, const char* pGameName, uint16_t startAddr, uint16_t endAddr)
{
	if (state.ItemList.empty())
		return false;

	FILE* fp = nullptr;
	fopen_s(&fp, pTextFileName, "wt");

	if (fp == nullptr)
		return false;

	fprintf(fp, "; SkoolKit control file for %s\n", pGameName);
	fprintf(fp, "; This file was auto-generated\n");
	fprintf(fp, "; Based on SkoolKit version 8.6\n");
	fprintf(fp, "; To create a skool file use the following Skoolkit command:\n");
	fprintf(fp, ";    sna2skool.py -c %s.ctl -H %s.z80 > %s.skool\n\n", pGameName, pGameName, pGameName);

	// In order to make it easier to iterate through the data we create a map that holds all the items for each memory address.
	// It is possible for either pLabel or pItem to be null.
	struct SAddressItems
	{
		const FLabelInfo* pLabel = nullptr;
		const FItem* pItem = nullptr;
	};

	// Map memory address to SAddressItems
	std::map<int, SAddressItems> itemsForAddress;

	for (FItem* pItem : state.ItemList)
	{
		if (pItem->Address < startAddr || pItem->Address > endAddr)
			continue;

		if (pItem->Type == ItemType::Label)
		{
			const FLabelInfo* pLabelInfo = static_cast<const FLabelInfo*>(pItem);
			itemsForAddress[pItem->Address].pLabel = pLabelInfo;
		}
		else
		{
			itemsForAddress[pItem->Address].pItem = pItem;
		}
	}

	char curDirective = '-';

	for (auto const& [address, items] : itemsForAddress)
	{
		char itemDirective = '-';
		std::string comment;

		// if we have an item determine what the control directive should be
		if (items.pItem)
		{
			if (items.pItem->Type == ItemType::Code)
			{
				itemDirective = 'c';
			}
			else if (items.pItem->Type == ItemType::Data)
			{
				const FDataInfo* pDataInfo = static_cast<const FDataInfo*>(items.pItem);

				switch (pDataInfo->DataType)
				{
				case DataType::Byte:
					itemDirective = 'b';
					break;
				case DataType::Word:
					itemDirective = 'w';
					break;
				case DataType::Text:
					itemDirective = 't';
					break;
				case DataType::Graphics:
				case DataType::Blob:
				default:
					itemDirective = 'b';
				}
			}

			if (!items.pItem->Comment.empty())
				comment = items.pItem->Comment;
		}

		if (items.pLabel)
		{
			// In the case where an item doesn't exist we won't have a directive.
			// If that happens we derive the directive from the label type.
			if (itemDirective == '-')
			{
				switch (items.pLabel->LabelType)
				{
				case LabelType::Function:
				case LabelType::Code:
					itemDirective = 'c';
					break;
				case LabelType::Data:
					itemDirective = 'b';
					break;
				}
			}

			if (!items.pLabel->Comment.empty())
			{
				// It is possible to have a comment to a label in addition to a comment to an item.
				// In this case we join the two comments together.
				if (!comment.empty())
					comment += ". ";
				comment += items.pLabel->Comment;
			}
		}

		if (itemDirective == '-')
		{
			itemDirective = '-';
		}
		if (itemDirective != curDirective || items.pLabel)
		{
			fprintf(fp, "%c $%x %s\n", itemDirective, address, items.pLabel ? items.pLabel->Name.c_str() : "");
		}

		// write the label after the main directive
		if (items.pLabel)
		{
			fprintf(fp, "@ $%x label=%s\n", address, items.pLabel->Name.c_str());
		}

		if (!comment.empty())
		{
			fprintf(fp, "%c $%x %s\n", toupper(itemDirective), address, comment.c_str());
		}

		curDirective = itemDirective;
	}

	fclose(fp);
	return true;
}