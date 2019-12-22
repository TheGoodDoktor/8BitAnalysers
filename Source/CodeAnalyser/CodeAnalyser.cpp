#include "CodeAnalyser.h"
#include <cstdint>
#include "UI/SpeccyUI.h"

#include "ROMLabels.h"
#include <algorithm>

bool CheckPointerIndirectionInstruction(FSpeccy* pSpeccy, uint16_t pc, uint16_t* out_addr)
{
	const uint8_t instrByte = ReadSpeccyByte(pSpeccy, pc);

	switch (instrByte)
	{
		// LD (nnnn),x
		case 0x22:
		case 0x32:
			// LD x,(nnnn)
		case 0x2A:
		case 0x3A:
			*out_addr = (ReadSpeccyByte(pSpeccy, pc + 2) << 8) | ReadSpeccyByte(pSpeccy, pc + 1);
			return true;
			// extended instructions
		case 0xED:
		{
			const uint8_t exInstrByte = ReadSpeccyByte(pSpeccy, pc + 1);
			switch (exInstrByte)
			{
			case 0x43://ld (**),bc
			case 0x4B://ld bc,(**)
			case 0x53://ld (**),de
			case 0x5B://ld de,(**)
			case 0x63://ld (**),hl
			case 0x6B://ld hl,(**)
			case 0x73://ld (**),sp
			case 0x7B://ld sp,(**)
				*out_addr = (ReadSpeccyByte(pSpeccy, pc + 3) << 8) | ReadSpeccyByte(pSpeccy, pc + 2);
				return true;
			}

		}

		// IX/IY instructions
		case 0xDD:
		case 0xFD:
		{
			const uint8_t exInstrByte = ReadSpeccyByte(pSpeccy, pc + 1);
			switch (exInstrByte)
			{
			case 0x22://ld (**),ix/iy
			case 0x2A://ld ix/iy,(**)
				*out_addr = (ReadSpeccyByte(pSpeccy, pc + 3) << 8) | ReadSpeccyByte(pSpeccy, pc + 2);
				return true;
			}
		}
	}

	return false;
}

bool CheckPointerRefInstruction(FSpeccy* pSpeccy, uint16_t pc, uint16_t* out_addr)
{
	if (CheckPointerIndirectionInstruction(pSpeccy, pc, out_addr))
		return true;
	
	const uint8_t instrByte = ReadSpeccyByte(pSpeccy, pc);

	switch (instrByte)
	{
		// LD x,nnnn
	case 0x01:
	case 0x11:
	case 0x21:
		*out_addr = (ReadSpeccyByte(pSpeccy, pc + 2) << 8) | ReadSpeccyByte(pSpeccy, pc + 1);
		return true;
	}
	return false;
}

bool CheckJumpInstruction(FSpeccy* pSpeccy, uint16_t pc, uint16_t* out_addr)
{
	const uint8_t instrByte = ReadSpeccyByte(pSpeccy, pc);

	switch (instrByte)
	{
		/* CALL nnnn */
	case 0xCD:
		/* CALL cc,nnnn */
	case 0xDC: case 0xFC: case 0xD4: case 0xC4:
	case 0xF4: case 0xEC: case 0xE4: case 0xCC:
		/* JP nnnn */
	case 0xC3:
		/* JP cc,nnnn */
	case 0xDA: case 0xFA: case 0xD2: case 0xC2:
	case 0xF2: case 0xEA: case 0xE2: case 0xCA:
		*out_addr = (ReadSpeccyByte(pSpeccy, pc + 2) << 8) | ReadSpeccyByte(pSpeccy, pc + 1);
		return true;

		/* DJNZ d */
	case 0x10:
		/* JR d */
	case 0x18:
		/* JR cc,d */
	case 0x38: case 0x30: case 0x20: case 0x28:
	{
		const int8_t relJump = (int8_t)ReadSpeccyByte(pSpeccy, pc + 1);
		*out_addr = pc + 2 + relJump;	// +2 because it's relative to the next instruction
	}
		return true;
		/* RST */
	case 0xC7:  *out_addr = 0x00; return true;
	case 0xCF:  *out_addr = 0x08; return true;
	case 0xD7:  *out_addr = 0x10; return true;
	case 0xDF:  *out_addr = 0x18; return true;
	case 0xE7:  *out_addr = 0x20; return true;
	case 0xEF:  *out_addr = 0x28; return true;
	case 0xF7:  *out_addr = 0x30; return true;
	case 0xFF:  *out_addr = 0x38; return true;
	}

	return false;
}

bool CheckCallInstruction(FSpeccy* pSpeccy, uint16_t pc)
{
	const uint8_t instrByte = ReadSpeccyByte(pSpeccy, pc);

	switch (instrByte)
	{
		/* CALL nnnn */
	case 0xCD:
		/* CALL cc,nnnn */
	case 0xDC: case 0xFC: case 0xD4: case 0xC4:
	case 0xF4: case 0xEC: case 0xE4: case 0xCC:
		/* RST */
	case 0xC7:
	case 0xCF:
	case 0xD7:
	case 0xDF:
	case 0xE7:
	case 0xEF:
	case 0xF7:
	case 0xFF:
		return true;
	}
	return false;
}

// check if function should stop static analysis
// this would be a function that unconditionally affects the PC
bool CheckStopInstruction(FSpeccy* pSpeccy, uint16_t pc)
{
	const uint8_t instrByte = ReadSpeccyByte(pSpeccy, pc);

	switch(instrByte)
	{
		/* CALL nnnn */
	case 0xCD:
		/* CALL cc,nnnn */
	case 0xDC: case 0xFC: case 0xD4: case 0xC4:
	case 0xF4: case 0xEC: case 0xE4: case 0xCC:
		/* RST */
	case 0xC7:
	case 0xCF:
	case 0xD7:
	case 0xDF:
	case 0xE7:
	case 0xEF:
	case 0xF7:
	case 0xFF:
		// ret
	case 0xC8:
	case 0xC9:
	case 0xD8:
	case 0xE8:
	case 0xF8:
	
		
	case 0xc3:// jp
	case 0xe9:// jp(hl)
	case 0x18://jr
		return true;
	case 0xED:	// extended instructions
	{
		const uint8_t extInstrByte = ReadSpeccyByte(pSpeccy, pc+1);
		switch(extInstrByte)
		{
		case 0x4D:	// more RET functions
		case 0x5D:
		case 0x6D:
		case 0x7D:
		case 0x45:	
		case 0x55:
		case 0x65:
		case 0x75:
			return true;
		}
	}
	// index register instructions
	case 0xDD:	// IX
	case 0xFD:	// IY
	{
		const uint8_t extInstrByte = ReadSpeccyByte(pSpeccy, pc + 1);
		switch (extInstrByte)
		{
		case 0xE9:	// JP(IX)
			return true;
		}
	}
	default:
		return false;
	}
}

bool GenerateLabelForAddress(FCodeAnalysisState &state, uint16_t pc, LabelType labelType)
{
	FLabelInfo* pLabel = state.Labels[pc];
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
			sprintf(label, "function_%04X", pc);
			break;
		case LabelType::Code:
			sprintf(label, "label_%04X", pc);
			break;
		case LabelType::Data:
			sprintf(label, "data_%04X", pc);
			pLabel->Global = true;
			break;
		}

		pLabel->Name = label;
		state.Labels[pc] = pLabel;
		return true;
	}

	return false;
}


struct FAnalysisDasmState
{
	FSpeccy*		pSpeccy;
	uint16_t		CurrentAddress;
	std::string		Text;
};


/* disassembler callback to fetch the next instruction byte */
static uint8_t AnalysisDasmInputCB(void* pUserData)
{
	FAnalysisDasmState* pDasmState = (FAnalysisDasmState*)pUserData;

	const uint8_t val = ReadSpeccyByte(pDasmState->pSpeccy, pDasmState->CurrentAddress++);

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

uint16_t WriteCodeInfoForAddress(FCodeAnalysisState &state, uint16_t pc)
{
	FSpeccy *pSpeccy = state.pSpeccy;

	FAnalysisDasmState dasmState;
	dasmState.pSpeccy = state.pSpeccy;
	dasmState.CurrentAddress = pc;
	const uint16_t newPC = z80dasm_op(pc, AnalysisDasmInputCB, AnalysisOutputCB, &dasmState);

	FCodeInfo *pCodeInfo = state.CodeInfo[pc];
	if (pCodeInfo == nullptr)
	{
		pCodeInfo = new FCodeInfo;
		state.CodeInfo[pc] = pCodeInfo;
	}

	pCodeInfo->Text = dasmState.Text;
	pCodeInfo->Address = pc;
	pCodeInfo->ByteSize = newPC - pc;
	state.CodeInfo[pc] = pCodeInfo;	// just set to first instruction?

	// does this function branch?
	uint16_t jumpAddr;
	if (CheckJumpInstruction(pSpeccy, pc, &jumpAddr))
	{
		const bool isCall = CheckCallInstruction(pSpeccy, pc);
		if (GenerateLabelForAddress(state, jumpAddr, isCall ? LabelType::Function : LabelType::Code))
			state.Labels[jumpAddr]->References[pc]++;

		pCodeInfo->JumpAddress = jumpAddr;
	}

	uint16_t ptr;
	if (CheckPointerRefInstruction(pSpeccy, pc, &ptr))
		pCodeInfo->PointerAddress = ptr;

	if (CheckPointerIndirectionInstruction(pSpeccy, pc, &ptr))
	{
		if (GenerateLabelForAddress(state, ptr, LabelType::Data))
			state.Labels[ptr]->References[pc]++;
	}

	return newPC;
}

void AnalyseFromPC(FCodeAnalysisState &state, uint16_t pc)
{
	FSpeccy *pSpeccy = state.pSpeccy;

	// update branch reference counters
	uint16_t jumpAddr;
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
	}

	if (state.CodeInfo[pc] != nullptr)	// already been analysed
		return;

	/*if(pc >= 0x4000 && pc <= 0x5AFF)	// screen memory
	{
		assert(0);
	}*/

	state.bCodeAnalysisDataDirty = true;

	/*if(pc == 0x7e38)
	{
		assert(0);

	}*/
	bool bStop = false;

	while (bStop == false)
	{
		const uint16_t newPC = WriteCodeInfoForAddress(state, pc);

		if (CheckJumpInstruction(pSpeccy, pc, &jumpAddr))
		{
			AnalyseFromPC(state, jumpAddr);
			bStop = false;	// should just be call & rst really
		}

		// do we need to stop tracing ??
		if (CheckStopInstruction(pSpeccy, pc) || newPC < pc)
			bStop = true;
		else
			pc = newPC;
	}
	
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
	if(bWrite)
		state.DataInfo[dataAddr]->Writes[pc]++;
	else
		state.DataInfo[dataAddr]->Reads[pc]++;
}

void ReAnalyseCode(FCodeAnalysisState &state)
{
	for (int i = 0; i < (1 << 16); i++)
	{
		if (state.CodeInfo[i] != nullptr)
		{
			WriteCodeInfoForAddress(state, (uint16_t)i);
		}

		if (state.CodeInfo[i] == nullptr && state.DataInfo[i] == nullptr)
		{
			// set up data entry for address
			FDataInfo *pDataInfo = new FDataInfo;
			pDataInfo->Address = (uint16_t)i;
			pDataInfo->ByteSize = 1;
			pDataInfo->DataType = DataType::Byte;
			state.DataInfo[i] = pDataInfo;
		}
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
	state.Labels[address] = pLabel;
	return pLabel;
}

void GenerateGlobalInfo(FCodeAnalysisState &state)
{
	state.GlobalDataItems.clear();
	state.GlobalFunctions.clear();

	for (int i = 0; i < (1 << 16); i++)
	{
		FLabelInfo *pLabel = state.Labels[i];
		
		if (pLabel != nullptr)
		{
			if (pLabel->LabelType == LabelType::Data && pLabel->Global)
				state.GlobalDataItems.push_back(pLabel);
			if (pLabel->LabelType == LabelType::Function)
				state.GlobalFunctions.push_back(pLabel);
		}
		
	}
}


void InsertROMLabels(FCodeAnalysisState &state)
{
	for (const auto &label : g_RomLabels)
	{
		AddLabel(state, label.Address, label.pLabelName, label.LabelType);

		// run static analysis on all code labels
		if(label.LabelType == LabelType::Code || label.LabelType == LabelType::Function)
			RunStaticCodeAnalysis(state, label.Address);
	}

	for (const auto &label : g_SysVariables)
	{
		AddLabel(state, label.Address, label.pLabelName, LabelType::Data);
		// TODO: Set up data?
	}
}

void InsertSystemLabels(FCodeAnalysisState &state)
{
	// screen memory start
	AddLabel(state, 0x4000, "ScreenPixels", LabelType::Data);
	
	FDataInfo *pScreenPixData = new FDataInfo;
	pScreenPixData->DataType = DataType::Graphics;
	pScreenPixData->Address = 0x4000;
	pScreenPixData->ByteSize = 0x1800;
	state.DataInfo[pScreenPixData->Address] = pScreenPixData;

	AddLabel(state, 0x5800, "ScreenAttributes", LabelType::Data);
	FDataInfo *pScreenAttrData = new FDataInfo;
	pScreenAttrData->DataType = DataType::Blob;
	pScreenAttrData->Address = 0x5800;
	pScreenAttrData->ByteSize = 0x400;
	state.DataInfo[pScreenAttrData->Address] = pScreenAttrData;

	// system variables?
}

void InitialiseCodeAnalysis(FCodeAnalysisState &state,FSpeccy* pSpeccy)
{
	for (int i = 0; i < (1 << 16); i++)	// loop across address range
	{
		delete state.Labels[i];
		state.Labels[i] = nullptr;

		delete state.CodeInfo[i];
		state.CodeInfo[i] = nullptr;

		delete state.DataInfo[i];

		// set up data entry for address
		FDataInfo *pDataInfo = new FDataInfo;
		pDataInfo->Address = (uint16_t)i;
		pDataInfo->ByteSize = 1;
		pDataInfo->DataType = DataType::Byte;
		state.DataInfo[i] = pDataInfo;
	}

	state.CursorItemIndex = -1;
	state.pCursorItem = nullptr;
	
	state.pSpeccy = pSpeccy;
	uint16_t initialPC = z80_pc(&state.pSpeccy->CurrentState.cpu);
	InsertROMLabels(state);
	InsertSystemLabels(state);
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
			while (state.DataInfo[charAddr] != nullptr && state.DataInfo[charAddr]->DataType == DataType::Byte)
			{
				const uint8_t val = ReadSpeccyByte(state.pSpeccy, charAddr);
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
	if (state.Labels[address] == nullptr)
	{
		LabelType labelType = LabelType::Data;
		if (state.CodeInfo[address] != nullptr)
			labelType = LabelType::Code;

		GenerateLabelForAddress(state, address, labelType);
		state.bCodeAnalysisDataDirty = true;
	}
}

void RemoveLabelAtAddress(FCodeAnalysisState &state, uint16_t address)
{
	if (state.Labels[address] != nullptr)
	{
		delete state.Labels[address];
		state.Labels[address] = nullptr;
		state.bCodeAnalysisDataDirty = true;
	}
}

void SetLabelName(FCodeAnalysisState &state, FLabelInfo *pLabel, const char *pText)
{
	pLabel->Name = pText;
}

void SetItemCommentText(FCodeAnalysisState &state, FItem *pItem, const char *pText)
{
	pItem->Comment = pText;
}