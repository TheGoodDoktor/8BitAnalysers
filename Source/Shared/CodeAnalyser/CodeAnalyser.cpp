#include "CodeAnalyser.h"

#include <cstdint>
#include <algorithm>
#include <cassert>
#include <stdio.h>
#include <string.h>

#include <imgui.h>

#include "Util/Misc.h"
#include "Util/GraphicsView.h"
#include "UI/ImageViewer.h"

#include "Z80/CodeAnalyserZ80.h"
#include "6502/CodeAnalyser6502.h"
#include <Debug/DebugLog.h>
#include "Commands/CommandProcessor.h"
#include "Commands/SetItemDataCommand.h"
#include "Commands/SetCommentCommand.h"
#include "Z80/Z80Disassembler.h"
#include "6502/M6502Disassembler.h"
#include "UI/CodeAnalyserUI.h"
#include "DataTypes.h"

#include "Misc/EmuBase.h"

// memory bank code

// create a bank
// a bank is a list of memory pages
int16_t	FCodeAnalysisState::CreateBank(const char* bankName, int noKb,uint8_t* pBankMem, bool bReadOnly, uint16_t initialAddress, bool bFixed)
{
	const int16_t bankId = GetNextBankId();
	assert(bankId == (int16_t)Banks.size());
	const int noPages = noKb;

	FCodeAnalysisBank& newBank = Banks.emplace_back();
	newBank.Id = bankId;
	newBank.NoPages = noPages;
	newBank.SizeMask = (noPages * FCodeAnalysisPage::kPageSize) - 1;
	newBank.Memory = pBankMem;
	newBank.Pages = new FCodeAnalysisPage[noPages];
	newBank.Name = bankName;
	newBank.bReadOnly = bReadOnly;
	newBank.bFixed = bFixed;
	newBank.PrimaryMappedPage = initialAddress / 1024;	// byte addres to 1kb page address
	for (int pageNo = 0; pageNo < noPages; pageNo++)
	{
		newBank.Pages[pageNo].Initialise();
		char pageName[32];
		snprintf(pageName,32, "%s:%d", bankName, pageNo);
		RegisterPage(&newBank.Pages[pageNo], pageName);
	}
	return bankId;
}

bool FCodeAnalysisState::FreeBanksFrom(int16_t bankId)
{
	Banks.resize(bankId);
	return true;
}

bool FCodeAnalysisState::SetBankPrimaryPage(int16_t bankId, int startPageNo)
{
	FCodeAnalysisBank* pBank = GetBank(bankId);
	if (pBank == nullptr)	// not found 
		return false;

	pBank->PrimaryMappedPage = startPageNo;
	return true;
}

// Set bank to memory pages starting at pageNo
bool FCodeAnalysisState::MapBank(int16_t bankId, int startPageNo, EBankAccess access)
{
	FCodeAnalysisBank* pBank = GetBank(bankId);

	if (pBank == nullptr)	// not found or already mapped to this locatiom
		return false;

	if (pBank->bEverBeenMapped == false )	// Newly mapped?
	{
		if (pBank->PrimaryMappedPage == -1)
			pBank->PrimaryMappedPage = startPageNo;
		pBank->bIsDirty = true;
	}
	assert(pBank->PrimaryMappedPage != -1);

	// unmap old banks
	if (access == EBankAccess::Read || access == EBankAccess::ReadWrite)
	{
		FCodeAnalysisBank* pOldBank = GetBank(MappedReadBanks[startPageNo]);
		if (pOldBank != nullptr && pOldBank != pBank)
			pOldBank->UnmapFromPage(startPageNo, EBankAccess::Read);
	}
	if (access == EBankAccess::Write || access == EBankAccess::ReadWrite)
	{
		FCodeAnalysisBank* pOldBank = GetBank(MappedWriteBanks[startPageNo]);
		if (pOldBank != nullptr && pOldBank != pBank)
			pOldBank->UnmapFromPage(startPageNo, EBankAccess::Write);
	}

	pBank->MapToPage(startPageNo, access);
	for (int bankPageNo = 0; bankPageNo < pBank->NoPages; bankPageNo++)
	{
		// Set Read Page
		if(access == EBankAccess::Read || access == EBankAccess::ReadWrite)
		{
			MappedReadBanks[startPageNo + bankPageNo] = bankId;
			SetCodeAnalysisReadPage(startPageNo + bankPageNo, &pBank->Pages[bankPageNo]);	// Read
		}

		// Set Write Page
		if (access == EBankAccess::Write || access == EBankAccess::ReadWrite)
		{
			MappedWriteBanks[startPageNo + bankPageNo] = bankId;
			SetCodeAnalysisWritePage(startPageNo + bankPageNo, &pBank->Pages[bankPageNo]);	// Write
		}
	}
	bMemoryRemapped = true;

	bCodeAnalysisDataDirty = true;

	return true;
}

#if 0
bool FCodeAnalysisState::UnMapBank(int16_t bankId, int startPageNo, EBankAccess access)
{
	FCodeAnalysisBank* pBank = GetBank(bankId);

	// TODO: this needs proper logic
	if (pBank == nullptr || MappedReadBanks[startPageNo] != bankId)
		return false;

	for (int bankPage = 0; bankPage < pBank->NoPages; bankPage++)
	{
		if (access == EBankAccess::Read || access == EBankAccess::ReadWrite)
			MappedReadBanks[startPageNo + bankPage] = -1;
		if (access == EBankAccess::Write || access == EBankAccess::ReadWrite)
			MappedWriteBanks[startPageNo + bankPage] = -1;
	}

	pBank->UnmapFromPage(startPageNo,access);

	return true;
}
#endif
bool FCodeAnalysisState::IsBankIdMapped(int16_t bankId) const
{
	for (int bankIdx = 0; bankIdx < kNoPagesInAddressSpace; bankIdx++)
	{
		if (MappedReadBanks[bankIdx] == bankId)
			return true;
		if (MappedWriteBanks[bankIdx] == bankId)
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
	for (int i = 0; i < kNoPagesInAddressSpace; i++)
	{
#ifdef _DEBUG
		const FCodeAnalysisBank* pMappedBank = GetBank(MappedReadBanks[i]);
		assert(pMappedBank->PrimaryMappedPage != -1);
#endif
		MappedReadBanksBackup[i] = MappedReadBanks[i];
		MappedWriteBanksBackup[i] = MappedWriteBanks[i];
	}

	const int startPageNo = bank.PrimaryMappedPage;
	assert(startPageNo != -1);
	for (int bankPageNo = 0; bankPageNo < bank.NoPages; bankPageNo++)
	{
		MappedReadBanks[startPageNo + bankPageNo] = bank.Id;
		MappedWriteBanks[startPageNo + bankPageNo] = bank.Id;
		MappedMem[startPageNo + bankPageNo] = &bank.Memory[bankPageNo * FCodeAnalysisPage::kPageSize];
		SetCodeAnalysisRWPage(startPageNo + bankPageNo, &bank.Pages[bankPageNo], &bank.Pages[bankPageNo]);	// Read/Write
	}

	return true;
}

void FCodeAnalysisState::UnMapAnalysisBanks()
{
	for (int i = 0; i < kNoPagesInAddressSpace; i++)
	{
		MappedReadBanks[i] = MappedReadBanksBackup[i];
		MappedWriteBanks[i] = MappedWriteBanksBackup[i];
		const FCodeAnalysisBank* pMappedBank = GetBank(MappedReadBanks[i]);
		assert(pMappedBank->PrimaryMappedPage != -1);
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
	return Debugger.IsAddressBreakpointed(addr);
}

bool FCodeAnalysisState::ToggleExecBreakpointAtAddress(FAddressRef addr)
{
	if (Debugger.IsAddressBreakpointed(addr))
		return Debugger.RemoveBreakpoint(addr);
	else
		return Debugger.AddExecBreakpoint(addr);
}

bool FCodeAnalysisState::ToggleDataBreakpointAtAddress(FAddressRef addr, uint16_t dataSize)
{
	if (Debugger.IsAddressBreakpointed(addr))
		return Debugger.RemoveBreakpoint(addr);
	else
		return Debugger.AddDataBreakpoint(addr, dataSize);
}


std::vector<FAddressRef> FCodeAnalysisState::FindAllMemoryPatterns(const uint8_t* pData, size_t dataSize, bool bROM, bool bPhysicalOnly)
{
	std::vector<FAddressRef> results;
	// iterate through banks
	for (auto& bank : Banks)
	{
		if (bank.bReadOnly && bROM == false)
			continue;

		if (bank.IsMapped() == false && bPhysicalOnly)
			continue;

		const int bankByteSize = bank.GetSizeBytes();
		for (int bAddr = 0; bAddr < bankByteSize - dataSize; bAddr++)
		{
			bool bFound = true;
			for (int byteNo = 0; byteNo < dataSize; byteNo++)
			{
				const uint8_t byte = bank.Memory[bAddr + byteNo];
				if (byte != pData[byteNo])
				{
					bFound = false;
					break;
				}
			}

			if (bFound)
			{
				results.push_back(FAddressRef(bank.Id, bAddr + bank.GetMappedAddress()));
			}
		}
	}

	return results;	
}

bool IsAscii(uint8_t byte)
{
	return byte >= 32 && byte <= 126;
}

bool IsLetter(char c)
{
	return (c >= 'A' && c <= 'Z') || (c >= 'a' && c <= 'z');
}

bool IsNumber(char c)
{
	return c >= '0' && c <= '9';
}

bool IsAlphanumeric(char c)
{
	return IsLetter(c) || IsNumber(c);
}

bool IsVowel(char c)
{
	const char vowels[] = {'a','e','i','o','u'};
	const char lc = tolower(c);
	for (int i = 0; i < sizeof(vowels); i++)
	{
		if (lc == vowels[i])
			return true;
	}

	return false;
}

bool IsPunctuation(char c)
{
	const char punctuation[] = { ' ',',','!','.','?','\'','`','"','@','$','-'};
	for (int i = 0; i < sizeof(punctuation); i++)
	{
		if (c == punctuation[i])
			return true;
	}

	return false;
}

bool IsValidStringChar(char c)
{
	return IsAlphanumeric(c) || IsPunctuation(c);
}

std::vector<FFoundString> FCodeAnalysisState::FindAllStrings(bool bROM, bool bPhysicalOnly)
{
	std::vector<FFoundString> results;

	for (auto& bank : Banks)
	{
		if (bank.bReadOnly && bROM == false)
			continue;

		if (bank.IsMapped() == false && bPhysicalOnly)
			continue;

		FAddressRef stringStart;	
		int stringLength = 0;
		int vowelCount = 0;
		std::string foundString;

		const int bankByteSize = bank.GetSizeBytes();
		int stringStartBankPos = 0;
		
		for(int bAddr = 0; bAddr < bankByteSize;bAddr++)
		{
			FAddressRef addressRef(bank.Id, bank.GetMappedAddress() + bAddr);
			char c = bank.Memory[bAddr];
			bool bTerminated = false;

			// Check for code & skip
			const FCodeInfo* pCodeInfo = GetCodeInfoForAddress(addressRef);
			if (pCodeInfo != nullptr)
				continue;

			// check data
			const FDataInfo* pDataInfo = GetDataInfoForAddress(addressRef);
			if (pDataInfo->DataType != EDataType::Byte && pDataInfo->DataType != EDataType::Text)
				continue;
			if (pDataInfo->LastFrameWritten != -1)
				continue;

			if (c & (1 << 7))	// high bit terminated strings
			{
				c = c & ~(1 << 7);
				bTerminated = true;
			}

			if(IsValidStringChar(c))
			{
				if (stringStart.IsValid() == false)	// string start
				{
					stringStart = addressRef;
					stringStartBankPos = bAddr;
					stringLength = 0;
				}

				stringLength++;
				if (IsVowel(c))
					vowelCount++;
				foundString.push_back(c);

				//if (stringLength == 4 && vowelCount == 0)	// string of 4 chars must have a vowel
				//	bTerminated = true;
			}
			else
			{
				bTerminated = true;
			}

			if(bTerminated && foundString.empty() == false)
			{
				// Run through (simple) acceptance filter
				if (stringLength > 2 && vowelCount > 0)
				{
					results.push_back({ stringStart, foundString });
				}
				else
				{
					bAddr = stringStartBankPos;	// wind back to start of string
				}
				stringStart.SetInvalid();;
				stringLength = 0;
				vowelCount = 0;
				foundString.clear();
			}
		}
	}

	return results;
}

#if 0
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
#endif

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
	FDataInfo* pDataInfo = state.GetDataInfoForAddress(address);
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
		return pLabel;

		
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
		{
			FDataInfo* pDataInfo = state.GetDataInfoForAddress(address);
			if(pDataInfo->DataType == EDataType::InstructionOperand)
				snprintf(label, kLabelSize, "operand_%04X", address.Address);
			else
				snprintf(label, kLabelSize, "data_%04X", address.Address);

			// zero page labels for 6502
			if (state.CPUInterface->CPUType == ECPUType::M6502 && address.Address < 256)
				snprintf(label, kLabelSize, "zp_%02X", address.Address);

			pLabel->Global = true;
		}
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
        default:
        break;
	}

	pLabel->InitialiseName(label);
	if (pLabel->Global)
		GenerateGlobalInfo(state);
	state.SetLabelForAddress(address, pLabel);
	state.SetCodeAnalysisDirty(address);
	return pLabel;	
}

void UpdateCodeInfoForAddress(FCodeAnalysisState &state, uint16_t pc)
{
	FCodeInfo* pCodeInfo = state.GetCodeInfoForPhysicalAddress(pc);
	if (pCodeInfo == nullptr)	// code info could have been cleared
		return;

	pCodeInfo->bIsCall = CheckCallInstruction(state, pc);

	if (state.CPUInterface->CPUType == ECPUType::Z80)
		Z80DisassembleCodeInfoItem(pc, state, pCodeInfo);
	else if (state.CPUInterface->CPUType == ECPUType::M6502)
		M6502DisassembleCodeInfoItem(pc, state, pCodeInfo);
}

// This assumes that the address passed in is mapped to physical memory
uint16_t WriteCodeInfoForAddress(FCodeAnalysisState &state, uint16_t pc)
{
	FCodeInfo* pCodeInfo = state.GetCodeInfoForPhysicalAddress(pc);
	if (pCodeInfo == nullptr)
	{
		pCodeInfo = FCodeInfo::Allocate();
		state.SetCodeInfoForAddress(pc, pCodeInfo);
	}	

	// does this function branch?
	uint16_t jumpAddr;
	if (CheckJumpInstruction(state, pc, &jumpAddr))
	{
		pCodeInfo->bIsCall = CheckCallInstruction(state, pc);
		FLabelInfo* pLabel = GenerateLabelForAddress(state, state.AddressRefFromPhysicalAddress(jumpAddr), pCodeInfo->bIsCall ? ELabelType::Function : ELabelType::Code);
		if(pLabel)
			pLabel->References.RegisterAccess(state.AddressRefFromPhysicalAddress(pc));

		pCodeInfo->OperandAddress = state.AddressRefFromPhysicalAddress(jumpAddr);
		assert(state.IsAddressValid(pCodeInfo->OperandAddress));

		if (pCodeInfo->OperandType == EOperandType::Unknown)
			pCodeInfo->OperandType = EOperandType::JumpAddress;
	}
	else
	{
		uint16_t ptr;
		if (CheckPointerRefInstruction(state, pc, &ptr))	// this is just a 16 bit number so don't assume a pointer
		{
			const FAddressRef ptrAddr = state.AddressRefFromPhysicalAddress(ptr);
			pCodeInfo->OperandAddress = ptrAddr;
			//if(pCodeInfo->OperandType == EOperandType::Unknown)
			//	pCodeInfo->OperandType = EOperandType::Pointer;

			//FLabelInfo* pLabel = GenerateLabelForAddress(state, ptrAddr, ELabelType::Data);
			//if (pLabel)
			//	pLabel->References.RegisterAccess(state.AddressRefFromPhysicalAddress(pc));
		}
		else if (CheckPointerIndirectionInstruction(state, pc, &ptr))
		{
			const FAddressRef ptrAddr = state.AddressRefFromPhysicalAddress(ptr);
			pCodeInfo->OperandAddress = ptrAddr;
			if (pCodeInfo->OperandType == EOperandType::Unknown)
				pCodeInfo->OperandType = EOperandType::Pointer;
			
			FLabelInfo* pLabel = GenerateLabelForAddress(state, ptrAddr, ELabelType::Data);
			if (pLabel)
				pLabel->References.RegisterAccess(state.AddressRefFromPhysicalAddress(pc));
		}
	}

	// generate disassembly
	uint16_t newPC = pc;

	if (state.CPUInterface->CPUType == ECPUType::Z80)
		newPC = Z80DisassembleCodeInfoItem(pc,state,pCodeInfo);
	else if (state.CPUInterface->CPUType == ECPUType::M6502)
		newPC = M6502DisassembleCodeInfoItem(pc, state, pCodeInfo);

	state.SetCodeInfoForAddress(pc, pCodeInfo);	

	// set operands as data item
	for (uint16_t codeAddr = pc + 1; codeAddr < newPC; codeAddr++)
	{
		FDataInfo* pOperandData = state.GetReadDataInfoForAddress(codeAddr);
		pOperandData->DataType = EDataType::InstructionOperand;
		pOperandData->ByteSize = 1;
		pOperandData->InstructionAddress = state.AddressRefFromPhysicalAddress(pc);
	}
	pCodeInfo->ByteSize = newPC - pc;

	return newPC;
}

// return if we should continue
bool AnalyseAtPC(FCodeAnalysisState &state, uint16_t& pc)
{
	FCodeInfo* pCodeInfo = state.GetCodeInfoForPhysicalAddress(pc);

	// Register Code accesses
	// 
	// set jump reference
	uint16_t jumpPhysAddr;
	if (CheckJumpInstruction(state, pc, &jumpPhysAddr))
	{
		const FAddressRef jumpAddr = state.AddressRefFromPhysicalAddress(jumpPhysAddr);
		assert(state.IsAddressValid(jumpAddr));

		FLabelInfo* pLabel = state.GetLabelForPhysicalAddress(jumpPhysAddr);
		if (pLabel != nullptr)
			pLabel->References.RegisterAccess(state.AddressRefFromPhysicalAddress(pc));
		if (pCodeInfo != nullptr)
			pCodeInfo->OperandAddress = jumpAddr;

	}

	// set pointer reference
	uint16_t ptr;
	if (CheckPointerRefInstruction(state, pc, &ptr))
	{
		FLabelInfo* pLabel = state.GetLabelForPhysicalAddress(ptr); // NOTE: we have to use the physical address because of banks mapped twice
		if (pLabel != nullptr)
			pLabel->References.RegisterAccess(state.AddressRefFromPhysicalAddress(pc));

		if (pCodeInfo != nullptr)
			pCodeInfo->OperandAddress = state.AddressRefFromPhysicalAddress(ptr);
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
	pCodeInfo = state.GetCodeInfoForPhysicalAddress(pc);
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

bool RegisterCodeExecuted(FCodeAnalysisState &state, uint16_t pc, uint16_t oldpc)
{
	AnalyseAtPC(state, pc);

	FCodeInfo* pCodeInfo = state.GetCodeInfoForPhysicalAddress(pc);
	if (pCodeInfo != nullptr)
	{
		pCodeInfo->FrameLastExecuted = state.CurrentFrameNo;
		pCodeInfo->ExecutionCount++;
	}

	if (state.CPUInterface->CPUType == ECPUType::Z80)
		return RegisterCodeExecutedZ80(state, pc, oldpc);
	else if (state.CPUInterface->CPUType == ECPUType::M6502)
		return RegisterCodeExecuted6502(state, pc, oldpc);

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

	if (state.GetCodeInfoForPhysicalAddress(dataAddr) == nullptr)	// don't register instruction data reads
	{
		FDataInfo* pDataInfo = state.GetReadDataInfoForAddress(dataAddr);
		if(pDataInfo->DataType != EDataType::InstructionOperand)
		{
			pDataInfo->ReadCount++;
			pDataInfo->LastFrameRead = state.CurrentFrameNo;
			pDataInfo->Reads.RegisterAccess(state.AddressRefFromPhysicalAddress(pc));
		
			FCodeInfo* pCodeInfo = state.GetCodeInfoForAddress(state.AddressRefFromPhysicalAddress(pc));
			if (pCodeInfo)
				pCodeInfo->Reads.RegisterAccess(state.AddressRefFromPhysicalReadAddress(dataAddr));
		}
	}
}

void RegisterDataWrite(FCodeAnalysisState &state, uint16_t pc,uint16_t dataAddr,uint8_t value)
{
	const FAddressRef pcAddr = state.AddressRefFromPhysicalAddress(pc);
	FDataInfo* pDataInfo = state.GetWriteDataInfoForAddress(dataAddr);
	pDataInfo->WriteCount++;
	pDataInfo->LastFrameWritten = state.CurrentFrameNo;
	pDataInfo->Writes.RegisterAccess(pcAddr);

	// check for SMC
	if (pDataInfo->DataType == EDataType::InstructionOperand)
	{
		// TODO: record some info such as what byte was written
		FCodeInfo* pCodeWrittenTo = state.GetCodeInfoForAddress(pDataInfo->InstructionAddress);
		if (pCodeWrittenTo != nullptr)	// sometime data can be malformed so do a defensive check
			pCodeWrittenTo->bSelfModifyingCode = true;
	}

	FCodeInfo* pCodeInfo = state.GetCodeInfoForAddress(pcAddr);
	if(pCodeInfo)
	{
		pCodeInfo->Writes.RegisterAccess(state.AddressRefFromPhysicalWriteAddress(dataAddr));
	}
}

// TODO: this needs to be rewritten for banks
void ReAnalyseCode(FCodeAnalysisState &state)
{
	int addr = 0;
	while ( addr < (1 << 16))
	{
		FCodeInfo* pCodeInfo = state.GetCodeInfoForPhysicalAddress(addr);
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

			if (pCodeInfo->ByteSize == 0)
			{
				state.SetCodeInfoForAddress(addr, nullptr);
				addr++;
			}
			else
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

		FLabelInfo* pLabelInfo = state.GetLabelForPhysicalAddress(i);
		if (pLabelInfo != nullptr)
		{
			pLabelInfo->References.Reset();
		}

		FCodeInfo* pCodeInfo = state.GetCodeInfoForPhysicalAddress(i);
		if(pCodeInfo != nullptr)
		{
			pCodeInfo->bSelfModifyingCode = false;
			pCodeInfo->FrameLastExecuted = -1;
			pCodeInfo->Reads.Reset();
			pCodeInfo->Writes.Reset();
		}


		state.SetLastWriterForAddress(i,  FAddressRef());
	}
}

// TODO: Phase this out
FLabelInfo* AddLabel(FCodeAnalysisState &state, uint16_t address,const char *name,ELabelType type)
{
	FLabelInfo *pLabel = FLabelInfo::Allocate();
	pLabel->InitialiseName(name);
	pLabel->LabelType = type;
	//pLabel->Address = address;
	pLabel->ByteSize = 1;
	pLabel->Global = type == ELabelType::Function;
	state.SetLabelForPhysicalAddress(address, pLabel);

	if (pLabel->Global)
		GenerateGlobalInfo(state);

	return pLabel;
}

FLabelInfo* AddLabel(FCodeAnalysisState& state, FAddressRef address, const char* name, ELabelType type)
{
	FLabelInfo* pLabel = FLabelInfo::Allocate();
	pLabel->InitialiseName(name);
	pLabel->LabelType = type;
	//pLabel->Address = address;
	pLabel->ByteSize = 1;
	pLabel->Global = type == ELabelType::Function;
	state.SetLabelForAddress(address, pLabel);

	if (pLabel->Global)
		GenerateGlobalInfo(state);

	return pLabel;
}

FCommentBlock* AddCommentBlock(FCodeAnalysisState& state, FAddressRef addressRef)
{
	FCommentBlock* pExistingBlock = state.GetCommentBlockForAddress(addressRef);
	if(pExistingBlock == nullptr)
	{
		FCommentBlock* pCommentBlock = FCommentBlock::Allocate();
		pCommentBlock->Comment = "";
		pCommentBlock->ByteSize = 1;
		state.SetCommentBlockForAddress(addressRef, pCommentBlock);
		state.SetCodeAnalysisDirty(addressRef);
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
		MappedReadBanks[i] = -1;
		MappedWriteBanks[i] = -1;
		MappedReadBanksBackup[i] = -1;
		MappedWriteBanksBackup[i] = -1;
		ReadPageTable[i] = nullptr;
		WritePageTable[i] = nullptr;
	}

    pDataTypes = new FDataTypes;
}

// Called each time a new game is loaded up
void FCodeAnalysisState::Init(FEmuBase* pEmu)
{
	InitImageViewers();
	InitCharacterSets();
	
	FLabelInfo::ResetLabelNames();
	ItemList.clear();

	// reset registered pages
	for (FCodeAnalysisPage* pPage : GetRegisteredPages())
	{
		pPage->Reset();
		//pPage->bUsed = false;

		/*for (int addr = 0; addr < FCodeAnalysisPage::kPageSize; addr++)
		{
			pPage->Labels[addr] = nullptr;
			pPage->CommentBlocks[addr] = nullptr;
			pPage->CodeInfo[addr] = nullptr;
			pPage->DataInfo[addr].Reset();
			pPage->MachineState[addr] = nullptr;
		}*/
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
	for (FCodeAnalysisBank& bank : Banks)
	{
		bank.Description.clear();
		bank.ItemList.clear();
	}

	pEmulator = pEmu;
	CPUInterface = pEmu;
	//uint16_t initialPC = pCPUInterface->GetPC();
	//RunStaticCodeAnalysis(*this, initialPC);

	// Key Config
	KeyConfig[(int)EKey::SetItemData] = ImGuiKey_D;
	KeyConfig[(int)EKey::SetItemText] = ImGuiKey_T;
	KeyConfig[(int)EKey::SetItemCode] = ImGuiKey_C;
	KeyConfig[(int)EKey::SetItemImage] = ImGuiKey_I;
	KeyConfig[(int)EKey::SetItemBinary] = ImGuiKey_B;
	KeyConfig[(int)EKey::SetItemPointer] = ImGuiKey_P;
	KeyConfig[(int)EKey::SetItemNumber] = ImGuiKey_N;
	KeyConfig[(int)EKey::SetItemUnknown] = ImGuiKey_U;
	KeyConfig[(int)EKey::AddLabel] = ImGuiKey_L;
	KeyConfig[(int)EKey::Rename] = ImGuiKey_R;
	KeyConfig[(int)EKey::Comment] = ImGuiKey_Semicolon; 
	KeyConfig[(int)EKey::CommentLegacy] = ImGuiKey_Slash; // '/'
	KeyConfig[(int)EKey::BreakContinue] = ImGuiKey_F5;
	KeyConfig[(int)EKey::StepInto] = ImGuiKey_F11;
	KeyConfig[(int)EKey::StepOver] = ImGuiKey_F10;
	KeyConfig[(int)EKey::StepFrame] = ImGuiKey_F6;
	KeyConfig[(int)EKey::StepScreenWrite] = ImGuiKey_F7;
	KeyConfig[(int)EKey::Breakpoint] = ImGuiKey_F9;

	Debugger.Init(this);
	MemoryAnalyser.Init(this);
	IOAnalyser.Init(this);
    
    pDataTypes->Reset();
}

// Start/End handlers for host (imgui) frame
void FCodeAnalysisState::OnFrameStart()
{
	Debugger.StartFrame();
}

void FCodeAnalysisState::OnFrameEnd()
{
	UpdateRegionDescs();
	MemoryAnalyser.FrameTick();
	IOAnalyser.FrameTick();
	if (Debugger.FrameTick())
	{
		GetFocussedViewState().GoToAddress(CPUInterface->GetPC());
	}
}

// Start/End handlers for machine frame
void	FCodeAnalysisState::OnMachineFrameStart()
{
	IOAnalyser.OnMachineFrameStart();
	Debugger.OnMachineFrameStart();
}
void	FCodeAnalysisState::OnMachineFrameEnd()
{
	IOAnalyser.OnMachineFrameEnd();
	Debugger.OnMachineFrameEnd();
    if (Debugger.IsStopped() == false)
        CurrentFrameNo++;
}

void FCodeAnalysisState::OnCPUTick(uint64_t pins)
{
	// Only Z80 has IO operations
	if(CPUInterface->CPUType == ECPUType::Z80)
	{
		// Handle IO operations
		if (pins & Z80_IORQ)
		{
			const uint8_t data = Z80_GET_DATA(pins);
			const uint16_t addr = Z80_GET_ADDR(pins);

			if (pins & Z80_RD)
				IOAnalyser.RegisterIORead(Debugger.GetPC(), addr, data);
			else if (pins & Z80_WR)
				IOAnalyser.RegisterIOWrite(Debugger.GetPC(), addr, data);
		}
	}

	Debugger.CPUTick(pins);
}

void FixupDataInfoAddressRefs(FCodeAnalysisState& state, FDataInfo* pDataInfo)
{
	// Because this is a union, it will also fixup GraphicsSetRef and CharSetAddress
	FixupAddressRef(state, pDataInfo->InstructionAddress);
	FixupAddressRef(state, pDataInfo->LastWriter);
	FixupAddressRefList(state, pDataInfo->Reads.GetReferences());
	FixupAddressRefList(state, pDataInfo->Writes.GetReferences());
}

void FixupCodeInfoAddressRefs(FCodeAnalysisState& state, FCodeInfo* pCodeInfo)
{
	FixupAddressRef(state, pCodeInfo->OperandAddress);
	FixupAddressRefList(state, pCodeInfo->Reads.GetReferences());
	FixupAddressRefList(state, pCodeInfo->Writes.GetReferences());
}

void FCodeAnalysisState::FixupAddressRefs()
{
	FixupAddressRef(*this, CopiedAddress);
	Debugger.FixupAddresRefs();
	MemoryAnalyser.FixupAddressRefs();
	for (int i = 0; i < FCodeAnalysisState::kNoViewStates; i++)
	{
		ViewState[i].FixupAddressRefs(*this);
	}

	for (FCommand* pCommand : CommandStack)
	{
		pCommand->FixupAddressRefs(*this);
	}

	// Go through the entire physical address range to fix up labels, code and data items.
	int addr = 0;
	while (addr <= 0xffff)
	{
		const FAddressRef wAddressRef(GetWriteBankFromAddress(addr), addr);
		const FAddressRef rAddressRef(GetReadBankFromAddress(addr), addr);

		// Fixup the code and data items in RAM.
		if (FCodeInfo* pWriteCode = GetCodeInfoForAddress(wAddressRef))
			FixupCodeInfoAddressRefs(*this, pWriteCode);

		if (FDataInfo* pWriteData = GetDataInfoForAddress(wAddressRef))
			FixupDataInfoAddressRefs(*this, pWriteData);

		if (FLabelInfo* pWriteLabel = GetLabelForAddress(wAddressRef))
			FixupAddressRefList(*this, pWriteLabel->References.GetReferences());

		if (wAddressRef.BankId != rAddressRef.BankId)
		{
			// If we have RAM behind ROM then fixup the ROM separately
			if (FCodeInfo* pReadCode = GetCodeInfoForAddress(rAddressRef))
				FixupCodeInfoAddressRefs(*this, pReadCode);

			if (FDataInfo* pReadData = GetDataInfoForAddress(rAddressRef))
				FixupDataInfoAddressRefs(*this, pReadData);

			if (FLabelInfo* pReadLabel = GetLabelForAddress(wAddressRef))
				FixupAddressRefList(*this, pReadLabel->References.GetReferences());
		}

		addr++;
	}
}

void SetItemCode(FCodeAnalysisState &state, FAddressRef address)
{
	DoCommand(state, new FSetItemCodeCommand(address));
}

void SetItemData(FCodeAnalysisState &state, const FCodeAnalysisItem& item)
{
	DoCommand(state, new FSetItemDataCommand(item));
}

void SetDataItemDisplayType(FCodeAnalysisState& state, const FCodeAnalysisItem& item, EDataItemDisplayType displayType)
{
	DoCommand(state, new FSetDataItemDisplayTypeCommand(item,displayType));
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
			FDataInfo* pDataInfo = state.GetDataInfoForAddress(charAddr);
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

#if 0
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
#endif

FLabelInfo* AddLabelAtAddress(FCodeAnalysisState &state, FAddressRef address)
{
	FLabelInfo* pNewLabel = nullptr;

	if (state.GetLabelForAddress(address) == nullptr)
	{
		ELabelType labelType = ELabelType::Data;
		const FDataInfo* pDataInfo = state.GetDataInfoForAddress(address);
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

void SetItemCommentText(FCodeAnalysisState &state, const FCodeAnalysisItem& item, const char *pText)
{
	//DoCommand(state, new FSetItemCommentCommand(item,pText));
	item.Item->Comment = pText;
}


void FormatData(FCodeAnalysisState& state, const FDataFormattingOptions& options)
{
	FFormatDataCommand* pFormatCommand = new FFormatDataCommand(options);
	DoCommand(state,pFormatCommand);	
}

void BatchFormatData(FCodeAnalysisState& state, const FBatchDataFormattingOptions& options)
{
	FBatchFormatDataCommand* pFormatCommand = new FBatchFormatDataCommand(options);
	DoCommand(state, pFormatCommand);
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
    default:
    break;
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
    default:
    break;
	}
}

void FixupAddressRef(FCodeAnalysisState& state, FAddressRef& addr)
{
	if (FCodeAnalysisBank* pBank = state.GetBank(addr.BankId))
	{
		const uint16_t bankOffset = (addr.Address & pBank->SizeMask);
		addr.Address = pBank->GetMappedAddress() + bankOffset;
	}
}

void FixupAddressRefList(FCodeAnalysisState& state, std::vector<FAddressRef>& addrList)
{
	for (FAddressRef& addr : addrList)
	{
		FixupAddressRef(state, addr);
	}
}