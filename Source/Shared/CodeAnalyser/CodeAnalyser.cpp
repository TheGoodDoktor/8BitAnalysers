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

// memory bank code

bool FCodeAnalysisBank::IsAddressBreakpointed(uint16_t addr) const
{
	for (const auto& bpIt : BreakPoints)
	{
		if (bpIt.Address == addr)
			return true;
	}

	return false;
}
bool FCodeAnalysisBank::ToggleExecBreakpointAtAddress(uint16_t addr)
{
	for (auto bpIt = BreakPoints.begin();bpIt!=BreakPoints.end();++bpIt)
	{
		if (bpIt->Address == addr)
		{
			BreakPoints.erase(bpIt);
			return false;
		}
	}

	BreakPoints.emplace_back(addr, FCodeAnalysisBankBP::EType::Exe, 0);
	return true;
}

bool FCodeAnalysisBank::ToggleDataBreakpointAtAddress(uint16_t addr, uint16_t dataSize)
{
	for (auto bpIt = BreakPoints.begin(); bpIt != BreakPoints.end(); ++bpIt)
	{
		if (bpIt->Address == addr)
		{
			BreakPoints.erase(bpIt);
			return false;
		}
	}

	if(dataSize == 1)
		BreakPoints.emplace_back(addr, FCodeAnalysisBankBP::EType::Byte, Memory[addr]);
	else
		BreakPoints.emplace_back(addr, FCodeAnalysisBankBP::EType::Word, Memory[addr] | (Memory[addr+1] << 8));

	return true;
}

// create a bank
// a bank is a list of memory pages
int16_t	FCodeAnalysisState::CreateBank(const char* bankName, int noKb,uint8_t* pBankMem, bool bReadOnly)
{
	const int16_t bankId = (int16_t)Banks.size();
	const int noPages = noKb;

	FCodeAnalysisBank& newBank = Banks.emplace_back();
	newBank.Id = bankId;
	newBank.NoPages = noPages;
	newBank.SizeMask = (noPages * FCodeAnalysisPage::kPageSize) - 1;
	newBank.Memory = pBankMem;
	newBank.Pages = new FCodeAnalysisPage[noPages];
	newBank.Name = bankName;
	newBank.bReadOnly = bReadOnly;
	for (int pageNo = 0; pageNo < noPages; pageNo++)
	{
		char pageName[32];
		sprintf(pageName, "%s:%d", bankName, pageNo);
		RegisterPage(&newBank.Pages[pageNo], pageName);
	}
	return bankId;
}

/*FCodeAnalysisBank* FCodeAnalysisState::GetBank(int16_t bankId)
{
	if (bankId < 0 || bankId >= Banks.size())
		return nullptr;

	return &Banks[bankId];
}*/

// Set bank to memory pages starting at pageNo
bool FCodeAnalysisState::MapBank(int16_t bankId, int startPageNo)
{
	FCodeAnalysisBank* pBank = GetBank(bankId);
	if (pBank == nullptr || MappedBanks[startPageNo] == bankId)	// not found or already mapped to this locatiom
		return false;

	if (pBank->bReadOnly == true && startPageNo > 0)
		assert(0);

	if (pBank->PrimaryMappedPage == -1 )	// Newly mapped?
	{
		pBank->PrimaryMappedPage = startPageNo;
		pBank->bIsDirty = true;
	}
	assert(pBank->PrimaryMappedPage != -1);

	pBank->MappedPages.push_back(startPageNo);
	for (int bankPageNo = 0; bankPageNo < pBank->NoPages; bankPageNo++)
	{
		//if(pBank->bReadOnly)
		//	SetCodeAnalysisRWPage(startPageNo + bankPageNo, &pBank->Pages[bankPageNo], nullptr);	// Read only
		//else
		SetCodeAnalysisRWPage(startPageNo + bankPageNo, &pBank->Pages[bankPageNo], &pBank->Pages[bankPageNo]);	// Read/Write

		MappedBanks[startPageNo + bankPageNo] = bankId;
	}
	bMemoryRemapped = true;

	// enable breakpoints in the bank we're switching in
	for (const auto& bp : pBank->BreakPoints)
	{
		const uint16_t addr = pBank->GetMappedAddress() + bp.Address;

		switch (bp.Type)
		{
		case FCodeAnalysisBankBP::EType::Exe:
			CPUInterface->SetExecBreakpointAtAddress(addr, true);
			break;
		case FCodeAnalysisBankBP::EType::Byte:
			CPUInterface->SetDataBreakpointAtAddress(addr, 1, true);
			break;
		case FCodeAnalysisBankBP::EType::Word:
			CPUInterface->SetDataBreakpointAtAddress(addr, 2, true);
			break;
		}
	}

	//RemappedBanks.push_back(bankId);
	bCodeAnalysisDataDirty = true;

	return true;
}

bool FCodeAnalysisState::UnMapBank(int16_t bankId, int startPageNo)
{
	FCodeAnalysisBank* pBank = GetBank(bankId);
	if (pBank == nullptr || MappedBanks[startPageNo] != bankId)
		return false;

	// disable breakpoints in the bank we're switching out
	for (const auto& bp : pBank->BreakPoints)
	{
		const uint16_t addr = pBank->GetMappedAddress() + bp.Address;

		switch (bp.Type)
		{
		case FCodeAnalysisBankBP::EType::Exe:
			CPUInterface->SetExecBreakpointAtAddress(addr, false);
			break;
		case FCodeAnalysisBankBP::EType::Byte:
			CPUInterface->SetDataBreakpointAtAddress(addr, 1, false);
			break;
		case FCodeAnalysisBankBP::EType::Word:
			CPUInterface->SetDataBreakpointAtAddress(addr, 2, false);
			break;
		}
	}
	
	for (int bankPage = 0; bankPage < pBank->NoPages; bankPage++)
		MappedBanks[startPageNo + bankPage] = -1;

	// erase from mapped pages - better way?
	auto it = pBank->MappedPages.begin();

	while (it != pBank->MappedPages.end())
	{
		if (*it == startPageNo)
			it = pBank->MappedPages.erase(it);
		else
			++it;
	}

	return true;
}

bool FCodeAnalysisState::IsBankIdMapped(int16_t bankId) const
{
	for (int bankIdx = 0; bankIdx < kNoPagesInAddressSpace; bankIdx++)
	{
		if (MappedBanks[bankIdx] == bankId)
			return true;
	}

	return false;
}

bool FCodeAnalysisState::IsAddressValid(FAddressRef addr) const
{
	const FCodeAnalysisBank* pBank = GetBank(addr.BankId);
	if (pBank == nullptr)
		return false;

	if(addr.Address < pBank->GetMappedAddress() || addr.Address > (pBank->GetMappedAddress() + pBank->GetSizeBytes()))
		return false;

	return true;
}

bool FCodeAnalysisState::MapBankForAnalysis(FCodeAnalysisBank& bank)
{
	for(int i=0;i< kNoPagesInAddressSpace;i++)
		MappedBanksBackup[i] = MappedBanks[i];

	const int startPageNo = bank.PrimaryMappedPage;
	for (int bankPageNo = 0; bankPageNo < bank.NoPages; bankPageNo++)
	{
		MappedBanks[startPageNo + bankPageNo] = bank.Id;
		MappedMem[startPageNo + bankPageNo] = &bank.Memory[bankPageNo * FCodeAnalysisPage::kPageSize];
		SetCodeAnalysisRWPage(startPageNo + bankPageNo, &bank.Pages[bankPageNo], &bank.Pages[bankPageNo]);	// Read/Write
	}

	return true;
}

void FCodeAnalysisState::UnMapAnalysisBanks()
{
	for (int i = 0; i < kNoPagesInAddressSpace; i++)
	{
		MappedBanks[i] = MappedBanksBackup[i];
		const FCodeAnalysisBank* pMappedBank = GetBank(MappedBanks[i]);
		if (MappedMem[i] != nullptr)
		{
			const int mappedPage = i - pMappedBank->PrimaryMappedPage;
			SetCodeAnalysisRWPage(i, &pMappedBank->Pages[mappedPage], &pMappedBank->Pages[mappedPage]);	// Read/Write
			MappedMem[i] = nullptr;
		}
	}
}

bool FCodeAnalysisState::IsAddressBreakpointed(FAddressRef addr) const
{
	const FCodeAnalysisBank* pBank = GetBank(addr.BankId);
	assert(pBank != nullptr);
	return pBank->IsAddressBreakpointed(addr.Address & pBank->SizeMask);
}

bool FCodeAnalysisState::ToggleExecBreakpointAtAddress(FAddressRef addr)
{
	FCodeAnalysisBank* pBank = GetBank(addr.BankId);
	assert(pBank != nullptr);
	const bool bpSet = pBank->ToggleExecBreakpointAtAddress(addr.Address & pBank->SizeMask);
	if (pBank->IsMapped())
		CPUInterface->SetExecBreakpointAtAddress(addr.Address, bpSet);

	return bpSet;
}

bool FCodeAnalysisState::ToggleDataBreakpointAtAddress(FAddressRef addr, uint16_t dataSize)
{
	FCodeAnalysisBank* pBank = GetBank(addr.BankId);
	assert(pBank != nullptr);
	const bool bpSet = pBank->ToggleDataBreakpointAtAddress(addr.Address & pBank->SizeMask,dataSize);
	if (pBank->IsMapped())
		CPUInterface->SetDataBreakpointAtAddress(addr.Address, dataSize, bpSet);

	return bpSet;
}


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
			const uint8_t byte = ReadByte(address + byteNo);
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
	const ICPUInterface* pCPUInterface = CPUInterface;
	const int kStringMinLength = 4;
	int stringLength = 0;
	uint16_t stringStart = 0;
	std::string dbgString;

	do
	{
		FCodeInfo* pCodeInfo = GetCodeInfoForAddress(address);
		FDataInfo* pDataInfo = GetReadDataInfoForAddress(address);
		const uint8_t byte = ReadByte(address++);

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



bool CheckPointerIndirectionInstruction(FCodeAnalysisState& state, uint16_t pc, uint16_t* out_addr)
{
	const ICPUInterface* pCPUInterface = state.CPUInterface;

	if (pCPUInterface->CPUType == ECPUType::Z80)
		return CheckPointerIndirectionInstructionZ80(state, pc, out_addr);
	else if (pCPUInterface->CPUType == ECPUType::M6502)
		return CheckPointerIndirectionInstruction6502(state, pc, out_addr);
	else
		return false;
}

bool CheckPointerRefInstruction(FCodeAnalysisState& state, uint16_t pc, uint16_t* out_addr)
{
	const ICPUInterface* pCPUInterface = state.CPUInterface;

	if (pCPUInterface->CPUType == ECPUType::Z80)
		return CheckPointerRefInstructionZ80(state, pc, out_addr);
	else if (pCPUInterface->CPUType == ECPUType::M6502)
		return CheckPointerRefInstruction6502(state, pc, out_addr);
	else
		return false;
}


bool CheckJumpInstruction(FCodeAnalysisState& state, uint16_t pc, uint16_t* out_addr)
{
	const ICPUInterface* pCPUInterface = state.CPUInterface;

	if (pCPUInterface->CPUType == ECPUType::Z80)
		return CheckJumpInstructionZ80(state, pc, out_addr);
	else if (pCPUInterface->CPUType == ECPUType::M6502)
		return CheckJumpInstruction6502(state, pc, out_addr);
	else
		return false;
}



bool CheckCallInstruction(FCodeAnalysisState& state, uint16_t pc)
{
	const ICPUInterface* pCPUInterface = state.CPUInterface;

	if (pCPUInterface->CPUType == ECPUType::Z80)
		return CheckCallInstructionZ80(state, pc);
	else if(pCPUInterface->CPUType == ECPUType::M6502)
		return CheckCallInstruction6502(state, pc);
	else
		return false;
}

// check if function should stop static analysis
// this would be a function that unconditionally affects the PC
bool CheckStopInstruction(FCodeAnalysisState& state, uint16_t pc)
{
	const ICPUInterface* pCPUInterface = state.CPUInterface;

	if (pCPUInterface->CPUType == ECPUType::Z80)
		return CheckStopInstructionZ80(state, pc);
	else if(pCPUInterface->CPUType == ECPUType::M6502)
		return CheckStopInstruction6502(state, pc);
	else
		return false;
}

// this function assumes the text is mapped in
std::string GetItemText(FCodeAnalysisState& state, FAddressRef address)
{
	FDataInfo* pDataInfo = state.GetReadDataInfoForAddress(address);
	std::string textString;

	if (pDataInfo->DataType != EDataType::Text)
		return textString;	// error text?

	for (int i = 0; i < pDataInfo->ByteSize; i++)
	{
		const char ch = state.ReadByte(address.Address + i);
		if (ch == '\n')
			textString += "<cr>";
		if (pDataInfo->bBit7Terminator && ch & (1 << 7))	// check bit 7 terminator flag
			textString += ch & ~(1 << 7);	// remove bit 7
		else
			textString += ch;
	}

	return textString;
}

FLabelInfo* GenerateLabelForAddress(FCodeAnalysisState &state, FAddressRef address, ELabelType labelType)
{
	FLabelInfo* pLabel = state.GetLabelForAddress(address);
	if (pLabel != nullptr)
		return nullptr;
		
	pLabel = FLabelInfo::Allocate();
	pLabel->LabelType = labelType;
	//pLabel->Address = address;
	pLabel->ByteSize = 0;

	const int kLabelSize = 32;
	char label[kLabelSize] = { 0 };
	switch (labelType)
	{
	case ELabelType::Function:
		snprintf(label, kLabelSize,"function_%04X", address.Address);
		break;
	case ELabelType::Code:
		snprintf(label, kLabelSize, "label_%04X", address.Address);
		break;
	case ELabelType::Data:
		snprintf(label, kLabelSize, "data_%04X", address.Address);
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
	return pLabel;	
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
			if (pCodeInfoItem->OperandType == EOperandType::Binary)
				dispMode = ENumberDisplayMode::Binary;

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
			if (pCodeInfoItem->OperandType == EOperandType::Binary)
				dispMode = ENumberDisplayMode::Binary;

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

	return pDasmState->CodeAnalysisState->ReadByte( pDasmState->CurrentAddress++);
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

// This assumes that the address passed in is mapped to physical memory
uint16_t WriteCodeInfoForAddress(FCodeAnalysisState &state, uint16_t pc)
{
	FCodeInfo* pCodeInfo = state.GetCodeInfoForAddress(pc);
	if (pCodeInfo == nullptr)
	{
		pCodeInfo = FCodeInfo::Allocate();
		state.SetCodeInfoForAddress(pc, pCodeInfo);
	}	

	// does this function branch?
	uint16_t jumpAddr;
	if (CheckJumpInstruction(state, pc, &jumpAddr))
	{
		const bool isCall = CheckCallInstruction(state, pc);
		FLabelInfo* pLabel = GenerateLabelForAddress(state, state.AddressRefFromPhysicalAddress(jumpAddr), isCall ? ELabelType::Function : ELabelType::Code);
		if(pLabel)
			pLabel->References.RegisterAccess(state.AddressRefFromPhysicalAddress(pc));

		pCodeInfo->JumpAddress = state.AddressRefFromPhysicalAddress(jumpAddr);
		assert(state.IsAddressValid(pCodeInfo->JumpAddress));

		if (pCodeInfo->OperandType == EOperandType::Unknown)
			pCodeInfo->OperandType = EOperandType::JumpAddress;
	}
	else
	{
		uint16_t ptr;
		if (CheckPointerRefInstruction(state, pc, &ptr))
		{
			if(pCodeInfo->OperandType == EOperandType::Unknown)
				pCodeInfo->OperandType = EOperandType::Pointer;
			pCodeInfo->PointerAddress = state.AddressRefFromPhysicalAddress(ptr);
		}

		if (CheckPointerIndirectionInstruction(state, pc, &ptr))
		{
			pCodeInfo->PointerAddress = state.AddressRefFromPhysicalAddress(ptr);
			if (pCodeInfo->OperandType == EOperandType::Unknown)
				pCodeInfo->OperandType = EOperandType::Pointer;
			
			FLabelInfo* pLabel = GenerateLabelForAddress(state, state.AddressRefFromPhysicalAddress(ptr), ELabelType::Data);
			if (pLabel)
				pLabel->References.RegisterAccess(state.AddressRefFromPhysicalAddress(pc));
		}
	}

	// generate disassembly
	FAnalysisDasmState dasmState;
	dasmState.pCodeInfoItem = pCodeInfo;
	dasmState.CodeAnalysisState = &state;
	dasmState.CurrentAddress = pc;
	SetNumberOutput(&dasmState);	// not particularly happy with this - pointer to stack held globally
	uint16_t newPC = pc;

	if (state.CPUInterface->CPUType == ECPUType::Z80)
		newPC = z80dasm_op(pc, AnalysisDasmInputCB, AnalysisOutputCB, &dasmState);
	else if (state.CPUInterface->CPUType == ECPUType::M6502)
		newPC = m6502dasm_op(pc, AnalysisDasmInputCB, AnalysisOutputCB, &dasmState);

	SetNumberOutput(nullptr);

	state.SetCodeInfoForAddress(pc, pCodeInfo);	

	// set operands as data item
	for (uint16_t codeAddr = pc + 1; codeAddr < newPC; codeAddr++)
	{
		FDataInfo* pOperandData = state.GetReadDataInfoForAddress(codeAddr);
		pOperandData->DataType = EDataType::InstructionOperand;
		pOperandData->ByteSize = 1;
		pOperandData->InstructionAddress = state.AddressRefFromPhysicalAddress(pc);
	}
	pCodeInfo->Text = dasmState.Text;
	pCodeInfo->ByteSize = newPC - pc;

	return newPC;
}

// return if we should continue
bool AnalyseAtPC(FCodeAnalysisState &state, uint16_t& pc)
{
	FCodeInfo* pCodeInfo = state.GetCodeInfoForAddress(pc);

	// Register Code accesses
	// 
	// set jump reference
	uint16_t jumpAddr;
	if (CheckJumpInstruction(state, pc, &jumpAddr))
	{
		FLabelInfo* pLabel = state.GetLabelForAddress(jumpAddr);
		if (pLabel != nullptr)
			pLabel->References.RegisterAccess(state.AddressRefFromPhysicalAddress(pc));
		if (pCodeInfo != nullptr)
		{
			pCodeInfo->JumpAddress = state.AddressRefFromPhysicalAddress(jumpAddr);
			assert(state.IsAddressValid(pCodeInfo->JumpAddress));
		}

	}

	// set pointer reference
	uint16_t ptr;
	if (CheckPointerRefInstruction(state, pc, &ptr))
	{
		FLabelInfo* pLabel = state.GetLabelForAddress(ptr);
		if (pLabel != nullptr)
			pLabel->References.RegisterAccess(state.AddressRefFromPhysicalAddress(pc));
		if (pCodeInfo != nullptr)
			pCodeInfo->PointerAddress = state.AddressRefFromPhysicalAddress(ptr);
	}

	const char* pOldComment = nullptr;
	if (pCodeInfo != nullptr)
	{
		if(pCodeInfo->bSelfModifyingCode)	// check SMC
		{
			pCodeInfo->bSelfModifyingCode = false;
			for(uint16_t operandAddr = 0;operandAddr<pCodeInfo->ByteSize;operandAddr++)
			{
				FDataInfo* pOpDataInfo = state.GetReadDataInfoForAddress(pc + operandAddr);
				if(pOpDataInfo->Writes.IsEmpty() == false)
				{
					pCodeInfo->bSelfModifyingCode = true;
				}					
			}
		}
		return false;
	}

	const uint16_t newPC = WriteCodeInfoForAddress(state, pc);

	// get new code info
	pCodeInfo = state.GetCodeInfoForAddress(pc);
	if (pOldComment != nullptr)	// restore old comment
		pCodeInfo->Comment = std::string(pOldComment);

	if (CheckStopInstruction(state, pc) || newPC < pc)
		return false;
	
	pc = newPC;
	state.SetCodeAnalysisDirty(pc);
	return true;
}

// Step through and analyse code from a location
void AnalyseFromPC(FCodeAnalysisState &state, uint16_t pc)
{
	while(AnalyseAtPC(state,pc))
		state.SetCodeAnalysisDirty(pc);

	return;
}

bool RegisterCodeExecuted(FCodeAnalysisState &state, uint16_t pc, uint16_t nextpc)
{
	AnalyseAtPC(state, pc);

	state.FrameTrace.push_back(state.AddressRefFromPhysicalAddress(pc));
	
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

uint16_t g_DbgReadAddress = 0xddf8;

void RegisterDataRead(FCodeAnalysisState& state, uint16_t pc, uint16_t dataAddr)
{
	if (dataAddr == g_DbgReadAddress)
	{
		LOGINFO("Access 0x%04X at PC:", g_DbgReadAddress, pc);
	}

	if (state.GetCodeInfoForAddress(dataAddr) == nullptr)	// don't register instruction data reads
	{
		FDataInfo* pDataInfo = state.GetReadDataInfoForAddress(dataAddr);
		pDataInfo->LastFrameRead = state.CurrentFrameNo;
		pDataInfo->Reads.RegisterAccess(state.AddressRefFromPhysicalAddress(pc));
	}
}

void RegisterDataWrite(FCodeAnalysisState &state, uint16_t pc,uint16_t dataAddr,uint8_t value)
{
	FDataInfo* pDataInfo = state.GetWriteDataInfoForAddress(dataAddr);
	pDataInfo->LastFrameWritten = state.CurrentFrameNo;
	pDataInfo->Writes.RegisterAccess(state.AddressRefFromPhysicalAddress(pc));
}

void ReAnalyseCode(FCodeAnalysisState &state)
{
	int addr = 0;
	while ( addr < (1 << 16))
	{
		FCodeInfo* pCodeInfo = state.GetCodeInfoForAddress(addr);
		if (pCodeInfo != nullptr)
		{
			pCodeInfo->bSelfModifyingCode = false;

			for (int i = 0; i < pCodeInfo->ByteSize; i++)
			{
				FDataInfo* pOperandData = state.GetReadDataInfoForAddress(addr + i);
				pOperandData->ByteSize = 1;
				pOperandData->DataType = EDataType::InstructionOperand;
				pOperandData->InstructionAddress = state.AddressRefFromPhysicalAddress(addr);
				if (pOperandData->Writes.IsEmpty() == false)
					pCodeInfo->bSelfModifyingCode = true;
				if (i > 0)	// make sure other entries after are null
					state.SetCodeInfoForAddress(addr + i, nullptr);
			}

			addr += pCodeInfo->ByteSize;
		}
		else
		{
			FDataInfo* pDataInfo = state.GetReadDataInfoForAddress(addr);
			addr++;
		}
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
			pDataInfo->Reads.Reset();
			pDataInfo->LastFrameWritten = -1;
			pDataInfo->Writes.Reset();
		}

		FLabelInfo* pLabelInfo = state.GetLabelForAddress(i);
		if (pLabelInfo != nullptr)
		{
			pLabelInfo->References.Reset();
		}

		state.SetLastWriterForAddress(i,  FAddressRef());
	}
}



FLabelInfo* AddLabel(FCodeAnalysisState &state, uint16_t address,const char *name,ELabelType type)
{
	FLabelInfo *pLabel = FLabelInfo::Allocate();
	pLabel->Name = name;
	pLabel->LabelType = type;
	//pLabel->Address = address;
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
		//pCommentBlock->Address = address;
		pCommentBlock->ByteSize = 1;
		state.SetCommentBlockForAddress(address, pCommentBlock);
		state.SetCodeAnalysisDirty(address);
		return pCommentBlock;
	}

	return pExistingBlock;
}

// Generate Global Info for items in address space
void GenerateGlobalInfo(FCodeAnalysisState &state)
{
	state.GlobalDataItems.clear();
	state.GlobalFunctions.clear();

	// Make global list from what's in all banks
	for (auto& bank : state.GetBanks())
	{
		if (bank.PrimaryMappedPage == -1)
			continue;

		for (int pageNo = 0; pageNo < bank.NoPages; pageNo++)
		{
			const FCodeAnalysisPage& page = bank.Pages[pageNo];
			const uint16_t pageBaseAddr = bank.GetMappedAddress() + (pageNo * FCodeAnalysisPage::kPageSize);

			for (int pageAddr = 0; pageAddr < FCodeAnalysisPage::kPageSize; pageAddr++)
			{
				FLabelInfo* pLabel = page.Labels[pageAddr];
				if (pLabel != nullptr)
				{
					if (pLabel->LabelType == ELabelType::Data && pLabel->Global)
						state.GlobalDataItems.emplace_back(pLabel, FAddressRef(bank.Id, pageBaseAddr + pageAddr));
					if (pLabel->LabelType == ELabelType::Function)
						state.GlobalFunctions.emplace_back(pLabel, FAddressRef(bank.Id, pageBaseAddr + pageAddr));
				}
			}

		}
	}

	/*for (int addr = 0; addr < (1 << 16); addr++)
	{
		FLabelInfo *pLabel = state.GetLabelForAddress(addr);
		
		if (pLabel != nullptr)
		{
			const int16_t bankId = state.GetBankFromAddress(addr);
		
			if (pLabel->LabelType == ELabelType::Data && pLabel->Global)
				state.GlobalDataItems.emplace_back(pLabel,bankId, addr);
			if (pLabel->LabelType == ELabelType::Function)
				state.GlobalFunctions.emplace_back(pLabel, bankId, addr);
		}
		
	}*/

	state.bRebuildFilteredGlobalDataItems = true;
	state.bRebuildFilteredGlobalFunctions = true;
}

FCodeAnalysisState::FCodeAnalysisState()
{
	for (int i = 0; i < kNoPagesInAddressSpace; i++)
	{
		MappedMem[i] = nullptr;
		MappedBanks[i] = -1;
		MappedBanksBackup[i] = -1;
		ReadPageTable[i] = nullptr;
		WritePageTable[i] = nullptr;
	}

}

// Called each time a new game is loaded up
void FCodeAnalysisState::Init(ICPUInterface* pCPUInterface)
{
	InitImageViewers();
	InitCharacterSets();
	
	InitWatches();
	ResetLabelNames();
	ItemList.clear();

	// reset registered pages
	for (FCodeAnalysisPage* pPage : GetRegisteredPages())
	{
		//pPage->bUsed = false;

		for (int addr = 0; addr < FCodeAnalysisPage::kPageSize; addr++)
		{
			pPage->Labels[addr] = nullptr;
			pPage->CommentBlocks[addr] = nullptr;
			pPage->CodeInfo[addr] = nullptr;
			pPage->DataInfo[addr].Reset();
			pPage->MachineState[addr] = nullptr;
		}
	}	

	// clear mapped mem
	for (int i = 0; i < kNoPagesInAddressSpace; i++)
	{
		MappedMem[i] = nullptr;
	}
	
	FreeMachineStates(*this);
	FLabelInfo::FreeAll();
	FCodeInfo::FreeAll();
	FCommentBlock::FreeAll();

	for (int i = 0; i < FCodeAnalysisState::kNoViewStates; i++)
	{
		//ViewState[i].CursorItemIndex = -1;
		ViewState[i].SetCursorItem(FCodeAnalysisItem());
	}

	// reset banks
	for (auto& bank : Banks)
	{
		bank.Description.clear();
		bank.ItemList.clear();
	}

	CPUInterface = pCPUInterface;
	//uint16_t initialPC = pCPUInterface->GetPC();
	//RunStaticCodeAnalysis(*this, initialPC);

	// Key Config
	KeyConfig[(int)EKey::SetItemData] = ImGuiKey_D;
	KeyConfig[(int)EKey::SetItemText] = ImGuiKey_T;
	KeyConfig[(int)EKey::SetItemCode] = ImGuiKey_C;
	KeyConfig[(int)EKey::SetItemImage] = ImGuiKey_I;
	KeyConfig[(int)EKey::ToggleItemBinary] = ImGuiKey_B;
	KeyConfig[(int)EKey::AddLabel] = ImGuiKey_L;
	KeyConfig[(int)EKey::Rename] = ImGuiKey_R;
	KeyConfig[(int)EKey::Comment] = ImGuiKey_Slash; // '/'
	KeyConfig[(int)EKey::AddCommentBlock] = ImGuiKey_Semicolon;	// ';'
	KeyConfig[(int)EKey::BreakContinue] = ImGuiKey_F5;
	KeyConfig[(int)EKey::StepInto] = ImGuiKey_F11;
	KeyConfig[(int)EKey::StepOver] = ImGuiKey_F10;
	KeyConfig[(int)EKey::StepFrame] = ImGuiKey_F6;
	KeyConfig[(int)EKey::StepScreenWrite] = ImGuiKey_F7;
	KeyConfig[(int)EKey::Breakpoint] = ImGuiKey_F9;

	StackMin = 0xffff;
	StackMax = 0;
}


void SetItemCode(FCodeAnalysisState &state, FAddressRef address)
{
	DoCommand(state, new FSetItemCodeCommand(address));
}

void SetItemData(FCodeAnalysisState &state, const FCodeAnalysisItem& item)
{
	DoCommand(state, new FSetItemDataCommand(item));
}

void SetItemText(FCodeAnalysisState &state, const FCodeAnalysisItem& item)
{
	if (item.IsValid() == false)
		return;

	if (item.Item->Type == EItemType::Data)
	{
		FDataInfo *pDataItem = static_cast<FDataInfo *>(item.Item);
		if (pDataItem->DataType == EDataType::Byte)
		{
			// set to ascii
			pDataItem->ByteSize = 0;	// reset byte counter

			FAddressRef charAddr = item.AddressRef;
			FDataInfo* pDataInfo = state.GetReadDataInfoForAddress(charAddr);
			while (pDataInfo != nullptr && pDataInfo->DataType == EDataType::Byte)
			{
				const uint8_t val = state.ReadByte(charAddr);
				if (val == 0 || val == 0xff)	// some strings are terminated by 0xff
					break;
				pDataItem->ByteSize++;

				// bit 7 terminated character
				if (val & (1 << 7))
				{
					pDataItem->bBit7Terminator = true;
					break;
				}
				charAddr.Address++;
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
				state.SetCodeAnalysisDirty(item.AddressRef);
			}
		}
	}
}

void SetItemImage(FCodeAnalysisState& state, const FCodeAnalysisItem& item)
{
	FDataInfo* pDataItem = static_cast<FDataInfo*>(item.Item);
	if (pDataItem->DataType != EDataType::Image)
	{
		pDataItem->DataType = EDataType::Image;

		if (pDataItem->ImageData == nullptr)
			pDataItem->ImageData = new FImageData;

		pDataItem->ImageData->ViewerId = 0;	// default to None
		pDataItem->ByteSize = pDataItem->ImageData->SetSizeChars(1,1);
	}
}

FLabelInfo* AddLabelAtAddress(FCodeAnalysisState &state, FAddressRef address)
{
	FLabelInfo* pNewLabel = nullptr;

	if (state.GetLabelForAddress(address) == nullptr)
	{
		ELabelType labelType = ELabelType::Data;
		const FDataInfo* pDataInfo = state.GetReadDataInfoForAddress(address);
		if (pDataInfo && pDataInfo->DataType == EDataType::Text)
			labelType = ELabelType::Text;
		const FCodeInfo* pCodeInfo = state.GetCodeInfoForAddress(address);
		if (pCodeInfo != nullptr && pCodeInfo->bDisabled == false)
			labelType = ELabelType::Code;

		pNewLabel = GenerateLabelForAddress(state, address, labelType);
		
		state.SetCodeAnalysisDirty(address);
	}

	return pNewLabel;
}

void RemoveLabelAtAddress(FCodeAnalysisState &state, FAddressRef address)
{
	FLabelInfo* pLabelInfo = state.GetLabelForAddress(address);

	if (pLabelInfo != nullptr)
	{
		state.SetLabelForAddress(address, nullptr);
		// Remove from globals
		if (pLabelInfo->Global || pLabelInfo->LabelType == ELabelType::Function)
			GenerateGlobalInfo(state);

		state.SetCodeAnalysisDirty(address);
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

void SetItemCommentText(FCodeAnalysisState &state, const FCodeAnalysisItem& item, const char *pText)
{
	item.Item->Comment = pText;
}

void FormatData(FCodeAnalysisState& state, const FDataFormattingOptions& options)
{
	uint16_t dataAddress = options.StartAddress;

	// TODO: Register Character Maps here?
	if (options.DataType == EDataType::CharacterMap)
	{
		FCharMapCreateParams charMapParams;
		charMapParams.Address = state.AddressRefFromPhysicalAddress(dataAddress);
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
				RemoveLabelAtAddress(state, state.AddressRefFromPhysicalAddress(dataAddress));

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

// machine state
FMachineState* AllocateMachineState(FCodeAnalysisState& state)
{
	switch (state.CPUInterface->CPUType)
	{
	case ECPUType::Z80:
		return AllocateMachineStateZ80();
	case ECPUType::M6502:
		return nullptr;// TODO: this needs to be implemented
	default:
		return nullptr;
	}
}

void FreeMachineStates(FCodeAnalysisState& state)
{
	if (state.CPUInterface == nullptr)
		return;

	switch (state.CPUInterface->CPUType)
	{
	case ECPUType::Z80:
		return FreeMachineStatesZ80();
	case ECPUType::M6502:
		return;// TODO: this needs to be implemented
	}
}

void CaptureMachineState(FMachineState* pMachineState, ICPUInterface* pCPUInterface)
{
	switch (pCPUInterface->CPUType)
	{
	case ECPUType::Z80:
		CaptureMachineStateZ80(pMachineState,pCPUInterface);
		return;
	case ECPUType::M6502:
		// TODO: this needs to be implemented
		return;
	}
}
