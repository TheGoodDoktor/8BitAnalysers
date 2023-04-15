#include "CodeAnalysisState.h"

#include <stdint.h>
#include "CodeAnaysisPage.h"
#include "CodeAnalyser.h"

const uint32_t kAnalysisStateMagic = 0xBeefCafe;
const uint32_t kAnalysisStatePageMagic = 0xDeadCafe;
const uint32_t kAnalysisStateVersion = 1;

const uint16_t kLabelId = 0x8000;
const uint16_t kDataId = 0x4000;
const uint16_t kTerminatorId = 0xffff;

void WritePageState(const FCodeAnalysisPage& page, FILE* fp)
{
	fwrite(&kAnalysisStatePageMagic, sizeof(kAnalysisStatePageMagic), 1, fp);
	fwrite(&page.PageId, sizeof(page.PageId), 1, fp);

	uint16_t pageAddr = 0;
	uint16_t tempU16;

	while (pageAddr < FCodeAnalysisPage::kPageSize)
	{
		const FLabelInfo* pLabelInfo = page.Labels[pageAddr];
		if (pLabelInfo && pLabelInfo->References.GetReferences().empty() == false)
		{
			const uint16_t itemId = pageAddr | kLabelId;
			fwrite(&itemId, sizeof(itemId), 1, fp);
			tempU16 = (uint16_t)pLabelInfo->References.GetReferences().size();
			fwrite(&tempU16, sizeof(tempU16), 1, fp);
			for (auto& reference : pLabelInfo->References.GetReferences())
				fwrite(&reference.Val, sizeof(reference.Val), 1, fp);
		}

		const FCodeInfo* pCodeInfoItem = page.CodeInfo[pageAddr];

		// we do want data info for SMC operands
		if (pCodeInfoItem == nullptr || pCodeInfoItem->bSelfModifyingCode == true)
		{
			const FDataInfo* pDataInfo = &page.DataInfo[pageAddr];

			// check if we need to write
			if (pDataInfo->Reads.GetReferences().empty() == false ||
				pDataInfo->Writes.GetReferences().empty() == false ||
				pDataInfo->LastWriter.IsValid())
			{
				const uint16_t itemId = pageAddr | kDataId;
				fwrite(&itemId, sizeof(itemId), 1, fp);

				// Reads
				tempU16 = (uint16_t)pDataInfo->Reads.GetReferences().size();
				fwrite(&tempU16, sizeof(tempU16), 1, fp);
				for (const auto& read : pDataInfo->Reads.GetReferences())
					fwrite(&read.Val, sizeof(read.Val), 1, fp);

				// Writes
				tempU16 = (uint16_t)pDataInfo->Writes.GetReferences().size();
				fwrite(&tempU16, sizeof(tempU16), 1, fp);
				for (const auto& write : pDataInfo->Writes.GetReferences())
					fwrite(&write.Val, sizeof(write.Val), 1, fp);

				// Last Writer
				fwrite(&pDataInfo->LastWriter.Val, sizeof(pDataInfo->LastWriter), 1, fp);
			}

			pageAddr += pDataInfo->ByteSize;
		}
		else if (pCodeInfoItem != nullptr)
		{
			pageAddr += pCodeInfoItem->ByteSize;
		}

	}

	fwrite(&kTerminatorId, sizeof(kTerminatorId), 1, fp);
}

void ReadPageState(FCodeAnalysisPage& page, FILE* fp)
{
	uint16_t itemId;
	fread(&itemId, sizeof(itemId), 1, fp);

	while (itemId != kTerminatorId)
	{
		const uint16_t pageAddr = itemId & FCodeAnalysisPage::kPageMask;

		if (itemId & kLabelId)
		{
			FLabelInfo* pLabelInfo = page.Labels[pageAddr];
			//assert(pLabelInfo != nullptr);

			uint16_t count;
			fread(&count, sizeof(count), 1, fp);

			if (pLabelInfo != nullptr)
				pLabelInfo->References.Reset();
			for (int i = 0; i < count; i++)
			{
				FAddressRef ref;
				fread(&ref.Val, sizeof(ref.Val), 1, fp);
				if (pLabelInfo != nullptr)
					pLabelInfo->References.RegisterAccess(ref);
			}
		}
		else if (itemId & kDataId)
		{
			FDataInfo& dataItem = page.DataInfo[pageAddr];
			uint16_t count;

			// Reads
			fread(&count, sizeof(count), 1, fp);
			dataItem.Reads.Reset();
			for (int i = 0; i < count; i++)
			{
				FAddressRef ref;
				fread(&ref.Val, sizeof(ref.Val), 1, fp);
				dataItem.Reads.RegisterAccess(ref);
			}

			// Writes
			fread(&count, sizeof(count), 1, fp);
			dataItem.Writes.Reset();
			for (int i = 0; i < count; i++)
			{
				FAddressRef ref;
				fread(&ref.Val, sizeof(ref.Val), 1, fp);
				dataItem.Writes.RegisterAccess(ref);
			}

			// Last Writer
			fread(&dataItem.LastWriter.Val, sizeof(dataItem.LastWriter.Val), 1, fp);
		}

		fread(&itemId, sizeof(itemId), 1, fp);
	}
}

bool ExportAnalysisState(FCodeAnalysisState& state, const char* pAnalysisBinFile)
{
	FILE* fp = fopen(pAnalysisBinFile, "wb");
	if (fp == nullptr)
		return false;

	fwrite(&kAnalysisStateMagic, sizeof(kAnalysisStateMagic), 1, fp);
	fwrite(&kAnalysisStateVersion, sizeof(kAnalysisStateVersion), 1, fp);

	const auto& banks = state.GetBanks();
	int pagesWritten = 0;

	// iterate through all registered banks
	for (int bankNo = 0; bankNo < banks.size(); bankNo++)
	{
		const FCodeAnalysisBank& bank = banks[bankNo];
		if (bank.bReadOnly)	// skip read only banks - ROM
			continue;

		for (int pageNo = 0; pageNo < bank.NoPages; pageNo++)
		{
			const FCodeAnalysisPage& page = bank.Pages[pageNo];
			if (page.bUsed)
			{
				WritePageState(page, fp);
				pagesWritten++;
			}
		}
	}

	fwrite(&kAnalysisStatePageMagic, sizeof(kAnalysisStatePageMagic), 1, fp);
	fwrite(&kTerminatorId, sizeof(kTerminatorId), 1, fp);

	fclose(fp);
	return true;
}

bool ImportAnalysisState(FCodeAnalysisState& state, const char* pAnalysisBinFile)
{
	FILE* fp = fopen(pAnalysisBinFile, "rb");
	if (fp == nullptr)
		return false;

	uint32_t magic;
	uint32_t version;

	fread(&magic, sizeof(magic), 1, fp);
	fread(&version, sizeof(version), 1, fp);

	if (magic != kAnalysisStateMagic)
	{
		fclose(fp);
		return false;
	}

	uint16_t pageId;
	fread(&magic, sizeof(magic), 1, fp);
	assert(magic == kAnalysisStatePageMagic);
	fread(&pageId, sizeof(pageId), 1, fp);

	while (pageId != kTerminatorId)
	{
		FCodeAnalysisPage* pPage = state.GetPage(pageId);
		assert(pPage != nullptr);
		ReadPageState(*pPage, fp);

		// get next pageId
		fread(&magic, sizeof(magic), 1, fp);
		assert(magic == kAnalysisStatePageMagic);

		fread(&pageId, sizeof(pageId), 1, fp);
	}

	fclose(fp);
	return true;
}
