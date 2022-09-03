#include "CodeAnalyser.h"

#include <cstdint>
#include <algorithm>
#include <cassert>
#include <util/z80dasm.h>
#include <util/m6502dasm.h>

#include "Z80/CodeAnalyserZ80.h"
#include "6502/CodeAnalyser6502.h"

bool FCodeAnalysisState::EnsureUniqueLabelName(std::string& labelName)
{
	auto labelIt = LabelUsage.find(labelName);
	if (labelIt == LabelUsage.end())
	{
		LabelUsage[labelName] = 0;
		return false;
	}

	char postFix[32];
	sprintf_s(postFix, "_%d", ++LabelUsage[labelName]);
	labelName += std::string(postFix);

	return true;
}

bool FCodeAnalysisState::RemoveLabelName(const std::string& labelName)
{
	auto labelIt = LabelUsage.find(labelName);
	assert(labelIt != LabelUsage.end());	// shouldn't happen

	if (labelIt->second == 0)	// only a single use so we can remove from the map
	{
		LabelUsage.erase(labelIt);
		return true;
	}

	return false;
}

// Search memory space for a block of data
bool FCodeAnalysisState::FindMemoryPattern(uint8_t* pData, size_t dataSize, uint16_t& outAddr)
{
	uint16_t address = 0;
	ICPUInterface* pCPUInterface = CPUInterface;
	size_t dataOffset = 0;

	do
	{
		const uint8_t byte = pCPUInterface->ReadByte(address);
		if (byte == pData[dataOffset])
		{
			if (dataOffset == dataSize - 1)	// found the whole run?
			{
				outAddr = static_cast<uint16_t>(address - dataOffset);
				return true;
			}
			else
			{
				dataOffset++;	// look for next byte
			}
		}
		else
		{
			dataOffset = 0;	// reset offset and look for start of data
		}

		address++;
	} while (address != 0);	// 16 bit address overflow

	return false;
}



bool CheckPointerIndirectionInstruction(ICPUInterface* pCPUInterface, uint16_t pc, uint16_t* out_addr)
{
	if (pCPUInterface->CPUType == ECPUType::Z80)
		return CheckPointerIndirectionInstructionZ80(pCPUInterface, pc, out_addr);
	else if (pCPUInterface->CPUType == ECPUType::M6502)
		return CheckPointerIndirectionInstruction6502(pCPUInterface, pc, out_addr);
	else
		return false;
}

bool CheckPointerRefInstruction(ICPUInterface* pCPUInterface, uint16_t pc, uint16_t* out_addr)
{
	if (pCPUInterface->CPUType == ECPUType::Z80)
		return CheckPointerRefInstructionZ80(pCPUInterface, pc, out_addr);
	else if (pCPUInterface->CPUType == ECPUType::M6502)
		return CheckPointerRefInstruction6502(pCPUInterface, pc, out_addr);
	else
		return false;
}


bool CheckJumpInstruction(ICPUInterface* pCPUInterface, uint16_t pc, uint16_t* out_addr)
{
	if (pCPUInterface->CPUType == ECPUType::Z80)
		return CheckJumpInstructionZ80(pCPUInterface, pc, out_addr);
	else if (pCPUInterface->CPUType == ECPUType::M6502)
		return CheckJumpInstruction6502(pCPUInterface, pc, out_addr);
	else
		return false;
}



bool CheckCallInstruction(ICPUInterface* pCPUInterface, uint16_t pc)
{
	if (pCPUInterface->CPUType == ECPUType::Z80)
		return CheckCallInstructionZ80(pCPUInterface, pc);
	else if(pCPUInterface->CPUType == ECPUType::M6502)
		return CheckCallInstruction6502(pCPUInterface, pc);
	else
		return false;
}

// check if function should stop static analysis
// this would be a function that unconditionally affects the PC
bool CheckStopInstruction(ICPUInterface* pCPUInterface, uint16_t pc)
{
	if (pCPUInterface->CPUType == ECPUType::Z80)
		return CheckStopInstructionZ80(pCPUInterface, pc);
	else if(pCPUInterface->CPUType == ECPUType::M6502)
		return CheckStopInstruction6502(pCPUInterface, pc);
	else
		return false;
}

bool GenerateLabelForAddress(FCodeAnalysisState &state, uint16_t pc, LabelType labelType)
{
	FLabelInfo* pLabel = state.GetLabelForAddress(pc);
	if (pLabel == nullptr)
	{
		pLabel = new FLabelInfo;
		pLabel->LabelType = labelType;
		pLabel->Address = pc;
		pLabel->ByteSize = 0;
		
		char label[32];
		switch (labelType)
		{
		case LabelType::Function:
			sprintf_s(label, "function_%04X", pc);
			break;
		case LabelType::Code:
			sprintf_s(label, "label_%04X", pc);
			break;
		case LabelType::Data:
			sprintf_s(label, "data_%04X", pc);
			pLabel->Global = true;
			break;
		}

		pLabel->Name = label;
		state.SetLabelForAddress(pc, pLabel);
		return true;
	}

	return false;
}


struct FAnalysisDasmState
{
	ICPUInterface*	CPUInterface;
	uint16_t		CurrentAddress;
	std::string		Text;
};


/* disassembler callback to fetch the next instruction byte */
static uint8_t AnalysisDasmInputCB(void* pUserData)
{
	FAnalysisDasmState* pDasmState = (FAnalysisDasmState*)pUserData;

	const uint8_t val = pDasmState->CPUInterface->ReadByte( pDasmState->CurrentAddress++);

	// push into binary buffer
	//if (pDasm->bin_pos < DASM_MAX_BINLEN)
	//	pDasm->bin_buf[pDasm->bin_pos++] = val;

	return val;
}

/* disassembler callback to output a character */
static void AnalysisOutputCB(char c, void* pUserData)
{
	FAnalysisDasmState* pDasmState = (FAnalysisDasmState*)pUserData;

	// add character to string
	pDasmState->Text += c;
}

void UpdateCodeInfoForAddress(FCodeAnalysisState &state, uint16_t pc)
{
	FAnalysisDasmState dasmState;
	dasmState.CPUInterface = state.CPUInterface;
	dasmState.CurrentAddress = pc;
	if(state.CPUInterface->CPUType == ECPUType::Z80)
		z80dasm_op(pc, AnalysisDasmInputCB, AnalysisOutputCB, &dasmState);
	else if(state.CPUInterface->CPUType == ECPUType::M6502)
		m6502dasm_op(pc, AnalysisDasmInputCB, AnalysisOutputCB, &dasmState);

	FCodeInfo *pCodeInfo = state.GetCodeInfoForAddress(pc);
	assert(pCodeInfo != nullptr);
	pCodeInfo->Text = dasmState.Text;
}


uint16_t WriteCodeInfoForAddress(FCodeAnalysisState &state, uint16_t pc)
{
	FAnalysisDasmState dasmState;
	dasmState.CPUInterface = state.CPUInterface;
	dasmState.CurrentAddress = pc;
	uint16_t newPC = pc;

	if (state.CPUInterface->CPUType == ECPUType::Z80)
		newPC = z80dasm_op(pc, AnalysisDasmInputCB, AnalysisOutputCB, &dasmState);
	else if(state.CPUInterface->CPUType == ECPUType::M6502)
		newPC = m6502dasm_op(pc, AnalysisDasmInputCB, AnalysisOutputCB, &dasmState);

	FCodeInfo* pCodeInfo = state.GetCodeInfoForAddress(pc);
	if (pCodeInfo == nullptr)
	{
		pCodeInfo = FCodeInfo::Allocate();
		state.SetCodeInfoForAddress(pc,pCodeInfo);
	}

	pCodeInfo->Text = dasmState.Text;
	pCodeInfo->Address = pc;
	pCodeInfo->ByteSize = newPC - pc;
	for(uint16_t codeAddr=pc;codeAddr<newPC;codeAddr++)
		state.SetCodeInfoForAddress(codeAddr, pCodeInfo);	// just set to first instruction?

	// does this function branch?
	uint16_t jumpAddr;
	if (CheckJumpInstruction(state.CPUInterface, pc, &jumpAddr))
	{
		const bool isCall = CheckCallInstruction(state.CPUInterface, pc);
		if (GenerateLabelForAddress(state, jumpAddr, isCall ? LabelType::Function : LabelType::Code))
			state.GetLabelForAddress(jumpAddr)->References[pc]++;

		pCodeInfo->JumpAddress = jumpAddr;
	}

	uint16_t ptr;
	if (CheckPointerRefInstruction(state.CPUInterface, pc, &ptr))
		pCodeInfo->PointerAddress = ptr;

	if (CheckPointerIndirectionInstruction(state.CPUInterface, pc, &ptr))
	{
		pCodeInfo->PointerAddress = ptr;
		if (GenerateLabelForAddress(state, ptr, LabelType::Data))
			state.GetLabelForAddress(ptr)->References[pc]++;
	}

	return newPC;
}

// return if we should continue
bool AnalyseAtPC(FCodeAnalysisState &state, uint16_t& pc)
{
	// update branch reference counters
	uint16_t jumpAddr;
	if (CheckJumpInstruction(state.CPUInterface, pc, &jumpAddr))
	{
		FLabelInfo* pLabel = state.GetLabelForAddress(jumpAddr);
		if (pLabel != nullptr)
			pLabel->References[pc]++;	// add/increment reference
	}

	uint16_t ptr;
	if (CheckPointerRefInstruction(state.CPUInterface, pc, &ptr))
	{
		FLabelInfo* pLabel = state.GetLabelForAddress(ptr);
		if (pLabel != nullptr)
			pLabel->References[pc]++;	// add/increment reference
	}

	if (state.GetCodeInfoForAddress(pc) != nullptr)	// already been analysed
		return false;

	uint16_t newPC = WriteCodeInfoForAddress(state, pc);
	if (CheckStopInstruction(state.CPUInterface, pc) || newPC < pc)
		return false;
	
	pc = newPC;
	state.bCodeAnalysisDataDirty = true;
	return true;
}

// TODO: make this use above function
void AnalyseFromPC(FCodeAnalysisState &state, uint16_t pc)
{
	//FSpeccy *pSpeccy = state.pSpeccy;

	// update branch reference counters
	/*uint16_t jumpAddr;
	if (CheckJumpInstruction(pSpeccy, pc, &jumpAddr))
	{
		FLabelInfo* pLabel = state.Labels[jumpAddr];
		if (pLabel != nullptr)
			pLabel->References[pc]++;	// add/increment reference
	}

	uint16_t ptr;
	if (CheckPointerRefInstruction(pSpeccy, pc, &ptr))
	{
		FLabelInfo* pLabel = state.Labels[ptr];
		if (pLabel != nullptr)
			pLabel->References[pc]++;	// add/increment reference
	}*/

	if (state.GetCodeInfoForAddress(pc) != nullptr)	// already been analysed
		return;

	state.bCodeAnalysisDataDirty = true;

	
	bool bStop = false;

	while (bStop == false)
	{
		uint16_t jumpAddr = 0;
		const uint16_t newPC = WriteCodeInfoForAddress(state, pc);

		if (CheckJumpInstruction(state.CPUInterface, pc, &jumpAddr))
		{
			AnalyseFromPC(state, jumpAddr);
			bStop = false;	// should just be call & rst really
		}

		// do we need to stop tracing ??
		if (CheckStopInstruction(state.CPUInterface, pc) || newPC < pc)
			bStop = true;
		else
			pc = newPC;
	}
}

void RegisterCodeExecuted(FCodeAnalysisState &state, uint16_t pc, uint16_t nextpc)
{
	AnalyseAtPC(state, pc);

	state.FrameTrace.push_back(pc);

	if (state.CPUInterface->CPUType == ECPUType::Z80)
		RegisterCodeExecutedZ80(state, pc, nextpc);
	else if (state.CPUInterface->CPUType == ECPUType::M6502)
		RegisterCodeExecuted6502(state, pc, nextpc);
}

void RunStaticCodeAnalysis(FCodeAnalysisState &state, uint16_t pc)
{
	//const uint8_t instrByte = ReadSpeccyByte(pUI->pSpeccy, pc);
	
	AnalyseFromPC(state, pc);
	
	//if (bRead)
	//	pUI->MemStats.ReadCount[addr]++;
	//if (bWrite)
	//	pUI->MemStats.WriteCount[addr]++;
	//
}

void RegisterDataAccess(FCodeAnalysisState &state, uint16_t pc,uint16_t dataAddr, bool bWrite)
{
	if (bWrite)
	{
		FDataInfo* pDataInfo = state.GetWriteDataInfoForAddress(dataAddr);
		pDataInfo->LastFrameWritten = state.CurrentFrameNo;
		pDataInfo->Writes[pc]++;
	}
	else
	{
		if (state.GetCodeInfoForAddress(dataAddr) == nullptr)	// don't register instruction data reads
		{
			FDataInfo* pDataInfo = state.GetReadDataInfoForAddress(dataAddr);
			pDataInfo->LastFrameRead = state.CurrentFrameNo;
			pDataInfo->Reads[pc]++;
		}
	}
}

void ReAnalyseCode(FCodeAnalysisState &state)
{
	int nextItemAddress = 0;
	for (int i = 0; i < (1 << 16); i++)
	{
		FCodeInfo* pCodeInfo = state.GetCodeInfoForAddress(i);
		if (i == nextItemAddress)
		{
			if (pCodeInfo != nullptr)
			{
				nextItemAddress = WriteCodeInfoForAddress(state, (uint16_t)i);
			}

			if (pCodeInfo == nullptr && state.GetReadDataInfoForAddress(i) == nullptr)
			{
				// set up data entry for address
				FDataInfo *pDataInfo = state.GetReadDataInfoForAddress(i);
				pDataInfo->Address = (uint16_t)i;
				pDataInfo->ByteSize = 1;
				pDataInfo->DataType = DataType::Byte;
			}
		}

		const FLabelInfo* pLabelInfo = state.GetLabelForAddress(i);
		if ((pCodeInfo != nullptr) && (pLabelInfo != nullptr) && (pLabelInfo->LabelType == LabelType::Data))
		{
			pCodeInfo->bSelfModifyingCode = true;
		}
	}
}

void ResetMemoryLogs(FCodeAnalysisState &state)
{
	for (int i = 0; i < (1 << 16); i++)
	{
		FDataInfo* pDataInfo = state.GetReadDataInfoForAddress(i);
		if (pDataInfo != nullptr)
		{
			pDataInfo->LastFrameRead = 0;
			pDataInfo->Reads.clear();
			pDataInfo->LastFrameWritten = 0;
			pDataInfo->Writes.clear();
		}

		state.SetLastWriterForAddress(i,  0);
	}
}



FLabelInfo* AddLabel(FCodeAnalysisState &state, uint16_t address,const char *name,LabelType type)
{
	FLabelInfo *pLabel = new FLabelInfo;
	pLabel->Name = name;
	pLabel->LabelType = type;
	pLabel->Address = address;
	pLabel->ByteSize = 1;
	pLabel->Global = type == LabelType::Function;
	state.SetLabelForAddress(address, pLabel);
	return pLabel;
}

void GenerateGlobalInfo(FCodeAnalysisState &state)
{
	state.GlobalDataItems.clear();
	state.GlobalFunctions.clear();

	for (int i = 0; i < (1 << 16); i++)
	{
		FLabelInfo *pLabel = state.GetLabelForAddress(i);
		
		if (pLabel != nullptr)
		{
			if (pLabel->LabelType == LabelType::Data && pLabel->Global)
				state.GlobalDataItems.push_back(pLabel);
			if (pLabel->LabelType == LabelType::Function)
				state.GlobalFunctions.push_back(pLabel);
		}
		
	}
}

void RegisterCodeAnalysisPage(FCodeAnalysisState& state, FCodeAnalysisPage& page, const char* pName)
{

}


void InitialiseCodeAnalysis(FCodeAnalysisState &state, ICPUInterface* pCPUInterface)
{
	state.ResetLabelNames();

	for (int i = 0; i < (1 << 16); i++)	// loop across address range
	{
		FLabelInfo* pLabel = state.GetLabelForAddress(i);
		delete pLabel;
		state.SetLabelForAddress(i, nullptr);

		state.SetCodeInfoForAddress(i, nullptr);

		// set up data entry for address
		FDataInfo* pDataInfo = state.GetReadDataInfoForAddress(i);
		pDataInfo->Address = (uint16_t)i;
		pDataInfo->ByteSize = 1;
		pDataInfo->DataType = DataType::Byte;
	}

	FCodeInfo::FreeAll();

	state.CursorItemIndex = -1;
	state.pCursorItem = nullptr;
	
	state.CPUInterface = pCPUInterface;
	uint16_t initialPC = pCPUInterface->GetPC();// z80_pc(&state.pSpeccy->CurrentState.cpu);
	pCPUInterface->InsertROMLabels(state);
	pCPUInterface->InsertSystemLabels(state);
	RunStaticCodeAnalysis(state, initialPC);

	// Key Config
	state.KeyConfig[(int)Key::SetItemData] = 'D';
	state.KeyConfig[(int)Key::SetItemText] = 'T';
	state.KeyConfig[(int)Key::SetItemCode] = 'C';
	state.KeyConfig[(int)Key::AddLabel] = 'L';
	state.KeyConfig[(int)Key::Rename] = 'R';
	state.KeyConfig[(int)Key::Comment] = 0xBF;
}

// Command Processing
void DoCommand(FCodeAnalysisState &state, FCommand *pCommand)
{
	state.CommandStack.push_back(pCommand);
	pCommand->Do(state);
}

void Undo(FCodeAnalysisState &state)
{
	if (state.CommandStack.empty() == false)
	{
		state.CommandStack.back()->Undo(state);
		state.CommandStack.pop_back();
	}
}

class FSetItemDataCommand : public FCommand
{
public:
	FSetItemDataCommand(FItem *_pItem) :pItem(_pItem) {}

	virtual void Do(FCodeAnalysisState &state) override
	{

		if (pItem->Type == ItemType::Data)
		{
			FDataInfo *pDataItem = static_cast<FDataInfo *>(pItem);

			oldDataType = pDataItem->DataType;
			oldDataSize = pDataItem->ByteSize;

			if (pDataItem->DataType == DataType::Byte)
			{
				pDataItem->DataType = DataType::Word;
				pDataItem->ByteSize = 2;
				state.bCodeAnalysisDataDirty = true;
			}
			else if (pDataItem->DataType == DataType::Word)
			{
				pDataItem->DataType = DataType::Byte;
				pDataItem->ByteSize = 1;
				state.bCodeAnalysisDataDirty = true;
			}
			else if ( pDataItem->DataType == DataType::Text )
			{
				pDataItem->DataType = DataType::Byte;
				pDataItem->ByteSize = 1;
				state.bCodeAnalysisDataDirty = true;
			}
		}
		else if (pItem->Type == ItemType::Code)
		{
			FCodeInfo *pCodeItem = static_cast<FCodeInfo *>(pItem);
			if (pCodeItem->bDisabled == false)
			{
				pCodeItem->bDisabled = true;
				state.bCodeAnalysisDataDirty = true;

				FLabelInfo* pLabelInfo = state.GetLabelForAddress(pItem->Address);
				if (pLabelInfo != nullptr)
					pLabelInfo->LabelType = LabelType::Data;
			}
		}
	}
	virtual void Undo(FCodeAnalysisState &state) override
	{
		FDataInfo *pDataItem = static_cast<FDataInfo *>(pItem);
		pDataItem->DataType = oldDataType;
		pDataItem->ByteSize = oldDataSize;
	}

	FItem *	pItem;

	DataType	oldDataType;
	uint16_t	oldDataSize;
};

void SetItemCode(FCodeAnalysisState &state, FItem *pItem)
{
	FCodeInfo* pCodeInfo = state.GetCodeInfoForAddress(pItem->Address);
	if(pCodeInfo !=nullptr && pCodeInfo->bDisabled == true)
	{
		pCodeInfo->bDisabled = false;
		WriteCodeInfoForAddress(state, pItem->Address);
	}
	else
	{
		RunStaticCodeAnalysis(state, pItem->Address);
		UpdateCodeInfoForAddress(state, pItem->Address);
	}
	state.bCodeAnalysisDataDirty = true;
}

void SetItemData(FCodeAnalysisState &state, FItem *pItem)
{
	DoCommand(state, new FSetItemDataCommand(pItem));
}

void SetItemText(FCodeAnalysisState &state, FItem *pItem)
{
	if (pItem->Type == ItemType::Data)
	{
		FDataInfo *pDataItem = static_cast<FDataInfo *>(pItem);
		if (pDataItem->DataType == DataType::Byte)
		{
			// set to ascii
			pDataItem->ByteSize = 0;	// reset byte counter

			uint16_t charAddr = pDataItem->Address;
			FDataInfo* pDataInfo = state.GetReadDataInfoForAddress(charAddr);
			while (pDataInfo != nullptr && pDataInfo->DataType == DataType::Byte)
			{
				const uint8_t val = state.CPUInterface->ReadByte(charAddr);
				if (val == 0 || val > 0x80)
					break;
				pDataItem->ByteSize++;
				charAddr++;
			}

			// did the operation fail? -revert to byte
			if (pDataItem->ByteSize == 0)
			{
				pDataItem->DataType = DataType::Byte;
				pDataItem->ByteSize = 1;
			}
			else
			{
				pDataItem->DataType = DataType::Text;
				state.bCodeAnalysisDataDirty = true;
			}
		}
	}
}
void AddLabelAtAddress(FCodeAnalysisState &state, uint16_t address)
{
	if (state.GetLabelForAddress(address) == nullptr)
	{
		LabelType labelType = LabelType::Data;
		const FCodeInfo* pCodeInfo = state.GetCodeInfoForAddress(address);
		if (pCodeInfo != nullptr && pCodeInfo->bDisabled == false)
			labelType = LabelType::Code;

		GenerateLabelForAddress(state, address, labelType);
		state.bCodeAnalysisDataDirty = true;
	}
}

void RemoveLabelAtAddress(FCodeAnalysisState &state, uint16_t address)
{
	FLabelInfo* pLabelInfo = state.GetLabelForAddress(address);

	if (pLabelInfo != nullptr)
	{
		delete pLabelInfo;
		state.SetLabelForAddress(address, nullptr);
		state.bCodeAnalysisDataDirty = true;
	}
}

void SetLabelName(FCodeAnalysisState &state, FLabelInfo *pLabel, const char *pText)
{
	state.RemoveLabelName(pLabel->Name);
	pLabel->Name = pText;
	state.EnsureUniqueLabelName(pLabel->Name);
}

void SetItemCommentText(FCodeAnalysisState &state, FItem *pItem, const char *pText)
{
	pItem->Comment = pText;
}

// text generation

std::string GenerateAddressLabelString(FCodeAnalysisState &state, uint16_t addr)
{
	int labelOffset = 0;
	const char *pLabelString = nullptr;
	std::string labelStr;
	
	for (int addrVal = addr; addrVal >= 0; addrVal--)
	{
		FLabelInfo* pLabelInfo = state.GetLabelForAddress(addrVal);
		if (pLabelInfo != nullptr)
		{
			labelStr = "[" + pLabelInfo->Name;
			break;
		}

		labelOffset++;
	}

	if (labelStr.empty() == false)
	{
		if (labelOffset > 0)	// add offset string
		{
			char offsetString[16];
			sprintf_s(offsetString, " + %d]", labelOffset);
			labelStr += offsetString;
		}
		else
		{
			labelStr += "]";
		}
	}

	return labelStr;
}

bool OutputCodeAnalysisToTextFile(FCodeAnalysisState &state, const char *pTextFileName,uint16_t startAddr,uint16_t endAddr)
{
	FILE* fp = nullptr;
	fopen_s(&fp, pTextFileName, "wt");
	
	if (fp == nullptr)
		return false;
	
	for(FItem* pItem : state.ItemList)
	{
		if (pItem->Address < startAddr || pItem->Address > endAddr)
			continue;
		
		switch (pItem->Type)
		{
		case ItemType::Label:
			{
				const FLabelInfo *pLabelInfo = static_cast<FLabelInfo *>(pItem);
				fprintf(fp, "%s:", pLabelInfo->Name.c_str());
			}
			break;
		case ItemType::Code:
			{
				const FCodeInfo *pCodeInfo = static_cast<FCodeInfo *>(pItem);
				fprintf(fp, "\t%s", pCodeInfo->Text.c_str());

				if (pCodeInfo->JumpAddress != 0)
				{
					const std::string labelStr = GenerateAddressLabelString(state, pCodeInfo->JumpAddress);
					if(labelStr.empty() == false)
						fprintf(fp,"\t;%s", labelStr.c_str());
					
				}
				else if (pCodeInfo->PointerAddress != 0)
				{
					const std::string labelStr = GenerateAddressLabelString(state, pCodeInfo->PointerAddress);
					if (labelStr.empty() == false)
						fprintf(fp, "\t;%s", labelStr.c_str());
				}
			}
			
			break;
		case ItemType::Data:
			{
				const FDataInfo *pDataInfo = static_cast<FDataInfo *>(pItem);

				fprintf(fp, "\t");
				switch (pDataInfo->DataType)
				{
				case DataType::Byte:
				{
					const uint8_t val = state.CPUInterface->ReadByte(pDataInfo->Address);
					fprintf(fp,"db %02Xh", val);
				}
				break;

				case DataType::Word:
				{
					const uint16_t val = state.CPUInterface->ReadByte(pDataInfo->Address) | (state.CPUInterface->ReadByte(pDataInfo->Address + 1) << 8);
					fprintf(fp, "dw %04Xh", val);
				}
				break;

				case DataType::Text:
				{
					std::string textString;
					for (int i = 0; i < pDataInfo->ByteSize; i++)
					{
						const char ch = state.CPUInterface->ReadByte(pDataInfo->Address + i);
						if (ch == '\n')
							textString += "<cr>";
						else
							textString += ch;
					}
					fprintf(fp, "ascii '%s'", textString.c_str());
				}
				break;

				case DataType::Graphics:
				case DataType::Blob:
				default:
					fprintf(fp, "%d Bytes", pDataInfo->ByteSize);
					break;
				}
			}
			break;
		}

		// put comment on the end
		if(pItem->Comment.empty() == false)
		fprintf(fp, "\t;%s", pItem->Comment.c_str());
		fprintf(fp, "\n");
	}

	fclose(fp);
	return true;
}

// See here for a description of Skoolkit control files
// https://skoolkit.ca/docs/skoolkit/control-files.html
bool OutputSkoolKitControlFile(FCodeAnalysisState& state, const char* pTextFileName, const char* pGameName, uint16_t startAddr, uint16_t endAddr)
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