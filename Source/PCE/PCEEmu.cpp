#include <imgui.h>

#include "PCEEmu.h"

#include <chrono>

#include "AsmExportValidator.h"
#include "Constants.h"
#include "PCEConfig.h"
#include "Util/FileUtil.h"
#include "Util/GraphicsView.h"
#include "Viewers/PCEViewer.h"
#include "Viewers/BatchGameLoadViewer.h"
#include "Viewers/BackgroundViewer.h"
#include "Viewers/BanksViewer.h"
#include "Viewers/DebugStatsViewer.h"
#include "Viewers/JoypadViewer.h"
#include "Viewers/PaletteViewer.h"
#include "Viewers/SpriteViewer.h"
#include "Viewers/VRAMViewer.h"
#include "Viewers/CDROMViewer.h"
#include "Viewers/RecentMemoryAccessViewer.h"
#include "VRAMAnalyser.h"
#include "CDROMAnalyser.h"
#include "RecentMemoryAccess.h"

#include "Viewers/PCEGraphicsViewer.h"
#include "Viewers/PCENewGraphicsViewer.h"
#include "Viewers/MemoryViewer.h"
#include "Viewers/ZeroPageViewer.h"
#include "Viewers/GameDbViewer.h"
#include "CodeAnalyser/AssemblerExport.h"
#include "CodeAnalyser/6502/CodeAnalyserHuC6280.h"
#include "CodeAnalyser/UI/6502/RegisterView6502.h"
#include "CodeAnalyser/UI/OverviewViewer.h"
#include "CodeAnalyser/UI/GlobalsViewer.h"
#include "Viewers/PCERegistersViewer.h"
#include <geargrafx_core.h>
#include "Debug/DebugLog.h"
#include "GameDb.h"
#include "DebugStats.h"

#include "ProjectDefines.h"
#include "App.h"
#include "PCEMCPTools.h"
#include <CodeAnalyser/CodeAnalysisState.h>
#include "CodeAnalyser/CodeAnalysisJson.h"
#include <json.hpp>
#include "PCEGameConfig.h"

#include "optick/optick.h"

bool InitPCEAsmExporters();


// ideas for increasing performance
// - UpdateItemList() less than once per frame

// I couldn't get this working.
#define IMPORT_BIOS_ANALYSIS_JSON 0
#define EXPORT_BIOS_ANALYSIS_JSON 0

const char* kGlobalConfigFilename = "GlobalConfig.json";
const std::string kAppTitle = "PCE Analyser";

#if IMPORT_BIOS_ANALYSIS_JSON
const char* kBiosInfoJsonFile = "SysCard3Info.json";
#endif

// Bank constants
constexpr uint8_t kNumCdRomRamBanks = 8;

constexpr uint8_t kBankCdRomRamStart = 0x80;
constexpr uint8_t kBankUnusedStart = 0x88;
constexpr uint8_t kBankSaveRAM = 0xf7;
constexpr uint8_t kBankWRAM0 = 0xf8;
constexpr uint8_t kBankWRAM1 = 0xf9;
constexpr uint8_t kBankWRAM2 = 0xfa;
constexpr uint8_t kBankWRAM3 = 0xfb;
constexpr uint8_t kBankHWPage = 0xff;

// The default initial address when creating a bank.
// This will get overwritten later when the bank gets mapped so this is just an arbitrary number.
constexpr uint16_t kDefaultPrimaryMappedPage = 8;
constexpr uint16_t kDefaultInitialBankAddr = kDefaultPrimaryMappedPage * FCodeAnalysisPage::kPageSize;

const char* FPCEEmu::kPCERomGameListName = "PCE ROM File";
const char* FPCEEmu::kCDRomGameListName = "CD-ROM Image";

// Hardcoded addresses for system vectors. These are hardcoded addresses that contain 16 bit values.
// The 16 bit values are pointers to the routines that will be called when servicing interrupts, timers or resetting the machine.
constexpr uint16_t kVecIRQ2  = 0xfff6;
constexpr uint16_t kVecIRQ1  = 0xfff8;
constexpr uint16_t kVecTimer = 0xfffa;
constexpr uint16_t kVecNMI	 = 0xfffc;
constexpr uint16_t kVecReset = 0xfffe;

#ifndef NDEBUG
#define BANK_SWITCH_DEBUG 0
#define BATCH_GAME_VIEWER 1
#define DEBUG_STATS_VIEWER 1
#define GAME_DB_VIEWER 1
#else
#define BATCH_GAME_VIEWER 0
#define DEBUG_STATS_VIEWER 0
#define GAME_DB_VIEWER 0
#endif

#if BANK_SWITCH_DEBUG
#define BANK_LOG(...)  LOGINFO("[BNK] " __VA_ARGS__)
#define BANK_ERROR(...)  LOGERROR("[BNK] " __VA_ARGS__)
#else
#define BANK_LOG(...)
#define BANK_ERROR(...)
#endif

enum class EEventType : uint8_t
{
	None = 0,
	BankAddressChange,
};

class FPCECPUEmulator6502 : public ICPUEmulator6502
{
public:
	FPCECPUEmulator6502(FPCEEmu* pEmu)
		: pPCEEmu(pEmu)
	{
		p6280State = pEmu->GetCore()->GetHuC6280()->GetState();
	}
	virtual void* GetImpl() const 
	{
		// should this be returning the p6280State?
		return (void*)pPCEEmu->GetCore();
	}
	virtual uint16_t GetPC() const
	{
		return p6280State->PC->GetValue();
	}
	virtual uint8_t GetA() const
	{
		return p6280State->A->GetValue();
	}
	virtual uint8_t GetX() const
	{
		return p6280State->X->GetValue();
	}
	virtual uint8_t GetY() const
	{
		return p6280State->Y->GetValue();
	}
	virtual uint8_t GetS() const
	{
		return p6280State->S->GetValue();
	}
	virtual uint8_t GetP() const
	{
		return p6280State->P->GetValue();
	}
	virtual void SetA(uint8_t val)
	{
		return p6280State->A->SetValue(val);
	}
	virtual void SetX(uint8_t val)
	{
		return p6280State->X->SetValue(val);
	}
	virtual void SetY(uint8_t val)
	{
		return p6280State->Y->SetValue(val);
	}
	virtual void SetP(uint8_t val)
	{
		return p6280State->P->SetValue(val);
	}

	HuC6280_State* p6280State = nullptr;
	FPCEEmu* pPCEEmu = nullptr;
};

uint8_t FPCEEmu::ReadByte(uint16_t address) const
{
	// Need a better way to do this.
	// When the tool initially boots up there is no ROM loaded but we still need to present the user with a 
	// code analysis view that contains banks.
	if (!pMedia->IsReady())
		return 0;

	return pMemory->Read(address, /* is_cpu */ false);
}

uint16_t	FPCEEmu::ReadWord(uint16_t address) const 
{
	if (!pMedia->IsReady())
		return 0;

	return ReadByte(address) | (ReadByte(address + 1) << 8);
}

// this gets the mem ptr of physical memory.
const uint8_t* FPCEEmu::GetMemPtr(uint16_t address) const 
{
	const uint8_t mprIndex = address >> 13;
	const uint8_t bankIndex = pMemory->GetMpr(mprIndex);
	const uint16_t offset = address & 0x1FFF;
	// todo deal with hardware page properly. add it to the memory map
	if (bankIndex == kBankHWPage)
		return pMemory->GetHWPageMemory();

	return pMemory->GetMemoryMap()[bankIndex] + offset;
}

void FPCEEmu::WriteByte(uint16_t address, uint8_t value)
{
	if (!pMedia->IsReady())
		return;

	return pMemory->Write(address, value, false, false);
}

FAddressRef	FPCEEmu::GetPC(void) 
{
	return CodeAnalysis.GetCanonicalAddressRef(p6280State->PC->GetValue());
} 

uint16_t	FPCEEmu::GetSP(void)
{
	return STACK_ADDR + p6280State->S->GetValue();
}

ICPUEmulator* FPCEEmu::GetCPUEmulator(void) const
{
	return pPCE6502CPU;
}

// This is a PCE specific version of RegisterDataRead from CodeAnalyser.cpp
void FPCEEmu::RegisterDataRead(uint16_t pc, uint16_t dataAddr)
{
	FCodeAnalysisState& state = GetCodeAnalysis();

	// todo: fix order of execution issue here?
	// if this is the first time we've executed this location, then the memory won't be set as code.
	// this means this check will return a false positive.
	// memory gets set as code in the OnInstructionEnded callback that happens _after_ the instruction has executed.

	// can we check pc == dataAddr here instead?
	// this fails to register the read if the dest address is code.
	if (state.GetCodeInfoForPhysicalAddress(dataAddr) == nullptr)	// don't register instruction data reads
	{
		FDataInfo* pDataInfo = state.GetReadDataInfoForAddress(dataAddr);
		if (pDataInfo->DataType != EDataType::InstructionOperand)
		{
			// Register the read if we are not reading an instruction operand.
			pDataInfo->ReadCount++;
			pDataInfo->LastFrameRead = state.CurrentFrameNo;
			pDataInfo->LastRead = state.ExecutionCounter;

			// todo: pass the pc addressref in?
			const FAddressRef canonicalPc = state.GetCanonicalAddressRef(pc);
			pDataInfo->Reads.RegisterAccess(canonicalPc);

			// what is this codeinfo for?
			const FAddressRef dataAddrRef = state.GetCanonicalReadAddressRef(dataAddr);
			FCodeInfo* pCodeInfo = state.GetCodeInfoForAddress(canonicalPc);
			if (pCodeInfo)
				pCodeInfo->Reads.RegisterAccess(dataAddrRef);
		
			// This registers reads on memory that is not the instruction operand(s)
			pRecentMemoryAccess->Reads.RegisterAccess(dataAddrRef);
		}
	}
}

// This is a PCE specific version of RegisterDataWrite from CodeAnalyser.cpp
void FPCEEmu::RegisterDataWrite(uint16_t pc, uint16_t dataAddr, uint8_t value)
{
	FCodeAnalysisState& state = GetCodeAnalysis();

	const FAddressRef pcAddr = state.GetCanonicalAddressRef(pc);
	FDataInfo* pDataInfo = state.GetWriteDataInfoForAddress(dataAddr);
	pDataInfo->WriteCount++;
	pDataInfo->LastFrameWritten = state.CurrentFrameNo;
	pDataInfo->LastWritten = state.ExecutionCounter;
	pDataInfo->Writes.RegisterAccess(pcAddr);

	// check for SMC
	if (pDataInfo->DataType == EDataType::InstructionOperand)
	{
		// TODO: record some info such as what byte was written
		FCodeInfo* pCodeWrittenTo = state.GetCodeInfoForAddress(pDataInfo->InstructionAddress);
		if (pCodeWrittenTo != nullptr)	// sometime data can be malformed so do a defensive check
			pCodeWrittenTo->bSelfModifyingCode = true;
	}
	else
	{
		// Only register writes that are not SMC
		const FAddressRef writeAddr = state.AddressRefFromPhysicalWriteAddress(dataAddr);
		pRecentMemoryAccess->Writes.RegisterAccess(writeAddr);
	}

	FCodeInfo* pCodeInfo = state.GetCodeInfoForAddress(pcAddr);
	if (pCodeInfo)
	{
		pCodeInfo->Writes.RegisterAccess(state.GetCanonicalWriteAddressRef(dataAddr));
	}
}

static void OnInstructionStarted(void* pContext, uint16_t pc, u8 opcode)
{
	FPCEEmu* pEmu = static_cast<FPCEEmu*>(pContext);

	pEmu->OnInstructionStarted(pc, opcode);
}

// This is a geargfx specific version of FDebugger::Tick()
void OnInstructionFinished(void* pContext, uint16_t pc)
{
	//OPTICK_EVENT();

	FPCEEmu* pEmu = static_cast<FPCEEmu*>(pContext);
	pEmu->OnInstructionFinished(pc);
}

static void OnIRQ(void* pContext, uint16_t vector, uint16_t interruptedPc, uint16_t routineAddr)
{
	FPCEEmu* pEmu = static_cast<FPCEEmu*>(pContext);
	pEmu->OnIRQ(vector, interruptedPc, routineAddr);
}

static void OnMemoryRead(void* pContext, u16 dataAddr)
{
	//OPTICK_EVENT();

	FPCEEmu* pEmu = static_cast<FPCEEmu*>(pContext);
	FCodeAnalysisState& state = pEmu->GetCodeAnalysis();
	// todo: why not use PC from geargrafx here?
	const uint16_t pc = state.Debugger.GetPC().GetAddress();
	pEmu->RegisterDataRead(pc, dataAddr);
}

static void OnMemoryWritten(void* pContext, u16 dataAddr, u8 value)
{
	//OPTICK_EVENT();

	FPCEEmu* pEmu = static_cast<FPCEEmu*>(pContext);
	FCodeAnalysisState& state = pEmu->GetCodeAnalysis();
	FDebugger& debugger = state.Debugger;
	// todo: why not use PC from geargrafx here?
	const uint16_t pc = debugger.GetPC().GetAddress();
	pEmu->RegisterDataWrite(pc, dataAddr, value);

	const FAddressRef addrRef = state.GetCanonicalAddressRef(dataAddr);

	uint32_t bpMaskCheck = 0;
	int trapId = kTrapId_None;

	// only set the mask if we know we're on an address that's breakpointed
	if (FDataInfo* pDataInfo = state.GetWriteDataInfoForAddress(dataAddr))
	{
		if (pDataInfo->bHasBreakpoint)
		{
			// setup breakpoint mask to check
			bpMaskCheck |= BPMask_DataWrite;
		}
	}

	// iterate through data breakpoints
	// this can slow down if there are a lot of BPs
	// Do a mask check
	if (bpMaskCheck & debugger.GetBreakpointMask())
	{
		const std::vector<FBreakpoint>& breakpoints = debugger.GetBreakpoints();
		for (int i = 0; i < breakpoints.size(); i++)
		{
			const FBreakpoint& bp = breakpoints[i];

			if (bp.bEnabled)
			{
				switch (bp.Type)
				{
				case EBreakpointType::Data:
					if (addrRef.GetBankId() == bp.Address.GetBankId() &&
						addrRef.GetAddress() >= bp.Address.GetAddress() &&
						addrRef.GetAddress() < bp.Address.GetAddress() + bp.Size)
					{
						trapId = kTrapId_BpBase + i;
					}
					break;

				default:
					break;
				}
			}
		}
	}

	if (trapId != kTrapId_None)
	{
		debugger.Break();
	}
}

static void BankChangeCallback(void* pContext, u8 mprIndex, u8 oldBankIndex, u8 newBankIndex)
{
	OPTICK_EVENT();
	FPCEEmu* pEmu = static_cast<FPCEEmu*>(pContext);

	BANK_LOG("------------------------------BANK CHANGE START--------------------------------------------------");
	BANK_LOG("Map bank index 0x%x to mpr slot %d. [0x%x->0x%x]", newBankIndex, mprIndex, oldBankIndex, newBankIndex);

	if (oldBankIndex == newBankIndex)
	{
		BANK_LOG("Bank index has not changed. Doing nothing.");
		return;
	}

	const int bankSetIndex = pEmu->MprBankSet[mprIndex];
	if (bankSetIndex != -1)
		pEmu->GetBankSetPtr(bankSetIndex)->SetBankFreed(mprIndex);

	pEmu->MapMprBank(mprIndex, newBankIndex);
}

static void OnVRAMWritten(void* pContext, u16 vramAddr, u16 value)
{
	//OPTICK_EVENT();

	FPCEEmu* pEmu = static_cast<FPCEEmu*>(pContext);
	pEmu->OnVRAMWritten(vramAddr, value);
}

static void OnVRAMRead(void* pContext, u16 vramAddr, u16 value)
{
	FPCEEmu* pEmu = static_cast<FPCEEmu*>(pContext);
	pEmu->OnVRAMRead(vramAddr, value);
}

void FPCEEmu::OnVRAMRead(uint16_t vramAddr, uint16_t value)
{
	if (pVRAMState)
		pVRAMState->RegisterRead(vramAddr, CodeAnalysis.Debugger.GetPC());
}

void FPCEEmu::OnVRAMWritten(uint16_t vramAddr, uint16_t value)
{
	if (pVRAMState)
	{
		pVRAMState->RegisterWrite(vramAddr, CodeAnalysis.Debugger.GetPC());

		if (CodeAnalysis.Debugger.GetStepMode() == EDebugStepMode::ScreenWrite)
		{
			CodeAnalysis.Debugger.Break();
		}
	}
}

/*std::string GetBankType(Memory* pMemory, uint8_t bankIndex)
{
	Memory::MemoryBankType bankType = pMemory->GetBankType(bankIndex);

	switch (bankType)
	{
	case Memory::MEMORY_BANK_TYPE_ROM:
		return "ROM";
	case Memory::MEMORY_BANK_TYPE_BIOS:
		return "BIOS";
	case Memory::MEMORY_BANK_TYPE_CARD_RAM:
		return "CARD RAM";
	case Memory::MEMORY_BANK_TYPE_BACKUP_RAM:
		return "BACKUP RAM";
	case Memory::MEMORY_BANK_TYPE_WRAM:
		return "WORK RAM";
	case Memory::MEMORY_BANK_TYPE_CDROM_RAM:
		return "CD RAM";
	default:
		return "UNKNOWN";
	}
	return "UNKNOWN";
}*/

// Have we got the same bank index in 2 mpr slots?
// Note: this wont take into account dupe rom banks that could have a different bank index.
// ie. rom bank 00 will be in bank index slots 0 and 32. 
// Probably need to delete this.
/*bool HasDupeMprValues(Memory* memory)
{
	for (int i = 0; i < 7; i++)
	{
		for (int j = 0; j < 7; j++)
		{
			if (i != j && memory->GetMpr(i) == memory->GetMpr(j))
				return true;
		}
	}
	return false;
}*/


static bool ShouldTrackInstructionMemoryAccess(uint8_t opcode)
{
	return true;
	
	// todo: remove this if we dont need it
	/*const EAddressMode addressMode = GetInstructionAddressModeHuC6280(opcode);
	
	switch (addressMode)
	{
		case EAddressMode::ZPIndirect_X:       // (zp:X)
		case EAddressMode::ZP:                 // zp
		case EAddressMode::Immediate:          // #
		case EAddressMode::Absolute:           // abs
		case EAddressMode::ZPIndirect_Y:       // (zp):Y
		case EAddressMode::ZP_X:               // zp:X
		case EAddressMode::Absolute_Y:         // abs:Y
		case EAddressMode::Absolute_X:         // abs:X
		case EAddressMode::Accumulator:        // A
		case EAddressMode::ZPIndirect:         // (zp)
		case EAddressMode::ZP_Y:               // zp:Y
		case EAddressMode::Relative:           // rel
		case EAddressMode::AbsoluteIndirect:   // (abs)
		case EAddressMode::AbsoluteIndirect_X: // (abs:X)
		case EAddressMode::ZPRelative:         // zp:rel  (BBR/BBS)
		case EAddressMode::Block:              // src:dst:len
		case EAddressMode::ImmZP:              // #:zp    (TST)
		case EAddressMode::ImmAbs:             // #:abs   (TST)
		case EAddressMode::ImmZPX:             // #:zp:X  (TST)
		case EAddressMode::ImmAbsX:            // #:abs:X (TST)
		case EAddressMode::Implied:
			return true;
		default:
			return false;
	}*/
}

// An opcode was just read and an instruction is about to be executed.
void FPCEEmu::OnInstructionStarted(uint16_t pc, uint8_t opcode)
{
	// Is this an opcode we want to track memory reads/writes?
	/*const bool bTrackAccess = ShouldTrackInstructionMemoryAccess(opcode);
	if (bTrackAccess)
	{
		// todo: Remove this? Not sure we need it.
		// If we remove it, then remove OnInstructionStarted callback too.
		pRecentMemoryAccess->SetEnabled(true);
	}*/
}

// An instruction has finished.
// pc is the address of the instruction that just executed.
void FPCEEmu::OnInstructionFinished(uint16_t pc)
{
	FCodeAnalysisState& state = GetCodeAnalysis();

#if CDROM_SUPPORT
	// todo move somewhere else
	if (IsCDROM())
	{
		const FAddressRef pcAddrRef = GetPC();
		if (pcAddrRef.GetBankId() == BankSets[0].GetBankId())
		{
			if (pcAddrRef.GetAddress() == 0xEC05/*0xE009*/)
			{
				FBiosCDReadArgs cdReadArgs;
				if (GetBiosCDReadArgs(this, cdReadArgs))
				{
					pCDROMAnalyser->RegisterCDRead(cdReadArgs);
				}
			}
		}
	}
#endif

	// oldpc is unused on HuC6280 (RegisterCodeExecutedHuC6280 only reads opcode at pc).
	RegisterCodeExecuted(state, pc, /* oldpc */ 0);

	// Advance Debugger.PC to the next instruction _before_ the breakpoint check so:
	//  - the exec breakpoint check below fires against the instruction about to run, not the one that just ran
	//  - memory callbacks during the next RunInstruction attribute to the executing PC
	const uint16_t nextPc = p6280State->PC->GetValue();
	state.Debugger.SetPC(state.GetCanonicalAddressRef(nextPc));

	// The pins value is chips specific so we pass in a dummy value.
	const uint64_t dummyPins = 0;
	const int trapId = state.Debugger.OnInstructionExecuted(dummyPins);

	if (trapId != kTrapId_None)
	{
		// This signals to geargfx to stop exection
		state.Debugger.Break();
	}

	// todo: remove this? not sure we need it
	//pRecentMemoryAccess->SetEnabled(false);
}

void FPCEEmu::OnIRQ(uint16_t vector, uint16_t interruptedPC, uint16_t routineAddr)
{
	FCodeAnalysisState& state = GetCodeAnalysis();
	FCPUFunctionCall callInfo;
	const FAddressRef interruptedPCFixed = state.GetCanonicalAddressRef(interruptedPC);
	callInfo.CallAddr = interruptedPCFixed;
	callInfo.FunctionAddr = state.GetCanonicalAddressRef(routineAddr);
	callInfo.ReturnAddr = interruptedPCFixed;
	state.Debugger.GetCallstack().push_back(callInfo);
}

bool FPCEEmu::IsUnusedBank(int16_t bankId) const
{
	const FBankSet* pBankSet = GetBankSetFromBankId(bankId);
	return pBankSet == &BankSets[kBankUnusedStart];
}

// Bear in mind bankIds can be in multiple bank slots.
// This will return the lowest bank index that matches the bankId.
uint8_t FPCEEmu::GetHwBankIndex(uint16_t bankId)
{
	for (int i = 0; i < kNumHwBanks; i++)
	{
		if (BankSetPtrs[i]->GetBankId() == bankId)
		{
			const Memory::MemoryBankType bankType = pMemory->GetBankType(i);
			if (bankType == Memory::MEMORY_BANK_TYPE_ROM || bankType == Memory::MEMORY_BANK_TYPE_BIOS)
				return pMedia->GetRomBankIndex(i);
			else if (bankType == Memory::MEMORY_BANK_TYPE_CARD_RAM)
				return i - pMemory->GetCardRAMStart();
		}
	}

	// Invalid index.
	// This assumes that we will never ask for the bank index of the HW page
	return 0xff;
}

void FPCEEmu::BuildCanonicalBankIdLookup()
{
	// Initialise to each bankId mapping to itself.
	for (int i = 0; i < FCodeAnalysisState::kMaxBanks; i++)
		CanonicalBankIdLookup[i] = (int16_t)i;

	// Overwrite entries for duplicate bankIds with their primary bankId.
	for (int i = 0; i < kNumHwBanks; i++)
	{
		const FBankSet& bankSet = BankSets[i];

		// Treat unused banks as canonical.
		// Not 100% sure if this is the right thing to do.
		// It stops the spam from AddressRefFromPhysicalAddress() when unused banks are mapped in the initial state 
		// after a project is loaded (or we have just booted up without a game loaded).
		if (i == kBankUnusedStart)
			continue;

		if (bankSet.Banks.size() <= 1)
			continue;

		const int16_t primaryId = bankSet.GetBankId(0);
		for (int d = 1; d < (int)bankSet.Banks.size(); d++)
		{
			const int16_t dupeId = bankSet.Banks[d].BankId;
			if (dupeId >= 0 && dupeId < FCodeAnalysisState::kMaxBanks)
				CanonicalBankIdLookup[dupeId] = primaryId;
		}
	}
}

void FPCEEmu::BuildBankSetLookup()
{
	for (int i = 0; i < FCodeAnalysisState::kMaxBanks; i++)
		BankSetLookup[i] = nullptr;

	for (int i = 0; i < kNumHwBanks; i++)
	{
		FBankSet& bankSet = BankSets[i];
		for (const FBankSet::FBankSetEntry& entry : bankSet.Banks)
		{
			if (entry.BankId >= 0 && entry.BankId < FCodeAnalysisState::kMaxBanks)
				BankSetLookup[entry.BankId] = &bankSet;
		}
	}
}

int16_t FPCEEmu::GetCanonicalBankId(int16_t bankId) const
{
	if (bankId >= 0 && bankId < (int16_t)FCodeAnalysisState::kMaxBanks)
		return CanonicalBankIdLookup[bankId];
	return bankId;
}

static void NullInstructionStartedCallback(void*, uint16_t, u8 opcode) {}
static void NullInstructionFinishedCallback(void*, uint16_t) {}
static void NullIRQCallback(void*, uint16_t, uint16_t, uint16_t) {}
static void NullMemoryReadCallback(void*, uint16_t) {}
static void NullMemoryWriteCallback(void*, uint16_t, uint8_t) {}
static void NullMprCallback(void*, uint8_t, uint8_t, uint8_t) {}
static void NullVRAMWriteCallback(void*, uint16_t, uint16_t) {}
static void NullVRAMReadCallback(void*, uint16_t, uint16_t) {}
static void NullScanlineCallback(void*, int, u16, s32, u16, u16) {}
static void NullVBlankCallback(void*) {}

static void OnVBlank(void* pContext)
{
	FPCEEmu* pEmu = static_cast<FPCEEmu*>(pContext);
	pEmu->GetCodeAnalysis().OnMachineFrameEnd();
}

static void OnScanlineDraw(void* pContext, int rasterLine, uint16_t bxr, int32_t byrEff, uint16_t mwr, uint16_t cr)
{
	FPCEEmu* pEmu = static_cast<FPCEEmu*>(pContext);

	if (rasterLine == 0)
		pEmu->GetCodeAnalysis().OnMachineFrameStart();
}

void FPCEEmu::EnableGeargrafxCallbacks(bool bEnabled)
{
	if (bEnabled)
	{
		pCore->SetInstructionExecutedCallback(::OnInstructionFinished, this);
		pCore->GetHuC6280()->SetCallbacks(::OnIRQ, ::OnInstructionStarted, this);
		pMemory->SetMemoryCallbacks(OnMemoryRead, OnMemoryWritten, BankChangeCallback, this);
		pCore->GetHuC6270_1()->SetCallbacks(::OnVRAMWritten, ::OnVRAMRead, ::OnScanlineDraw, ::OnVBlank, this);
		pCore->GetHuC6270_2()->SetCallbacks(NullVRAMWriteCallback, NullVRAMReadCallback, NullScanlineCallback, NullVBlankCallback, this);
	}
	else
	{
		pCore->SetInstructionExecutedCallback(::NullInstructionFinishedCallback, this);
		pCore->GetHuC6280()->SetCallbacks(NullIRQCallback, NullInstructionStartedCallback, this);
		pMemory->SetMemoryCallbacks(NullMemoryReadCallback, NullMemoryWriteCallback, NullMprCallback, this);
		pCore->GetHuC6270_1()->SetCallbacks(NullVRAMWriteCallback, NullVRAMReadCallback, NullScanlineCallback, NullVBlankCallback, this);
		pCore->GetHuC6270_2()->SetCallbacks(NullVRAMWriteCallback, NullVRAMReadCallback, NullScanlineCallback, NullVBlankCallback, this);
	}
}

const FBankSet& FPCEEmu::GetBankSet(int hwBankIndex) const
{
	return BankSets[hwBankIndex];
}

FBankSet* FPCEEmu::GetBankSetPtr(int hwBankIndex) const
{
	return BankSetPtrs[hwBankIndex];
}

int FPCEEmu::GetGameBankCount() const
{
	// todo cache this?
	const bool bIsCdRom = pMedia->IsCDROM();
	const int romSize = bIsCdRom ? GG_BIOS_SYSCARD_SIZE : pMedia->GetROMSize();
	const int romBankCount = (romSize / 0x2000) + (romSize % 0x2000 ? 1 : 0);
	return romBankCount;
}

// Note: this function needs to be fast. 
// Careful when adding new code - especially debug code that uses containers that allocate memory. 
void FPCEEmu::MapMprBank(uint8_t mprIndex, uint8_t newBankIndex)
{
	FCodeAnalysisState& state = CodeAnalysis;

	// Get the bank id of the bank we are about to map in.
	const uint16_t newBankId = GetBankIdForMprSlot(newBankIndex, mprIndex);
	FCodeAnalysisBank* pInBank = CodeAnalysis.GetBank(newBankId);
	const int16_t outBankId = MprBankId[mprIndex];
	// This can be null if a bank is getting mapped to this slot for the very first time.
	FCodeAnalysisBank* pOutBank = CodeAnalysis.GetBank(outBankId);

	assert(pInBank);
	if (!pInBank)
		return;

	// It's possible for the in bank to be the same as the outbank.
	// This can happen because the same rom bank can be in the rom map multiple times.
	// For example a game with 64 rom banks, bank indices 32 & 64 will represent the same rom bank.
	// A game that does this is Dragon Saber.
	if (pInBank == pOutBank)
	{
		BANK_LOG("In bank is the same as the out bank.");
	}

	const uint16_t oldMappedAddress = pInBank->GetMappedAddress();
	const int oldPrimaryPage = pInBank->PrimaryMappedPage;
	// Hardcoding save ram to be RW. This is to work around the situation where save ram can be paged in but gg reports the memory as read only.
	const EBankAccess bankAccess = (newBankIndex == kBankHWPage || newBankIndex == kBankSaveRAM) ? EBankAccess::ReadWrite : pMemory->GetMemoryMapWrite()[newBankIndex] ? EBankAccess::ReadWrite : EBankAccess::Read;
	const int pageNo = mprIndex * 8;
	state.MapBank(newBankId, pageNo, bankAccess);
	pInBank->PrimaryMappedPage = pageNo;
	MprBankId[mprIndex] = newBankId;

	const bool bFirstTimeInThisSlot = !(BankSetPtrs[newBankIndex]->MappedSlotsMask & (1 << mprIndex));
	BankSetPtrs[newBankIndex]->RecordSlotMapping(mprIndex);

	// Case 1: first time this bank is in this slot — check if any current vectors point into it.
	// Requires slot 7 to already be mapped so the vector reads are valid.
	if (bFirstTimeInThisSlot && MprBankId[7] != -1)
		AddInterruptVectorFunctionLabels(newBankId);

	// Case 2: slot 7 just got new vectors for the first time — re-check all already-mapped
	// banks in case their routines are now pointed to by the new vectors.
	if (mprIndex == 7 && bFirstTimeInThisSlot)
	{
		for (int slot = 0; slot < 7; slot++)
		{
			if (MprBankId[slot] != -1)
				AddInterruptVectorFunctionLabels(MprBankId[slot]);
		}
	}

#if BANK_SWITCH_DEBUG
	BANK_LOG("[PC=%04x] IN: '%s' OUT: '%s' 0x%x->0x%x", GetPC().GetAddress(), pInBank->Name.c_str(), pOutBank ? pOutBank->Name.c_str() : "None", oldMappedAddress, pInBank->GetMappedAddress());
#endif

	// Deal with the case where a RW bank is getting replaced by a Read only bank.
	// MapBank() won't remove the Write mapping of the RW bank, so the RW bank will remain
	// mapped Write only.
	if (pOutBank != pInBank)
	{
		if (pOutBank && pOutBank->IsMapped())
		{
			BANK_LOG("Unmapping %d %s because it was still mapped after MapBank()", outBankId, pOutBank->Name.c_str());
			assert(pOutBank->Mapping == EBankAccess::Write);
			bool bUnMappedOk = state.UnMapBank(outBankId, pageNo, pOutBank->Mapping);
			assert(bUnMappedOk);
		}
	}
#if DEBUG_STATS_VIEWER
	// Keep track of bank related debug stats
	if (pCurrentProjectConfig)
	{
		if (pGameDebugStats)
		{
			// Track banks mapped to multiple physical memory ranges
			int slotCount = 0;
			for (int i = 0; i < kNumMprSlots; i++)
			{
				if (pMemory->GetMpr(i) == newBankIndex)
					slotCount++;
			}
	
			if (slotCount > 1)
			{
				int& perGameCount = pGameDebugStats->NumDupeBanks;
				perGameCount = MAX(perGameCount, slotCount);
				if (pGameDbEntry)
					pGameDbEntry->MaxDupeBanks = perGameCount;
			}
		}
		
		if (!pMedia->IsCDROM())
		{
			if (newBankIndex < kBankCdRomRamStart)
			{
				const int romIndex = pMedia->GetRomBankIndex(newBankIndex);
				if (pGameDbEntry)
				{
					if (romIndex < pGameDbEntry->Banks.size())
					{
						FGameDbBank& dbBank = pGameDbEntry->Banks[romIndex];
						const bool alreadyMapped = std::find(dbBank.MprSlots.begin(), dbBank.MprSlots.end(), mprIndex) != dbBank.MprSlots.end();
						if (!alreadyMapped)
						{
							if (!dbBank.MprSlots.empty())
							{
								CodeAnalysis.Debugger.RegisterEvent((uint8_t)EEventType::BankAddressChange, GetPC(), mprIndex * 0x2000, romIndex, 0);
								dbBank.bFixed = false;
							}
							dbBank.MprSlots.push_back(mprIndex);
						}
					}
				}
			}
		}
	}

	if (pDebugStats)
		pDebugStats->NumBankSwitchesThisFrame++;
#endif

#if BANK_SWITCH_DEBUG
	// Check we only have 8 banks mapped.
	// If we have any other number then something has gone wrong.
	int mappedBanks = 0;
	for (int b = 0; b < FCodeAnalysisState::BankCount; b++)
	{
		FCodeAnalysisBank& bank = state.GetBanks()[b];
		if (bank.IsMapped())
			mappedBanks++;
	}
	if (mappedBanks > kNumMprSlots)
	{
		for (int b = 0; b < FCodeAnalysisState::BankCount; b++)
		{
			FCodeAnalysisBank& bank = state.GetBanks()[b];
			if (bank.IsMapped())
				BANK_LOG("Mapped: %d %s. Mapping %d [0=None,1=R,2=W,3=RW]", bank.Id, bank.Name.c_str(), bank.Mapping);
		}
	}
	
	if (bDoneInitialBankMapping)
	{
		assert(mappedBanks == kNumMprSlots);
	}
	else
	{
		if (mappedBanks > kNumMprSlots)
			BANK_ERROR("%d Banks mapped.", mappedBanks);
	}

	for (int i = 0; i < kNumMprSlots; i++)
	{
		if (pMemory->GetMpr(i) != kBankHWPage)
		{
			const FCodeAnalysisBank* pBank = CodeAnalysis.GetBank(MprBankId[i]);
			//assert(pBank);
			if (pBank)
			{
				if (pBank->Memory != pMemory->GetMemoryMap()[pMemory->GetMpr(i)])
					BANK_ERROR("Mpr slot %d bank memory does not match geargfx", i);
			}
		}
	}

	int b = 0;
	for (int addrVal = 0; addrVal < 0xffff; addrVal += 0x2000, b++)
	{
		FCodeAnalysisBank* pBank = CodeAnalysis.GetBank(GetCodeAnalysis().GetBankFromAddress(addrVal));
		if (pBank)
			BANK_LOG("%d Address 0x%04x: Bank Id %03d '%-7s'. %d%s", b, addrVal, pBank->Id, pBank->Name.c_str(), pBank->PrimaryMappedPage, b == mprIndex ? " <---" : "");
		else
			BANK_LOG("%d Address 0x%04x: no bank mapped",b, addrVal);
	}

	if (bDoneInitialBankMapping)
		CheckDupeMprBankIds();
#endif // BANK_SWITCH_DEBUG 

#if !NEWADDRESSREF
	if (pInBank->PrimaryMappedPage != oldPrimaryPage)
	{
		BANK_LOG("Physical address modifed for '%s'. 0x%x->0x%x. Fixing address refs.", pInBank->Name.c_str(), oldMappedAddress, pInBank->GetMappedAddress());
		state.FixupAddressRefs();
	}
#endif

	// Force all banks to update their item list.
	// Also force the code analysis state to update it's ItemList too
	//state.SetAllBanksDirty();
	// This was causing perf issues. Replaced with SetAddressRangeDirty(). 
	// This is now done in FPCEEmuTick()
	//state.SetAddressRangeDirty();
}

// Get a BankId for a specific hw bank [0-255] in an mpr slot [0-7].
int16_t FPCEEmu::GetBankIdForMprSlot(uint8_t hwBankIndex, uint8_t mprIndex)
{
	if (BankSetPtrs[hwBankIndex] == nullptr)
		return -1;

	int16_t freeBank = BankSetPtrs[hwBankIndex]->GetFreeBank(mprIndex);
	if (freeBank != -1)
	{
		MprBankSet[mprIndex] = hwBankIndex;
		return freeBank;
	}

	// If we couldnt find a free bank return an unused bank
	freeBank = BankSetPtrs[kBankUnusedStart]->GetFreeBank(mprIndex);
	MprBankSet[mprIndex] = kBankUnusedStart;
	assert(freeBank != -1);
	return freeBank;
}

void FPCEEmu::CheckDupeMprBankIds()
{
	bool bDupe[8] = { 0, 0, 0, 0, 0, 0, 0, 0 };
	for (int i = 0; i < 8; i++)
	{
		for (int j = 0; j < 8; j++)
		{
			if (i != j)
			{
				if (MprBankId[i] == -1)
					continue;
				
				if (MprBankId[i] == MprBankId[j])
				{
					if (!bDupe[j])
					{
						FCodeAnalysisBank* pBank = CodeAnalysis.GetBank(MprBankId[i]);
						assert(pBank);
						LOGERROR("Dupe bank '%s' found in slots %d and %d", pBank->Name.c_str(), i, j);
						assert(0);
					}
					bDupe[i] = true;
				}
			}
		}
	}
}

void FPCEEmu::CheckPhysicalMemoryRangeIsMapped()
{
	BANK_LOG("Checking physical memory range bank mappings...");

	// check entire physical address range is mapped
	for (int addrVal = 0; addrVal < 0xffff; addrVal += 0x2000)
	{
		FCodeAnalysisBank* pBank = CodeAnalysis.GetBank(CodeAnalysis.GetBankFromAddress(addrVal));
		if (!pBank)
		{
			BANK_ERROR("Address 0x%x is not mapped", addrVal);
		}
		else
		{
			BANK_LOG("Bank '%s' is mapped to address 0x%x", pBank->Name.c_str(), addrVal);

			const uint16_t mappedAddrFromBank = pBank->GetMappedAddress();
			if (mappedAddrFromBank != addrVal)
			{
				BANK_ERROR("Bank '%s' is mapped to a different address than the code analysis. Bank mapped address = 0x%x. Code analysis = 0x%x", pBank->Name.c_str(), mappedAddrFromBank, addrVal);
			}
			assert(mappedAddrFromBank == addrVal);
		}
	}
}

void FPCEEmu::UpdateDebugStats()
{
#if DEBUG_STATS_VIEWER
	if (pGameDebugStats && pDebugStats)
	{
		FCodeAnalysisState& state = CodeAnalysis;

		// Track number of banks ever mapped
		std::set<uint16_t> bankIdsPreviouslyMapped;
		for (int i = 0; i < kBankCdRomRamStart; i++)
		{
			const int16_t bankId = BankSetPtrs[i]->GetBankId(0);
			if (FCodeAnalysisBank* pBank = state.GetBank(bankId))
			{
				if (pBank->bEverBeenMapped)
					bankIdsPreviouslyMapped.insert(bankId);
			}
		}
		
		if (!state.Debugger.IsStopped())
			pDebugStats->Update(ImGui::GetIO().DeltaTime);

		pGameDebugStats->NumBanksMapped = (int)bankIdsPreviouslyMapped.size();
		pGameDebugStats->MaxBankSwitches = MAX(pGameDebugStats->MaxBankSwitches, pDebugStats->NumBankSwitchesThisFrame);
		pGameDebugStats->AvgFrameRate = (float)pDebugStats->GetAverageFrameRate();
	
		// Keep track of number of non canonical banks that have labels and code.
		int nonCanonicalBanksWithLabels = 0;
		int nonCanonicalBanksWithCodeItems = 0;
		for (int i = 0; i < FPCEEmu::kNumHwBanks; i++)
		{
			const FBankSet& bankSet = BankSets[i];
			if (bankSet.Banks.empty())
				continue;

			const int16_t canonicalBankId = bankSet.GetBankId();
			for (auto& entry : bankSet.Banks)
			{
				if (canonicalBankId != entry.BankId)
				{
					FCodeAnalysisBank* pBank = state.GetBank(entry.BankId);
					if (pBank)
					{
						if (pBank->NumLabels)
							nonCanonicalBanksWithLabels++;
						if (pBank->NumCodeItems)
							nonCanonicalBanksWithCodeItems++;
					}
				}
			}

			// todo code items
			pGameDebugStats->NumNonCanonicalBanksWithLabels = nonCanonicalBanksWithLabels;
		}
	}
#endif
}

// Check the geargfx memory matches our bank's mapped memory
void FPCEEmu::CheckMemoryMap()
{
//#ifndef NDEBUG
#if 0
	Memory* pMemory = pCore->GetMemory();

	LOGINFO("Checking memory map");
	for (int i = 0; i <= 255; i++)
	{
		if (FCodeAnalysisBank* pBank = CodeAnalysis.GetBank(Banks[i][0]))
		{
			const int romIndex = i < 128 ? pCore->GetMedia()->GetRomBankIndex(i) : -1;
			if (pBank->Memory == pMemory->GetMemoryMap()[i])
			{
				if (i < 128)
					BANK_LOG("0x%02x: '%-7s' '%-8s'. id %03d. rom index %03d. Memory matches OK", i, pBank->Name.c_str(), GetBankType(pMemory, i).c_str(), pBank->Id, romIndex);
				else
					BANK_LOG("0x%02x: '%-7s' '%-8s'. id %03d. Memory matches OK", i, pBank->Name.c_str(), GetBankType(pMemory, i).c_str(), pBank->Id);
			}
			else
			{
				LOGERROR("0x%02x: '%-7s' '%-8s'. id %03d. rom index %03d. 0x%llx 0x%llx Memory DOES NOT MATCH", i, pBank->Name.c_str(), GetBankType(pMemory, i).c_str(), pBank->Id, romIndex, pBank->Memory, pMemory->GetMemoryMap()[i]);
			}
		}
	}
	LOGINFO("Done");
#endif
}

void EventShowBankAddressChange(FCodeAnalysisState& state, const FEvent& event)
{
	// todo: display snippet?
	ImGui::Text("ROM %02d", event.Value);
}

void FPCEEmu::CreateBanks()
{
	std::string bankPostFix[8] = { "", "_2", "_3", "_4", "_5", "_6", "_7", "_8" };
	char bankName[32];

	// Hardware page. (IO)
	// This is a bit of a hack. We use memory owned by Geargfx.
	// We write values to it every time a hw page location is read or written to.
	// To the user, it should look like a normal memory location in the code analysis view.
	for (int d = 0; d < kNumBankSetIds; d++)
	{
		// Creating as machine ROM, so it doesn't get exported by the asm exporter.
		sprintf(bankName, "HWPAGE%s", bankPostFix[d].c_str());
		BankSets[kBankHWPage].AddBankId(CodeAnalysis.CreateBank(bankName, 8, pCore->GetMemory()->GetHWPageMemory(), true /*bMachineROM*/, 0x0));
	}

	// Working RAM
	for (int d = 0; d < kNumBankSetIds; d++)
	{
		sprintf(bankName, "WRAM%s", bankPostFix[d].c_str());
		BankSets[kBankWRAM0].AddBankId(CodeAnalysis.CreateBank(bankName, 8, pMemory->GetWorkingRAM(), false /*bMachineROM*/, 0x2000));
	}

	// Save RAM.
	// Note: pMemory->GetBackupRAMSize() will report 2048 bytes but Geargfx actually has a 8192 bytes buffer.
	for (int d = 0; d < kNumBankSetIds; d++)
	{
		sprintf(bankName, "SRAM%s", bankPostFix[d].c_str());
		BankSets[kBankSaveRAM].AddBankId(CodeAnalysis.CreateBank(bankName, 8, pMemory->GetBackupRAM(), false /*bMachineROM*/, kDefaultInitialBankAddr));
	}

	// CD ROM RAM
	u8* pUnusedMem = pMemory->GetUnusedMemory();
	for (int i = 0, b = kBankCdRomRamStart; i < kNumCdRomRamBanks; i++, b++)
	{
		for (int d = 0; d < kNumBankSetIds; d++)
		{
			sprintf(bankName, "CDRAM_%d%s", i, bankPostFix[d].c_str());
			BankSets[b].AddBankId(CodeAnalysis.CreateBank(bankName, 8, pUnusedMem, false /*bMachineROM*/, kDefaultInitialBankAddr));
		}
	}

	// move this to reset banks?
	// do I need this any more?
	for (int d = kBankUnusedStart; d < kNumHwBanks; d++)
		BankSetPtrs[d] = &BankSets[kBankUnusedStart];

	// ROMs. Create with unused ram initially because the rom memory doesn't exist yet. 
	// The real memory gets set later after the game gets loaded. 
	for (int b = 0; b < kNumRomBanks; b++)
	{
		for (int d = 0; d < kNumBankSetIds; d++)
		{
			sprintf(bankName, "ROM_%03d%s", b, bankPostFix[d].c_str());
			BankSets[b].AddBankId(CodeAnalysis.CreateBank(bankName, 8, pUnusedMem, false /*bMachineROM*/, kDefaultInitialBankAddr));
		}
	}

	// Unmapped/unused banks. One for each mpr slot.
	for (int d = 0; d < kNumMprSlots; d++)
	{
		sprintf(bankName, "UNMAPPED_%02d", d);
		BankSets[kBankUnusedStart].AddBankId(CodeAnalysis.CreateBank(bankName, 8, pMemory->GetUnusedMemory(), false /*bMachineROM*/, kDefaultInitialBankAddr));
	}
}

bool FPCEEmu::Init(const FEmulatorLaunchConfig& config)
{
#ifndef NDEBUG
	auto t1 = std::chrono::high_resolution_clock::now();
#endif

	FEmuBase::Init(config);
	RegisterPCEMCPTools(this);

	const FPCELaunchConfig& PCELaunchConfig = (const FPCELaunchConfig&)config;
 
	const std::string windowTitle = kAppTitle + " - " + "No project loaded";
	SetWindowTitle(windowTitle.c_str());
	SetWindowIcon(GetBundlePath("PCELogo.png"));

#if DEBUG_STATS_VIEWER
	pDebugStats = new FEmuDebugStats;
#endif
#if ASSEMBLE_AFTER_ASM_EXPORT
	pAsmExportValidator = new FAsmExportValidator(this);
#endif

	// Initialise Emulator
	pCore = new GeargrafxCore();
	pCore->Init(CodeAnalysis.Debugger.GetDebuggerStoppedPtr());
	
	pMemory = pCore->GetMemory();

	EnableGeargrafxCallbacks(true);

	pMedia = pCore->GetMedia();
	pMedia->PreloadCdRom(true);

	pVPos = pCore->GetHuC6270_1()->GetState()->VPOS;

	p6280State = pCore->GetHuC6280()->GetState();
	p6270State = pCore->GetHuC6270_1()->GetState();

	pPCE6502CPU = new FPCECPUEmulator6502(this);

	pFrameBuffer = new uint8_t[FPCEEmu::kFramebufferSize];
	pAudioBuf = new int16_t[GG_AUDIO_BUFFER_SIZE];;

	CPUType = ECPUType::HuC6280;

	pGlobalConfig = new FPCEConfig();
	pGlobalConfig->Init();
	pGlobalConfig->Load(kGlobalConfigFilename);
	CodeAnalysis.SetGlobalConfig(pGlobalConfig);
	SetHexNumberDisplayMode(pGlobalConfig->NumberDisplayMode);
	SetNumberDisplayMode(pGlobalConfig->NumberDisplayMode);
	CodeAnalysis.Config.RomType = ESystemRom::None;
	CodeAnalysis.Config.bSupportedDataTypes[(int)EDataType::CharacterMap] = false;	// Character map feature isn't working on PCE yet - hide it from the user.
	CodeAnalysis.Config.bSupportedDataTypes[(int)EDataType::ColAttr] = false;	// ColAttr is ZX Spectrum specific (I think)
	CodeAnalysis.Config.bSupportedDataTypes[(int)EDataType::Struct] = false;	// Structs don't work properly yet.

#if CDROM_SUPPORT
	// todo: check this is system card 3.0.
	const std::string& biosFilePath = GetPCEGlobalConfig()->BiosFilePath;
	bool bBiosLoaded = pCore->LoadBios(biosFilePath.c_str(), true);
	LOGINFO("%s Bios '%s'", bBiosLoaded ? "Loaded" : "Failed to load", biosFilePath.c_str());
#endif

	// This needs to happen or GetCanonicalAddressRef() & GetPC() won't work
	CodeAnalysis.Init(this);

	CreateBanks();
	BuildCanonicalBankIdLookup();
	BuildBankSetLookup();
	ResetBanks();
	MapMprBanks();

	assert(MprBankIdsAreValid());

	// todo: move to loadproject
	//CheckMemoryMap();

	CodeAnalysis.Config.bShowBanks = true;
	CodeAnalysis.ViewState[0].Enabled = true;	// always have first view enabled
	
	// set supported bitmap formats
	CodeAnalysis.Config.bSupportedBitmapTypes[(int)EBitmapFormat::Bitmap_1Bpp] = true;
	CodeAnalysis.Config.bSupportedBitmapTypes[(int)EBitmapFormat::Sprite4Bpp_PCE] = true;
	CodeAnalysis.Config.bSupportedBitmapTypes[(int)EBitmapFormat::Sprite3Bpp_PCE] = true;
	CodeAnalysis.Config.bSupportedBitmapTypes[(int)EBitmapFormat::BGTile4Bpp_PCE] = true;
	
	for (int i = 0; i < FCodeAnalysisState::kNoViewStates; i++)
	{
		CodeAnalysis.ViewState[i].CurBitmapFormat = EBitmapFormat::Bitmap_1Bpp;
	}

	const FPCEConfig* pPCEConfig = GetPCEGlobalConfig();
	AddGamesList(kPCERomGameListName, GetPCEGlobalConfig()->SnapshotFolder.c_str());
#if CDROM_SUPPORT
	AddGamesList(kCDRomGameListName, GetPCEGlobalConfig()->CdRomFolder.c_str());
#endif
	LoadFont();

	pVRAMState = new FVRAMAnalysisState(this);
	pCDROMAnalyser = new FCDROMAnalyser(this);
	pRecentMemoryAccess = new FRecentMemoryAccess();

	// This is where we add the viewers we want
	pPCEViewer = new FPCEViewer(this);
	AddViewer(pPCEViewer);
	AddViewer(new FBanksViewer(this));
	AddViewer(new FPCERegistersViewer(this));

	FOverviewViewer* pOverviewViewer = new FOverviewViewer(this);
	pOverviewViewer->SetRomOptionEnabled(false); // this enables showing the entire physical address range.
	AddViewer(pOverviewViewer);
	AddViewer(new FPaletteViewer(this));
	AddViewer(new FJoypadViewer(this));
	pSpriteViewer = new FSpriteViewer(this);
	AddViewer(pSpriteViewer);
	AddViewer(new FBackgroundViewer(this));
	pVRAMViewer = new FVRAMViewer(this);
	AddViewer(pVRAMViewer);
	AddViewer(new FMemoryViewer(this));
	AddViewer(new FZeroPageViewer(this));
#ifndef NDEBUG
	AddViewer(new FPCEGraphicsViewer(this));
#endif
	AddViewer(new FRecentMemoryAccessViewer(this));
#if CDROM_SUPPORT
	pCDROMViewer = new FCDROMViewer(this);
	AddViewer(pCDROMViewer);
#endif
	pGraphicsViewer = new FPCENewGraphicsViewer(this);
	AddViewer(pGraphicsViewer);

#if BATCH_GAME_VIEWER
	pBatchGameLoadViewer = new FBatchGameLoadViewer(this);
	AddViewer(pBatchGameLoadViewer);
#endif
#if DEBUG_STATS_VIEWER
	AddViewer(new FDebugStatsViewer(this));
#endif
#if GAME_DB_VIEWER
	AddViewer(new FGameDbViewer(this));
#endif

	CodeAnalysis.ViewState[0].Enabled = true;	// always have first view enabled

	LoadPCEGameConfigs(this);

	// load the command line game if none specified then load the last game
	bool bLoadedGame = false;

	if (config.SpecificGame.empty() == false)
	{
		bLoadedGame = StartGameFromName(config.SpecificGame.c_str(), true);
	}
	else if (pGlobalConfig->LastGame.empty() == false)
	{
		bLoadedGame = StartGameFromName(pGlobalConfig->LastGame.c_str(), true);
	}

	if (!bLoadedGame)
	{
		CodeAnalysis.Debugger.SetPC(FAddressRef(MprBankId[0], 0));
		CodeAnalysis.Debugger.Break();
	}
	
	// Setup Debugger
	FDebugger& debugger = CodeAnalysis.Debugger;
	debugger.RegisterEventType((int)EEventType::BankAddressChange, "Bank Address Change", 0xff0000ff, nullptr, EventShowBankAddressChange);
	//debugger.RegisterEventType((int)EEventType::ScreenAttrWrite, "Screen Attr Write", 0xff007fff, nullptr, EventShowAttrValue);
	//debugger.RegisterEventType((int)EEventType::KempstonJoystickRead, "Kempston Read", 0xff007f1f, IOPortEventShowAddress, IOPortEventShowValue);

	// Setup Memory Analyser
	//pMemoryAnalyser->AddROMArea(kROMStart, kROMEnd);
	//pMemoryAnalyser->SetScreenMemoryArea(kScreenPixMemStart, kScreenAttrMemEnd);

	InitPCEAsmExporters();
	ExportStartAddress = 0x2000;
	ExportEndAddress = 0xffff;

	CodeAnalysis.pGlobalConfig->ExportAssembler = "PCEAS";

#ifndef NDEBUG
	std::chrono::duration<double, std::milli> ms_double = std::chrono::high_resolution_clock::now() - t1;
	LOGINFO("FPCEEmu::Init() took %.2f ms", ms_double);
#endif

	return true;
}

void FPCEEmu::ResetBanks()
{
	const bool bIsCdRom = pMedia->IsCDROM();
	const int romBankCount = GetGameBankCount();

#if BANK_SWITCH_DEBUG
	const int romSize = bIsCdRom ? GG_BIOS_SYSCARD_SIZE : pMedia->GetROMSize();
	BANK_LOG("ResetBanks()");
	if (bIsCdRom)
		BANK_LOG("BIOS size is %d bytes (%d Kb). BIOS bank count is %d", romSize, romSize / 1024, romBankCount);
	else
		BANK_LOG("Rom size is %d bytes (%d Kb). Bank count is %d", romSize, romSize / 1024, romBankCount);
#endif

	for (int bankNo = 0; bankNo < kNumHwBanks; bankNo++)
	{
		BankSets[bankNo].Reset();
		BankSetPtrs[bankNo] = &BankSets[kBankUnusedStart];
	}

	if (pMemory->IsBackupRamEnabled())
	{
		BankSetPtrs[kBankSaveRAM] = &BankSets[kBankSaveRAM];
	}
	
	BankSetPtrs[kBankHWPage] = &BankSets[kBankHWPage];
	BankSetPtrs[kBankWRAM0] = &BankSets[kBankWRAM0];
	BankSetPtrs[kBankWRAM1] = &BankSets[kBankWRAM0];
	BankSetPtrs[kBankWRAM2] = &BankSets[kBankWRAM0];
	BankSetPtrs[kBankWRAM3] = &BankSets[kBankWRAM0];

	// Set initial rom banks.
	for (int bankNo = 0; bankNo < 128; bankNo++)
	{
		const int bankIndex = romBankCount ? pCore->GetMedia()->GetRomBankIndex(bankNo) : bankNo;
		BankSetPtrs[bankNo] = &BankSets[bankIndex];
	}

	// Unmap the banks from the mpr slots.
	for (int mprNum = 0; mprNum < 8; mprNum++)
	{
		const int16_t bankId = MprBankId[mprNum];
		if (FCodeAnalysisBank* pBank = CodeAnalysis.GetBank(bankId))
		{
			CodeAnalysis.UnMapBank(bankId, mprNum * 8, pBank->Mapping);
		}
		MprBankId[mprNum] = -1;
		MprBankSet[mprNum] = -1;
	}

	// Reset all code analysis banks for re-use.
	// I am using PrimaryMappedPage being -1 as a way to mark a bank as unused.
	// I know this is not great but it's the best I could do without changing the code analysis code.
	for (int b = 0; b < FCodeAnalysisState::BankCount; b++)
	{
		FCodeAnalysisBank& bank = CodeAnalysis.GetBanks()[b];
	
		// do i need to call UnmapFromPage() here? the MappedReadPages and MappedWritePages wont get reset

		bank.PrimaryMappedPage = -1;
		bank.Mapping = EBankAccess::None;
		bank.bEverBeenMapped = false;
	}

	// Set ROM/BIOS banks primary mapped page to mark them as in use.
	// They will get their actual mapped address set when they are mapped in.
	// We do this because we can't have any banks in use with PrimaryMappedPage of -1.
	BankSets[kBankHWPage].SetPrimaryMappedPage(CodeAnalysis, 0, 0);
	BankSets[kBankWRAM0].SetPrimaryMappedPage(CodeAnalysis, 0, kDefaultPrimaryMappedPage);
	BankSets[kBankSaveRAM].SetPrimaryMappedPage(CodeAnalysis, 0, kDefaultPrimaryMappedPage);

	std::string bankPostFix[8] = { "", "_2", "_3", "_4", "_5", "_6", "_7", "_8" };
	char bankName[32];

	// Patch in the rom memory into the rom banks and set their primary mapped page.
	for (int b = 0; b < romBankCount; b++)
	{
		BankSets[b].SetPrimaryMappedPage(CodeAnalysis, 0, kDefaultPrimaryMappedPage);

		uint8_t* pBytes = bIsCdRom ? pMedia->GetSysCardBios() : pMedia->GetROM();
		uint8_t* pBankMemory = pBytes + b * 0x2000;
		for (int d = 0; d < kNumBankSetIds; d++)
		{
			FCodeAnalysisBank* pBank = CodeAnalysis.GetBank(BankSets[b].GetBankId(d));
			pBank->Memory = pBankMemory;

			sprintf(bankName, "%s_%02d%s", bIsCdRom ? "BIOS" : "ROM", b, bankPostFix[d].c_str());
			pBank->Name = bankName;

			pBank->bMachineROM = bIsCdRom;
		}
	}

	if (bIsCdRom)
	{
		// Set cd rom ram banks
		for (int i = 0, b = kBankCdRomRamStart; i < kNumCdRomRamBanks; i++, b++)
		{
			BankSetPtrs[b] = &BankSets[b];
			uint8_t* pBankMemory = pMemory->GetCDROMRAM() + i * 0x2000;

			BankSets[b].SetPrimaryMappedPage(CodeAnalysis, 0, kDefaultPrimaryMappedPage);

			for (int d = 0; d < kNumBankSetIds; d++)
			{
				FCodeAnalysisBank* pBank = CodeAnalysis.GetBank(BankSets[b].GetBankId(d));
				pBank->Memory = pBankMemory;
			}
		}
	}

	const int cardRAMSize = pMemory->GetCardRAMSize();
	if (cardRAMSize)
	{
		BANK_LOG("Card RAM size is %d bytes (%d kb).", cardRAMSize, cardRAMSize / 1024);

		// Set card ram banks
		const uint8_t cardRamStart = pMemory->GetCardRAMStart();
		const uint8_t cardRamEnd = pMemory->GetCardRAMEnd();
		for (uint8_t r = cardRamStart; r <= cardRamEnd; r++)
		{
			uint8_t* pBankMemory = pMemory->GetMemoryMap()[r];

			BankSets[r].SetPrimaryMappedPage(CodeAnalysis, 0, kDefaultPrimaryMappedPage);

			for (int d = 0; d < kNumBankSetIds; d++)
			{
				FCodeAnalysisBank* pBank = CodeAnalysis.GetBank(BankSets[r].GetBankId(d));
				pBank->Memory = pBankMemory;

				sprintf(bankName, "CARD_%02d%s", r - cardRamStart, bankPostFix[d].c_str());
				pBank->Name = bankName;
			}
			BankSetPtrs[r] = &BankSets[r];	
		}

		BANK_LOG("Num card RAM banks = %d.", (cardRamEnd - cardRamStart) + 1);
	}
}

void FPCEEmu::MapMprBanks()
{
#if BANK_SWITCH_DEBUG
	BANK_LOG("Mapping initial banks...");
	bDoneInitialBankMapping = false;
#endif

	// Go through each mpr slot and map a bank for each one
	for (int mprNum = 0; mprNum < 8; mprNum++)
	{
		MapMprBank(mprNum, pMemory->GetMpr(mprNum));
	}

#if BANK_SWITCH_DEBUG
	bDoneInitialBankMapping = true;
	BANK_LOG("Done mapping initial banks");
#endif

#ifndef NDEBUG
	CheckDupeMprBankIds();
	CheckPhysicalMemoryRangeIsMapped();
#endif
}

bool FPCEEmu::MapBankIdToMprSlot(uint8_t mprIndex, int16_t bankId)
{
	const int hwBankIndex = pMemory->GetMpr(mprIndex);

	// This can fail if the bank id saved in the project's analysis json is stale relative to
	// the mpr register value restored from the machine state (e.g. the two files were
	// saved at different points in time). 
	if (!BankSetPtrs[hwBankIndex]->ClaimSpecificBank(bankId, mprIndex))
	{
		LOGERROR("MapBankIdToMprSlot failed: bank id %d not found in bank set %d (mpr slot %d).", bankId, hwBankIndex, mprIndex);
		return false;
	}
	MprBankSet[mprIndex] = hwBankIndex;

	FCodeAnalysisBank* pInBank = CodeAnalysis.GetBank(bankId);

	assert(pInBank);
	if (!pInBank)
		return false;

	FCodeAnalysisState& state = CodeAnalysis;

	// Hardcoding save ram to be RW. This is to work around the situation where save ram can be paged in but gg reports the memory as read only.
	const EBankAccess bankAccess = (hwBankIndex == kBankHWPage || hwBankIndex == kBankSaveRAM) ? EBankAccess::ReadWrite : pMemory->GetMemoryMapWrite()[hwBankIndex] ? EBankAccess::ReadWrite : EBankAccess::Read;
	const int pageNo = mprIndex * 8;
	state.MapBank(bankId, pageNo, bankAccess);
	pInBank->PrimaryMappedPage = pageNo;
	MprBankId[mprIndex] = bankId;
	return true;
}


void FPCEEmu::Shutdown()
{
	if (pCurrentProjectConfig != nullptr)
	{
		// Save Global Config - move to function?
		pGlobalConfig->LastGame = pCurrentProjectConfig->Name;
		SaveProject();
	}
	else
	{
		// Is this the right thing to do?
		pGlobalConfig->LastGame = "";
	}

	pGlobalConfig->Save(kGlobalConfigFilename);

	delete pCore;

	FEmuBase::Shutdown();
}

// todo: deal with resetting state when this function fails.
// if ImportAnalysisState() fails when booting up, the game save state will still load instead
// of the memory being reset. 
bool FPCEEmu::LoadProject(FProjectConfig* pGameConfig, bool bLoadGameData /* =  true*/)
{
	assert(pGameConfig);
	LOGINFO("Load Project '%s'. bLoadGameData = %s", pGameConfig->Name.c_str(), bLoadGameData ? "True" : "False");

	// Save the last game's bank mapping progress
	// todo: remove in release build
	// todo: we may not want to save asm validation results here unless automation mode is active.
	// otherwise if we may overwrite a complete validation run when we load a new game.
	SaveGameDbEntry();

	ResetProject();

	const std::string windowTitle = kAppTitle + " - " + pGameConfig->Name;
	SetWindowTitle(windowTitle.c_str());

	// Set options from config
	for (int i = 0; i < FCodeAnalysisState::kNoViewStates; i++)
	{
		CodeAnalysis.ViewState[i].Enabled = pGameConfig->ViewConfigs[i].bEnabled;
		CodeAnalysis.ViewState[i].GoToAddress(pGameConfig->ViewConfigs[i].ViewAddress);
	}

	// Are we loading a previously saved game
	if (bLoadGameData)
	{
		const std::string root = pGlobalConfig->WorkspaceRoot;
		const std::string gameRoot = pGlobalConfig->WorkspaceRoot + pGameConfig->Name;
		std::string analysisJsonFName = gameRoot + "/Analysis.json";
		std::string graphicsSetsJsonFName = gameRoot + "/GraphicsSets.json";
		std::string analysisStateFName = gameRoot + "/AnalysisState.bin";

		if (!LoadEmulatorFile(&pGameConfig->EmulatorFile))
		{
			return false;
		}

		if (pMedia->IsCDROM() && !pMedia->IsLoadedBios())
		{
			LOGERROR("A bios is required to load this rom");
			return false;
		}

		if (!LoadMachineState(gameRoot.c_str()))
		{
			return false;
		}

		ResetBanks();

		// Banks must be mapped by the point FixupPostLoad() runs in ImportAnalysisJson().
		// They get mapped in ImportPlatformAnalysisJson _before_ FixupPostLoad()

		if (FileExists(analysisJsonFName.c_str()))
		{
			if (!ImportAnalysisJson(CodeAnalysis, analysisJsonFName.c_str()))
			{
				SetLastError("Failed to import analysis json file %s.", analysisJsonFName.c_str());
				return false;
			}

			if (!ImportAnalysisState(CodeAnalysis, analysisStateFName.c_str()))
			{
				SetLastError("Failed to import analysis state file %s", analysisStateFName.c_str());
				return false;
			}

			CheckPhysicalMemoryRangeIsMapped();
		}

		//pGraphicsViewer->LoadGraphicsSets(graphicsSetsJsonFName.c_str());
	}
	else
	{
		const bool bHasSnapshot = pGameConfig->EmulatorFile.FileName.empty() == false;
		if (bHasSnapshot)
		{
			LOGINFO("LoadEmulatorFile '%s'", pGameConfig->EmulatorFile.FileName.c_str());

			if (!LoadEmulatorFile(&pGameConfig->EmulatorFile))
			{
				return false;
			}
		}
		else
		{
			return false;
		}

		ResetBanks();
		MapMprBanks();

		// We only want to do this once when we create the project
		InitPalettes();
	}

	if (pMedia->IsCDROM())
	{
#if IMPORT_BIOS_ANALYSIS_JSON
		if (FileExists(GetBundlePath(kBiosInfoJsonFile)))
			ImportAnalysisJson(CodeAnalysis, GetBundlePath(kBiosInfoJsonFile));
#endif
		CodeAnalysis.Config.RomType = ESystemRom::Bios;

#if CDROM_SUPPORT
		pCDROMAnalyser->Reset();
#endif
	}
	else
	{
		CodeAnalysis.Config.RomType = ESystemRom::None;
	}

	//if (bLoadGameData == false)
		AddLabels();

	ReAnalyseCode(CodeAnalysis);
	GenerateGlobalInfo(CodeAnalysis);
	CodeAnalysis.SetAddressRangeDirty();

	if (pDebugStats)
		pDebugStats->InitForGame(this, pGameConfig->Name);

	CodeAnalysis.Debugger.Break();

	// some extra initialisation for creating new analysis from snapshot
	if(bLoadGameData == false)
	{
		const FAddressRef initialPC = GetPC();

		// Removed this for now. We can end up with operand addresses to labels in the UNMAPPED banks
		// in the initial code. this can prevent the exported asm from assembling.
		// It is better to let the WriteCodeInfoForAddress() happen when the code is executed.
		// This makes sure the operand addresses point to actual banks/roms in physical memory.
		//SetItemCode(CodeAnalysis, initialPC);
		CodeAnalysis.Debugger.SetPC(initialPC);

		// The initial PC needs to be in the address space of the only mapped ROM: ROM_00/BIOS_00.
		// If the PC is anything else something badly has gone wrong and the game won't work
		// because it will be trying to execute undefined memory.
		//assert(initialPC.GetAddress() >= 0xe000);
		if (initialPC.GetAddress() < 0xe000)
		{
			SetLastError("Invalid initial PC $%x", initialPC.GetAddress());
			return false;
		}
	
		// Make a label for the entry point.
		// Without this an exported asm file may not assemble.
		char labelTxt[40];
		snprintf(labelTxt, 40, "func_%s_%04X_entry_point", pMedia->IsCDROM() ? "BIOS_00" : "ROM_00", initialPC.GetAddress());
		AddLabel(CodeAnalysis, initialPC, labelTxt, ELabelType::Function);
	}

	//pGraphicsViewer->SetImagesRoot((pGlobalConfig->WorkspaceRoot + "/" + pGameConfig->Name + "/GraphicsSets/").c_str());

	pCurrentProjectConfig = pGameConfig;
	if (pDebugStats)
		pGameDebugStats = &pDebugStats->GameDebugStats[pGameConfig->Name];

	if (!pMedia->IsCDROM())
	{
		const std::string fname = GetPCEGlobalConfig()->GameDbFolder + pGameConfig->Name + ".json";
		if (!LoadGameDbEntry(pGameConfig->Name, fname.c_str()))
		{
			// Create new bank mappings if no file exists
			FGameDbEntry& dbEntry = CreateGameDbEntry(pGameConfig->Name, GetGameBankCount());
			dbEntry.Banks[0].MprSlots.push_back(7);
		}

		pGameDbEntry = ::GetGameDbEntry(pGameConfig->Name);
		assert(pGameDbEntry);
	}

	LoadLua();
	
	pGlobalConfig->AddProjectToRecents(GetProjectConfig()->Name);
	
	return true;
}

void FormatMemoryAsPtr(FCodeAnalysisState& state, uint16_t addr)
{
	if (FDataInfo* pDataItem = state.GetReadDataInfoForAddress(addr))
	{
		pDataItem->DataType = EDataType::Word;
		pDataItem->ByteSize = 2;
		pDataItem->DisplayType = EDataItemDisplayType::Pointer;
		pDataItem->PointerAddress = state.GetCanonicalAddressRef(state.ReadWord(addr));
		state.SetCodeAnalysisDirty(addr);
	}
}

void AddCodeLabel(FCodeAnalysisState& state, uint16_t addr, std::string name)
{
	// Purposefully not calling SetItemCode because we dont want to run static analysis
	// when UNMAPPED_* banks are mapped in.
	const FAddressRef addrRef = state.AddressRefFromPhysicalAddress(addr);
	UpdateCodeInfoForAddress(state, addr);
	state.SetCodeAnalysisDirty(addr);
	AddLabel(state, addrRef, name.c_str(), ELabelType::Function);
}

void FPCEEmu::AddLabels()
{
	FCodeAnalysisState& state = GetCodeAnalysis();

#if !IMPORT_BIOS_ANALYSIS_JSON
	if (pMedia->IsCDROM())
	{
		// Add labels for the jump table. This will be the same for all system card revisions.
		for (int i = 0; i < kBiosSymbolCount; i++)
		{
			AddCodeLabel(state, kBiosJmpSymbols[i].Address, kBiosJmpSymbols[i].Label);
		}

		// Add labels for the routines themselves.
		// Games shouldn't call these directly.
		for (int i = 0; i < kBiosSymbolCount; i++)
		{
			AddCodeLabel(state, kBiosRoutineSymbols[i].Address, kBiosRoutineSymbols[i].Label);
		}
	}
#endif

	if (pMedia->IsCDROM())
	{
		AddLabel(state, FAddressRef(BankSets[kBankWRAM0].GetBankId(), 0x2227), "joyena", ELabelType::Data, 1);
		AddLabel(state, FAddressRef(BankSets[kBankWRAM0].GetBankId(), 0x2228), "joy", ELabelType::Data, 5);
		AddLabel(state, FAddressRef(BankSets[kBankWRAM0].GetBankId(), 0x222d), "joytrg", ELabelType::Data, 5);
		AddLabel(state, FAddressRef(BankSets[kBankWRAM0].GetBankId(), 0x2232), "joyold", ELabelType::Data, 5);
	}

	// Add labels for the memory mapped registers. These are locations in the hardware page memory bank.
	for (int i = 0; i < kDebugLabelCount; i++)
	{
		const FAddressRef addr = FAddressRef(BankSets[kBankHWPage].GetBankId(), kDebugLabels[i].Address);
		// This causes AnalyseAtPC to be called on memory addresses that are not code.
		// An example of this is hello.pce at address 0xe001
		//SetItemCode(state, addr);
		AddLabel(state, addr, kDebugLabels[i].Label, ELabelType::Data);

		if (kDebugLabels[i].Comment != nullptr)
		{
			FCommentBlock* pComment = AddCommentBlock(state, addr);
			if (pComment->Comment.empty())
				pComment->Comment = kDebugLabels[i].Comment;
		}
	}


	// Add labels for system vector list.
	// These are the memory locations that hold the pointers to the routines - not the routines themselves.
	const int16_t rom0BankId = BankSets[0].GetBankId();
	FormatMemoryAsPtr(CodeAnalysis, kVecReset);
	AddLabel(state, FAddressRef(rom0BankId, kVecReset), "ResetVector", ELabelType::Data);
	
	FLabelInfo* pLabel = nullptr;
	FormatMemoryAsPtr(CodeAnalysis, kVecNMI);
	AddLabel(state, FAddressRef(rom0BankId, kVecNMI), "NMIVector", ELabelType::Data);

	FormatMemoryAsPtr(CodeAnalysis, kVecTimer);
	AddLabel(state, FAddressRef(rom0BankId, kVecTimer), "TimerVector", ELabelType::Data);
	
	FormatMemoryAsPtr(CodeAnalysis, kVecIRQ1);
	AddLabel(state, FAddressRef(rom0BankId, kVecIRQ1), "IRQ1Vector", ELabelType::Data);
	
	FormatMemoryAsPtr(CodeAnalysis, kVecIRQ2);
	AddLabel(state, FAddressRef(rom0BankId, kVecIRQ2), "IRQ2Vector", ELabelType::Data);

	// Create vector function labels - but only for routines in ROM 0.
	// Some games have interrupt vectors in other banks.
	// We need to create those function labels later - when the bank gets mapped in.
	AddInterruptVectorFunctionLabels(rom0BankId);
}

static void AddVectorFunctionLabel(FCodeAnalysisState& state, FCodeAnalysisBank* pBank, uint16_t routineAddr, uint8_t firstByte, const char* vecName)
{
	if (!pBank->AddressValid(routineAddr))
		return;

	char labelTxt[40];
	const FAddressRef ref(pBank->Id, routineAddr);
	if (firstByte == 0x40)
	{
		snprintf(labelTxt, 40, "func_%s_%04X_DummyVector", pBank->Name.c_str(), routineAddr);
		SetItemCode(state, ref);
	}
	else
		snprintf(labelTxt, 40, "func_%s_%04X_%sVector", pBank->Name.c_str(), routineAddr, vecName);

	FLabelInfo* pLabel = AddLabel(state, ref, labelTxt, ELabelType::Function);
	//LOGINFO("%s is %x. label %x", labelTxt, firstByte, pLabel);
}

void FPCEEmu::AddInterruptVectorFunctionLabels(int16_t bankId)
{
	// todo deal with non canonical banks.
	// call GetCanonicalBankId() before calling?

	if (IsUnusedBank(bankId))
		return;

	FCodeAnalysisState& state = GetCodeAnalysis();
	FCodeAnalysisBank* pBank = state.GetBank(bankId);
	if (!pBank)
		return;

	const uint16_t nmiAddr = ReadWord(kVecNMI);
	AddVectorFunctionLabel(state, pBank, nmiAddr, ReadByte(nmiAddr), "NMI");

	const uint16_t timerAddr = ReadWord(kVecTimer);
	AddVectorFunctionLabel(state, pBank, timerAddr, ReadByte(timerAddr), "Timer");

	const uint16_t irq1Addr = ReadWord(kVecIRQ1);
	AddVectorFunctionLabel(state, pBank, irq1Addr, ReadByte(irq1Addr), "IRQ1");

	const uint16_t irq2Addr = ReadWord(kVecIRQ2);
	AddVectorFunctionLabel(state, pBank, irq2Addr, ReadByte(irq2Addr), "IRQ2");
}

bool FPCEEmu::SaveMachineState(const char* path, int index /* = -1 */)
{
	return pCore->SaveState(path, index);
}

bool FPCEEmu::LoadMachineState(const char* path, int index /* = -1 */)
{
	return pCore->LoadState(path, index);
}

void FPCEEmu::ExportPlatformAnalysisJson(nlohmann::json& jsonDoc)
{
	nlohmann::json mprBankIds = nlohmann::json::array();
	for (int i = 0; i < kNumMprSlots; i++)
		mprBankIds.push_back(MprBankId[i]);
	jsonDoc["MprBankIds"] = mprBankIds;

	nlohmann::json bankSlotStats = nlohmann::json::array();
	for (int i = 0; i < kNumHwBanks; i++)
	{
		const FBankSet& bs = BankSets[i];
		if (bs.MappedSlotsMask == 0)
			continue;
		const int16_t primaryBankId = bs.GetBankId(0);
		if (primaryBankId == -1)
			continue;

		nlohmann::json entry;
		entry["BankId"] = primaryBankId;
		entry["MappedSlotsMask"] = bs.MappedSlotsMask;

		nlohmann::json counts = nlohmann::json::array();
		nlohmann::json orders = nlohmann::json::array();
		for (int s = 0; s < kNumMprSlots; s++)
		{
			counts.push_back(bs.SlotMapCount[s]);
			orders.push_back(bs.SlotFirstUseOrder[s]);
		}
		entry["SlotMapCount"] = counts;
		entry["SlotFirstUseOrder"] = orders;
		bankSlotStats.push_back(entry);
	}
	jsonDoc["BankSlotStats"] = bankSlotStats;
}

bool FPCEEmu::MprBankIdsAreValid() const
{
	for (int i = 0; i < kNumMprSlots; i++)
	{
		if (MprBankId[i] == -1)
			return false;
	}

	return true;
}

bool FPCEEmu::IsCDROM() const
{
	return pMedia->IsCDROM();
}


// Restore the MPR bank mappings to what they were when the project was saved to disk
bool FPCEEmu::ImportPlatformAnalysisJson(const nlohmann::json& jsonDoc)
{
	if (jsonDoc.contains("MprBankIds"))
	{
		const auto& mprBankIds = jsonDoc["MprBankIds"];
		for (int i = 0; i < kNumMprSlots && i < (int)mprBankIds.size(); i++)
		{
			if (!MapBankIdToMprSlot(i, (int16_t)mprBankIds[i]))
			{
				// The bank ids saved in the analysis json don't match the machine state we just restored. 
				SetLastError("Failed to restore mpr bank mapping for slot %d, bank id %d. Project data may be out of sync with the machine state.", i, (int16_t)mprBankIds[i]);
				return false;
			}
		}
	}

	if (jsonDoc.contains("BankSlotStats"))
	{
		for (const auto& entry : jsonDoc["BankSlotStats"])
		{
			const int16_t bankId = (int16_t)entry["BankId"];
			FBankSet* pBankSet = BankSetLookup[bankId];
			if (pBankSet == nullptr)
				continue;

			pBankSet->MappedSlotsMask = (uint8_t)entry["MappedSlotsMask"];
			pBankSet->NextFirstUseOrder = 1;

			const auto& counts = entry["SlotMapCount"];
			const auto& orders = entry["SlotFirstUseOrder"];
			for (int s = 0; s < kNumMprSlots && s < (int)counts.size(); s++)
			{
				pBankSet->SlotMapCount[s] = (uint32_t)counts[s];
				pBankSet->SlotFirstUseOrder[s] = (uint8_t)orders[s];
				if (pBankSet->SlotFirstUseOrder[s] >= pBankSet->NextFirstUseOrder)
					pBankSet->NextFirstUseOrder = pBankSet->SlotFirstUseOrder[s] + 1;
			}
		}
	}

	if (!MprBankIdsAreValid())
	{
		LOGERROR("Failed to import platform analysis json. MPR bank ids are invalid: %d %d %d %d %d %d %d %d", MprBankId[0], MprBankId[1], MprBankId[2], MprBankId[3], MprBankId[4], MprBankId[5], MprBankId[6], MprBankId[7]);
		//assert(0);
		return false;
	}

	return true;
}

void FPCEEmu::SaveGameDbEntry()
{
	if (pCurrentProjectConfig && !pMedia->IsCDROM())
	{
		const std::string gameDbPath = GetPCEGlobalConfig()->GameDbFolder;
		const std::string fname = gameDbPath + pCurrentProjectConfig->Name + ".json";
		EnsureDirectoryExists(gameDbPath.c_str());
		::SaveGameDbEntry(pCurrentProjectConfig->Name, fname);
	}
}

// save config & data
bool FPCEEmu::SaveProject()
{
	if (pCurrentProjectConfig == nullptr)
		return false;

	const std::string root = pGlobalConfig->WorkspaceRoot + pCurrentProjectConfig->Name;
	const std::string configFName = root + "/Config.json";
	const std::string analysisJsonFName = root + "/Analysis.json";
	const std::string graphicsSetsJsonFName = root + "/GraphicsSets.json";
	const std::string analysisStateFName = root + "/AnalysisState.bin";
	EnsureDirectoryExists(root.c_str());

	// set config values
	for (int i = 0; i < FCodeAnalysisState::kNoViewStates; i++)
	{
		const FCodeAnalysisViewState& viewState = CodeAnalysis.ViewState[i];
		FCodeAnalysisViewConfig& viewConfig = pCurrentProjectConfig->ViewConfigs[i];

		viewConfig.bEnabled = viewState.Enabled;
		viewConfig.ViewAddress = viewState.GetCursorItem().IsValid() ? viewState.GetCursorItem().AddressRef : FAddressRef::Invalid();
	}

	AddGameConfig(pCurrentProjectConfig);
	SaveGameConfigToFile(*pCurrentProjectConfig, configFName.c_str());

	SaveMachineState(root.c_str());
	ExportAnalysisJson(CodeAnalysis, analysisJsonFName.c_str());
	ExportAnalysisState(CodeAnalysis, analysisStateFName.c_str());
	//pGraphicsViewer->SaveGraphicsSets(graphicsSetsJsonFName.c_str());

	SaveGameDbEntry();
#if EXPORT_BIOS_ANALYSIS_JSON
	const std::string romJsonFName = GetBundlePath(kBiosInfoJsonFile);
	ExportAnalysisJson(CodeAnalysis, romJsonFName.c_str(), true);	// export ROMS only
#endif

	return true;
}

bool IsFileTypeSupported(EEmuFileType fileType)
{
	switch (fileType)
	{
#if CDROM_SUPPORT
	case EEmuFileType::CUE:
#endif
	case EEmuFileType::PCE:
#if CDROM_SUPPORT
	case EEmuFileType::ZIP:
#endif
		return true;
	default:
		return false;
	}
}

bool FPCEEmu::LoadEmulatorFile(const FEmulatorFile* pSnapshot)
{
	auto findIt = GamesLists.find(pSnapshot->ListName);
	if (findIt == GamesLists.end())
	{
		SetLastError("Games list '%s' not found", pSnapshot->ListName.c_str());
		return false;
	}

	const std::string fileName = findIt->second.GetRootDir() + pSnapshot->FileName;

	if (!IsFileTypeSupported(pSnapshot->Type))
	{
		SetLastError("Unsupported file type for '%s'", pSnapshot->FileName.c_str());
		return false;
	}

	const bool bMediaLoadedOk = pCore->LoadMedia(fileName.c_str());
	
	if (!bMediaLoadedOk)
	{
		SetLastError("Failed to load '%s'", fileName.c_str());
		return false;
	}

	if (pSnapshot->Type == EEmuFileType::CUE || pCore->GetMedia()->IsCDROM())
	{
		// todo: check bios is valid
		if (!pMedia->IsLoadedBios())
		{
			SetLastError("Bios not loaded: '%s'", GetPCEGlobalConfig()->BiosFilePath.c_str());
			return false;
		}
	}
	return true;
}

bool FPCEEmu::NewProjectFromEmulatorFile(const FEmulatorFile& snapshot)
{
	// Remove any existing config 
	RemoveGameConfig(snapshot.DisplayName.c_str());

	FPCEGameConfig* pNewConfig = CreateNewPCEGameConfigFromSnapshot(snapshot);

	if (pNewConfig != nullptr)
	{
		if (!LoadProject(pNewConfig, /* bLoadGameData */ false))
			return false;
		pNewConfig->EmulatorFile = snapshot;

		AddGameConfig(pNewConfig);
		SaveProject();

		return true;
	}
	return false;
}

void FPCEEmu::FileMenuAdditions(void)
{
	if (ImGui::MenuItem("Export ASM For Project", "Ctrl+E"))
	{
		ExportAsmForCurrentGame();
	}
}

void FPCEEmu::GlobalShortcuts(void)
{
	if (ImGui::IsKeyChordPressed(ImGuiMod_Ctrl | ImGuiKey_E))
		ExportAsmForCurrentGame();
	if (pCurrentProjectConfig && ImGui::IsKeyChordPressed(ImGuiMod_Ctrl | ImGuiKey_R))
		SoftResetMachine();
	
	// Toggle number base with Ctrl & B
	if (ImGui::IsKeyChordPressed(ImGuiMod_Ctrl | ImGuiKey_B))
	{
		ENumberDisplayMode numDisplayMode = GetNumberDisplayMode();
		if (numDisplayMode == ENumberDisplayMode::Decimal)
			numDisplayMode = ENumberDisplayMode::HexAitch;
		else if (numDisplayMode == ENumberDisplayMode::HexAitch)
			numDisplayMode = ENumberDisplayMode::HexDollar;
		else if (numDisplayMode == ENumberDisplayMode::HexDollar)
			numDisplayMode = ENumberDisplayMode::Decimal;
		else
			assert(0); // unsupported num display mode
		SetNumberDisplayMode(numDisplayMode);
	}
}

// todo: get this working on CD games
bool FPCEEmu::ExportAsmForCurrentGame()
{
	if (pCurrentProjectConfig == nullptr)
		return false;

	if (pMedia->IsCDROM())
		return false;

	std::string exportPath;

	if (pCurrentProjectConfig->AsmExportPath.empty() == false)
	{
		exportPath = pCurrentProjectConfig->AsmExportPath;
	}
	else if (pGlobalConfig->DefaultAsmExportPath.empty() == false)
	{
		exportPath = pGlobalConfig->DefaultAsmExportPath;
	}
	else
	{
		exportPath = GetGameWorkspaceRoot();
	}

	if (exportPath.back() != '/')
		exportPath += "/";

	const std::string outputAsmFname = exportPath + pCurrentProjectConfig->Name + ".asm";
	std::vector<int16_t> banksToExport;
	for (int i = 0; i < kBankCdRomRamStart; i++)
	{
		const int16_t bankId = BankSetPtrs[i]->GetBankId();

		if (pMemory->GetBankType(i) == Memory::MemoryBankType::MEMORY_BANK_TYPE_ROM)
		{
			if (std::find(banksToExport.begin(), banksToExport.end(), bankId) == banksToExport.end())
			{
				if (FCodeAnalysisBank* pBank = CodeAnalysis.GetBank(bankId))
				{
					//LOGINFO("%d Adding bank %s %d to export list", i, pBank->Name.c_str(), bankId);
					banksToExport.push_back(BankSetPtrs[i]->GetBankId());
				}
			}
		}
	}
	
	/*if (!bWriteCodeInfoWhenCodeExecuted)
	{
		LOGWARNING("'Write Code Info When Code Executed' option is not turned on. ASM Export may not work!");
	}*/

	if (!ExportAssemblerForBanks(this, outputAsmFname.c_str(), banksToExport))
	{
		return false;
	}

#if ASSEMBLE_AFTER_ASM_EXPORT
	// make this an option in the menu.
	// once the asm exporter is stable we wont need it.
	if (pAsmExportValidator && GetPCEGlobalConfig()->bUseAsmExportValidator)
	{
		pAsmExportValidator->Validate(banksToExport, outputAsmFname);
		
		const FAsmExportValidator::FResults& results = pAsmExportValidator->GetResults();
		if (pGameDbEntry)
		{
			pGameDbEntry->bValidated = true;
			pGameDbEntry->bAssemblesOk = results.bAssembledOk;
			pGameDbEntry->bEmulatorTestOk = results.bEmulatorTestOk;
			pGameDbEntry->bRomFileIdentical = results.bRomFileIdentical;
			pGameDbEntry->bRomFilePartialMatch = results.bRomFilePartialMatch;
			if (pBatchGameLoadViewer)
				pGameDbEntry->TestingMethodology = pBatchGameLoadViewer->GetTestingMethodology();
		}
	}
#endif

	return true;
}

void FPCEEmu::SystemMenuAdditions(void)
{
	if (pCurrentProjectConfig && ImGui::MenuItem("Soft Reset", "Ctrl+R"))
	{
		SoftResetMachine();
	}

	char buf[32];
	for (int i = 0; i < FProjectConfig::kNumSaveStateSlots; i++)
	{
		sprintf(buf, "Load Slot %d", i);
		if (ImGui::MenuItem(buf))
		{
			const std::string path = pGlobalConfig->WorkspaceRoot + pCurrentProjectConfig->Name;
			LoadMachineState(path.c_str(), i);
		}
	}

	for (int i = 0; i < FProjectConfig::kNumSaveStateSlots; i++)
	{
		sprintf(buf, "Save Slot %d", i);
		if (ImGui::MenuItem(buf))
		{
			const std::string path = pGlobalConfig->WorkspaceRoot + pCurrentProjectConfig->Name;
			SaveMachineState(path.c_str(), i);

			sprintf(buf, ".state%d", i);
			std::string slotFname = pCurrentProjectConfig->Name + buf;
			SetSaveStateSlot(i, slotFname.c_str());
		}
	}

}

void FPCEEmu::OptionsMenuAdditions(void)
{
#ifndef NDEBUG
	// sam. hack to workaround a bug.
	// asm export won't work due to missing labels.
	// remove this when bug is fixed.
	ImGui::MenuItem("Write Code Info When Code Executed", 0, &bWriteCodeInfoWhenCodeExecuted);

	if (ImGui::MenuItem("Callbacks Enabled", 0, &bCallbacksEnabled))
	{
		EnableGeargrafxCallbacks(bCallbacksEnabled);
	}
#endif

#if ASSEMBLE_AFTER_ASM_EXPORT
	FPCEConfig* pConfig = (FPCEConfig*)pGlobalConfig;
	ImGui::MenuItem("Validate ASM After Export", 0, &pConfig->bUseAsmExportValidator);
#endif
}

void FPCEEmu::ActionMenuAdditions(void)
{
	
}

void FPCEEmu::WindowsMenuAdditions(void)
{
}

void FPCEEmu::Tick()
{
	OPTICK_EVENT();

	FEmuBase::Tick();

	pPCEViewer->Tick();
	if (pVRAMState)
		pVRAMState->Tick();
	if (pSpriteViewer)
		pSpriteViewer->Tick();
	if (pCDROMViewer)
		pCDROMViewer->Tick();

	UpdateDebugStats();

	for (int i = 0; i < EmuFramesToRun; i++)
	{
		FDebugger& debugger = CodeAnalysis.Debugger;
		if (debugger.IsStopped() == false)
		{
			if (pDebugStats)
				pDebugStats->NumBankSwitchesThisFrame = 0;

			CodeAnalysis.OnFrameStart();
			StoreRegisters_6502(CodeAnalysis);
			//CodeAnalysis.OnMachineFrameStart();

			int audioSampleCount = 0;
			pCore->RunToVBlank(pFrameBuffer, pAudioBuf, &audioSampleCount);

			if (pAsmExportValidator && GetPCEGlobalConfig()->bUseAsmExportValidator)
				pAsmExportValidator->Tick();

			CodeAnalysis.OnFrameEnd();
			//CodeAnalysis.OnMachineFrameStart();
		}
	}

	UpdatePalettes();

	DetectDirtyBanks();

	// Draw UI
	DrawDockingView();
}

void FPCEEmu::ResetProject()
{
	LOGINFO("Resetting project");

	pCurrentProjectConfig = nullptr;
	pGameDebugStats = nullptr;
	pGameDbEntry = nullptr;

	memset(pFrameBuffer, 0, kFramebufferSize);

	// Initialise code analysis
	CodeAnalysis.Init(this);

	if (pAsmExportValidator)
		pAsmExportValidator->Reset(GetPCEGlobalConfig()->bUseAsmExportValidator);

	GetGlobalsViewer()->Reset();
	if (pVRAMState)
		pVRAMState->Reset();
	if (pSpriteViewer)
		pSpriteViewer->ResetForGame();
	pRecentMemoryAccess->Reset();

	CodeAnalysis.ViewState[0].Enabled = true;

	InitPalettes();
}

// Reset the code analysis to a default state.
// This will be called if a project fails to load, to prevent the code analysis state
// being in a volatile or partially setup state.
void FPCEEmu::Reset()
{
	// Reset all the emulator submodules.
	// I don't know if this is the right thing to do.
	// I was calling pCore->ResetMedia() but that doesn't reset any of this stuff if the media
	// is not loaded.
	
	// Kick the media out first. Memory::Reset() depends on there being no media loaded.
	pMedia->Reset();

	// Memory::Reset() needs to be called to set the mpr registers to point to UNUSED memory banks.
	pMemory->Reset();
	pCore->GetHuC6202()->Reset(false);
	pCore->GetHuC6260()->Reset();
	pCore->GetHuC6270_1()->Reset();
	pCore->GetHuC6270_2()->Reset();
	pCore->GetHuC6280()->Reset();
	pCore->GetCDROM()->Reset();
	pCore->GetScsiController()->Reset();
	pCore->GetCDROMAudio()->Reset();
	pCore->GetAdpcm()->Reset();
	pCore->GetAudio()->Reset(pMedia->IsCDROM());
	pCore->GetInput()->Reset();

	ResetProject();

	// MapMprBanks relies on the mpr registers being setup correctly before it is called.
	ResetBanks();
	MapMprBanks();
	assert(MprBankIdsAreValid());
	
	GenerateGlobalInfo(CodeAnalysis);
	CodeAnalysis.Debugger.Break();
	CodeAnalysis.Debugger.SetPC(0);

	const std::string windowTitle = kAppTitle + " - " + "No project loaded";
	SetWindowTitle(windowTitle.c_str());
}

// Reset the emulation without losing reverse engineering progress.
// Functions/labels are untouched.
// Disassembly is untouched.
void FPCEEmu::SoftResetMachine()
{
	assert(pCurrentProjectConfig);

	pCore->ResetMedia(false);

	if (pVRAMState)
		pVRAMState->Reset();
	
	pRecentMemoryAccess->Reset();

	memset(pFrameBuffer, 0, kFramebufferSize);

	ResetReferenceInfo(CodeAnalysis, true, true);
	ResetReadWriteCounts(CodeAnalysis, true, true);
	ResetExecutionCounts(CodeAnalysis);

	// MapMprBanks relies on the mpr registers being setup correctly before it is called.
	ResetBanks();
	MapMprBanks();
	assert(MprBankIdsAreValid());

	GenerateGlobalInfo(CodeAnalysis);

	CodeAnalysis.Debugger.GetCallstack().clear();
	CodeAnalysis.Debugger.SetPC(CodeAnalysis.GetCanonicalAddressRef(p6280State->PC->GetValue()));
}

void FPCEEmu::OnEnterEditMode(void)
{
	/*
	pCore->SaveState(nullptr, EditModeBackupStateSize);
	free(pEditModeBackupState);
	pEditModeBackupState = (uint8_t*)malloc(EditModeBackupStateSize);
	pCore->SaveState(pEditModeBackupState, EditModeBackupStateSize);
	*/
}

void FPCEEmu::OnExitEditMode(void)
{
	// This is disabled in EmuBase because I think there is a bug with it (needs confirmation).
	// Note from Claude: "This restores raw hardware state (CPU regs, RAM, MPR registers) but never resyncs the analyser-side 
	// bookkeeping (MprBankId/MprBankSet/FBankSet::SlotBankId) afterward — unlike LoadProject, which 
	// explicitly does ResetBanks(); MapMprBanks(); right after its own raw-state restore (PCEEmu.cpp:2424-2427)
	// Spectrum and CPC have the identical gap — zx_load_snapshot/cpc_load_snapshot restore raw state without resyncing their 
	// own bank caches (CurROMBank/CurRAMBank)."

	if (pEditModeBackupState != nullptr)
	{
		pCore->LoadState(pEditModeBackupState, EditModeBackupStateSize);
		
		// potential fix:
		// Resync MprBankId/MprBankSet/FBankSet::SlotBankId to the just-restored
		// MPR hardware registers — same pairing LoadProject uses after LoadMachineState().
		/*ResetBanks();
		MapMprBanks();*/
	}
}


void FPCEEmu::DrawEmulatorUI()
{
}

FGraphicsView* FPCEEmu::GetScreen() const
{
	return nullptr;
}

const uint8_t* FPCEEmu::GetScreenBuffer(int& width, int& height) const
{
	GG_Runtime_Info info;
	pCore->GetRuntimeInfo(info);
	width  = info.screen_width;
	height = info.screen_height;
	return pFrameBuffer;
}


void FPCEEmu::AppFocusCallback(int focused)
{
	if (focused)
	{
		for(auto& listIt : GamesLists)
		{ 
			listIt.second.EnumerateGames();
		}
	}
}

void FPCEEmu::InitPalettes()
{
	ClearPalettes();

	uint32_t palette[32] = { 0 };
	// Create a palette entry for all the HW palettes
	for (int i = 0; i < 32; i++)
	{
		for (int c = 0; c < 16; c++)
		{
			palette[c] = i;
		}
		
		// Get/create a palette.
		// This wont create a new palette if the colours are the same.
		const int p = GetPaletteNo(palette, 16);
	}
}

void FPCEEmu::UpdatePalettes()
{
	HuC6260* huc6260 = pCore->GetHuC6260();
	// This colour table has 333 colours
	u16* colorTable = huc6260->GetColorTable();
	
	for (int p = 0; p < 32; p++)
	{
		const u16* pPalette = &colorTable[p * 16];

		uint32_t* pColours = GetPaletteFromPaletteNo(p);
		if (pColours)
		{
			for (int c = 0; c < 16; c++)
			{
				// convert from 333 to u32 RGBA
				const int colour333 = pPalette[c];
				const uint8_t g = ((colour333 >> 6) & 0x07) * 255 / 7;
				const uint8_t r = ((colour333 >> 3) & 0x07) * 255 / 7;
				const uint8_t b = (colour333 & 0x07) * 255 / 7;
				pColours[c] = (0xff << 24) | (b << 16) | (g << 8) | r;
			}
		}
	}
}

// Create a static user palette.
// Will be created as copy of a dynamic palette index.
int FPCEEmu::CreateUserPalette(int dynamicPaletteIndex)
{
	const uint32_t* pPalette = GetPaletteFromPaletteNo(dynamicPaletteIndex);
	if (!pPalette)
		return dynamicPaletteIndex;
	// Copy before calling AddNewPaletteEntry — push_back may reallocate g_PaletteColours,
	// invalidating the pointer returned by GetPaletteFromPaletteNo.
	uint32_t paletteCopy[16];
	memcpy(paletteCopy, pPalette, 16 * sizeof(uint32_t));
	return AddNewPaletteEntry(paletteCopy, 16);
}

void FPCEEmu::DetectDirtyBanks()
{
	if (!CodeAnalysis.Debugger.IsStopped())
	{
		for (int i = 0; i < kNumMprSlots; i++)
		{
			if (MprBankId[i] != MprBankIdPrev[i])
			{
				CodeAnalysis.SetAddressRangeDirty();
				break;
			}
		}
	}

	memcpy(MprBankIdPrev, MprBankId, sizeof(MprBankId));
}

void FPCELaunchConfig::ParseCommandline(int argc, char** argv)
{
	FEmulatorLaunchConfig::ParseCommandline(argc,argv);	// call base class

	/*
	std::vector<std::string> argList;
	for (int arg = 0; arg < argc; arg++)
	{
		argList.emplace_back(argv[arg]);
	}

	auto argIt = argList.begin();
	argIt++;	// skip exe name
	while (argIt != argList.end())
	{
		if (*argIt == std::string("-128"))
		{
			Model = ESpectrumModel::Spectrum128K;
		}
		else if (*argIt == std::string("-game"))
		{
			if (++argIt == argList.end())
			{
				LOGERROR("-game : No game specified");
				break;
			}
			SpecificGame = *argIt;
		}
		else if (*argIt == std::string("-skoolfile"))
		{
			if (SpecificGame.empty())
			{
				LOGERROR("-skoolfile : A game must be specified with the -game argument.");
				break;
			}

			if (++argIt == argList.end())
			{
				LOGERROR("-skoolfile : No skoolkit file specified");
				break;
			}
			SkoolkitImport = *argIt;
		}

		++argIt;
	}*/
}

/*
// todo move somewhere else & include header
extern "C" int stbi_write_png(char const* filename, int x, int y, int comp, const void* data, int stride_bytes);
bool WriteScreenshot(FPCEEmu* pEmu, const char* pFilename)
{
	const int width = pEmu->GetCore()->GetHuC6260()->GetCurrentWidth();
	const int height = pEmu->GetCore()->GetHuC6260()->GetCurrentHeight();
	stbi_write_png(pFilename, width, height, 4, pEmu->GetFrameBuffer(), width * 4);
}
*/

// todo wire this up to a menu option (debug only)
bool bGGDebugLogsEnabled = false;

// A function to pipe geargrafx logs through our debug logger
// 0 : INFO
// 1 : DEBUG
void GGLogFunc(int level, const char* str)
{
	if (level == 0)
		LOGINFO("[GG] %s", str);
	else if (level == 1 && bGGDebugLogsEnabled)
		LOGDEBUG("[GG] %s", str);
}