#include "CodeAnaysisPage.h"
#include "CodeAnalyser.h"

#include "Util/MemoryBuffer.h"

//#include "json.hpp"

void FCodeAnalysisPage::Initialise(uint16_t address)
{
	memset(Labels, 0, sizeof(Labels));
	memset(CodeInfo, 0, sizeof(CodeInfo));
	memset(DataInfo, 0, sizeof(DataInfo));
	memset(LastWriter, 0, sizeof(LastWriter));

	for (int addr = 0; addr < FCodeAnalysisPage::kPageSize; addr++)
	{
		// set up data entry for address
		FDataInfo* pDataInfo = new FDataInfo;
		pDataInfo->Address = address + (uint16_t)addr;
		pDataInfo->ByteSize = 1;
		pDataInfo->DataType = DataType::Byte;
		DataInfo[addr] = pDataInfo;
	}
}

static const uint32_t kMagic = 0xc0de;
static const uint32_t kVersionNo = 1;

uint32_t kLabelMagic = 'LABL';
uint32_t kCodeMagic = 'CODE';
uint32_t kDataMagic = 'DATA';



void FCodeAnalysisPage::WriteToBuffer(FMemoryBuffer& buffer)
{
	buffer.Init();

	buffer.Write(kMagic);
	buffer.Write(kVersionNo);

	uint32_t noItems = 0;
	buffer.Write(kLabelMagic);
	for (int i = 0; i < kPageSize; i++)
	{
		if (Labels[i] != nullptr)
		{
			const FLabelInfo& label = *Labels[i];
			buffer.Write<uint16_t>(i);	// address in page
			buffer.Write(label.LabelType);
			buffer.Write(label.Address);
			buffer.Write(label.ByteSize);
			buffer.WriteString(label.Name);
			buffer.WriteString(label.Comment);
			buffer.Write(label.Global);
			// References?
			buffer.Write((uint16_t)label.References.size());
			for (const auto& ref : label.References)
			{
				buffer.Write(ref.first);
			}
		}
	}
	buffer.Write<uint16_t>(0xffff);	// terminator

	// Code Section
	buffer.Write(kCodeMagic);
	for (int i = 0; i < kPageSize; i++)
	{
		if (CodeInfo[i] != nullptr)
		{
			const FCodeInfo& codeInfo = *CodeInfo[i];
			buffer.Write<uint16_t>(i);	// address in page
			// TODO:
		}
	}
	buffer.Write<uint16_t>(0xffff);	// terminator

	// Data Section
	buffer.Write(kDataMagic);
	for (int i = 0; i < kPageSize; i++)
	{
		if (DataInfo[i] != nullptr)
		{
			const FDataInfo& dataInfo = *DataInfo[i];
			buffer.Write<uint16_t>(i);	// address in page
			// TODO:
		}
	}
	buffer.Write<uint16_t>(0xffff);	// terminator

}

bool FCodeAnalysisPage::ReadFromBuffer(FMemoryBuffer& buffer)
{
	if (buffer.Read<uint16_t>() != kMagic)
		return false;

	const uint16_t fileVersion = buffer.Read<uint16_t>();

	// Read Labels
	if (buffer.Read<uint16_t>() != kLabelMagic)
		return false;

	while (true)
	{
		const uint16_t pageAddr = buffer.Read<uint16_t>();
		if (pageAddr == 0xffff)
			break;
	}

	// Read Code
	if (buffer.Read<uint16_t>() != kCodeMagic)
		return false;

	while (true)
	{
		const uint16_t pageAddr = buffer.Read<uint16_t>();
		if (pageAddr == 0xffff)
			break;
	}

	// Read Data
	if (buffer.Read<uint16_t>() != kDataMagic)
		return false;

	while (true)
	{
		const uint16_t pageAddr = buffer.Read<uint16_t>();
		if (pageAddr == 0xffff)
			break;
	}

	return true;
}

#if 0
void FCodeAnalysisPage::WriteToJSon(nlohmann::json& jsonOutput)
{
	bool bInCode = CodeInfo[0] != nullptr;
	int sectionSize = 0;

	// determine sections
	for (int addr = 0; addr < FCodeAnalysisPage::kPageSize; addr++)
	{
		nlohmann::json addressInfo;

		const FLabelInfo* pLabelInfo = Labels[addr];
		if (pLabelInfo)
		{
			nlohmann::json labelInfo;
			// TODO: write Label Info
			addressInfo["Label"] = labelInfo;
		}

		jsonOutput.push_back(addressInfo);
	}
}
#endif