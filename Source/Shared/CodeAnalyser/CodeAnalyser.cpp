#include "CodeAnalyser.h"

#include <cstdint>
#include <algorithm>
#include <cassert>
#include <stdio.h>
#include <string.h>

#include <imgui.h>

#include <util/z80dasm.h>
#include <util/m6502dasm.h>

#include "Util/Misc.h"
#include "Util/GraphicsView.h"
#include "UI/ImageViewer.h"

#include "Z80/CodeAnalyserZ80.h"
#include "6502/CodeAnalyser6502.h"
#include <Debug/DebugLog.h>
#include "Commands/CommandProcessor.h"
#include "Commands/SetItemDataCommand.h"

bool FCodeAnalysisState::EnsureUniqueLabelName(std::string& labelName)
{
	auto labelIt = LabelUsage.find(labelName);
	if (labelIt == LabelUsage.end())
	{
		LabelUsage[labelName] = 0;
		return false;
	}

	char postFix[32];
	snprintf(postFix,32, "_%d", ++LabelUsage[labelName]);
	labelName += std::string(postFix);

	return true;
}

bool FCodeAnalysisState::RemoveLabelName(const std::string& labelName)
{
	auto labelIt = LabelUsage.find(labelName);
	//assert(labelIt != LabelUsage.end());	// shouldn't happen - it does though - investigate
	if (labelIt == LabelUsage.end())
		return false;

	if (labelIt->second == 0)	// only a single use so we can remove from the map
	{
		LabelUsage.erase(labelIt);
		return true;
	}

	return false;
}

// Search memory space for a block of data
bool FCodeAnalysisState::FindMemoryPattern(uint8_t* pData, size_t dataSize, uint16_t offset, uint16_t& outAddr)
{
	uint16_t address = offset;
	ICPUInterface* pCPUInterface = CPUInterface;
	size_t dataOffset = 0;

	do
	{
		bool bFound = true;
		for (int byteNo = 0; byteNo < 8; byteNo++)
		{
			const uint8_t byte = pCPUInterface->ReadByte(address + byteNo);
			if (byte != pData[byteNo])
			{
				bFound = false;
				break;
			}
		}

		if (bFound)
		{
			outAddr = static_cast<uint16_t>(address);
			return true;
		}

		address++;
	} while (address != 0);	// 16 bit address overflow

	return false;
}

bool IsAscii(uint8_t byte)
{
	return byte >= 32 && byte <= 126;
}

void FCodeAnalysisState::FindAsciiStrings(uint16_t startAddress)
{
	uint16_t address = startAddress;
	ICPUInterface* pCPUInterface = CPUInterface;
	const int kStringMinLength = 4;
	int stringLength = 0;
	uint16_t stringStart = 0;
	std::string dbgString;

	do
	{
		FCodeInfo* pCodeInfo = GetCodeInfoForAddress(address);
		FDataInfo* pDataInfo = GetReadDataInfoForAddress(address);
		const uint8_t byte = pCPUInterface->ReadByte(address++);

		// determine if char is ascii
		const bool bIsAscii = IsAscii(byte);
		const bool bCode = pCodeInfo != nullptr && pCodeInfo->bDisabled;

		if (bIsAscii && bCode == false)
		{
			if (stringLength == 0)
				stringStart = address;

			dbgString += byte;
			stringLength++;
		}
		else
		{
			if (stringLength >= kStringMinLength) // end of valid string
			{
				LOGINFO("Found string at:0x%04X \"%s\"", stringStart, dbgString.c_str());
			}
			stringLength = 0;
			dbgString.clear();
		}

	} while (address != 0);	// 16 bit address overflow

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

std::string GetItemText(FCodeAnalysisState& state, uint16_t address)
{
	FDataInfo* pDataInfo = state.GetReadDataInfoForAddress(address);
	std::string textString;

	if (pDataInfo->DataType != EDataType::Text)
		return textString;	// error text?

	for (int i = 0; i < pDataInfo->ByteSize; i++)
	{
		const char ch = state.CPUInterface->ReadByte(pDataInfo->Address + i);
		if (ch == '\n')
			textString += "<cr>";
		if (pDataInfo->bBit7Terminator && ch & (1 << 7))	// check bit 7 terminator flag
			textString += ch & ~(1 << 7);	// remove bit 7
		else
			textString += ch;
	}

	return textString;
}

bool GenerateLabelForAddress(FCodeAnalysisState &state, uint16_t address, ELabelType labelType)
{
	FLabelInfo* pLabel = state.GetLabelForAddress(address);
	if (pLabel != nullptr)
		return false;
		
	pLabel = FLabelInfo::Allocate();
	pLabel->LabelType = labelType;
	pLabel->Address = address;
	pLabel->ByteSize = 0;

	const int kLabelSize = 32;
	char label[kLabelSize] = { 0 };
	switch (labelType)
	{
	case ELabelType::Function:
		snprintf(label, kLabelSize,"function_%04X", address);
		break;
	case ELabelType::Code:
		snprintf(label, kLabelSize, "label_%04X", address);
		break;
	case ELabelType::Data:
		snprintf(label, kLabelSize, "data_%04X", address);
		pLabel->Global = true;
		break;
	case ELabelType::Text:
	{
		const char* pPrefix = "txt_";
		const std::string textString = GetItemText(state, address);
		std::string labelString;
		const int len = (int)std::min(textString.size(), (size_t)kLabelSize - strlen(pPrefix));
		for (int i = 0; i < len; i++)
		{
			if (textString[i] == ' ')
				labelString += '_';
			else
				labelString += textString[i];
		}

		snprintf(label, kLabelSize, "%s%s", pPrefix, labelString.c_str());
	}
	break;
	}

	pLabel->Name = label;
	if (pLabel->Global)
		GenerateGlobalInfo(state);
	state.SetLabelForAddress(address, pLabel);
	return true;
	
}


class FAnalysisDasmState : public FDasmStateBase
{
public:
	void OutputU8(uint8_t val, z80dasm_output_t outputCallback) override
	{
		if (outputCallback != nullptr)
		{
			ENumberDisplayMode dispMode = GetNumberDisplayMode();

			if (pCodeInfoItem->OperandType == EOperandType::Decimal)
				dispMode = ENumberDisplayMode::Decimal;
			if (pCodeInfoItem->OperandType == EOperandType::Hex)
				dispMode = ENumberDisplayMode::HexAitch;

			const char* outStr = NumStr(val, dispMode);
			for (int i = 0; i < strlen(outStr); i++)
				outputCallback(outStr[i], this);
		}
	}

	void OutputU16(uint16_t val, z80dasm_output_t outputCallback) override
	{
		if (outputCallback)
		{
			ENumberDisplayMode dispMode = GetNumberDisplayMode();

			if (pCodeInfoItem->OperandType == EOperandType::Decimal)
				dispMode = ENumberDisplayMode::Decimal;
			if (pCodeInfoItem->OperandType == EOperandType::Hex)
				dispMode = ENumberDisplayMode::HexAitch;

			const char* outStr = NumStr(val, dispMode);
			for (int i = 0; i < strlen(outStr); i++)
				outputCallback(outStr[i], this);
		}
	}

	void OutputD8(int8_t val, z80dasm_output_t outputCallback) override
	{
		if (outputCallback)
		{
			if (val < 0)
			{
				outputCallback('-', this);
				val = -val;
			}
			else
			{
				outputCallback('+', this);
			}
			const char* outStr = NumStr((uint8_t)val);
			for (int i = 0; i < strlen(outStr); i++)
				outputCallback(outStr[i], this);
		}
	}

	FCodeInfo* pCodeInfoItem = nullptr;
};


/* disassembler callback to fetch the next instruction byte */
static uint8_t AnalysisDasmInputCB(void* pUserData)
{
	FAnalysisDasmState* pDasmState = (FAnalysisDasmState*)pUserData;

	return pDasmState->CodeAnalysisState->CPUInterface->ReadByte( pDasmState->CurrentAddress++);
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
	FCodeInfo* pCodeInfo = state.GetCodeInfoForAddress(pc);
	if (pCodeInfo == nullptr)	// code info could have been cleared
		return;

	FAnalysisDasmState dasmState;
	dasmState.pCodeInfoItem = pCodeInfo;
	dasmState.CodeAnalysisState = &state;
	dasmState.CurrentAddress = pc;
	SetNumberOutput(&dasmState);
	if(state.CPUInterface->CPUType == ECPUType::Z80)
		z80dasm_op(pc, AnalysisDasmInputCB, AnalysisOutputCB, &dasmState);
	else if(state.CPUInterface->CPUType == ECPUType::M6502)
		m6502dasm_op(pc, AnalysisDasmInputCB, AnalysisOutputCB, &dasmState);

	assert(pCodeInfo != nullptr);
	pCodeInfo->Text = dasmState.Text;
	SetNumberOutput(nullptr);
}


uint16_t WriteCodeInfoForAddress(FCodeAnalysisState &state, uint16_t pc)
{
	FCodeInfo* pCodeInfo = state.GetCodeInfoForAddress(pc);
	if (pCodeInfo == nullptr)
	{
		pCodeInfo = FCodeInfo::Allocate();
		state.SetCodeInfoForAddress(pc, pCodeInfo);
	}

	FAnalysisDasmState dasmState;
	dasmState.pCodeInfoItem = pCodeInfo;
	dasmState.CodeAnalysisState = &state;
	dasmState.CurrentAddress = pc;	

	// does this function branch?
	uint16_t jumpAddr;
	if (CheckJumpInstruction(state.CPUInterface, pc, &jumpAddr))
	{
		const bool isCall = CheckCallInstruction(state.CPUInterface, pc);
		if (GenerateLabelForAddress(state, jumpAddr, isCall ? ELabelType::Function : ELabelType::Code))
			state.GetLabelForAddress(jumpAddr)->References[pc]++;

		pCodeInfo->JumpAddress = jumpAddr;
		if (pCodeInfo->OperandType == EOperandType::Unknown)
			pCodeInfo->OperandType = EOperandType::JumpAddress;
	}
	else
	{
		uint16_t ptr;
		if (CheckPointerRefInstruction(state.CPUInterface, pc, &ptr))
		{
			if(pCodeInfo->OperandType == EOperandType::Unknown)
				pCodeInfo->OperandType = EOperandType::Pointer;
			pCodeInfo->PointerAddress = ptr;
		}

		if (CheckPointerIndirectionInstruction(state.CPUInterface, pc, &ptr))
		{
			pCodeInfo->PointerAddress = ptr;
			if (pCodeInfo->OperandType == EOperandType::Unknown)
				pCodeInfo->OperandType = EOperandType::Pointer;
			if (GenerateLabelForAddress(state, ptr, ELabelType::Data))
				state.GetLabelForAddress(ptr)->References[pc]++;
		}
	}

	// generate disassembly
	SetNumberOutput(&dasmState);	// not particularly happy with this - pointer to stack held globally
	uint16_t newPC = pc;

	if (state.CPUInterface->CPUType == ECPUType::Z80)
		newPC = z80dasm_op(pc, AnalysisDasmInputCB, AnalysisOutputCB, &dasmState);
	else if (state.CPUInterface->CPUType == ECPUType::M6502)
		newPC = m6502dasm_op(pc, AnalysisDasmInputCB, AnalysisOutputCB, &dasmState);

	SetNumberOutput(nullptr);

	pCodeInfo->Address = pc;
	for (uint16_t codeAddr = pc; codeAddr < newPC; codeAddr++)
		state.SetCodeInfoForAddress(codeAddr, pCodeInfo);	// make sure all addresses spanned by instruction are set
	pCodeInfo->Text = dasmState.Text;
	pCodeInfo->ByteSize = newPC - pc;

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

	FCodeInfo* pCodeInfo = state.GetCodeInfoForAddress(pc);
	const char* pOldComment = nullptr;
	if (pCodeInfo != nullptr)
	{
		if (pCodeInfo->Address != pc) // check code integrity 
		{
			if(pCodeInfo->Comment.empty() == false)
				pOldComment = pCodeInfo->Comment.c_str();	// get pointer to old comment
			state.SetCodeInfoForAddress(pc, nullptr);
		}
		else if(pCodeInfo->bSelfModifyingCode)	// check SMC
		{
			pCodeInfo->bSelfModifyingCode = false;
			for(uint16_t operandAddr = 0;operandAddr<pCodeInfo->ByteSize;operandAddr++)
			{
				FDataInfo* pOpDataInfo = state.GetReadDataInfoForAddress(pCodeInfo->Address + operandAddr);
				if(pOpDataInfo->Writes.empty() == false)
				{
					pCodeInfo->bSelfModifyingCode = true;
				}					
			}

			return false;	// return
		}
		else
		{
			return false;
		}
	}

	uint16_t newPC = WriteCodeInfoForAddress(state, pc);
	// get new code info
	pCodeInfo = state.GetCodeInfoForAddress(pc);
	if (pOldComment != nullptr)	// restore old comment
		pCodeInfo->Comment = std::string(pOldComment);

	if (CheckStopInstruction(state.CPUInterface, pc) || newPC < pc)
		return false;
	
	pc = newPC;
	state.SetCodeAnalysisDirty();
	return true;
}

// Step through and analyse code from a location
void AnalyseFromPC(FCodeAnalysisState &state, uint16_t pc)
{
	while(AnalyseAtPC(state,pc))
		state.SetCodeAnalysisDirty();

	return;
}

bool RegisterCodeExecuted(FCodeAnalysisState &state, uint16_t pc, uint16_t nextpc)
{
	AnalyseAtPC(state, pc);

	state.FrameTrace.push_back(pc);
	
	FCodeInfo* pCodeInfo = state.GetCodeInfoForAddress(pc);
	if (pCodeInfo != nullptr)
		pCodeInfo->FrameLastExecuted = state.CurrentFrameNo;

	if (state.CPUInterface->CPUType == ECPUType::Z80)
		return RegisterCodeExecutedZ80(state, pc, nextpc);
	else if (state.CPUInterface->CPUType == ECPUType::M6502)
		return RegisterCodeExecuted6502(state, pc, nextpc);

	return false;
}

void RunStaticCodeAnalysis(FCodeAnalysisState &state, uint16_t pc)
{
	AnalyseFromPC(state, pc);
}

void RegisterDataRead(FCodeAnalysisState& state, uint16_t pc, uint16_t dataAddr)
{
	if (state.GetCodeInfoForAddress(dataAddr) == nullptr)	// don't register instruction data reads
	{
		FDataInfo* pDataInfo = state.GetReadDataInfoForAddress(dataAddr);
		pDataInfo->LastFrameRead = state.CurrentFrameNo;
		pDataInfo->Reads[pc]++;
	}
}

void RegisterDataWrite(FCodeAnalysisState &state, uint16_t pc,uint16_t dataAddr)
{
	FDataInfo* pDataInfo = state.GetWriteDataInfoForAddress(dataAddr);
	pDataInfo->LastFrameWritten = state.CurrentFrameNo;
	pDataInfo->Writes[pc]++;
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
				pDataInfo->DataType = EDataType::Byte;
			}
		}

		// ensure self modifying code is flagged properly
		if (pCodeInfo != nullptr && pCodeInfo->bSelfModifyingCode == true)
		{
			pCodeInfo->bSelfModifyingCode = false;
			for (uint16_t operandAddr = 0; operandAddr < pCodeInfo->ByteSize; operandAddr++)
			{
				FDataInfo* pOpDataInfo = state.GetReadDataInfoForAddress(pCodeInfo->Address + operandAddr);
				if (pOpDataInfo->Writes.empty() == false)
				{
					pCodeInfo->bSelfModifyingCode = true;
				}
			}
		}
		/*const FLabelInfo* pLabelInfo = state.GetLabelForAddress(i);
		if ((pCodeInfo != nullptr) && (pLabelInfo != nullptr) && (pLabelInfo->LabelType == LabelType::Data))
		{
			pCodeInfo->bSelfModifyingCode = true;
		}*/
	}
}

// Do we want to do this with every page?
void ResetReferenceInfo(FCodeAnalysisState &state)
{
	for (int i = 0; i < (1 << 16); i++)
	{
		FDataInfo* pDataInfo = state.GetReadDataInfoForAddress(i);
		if (pDataInfo != nullptr)
		{
			pDataInfo->LastFrameRead = -1;
			pDataInfo->Reads.clear();
			pDataInfo->LastFrameWritten = -1;
			pDataInfo->Writes.clear();
		}

		FLabelInfo* pLabelInfo = state.GetLabelForAddress(i);
		if (pLabelInfo != nullptr)
		{
			pLabelInfo->References.clear();
		}

		state.SetLastWriterForAddress(i,  0);
	}
}



FLabelInfo* AddLabel(FCodeAnalysisState &state, uint16_t address,const char *name,ELabelType type)
{
	FLabelInfo *pLabel = FLabelInfo::Allocate();
	pLabel->Name = name;
	pLabel->LabelType = type;
	pLabel->Address = address;
	pLabel->ByteSize = 1;
	pLabel->Global = type == ELabelType::Function;
	state.SetLabelForAddress(address, pLabel);

	if (pLabel->Global)
		GenerateGlobalInfo(state);

	return pLabel;
}

FCommentBlock* AddCommentBlock(FCodeAnalysisState& state, uint16_t address)
{
	FCommentBlock* pExistingBlock = state.GetCommentBlockForAddress(address);
	if(pExistingBlock == nullptr)
	{
		FCommentBlock* pCommentBlock = FCommentBlock::Allocate();
		pCommentBlock->Comment = "";
		pCommentBlock->Address = address;
		pCommentBlock->ByteSize = 1;
		state.SetCommentBlockForAddress(address, pCommentBlock);
		state.SetCodeAnalysisDirty();
		return pCommentBlock;
	}

	return pExistingBlock;
}

// Generate Global Info for items in address space
void GenerateGlobalInfo(FCodeAnalysisState &state)
{
	state.GlobalDataItems.clear();
	state.GlobalFunctions.clear();

	for (int i = 0; i < (1 << 16); i++)
	{
		FLabelInfo *pLabel = state.GetLabelForAddress(i);
		
		if (pLabel != nullptr)
		{
			if (pLabel->LabelType == ELabelType::Data && pLabel->Global)
				state.GlobalDataItems.push_back(pLabel);
			if (pLabel->LabelType == ELabelType::Function)
				state.GlobalFunctions.push_back(pLabel);
		}
		
	}

	state.bRebuildFilteredGlobalDataItems = true;
	state.bRebuildFilteredGlobalFunctions = true;
}

void InitialiseCodeAnalysis(FCodeAnalysisState &state, ICPUInterface* pCPUInterface)
{
	InitImageViewers();
	InitCharacterSets();
	
	state.InitWatches();
	state.ResetLabelNames();
	state.ItemList.clear();

	// This won't work with banked memory
	// we need to reset all the banks
	// the code analyser needs to know about them
	/*for (int i = 0; i < (1 << 16); i++)	// loop across address range
	{
		// clear item pointers
		state.SetLabelForAddress(i, nullptr);
		state.SetCommentBlockForAddress(i, nullptr);
		state.SetCodeInfoForAddress(i, nullptr);

		// set up data entry for address
		FDataInfo* pDataInfo = state.GetReadDataInfoForAddress(i);
		pDataInfo->Reset((uint16_t)i);

		FCodeAnalysisPage* pPage = state.GetReadPage(i);
		assert(i >= pPage->BaseAddress && i < pPage->BaseAddress + FCodeAnalysisPage::kPageSize);
	}*/

	// reset registered pages
	for (FCodeAnalysisPage* pPage : state.GetRegisteredPages())
	{
		for (int addr = 0; addr < FCodeAnalysisPage::kPageSize; addr++)
		{
			pPage->Labels[addr] = nullptr;
			pPage->CommentBlocks[addr] = nullptr;
			pPage->CodeInfo[addr] = nullptr;
			assert(pPage->DataInfo[addr].Address == pPage->BaseAddress + addr);
			pPage->DataInfo[addr].Reset(pPage->BaseAddress + addr);
		}
	}

	FLabelInfo::FreeAll();
	FCodeInfo::FreeAll();
	FCommentBlock::FreeAll();

	for (int i = 0; i < FCodeAnalysisState::kNoViewStates; i++)
	{
		state.ViewState[i].CursorItemIndex = -1;
		state.ViewState[i].SetCursorItem(nullptr);
	}

	state.CPUInterface = pCPUInterface;
	uint16_t initialPC = pCPUInterface->GetPC();
	RunStaticCodeAnalysis(state, initialPC);

	// Key Config
	state.KeyConfig[(int)EKey::SetItemData] = ImGuiKey_D;
	state.KeyConfig[(int)EKey::SetItemText] = ImGuiKey_T;
	state.KeyConfig[(int)EKey::SetItemCode] = ImGuiKey_C;
	state.KeyConfig[(int)EKey::SetItemImage] = ImGuiKey_I;
	state.KeyConfig[(int)EKey::ToggleItemBinary] = ImGuiKey_B;
	state.KeyConfig[(int)EKey::AddLabel] = ImGuiKey_L;
	state.KeyConfig[(int)EKey::Rename] = ImGuiKey_R;
	state.KeyConfig[(int)EKey::Comment] = ImGuiKey_Slash; // '/'
	state.KeyConfig[(int)EKey::AddCommentBlock] = ImGuiKey_Semicolon;	// ';'
	state.KeyConfig[(int)EKey::BreakContinue] = ImGuiKey_F5;
	state.KeyConfig[(int)EKey::StepInto] = ImGuiKey_F11;
	state.KeyConfig[(int)EKey::StepOver] = ImGuiKey_F10;
	state.KeyConfig[(int)EKey::StepFrame] = ImGuiKey_F6;
	state.KeyConfig[(int)EKey::StepScreenWrite] = ImGuiKey_F7;
	state.KeyConfig[(int)EKey::Breakpoint] = ImGuiKey_F9;

	state.StackMin = 0xffff;
	state.StackMax = 0;
}


void SetItemCode(FCodeAnalysisState &state, FItem *pItem)
{
	DoCommand(state, new FSetItemCodeCommand(pItem->Address));
}

void SetItemData(FCodeAnalysisState &state, FItem *pItem)
{
	DoCommand(state, new FSetItemDataCommand(pItem));
}

void SetItemText(FCodeAnalysisState &state, FItem *pItem)
{
	if (pItem->Type == EItemType::Data)
	{
		FDataInfo *pDataItem = static_cast<FDataInfo *>(pItem);
		if (pDataItem->DataType == EDataType::Byte)
		{
			// set to ascii
			pDataItem->ByteSize = 0;	// reset byte counter

			uint16_t charAddr = pDataItem->Address;
			FDataInfo* pDataInfo = state.GetReadDataInfoForAddress(charAddr);
			while (pDataInfo != nullptr && pDataInfo->DataType == EDataType::Byte)
			{
				const uint8_t val = state.CPUInterface->ReadByte(charAddr);
				if (val == 0 || val == 0xff)	// some strings are terminated by 0xff
					break;
				pDataItem->ByteSize++;

				// bit 7 terminated character
				if (val & (1 << 7))
				{
					pDataItem->bBit7Terminator = true;
					break;
				}
				charAddr++;
			}

			// did the operation fail? -revert to byte
			if (pDataItem->ByteSize == 0)
			{
				pDataItem->DataType = EDataType::Byte;
				pDataItem->ByteSize = 1;
			}
			else
			{
				pDataItem->DataType = EDataType::Text;
				state.SetCodeAnalysisDirty();
			}
		}
	}
}

void SetItemImage(FCodeAnalysisState& state, FItem* pItem)
{
	FDataInfo* pDataItem = static_cast<FDataInfo*>(pItem);
	if (pDataItem->DataType != EDataType::Image)
	{
		pDataItem->DataType = EDataType::Image;

		if (pDataItem->ImageData == nullptr)
			pDataItem->ImageData = new FImageData;

		pDataItem->ImageData->ViewerId = 0;	// default to None
		pDataItem->ByteSize = pDataItem->ImageData->SetSizeChars(1,1);
	}
}

void AddLabelAtAddress(FCodeAnalysisState &state, uint16_t address)
{
	if (state.GetLabelForAddress(address) == nullptr)
	{
		ELabelType labelType = ELabelType::Data;
		const FDataInfo* pDataInfo = state.GetReadDataInfoForAddress(address);
		if (pDataInfo && pDataInfo->DataType == EDataType::Text)
			labelType = ELabelType::Text;
		const FCodeInfo* pCodeInfo = state.GetCodeInfoForAddress(address);
		if (pCodeInfo != nullptr && pCodeInfo->bDisabled == false)
			labelType = ELabelType::Code;

		GenerateLabelForAddress(state, address, labelType);
		
		state.SetCodeAnalysisDirty();
	}
}

void RemoveLabelAtAddress(FCodeAnalysisState &state, uint16_t address)
{
	FLabelInfo* pLabelInfo = state.GetLabelForAddress(address);

	if (pLabelInfo != nullptr)
	{
		state.SetLabelForAddress(address, nullptr);
		// Remove from globals
		if (pLabelInfo->Global || pLabelInfo->LabelType == ELabelType::Function)
			GenerateGlobalInfo(state);

		state.SetCodeAnalysisDirty();
	}
}

void SetLabelName(FCodeAnalysisState &state, FLabelInfo *pLabel, const char *pText)
{
	if (strlen(pText) == 0)	// don't let a label be empty
		return;

	state.RemoveLabelName(pLabel->Name);
	pLabel->Name = pText;
	state.EnsureUniqueLabelName(pLabel->Name);
}

void SetItemCommentText(FCodeAnalysisState &state, FItem *pItem, const char *pText)
{
	pItem->Comment = pText;
}

void FormatData(FCodeAnalysisState& state, const FDataFormattingOptions& options)
{
	uint16_t dataAddress = options.StartAddress;

	// TODO: Register Character Maps here?
	if (options.DataType == EDataType::CharacterMap)
	{
		FCharMapCreateParams charMapParams;
		charMapParams.Address = dataAddress;
		charMapParams.CharacterSet = options.CharacterSet;
		charMapParams.Width = options.ItemSize;
		charMapParams.Height = options.NoItems;
		charMapParams.IgnoreCharacter = options.EmptyCharNo;
		CreateCharacterMap(state, charMapParams);
	}

	if (options.AddLabelAtStart && state.GetLabelForAddress(dataAddress) == nullptr)	// only add label if one doesn't already exist
	{
		char labelName[16];
		const char* pPrefix = "data";

		if (options.DataType == EDataType::Bitmap)
			pPrefix = "bitmap";
		else if (options.DataType == EDataType::CharacterMap)
			pPrefix = "charmap";
		else if (options.DataType == EDataType::Text)
			pPrefix = "text";

		snprintf(labelName,16, "%s_%s",pPrefix,NumStr(dataAddress));
		FLabelInfo* pLabel = AddLabel(state, dataAddress, labelName, ELabelType::Data);
		pLabel->Global = true;
	}

	for (int itemNo = 0; itemNo < options.NoItems; itemNo++)
	{
		FDataInfo* pDataInfo = state.GetReadDataInfoForAddress(dataAddress);

		pDataInfo->ByteSize = options.ItemSize;
		pDataInfo->DataType = options.DataType;

		if (options.DataType == EDataType::CharacterMap)
		{
			pDataInfo->CharSetAddress = options.CharacterSet;
			pDataInfo->EmptyCharNo = options.EmptyCharNo;
		}

		// iterate through each memory location
		for (int i = 0; i < options.ItemSize;i++)
		{
			if (options.ClearCodeInfo)
				state.SetCodeInfoForAddress(dataAddress, nullptr);
			
			if (options.ClearLabels && dataAddress != options.StartAddress)	// don't remove first label
				RemoveLabelAtAddress(state, dataAddress);

			dataAddress++;
		}
	}
}



// number output abstraction
IDasmNumberOutput* g_pNumberOutputObj = nullptr;
IDasmNumberOutput* GetNumberOutput()
{
	return g_pNumberOutputObj;
}

void SetNumberOutput(IDasmNumberOutput* pNumberOutputObj)
{
	g_pNumberOutputObj = pNumberOutputObj;
}