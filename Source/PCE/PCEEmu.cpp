#include <imgui.h>

#include "PCEEmu.h"

#include <chrono>

#include "PCEConfig.h"
#include "Util/FileUtil.h"
#include "Util/GraphicsView.h"
#include "Viewers/PCEViewer.h"
#include "Viewers/BatchGameLoadViewer.h"
#include "Viewers/BackgroundViewer.h"
#include "Viewers/DebugStatsViewer.h"
#include "Viewers/JoypadViewer.h"
#include "Viewers/PaletteViewer.h"
#include "Viewers/SpriteViewer.h"
#include "Viewers/VRAMViewer.h"
#include "Viewers/PCEGraphicsViewer.h"
#include "CodeAnalyser/UI/OverviewViewer.h"
#include "CodeAnalyser/UI/GlobalsViewer.h"
#include "Viewers/PCERegistersViewer.h"
#include <geargrafx_core.h>
#include "Debug/DebugLog.h"

#include "App.h"
#include <CodeAnalyser/CodeAnalysisState.h>
#include "CodeAnalyser/CodeAnalysisJson.h"
#include "PCEGameConfig.h"

#include "optick/optick.h"

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

#ifndef NDEBUG
//#define BANK_SWITCH_DEBUG
#endif
#ifdef BANK_SWITCH_DEBUG
#define BANK_LOG(...)  LOGINFO("[BNK] " __VA_ARGS__)
#define BANK_ERROR(...)  LOGERROR("[BNK] " __VA_ARGS__)
#else
#define BANK_LOG(...)
#define BANK_ERROR(...)
#endif


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
	HuC6280::HuC6280_State* p6280State = nullptr;
	FPCEEmu* pPCEEmu = nullptr;
};

// Hack to fix undefined symbol linker error.
// This function is used in Debugger.cpp
bool z80_opdone(z80_t* cpu) 
{
	return false;
}

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
	return CodeAnalysis.AddressRefFromPhysicalAddress(p6280State->PC->GetValue());
} 

uint16_t	FPCEEmu::GetSP(void)
{
	return 0x2000 + p6280State->S->GetValue();
}

ICPUEmulator* FPCEEmu::GetCPUEmulator(void) const
{
	return pPCE6502CPU;
}

// This is a geargfx specific version of FDebugger::Tick()
void OnInstructionExecuted(void* pContext, uint16_t pc)
{
	//OPTICK_EVENT();

	FPCEEmu* pEmu = static_cast<FPCEEmu*>(pContext);
	pEmu->OnInstructionExecuted(pc);
}

void OnMemoryRead(void* pContext, u16 dataAddr)
{
	//OPTICK_EVENT();

	FPCEEmu* pEmu = static_cast<FPCEEmu*>(pContext);
	RegisterDataRead(pEmu->GetCodeAnalysis(), pEmu->PrevPC, dataAddr);
}

void OnMemoryWritten(void* pContext, u16 dataAddr, u8 value)
{
	//OPTICK_EVENT();

	FPCEEmu* pEmu = static_cast<FPCEEmu*>(pContext);
	RegisterDataWrite(pEmu->GetCodeAnalysis(), pEmu->PrevPC, dataAddr, value);
}

void BankChangeCallback(void* pContext, u8 mprIndex, u8 oldBankIndex, u8 newBankIndex)
{
	OPTICK_EVENT();
	FPCEEmu* pEmu = static_cast<FPCEEmu*>(pContext);

	BANK_LOG("Map bank index 0x%x to mpr slot %d. [0x%x->0x%x]", newBankIndex, mprIndex, oldBankIndex, newBankIndex);

	if (oldBankIndex == newBankIndex)
	{
		BANK_LOG("Bank index has not changed. Doing nothing.");
		return;
	}

	const int bankSetIndex = pEmu->MprBankSet[mprIndex];
	if (bankSetIndex != -1)
		pEmu->Banks[bankSetIndex]->SetBankFreed(mprIndex);

	pEmu->MapMprBank(mprIndex, newBankIndex);
}

void OnVRAMWritten(void* pContext, u16 vramAddr, u16 value)
{
	//OPTICK_EVENT();

	FPCEEmu* pEmu = static_cast<FPCEEmu*>(pContext);
	pEmu->OnVRAMWritten(vramAddr, value);

}

void FPCEEmu::OnVRAMWritten(uint16_t vramAddr, uint16_t value)
{
	const uint16_t curInstructionAddr = PrevPC;
	GetVRAMViewer()->RegisterWrite  (vramAddr, GetCodeAnalysis().AddressRefFromPhysicalAddress(curInstructionAddr));

	if (GetCodeAnalysis().Debugger.GetStepMode() == EDebugStepMode::ScreenWrite)
	{
		GetCodeAnalysis().Debugger.Break();
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

// The PC passed in here is the address _after_ the instruction just executed.
void FPCEEmu::OnInstructionExecuted(uint16_t pc)
{
	FCodeAnalysisState& state = GetCodeAnalysis();

	const FAddressRef instrAddr = state.AddressRefFromPhysicalAddress(PrevPC);
	// Set the PC to the address of the instruction just executed
	state.Debugger.SetPC(instrAddr);

// Break when code execution flow moves to a bank of a different type.
// eg going from BIOS to RAM
#if 0
	static int prevBank = -1;
	const int curBank = pc >> 13;
	const int prevBank = PrevPC >> 13;
	if (curBank != prevBank)
	{
		const Memory::MemoryBankType curBankType = pMemory->GetBankType(pMemory->GetMpr(curBank));
		const Memory::MemoryBankType prevBankType = pMemory->GetBankType(pMemory->GetMpr(prevBank));
		if (curBankType != prevBankType)
		{
			const FAddressRef curAddr = state.AddressRefFromPhysicalAddress(pc);
			const uint16_t prevBankId = state.GetBankFromAddress(PrevPC);
			const uint16_t curBankId = curAddr.GetBankId();
			FCodeAnalysisBank* pPrevBank = state.GetBank(prevBankId);
			FCodeAnalysisBank* pCurBank = state.GetBank(curBankId);

			LOGINFO("%s -> %s", pPrevBank->Name.c_str(), pCurBank->Name.c_str());
			state.Debugger.Break();
		}
	}
#endif

	RegisterCodeExecuted(state, pc, pc);

	// this is a hack. OnInstructionExecuted() is chips specific so we pass in a dummy pins value. 
	const uint64_t dummyPins = 0;
	const int trapId = state.Debugger.OnInstructionExecuted(dummyPins);

	if (trapId != kTrapId_None)
	{
		// This signals to geargfx to stop exection
		state.Debugger.Break();
	}

	static int lastVpos = -1;
	const int vpos = GetVPos();
	if (lastVpos != vpos)
	{
		if (vpos == 0)
			state.OnMachineFrameStart();
		// todo: Improve this. I think this won't be right at the end of the frame, but on the scanline before it.
		// Maybe I need to use HuC6270::m_raster_line?
		else if (vpos == pCore->GetHuC6260()->GetTotalLines() - 1)
			state.OnMachineFrameEnd();
	}
	lastVpos = vpos;

	PrevPC = pc;
}

void FPCEEmu::MapMprBank(uint8_t mprIndex, uint8_t newBankIndex)
{
	FCodeAnalysisState& state = CodeAnalysis;

	// Get the bank id of the bank we are about to map in.
	const uint16_t newBankId = GetBankForMprSlot(newBankIndex, mprIndex);
	FCodeAnalysisBank* pInBank = CodeAnalysis.GetBank(newBankId);
	const int16_t outBankId = MprBankId[mprIndex];
	// This can be null if a bank is getting mapped to this slot for the very first time.
	const FCodeAnalysisBank* pOutBank = CodeAnalysis.GetBank(outBankId);

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
	const EBankAccess bankAccess = newBankIndex == kBankHWPage ? EBankAccess::ReadWrite : pMemory->GetMemoryMapWrite()[newBankIndex] ? EBankAccess::ReadWrite : EBankAccess::Read;
	const int pageNo = mprIndex * 8;
	state.MapBank(newBankId, pageNo, bankAccess);
	pInBank->PrimaryMappedPage = pageNo;
	MprBankId[mprIndex] = newBankId;

	// Deal with the case where a RW bank is getting replaced by a Read only bank.
	// MapBank() won't remove the Write mapping of the RW bank, so the RW bank will remain
	// mapped Write only.
	if (pOutBank != pInBank)
	{
		if (pOutBank && pOutBank->IsMapped())
		{
			assert(pOutBank->Mapping == EBankAccess::Write);
			BANK_LOG("Unmapping %d %s because it was still mapped after MapBank()", outBankId, pOutBank->Name.c_str());
			bool bUnMappedOk = state.UnMapBank(outBankId, pageNo, pOutBank->Mapping);
			assert(bUnMappedOk);
		}
	}

#ifndef NDEBUG
	int slotCount = 0;
	for (int i = 0; i < kNumMprSlots; i++)
	{
		if (pMemory->GetMpr(i) == newBankIndex)
			slotCount++;
	}
	if (slotCount > 1)
	{
		if (pCurrentProjectConfig)
		{
			int& perGameCount = DebugStats.GamesWithDupeBanks[pCurrentProjectConfig->Name];
			perGameCount = MAX(perGameCount, slotCount);
			int& perBankCount = DebugStats.BankIdsWithDupes[Banks[newBankIndex]->GetBankId(0)];
			perBankCount = MAX(perBankCount, slotCount);
		}
	}
#endif


#ifdef BANK_SWITCH_DEBUG
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

	BANK_LOG("IN: '%s' OUT: '%s' 0x%x->0x%x", pInBank->Name.c_str(), pOutBank ? pOutBank->Name.c_str() : "None", oldMappedAddress, pInBank->GetMappedAddress());

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
	state.SetAddressRangeDirty();
}

int16_t FPCEEmu::GetBankForMprSlot(uint8_t bankIndex, uint8_t mprIndex)
{
	if (Banks[bankIndex] == nullptr)
		return -1;

	int16_t freeBank = Banks[bankIndex]->GetFreeBank(mprIndex);
	if (freeBank != -1)
	{
		MprBankSet[mprIndex] = bankIndex;
		return freeBank;
	}

	// If we couldnt find a free bank return an unused bank
	freeBank = Banks[kBankUnusedStart]->GetFreeBank(mprIndex);
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

bool FPCEEmu::Init(const FEmulatorLaunchConfig& config)
{
#ifndef NDEBUG
	auto t1 = std::chrono::high_resolution_clock::now();
#endif

	FEmuBase::Init(config);
	
	const FPCELaunchConfig& PCELaunchConfig = (const FPCELaunchConfig&)config;
 
	SetWindowTitle(kAppTitle.c_str());
	SetWindowIcon(GetBundlePath("PCELogo.png"));

	// Initialise Emulator
	pCore = new GeargrafxCore();
	pCore->Init(CodeAnalysis.Debugger.GetDebuggerStoppedPtr());
	
	pMemory = pCore->GetMemory();
	pCore->SetInstructionExecutedCallback(::OnInstructionExecuted, this);
	pMemory->SetMemoryCallbacks(OnMemoryRead, OnMemoryWritten, BankChangeCallback, this);
	pCore->GetHuC6270_1()->SetCallback(::OnVRAMWritten, this);

	pMedia = pCore->GetMedia();
	pVPos = pCore->GetHuC6270_1()->GetState()->VPOS;

	p6280State = pCore->GetHuC6280()->GetState();
	p6270State = pCore->GetHuC6270_1()->GetState();

	pPCE6502CPU = new FPCECPUEmulator6502(this);

	pFrameBuffer = new uint8_t[2048 * 512 * 4];
	pAudioBuf = new int16_t[GG_AUDIO_BUFFER_SIZE];;

	CPUType = ECPUType::HuC6280;

	pGlobalConfig = new FPCEConfig();
	pGlobalConfig->Init();
	pGlobalConfig->Load(kGlobalConfigFilename);
	CodeAnalysis.SetGlobalConfig(pGlobalConfig);
	SetHexNumberDisplayMode(pGlobalConfig->NumberDisplayMode);
	SetNumberDisplayMode(pGlobalConfig->NumberDisplayMode);
	
	// todo: check this is system card 3.0.
	const std::string fullBiosPath = GetPCEGlobalConfig()->BiosPath + GetPCEGlobalConfig()->BiosFilename;
	const bool bLoadedBios = pCore->LoadBios(fullBiosPath.c_str(), true);
	LOGINFO("%s Bios '%s'", bLoadedBios ? "Loaded" : "Failed to load", fullBiosPath.c_str());

	std::string bankPostFix[8] = { "", " #2", " #3", " #4", " #5", " #6", " #7", " #8" };
	char bankName[32];

	// Hardware page. (IO)
	// This is a bit of a hack. We use memory owned by Geargfx.
	// We write values to it every time a hw page location is read or written to.
	// To the user, it should look like a normal memory location in the code analysis view.
	for (int d = 0; d < kNumBankSetIds; d++)
	{
		sprintf(bankName, "HW PAGE%s", bankPostFix[d].c_str());
		BankSets[kBankHWPage].AddBankId(CodeAnalysis.CreateBank(bankName, 8, pCore->GetMemory()->GetHWPageMemory(), false /*bMachineROM*/, 0x0));
	}

	// Working RAM
	for (int d = 0; d < kNumBankSetIds; d++)
	{
		sprintf(bankName, "WRAM%s", bankPostFix[d].c_str());
		BankSets[kBankWRAM0].AddBankId(CodeAnalysis.CreateBank(bankName, 8, pMemory->GetWorkingRAM(), false /*bMachineROM*/, 0x2000));
	}

	// Save RAM
	for (int d = 0; d < kNumBankSetIds; d++)
	{
		sprintf(bankName, "SAVE RAM%s", bankPostFix[d].c_str());
		BankSets[kBankSaveRAM].AddBankId(CodeAnalysis.CreateBank(bankName, pMemory->GetBackupRAMSize() / 1024, pMemory->GetBackupRAM(), false /*bMachineROM*/, kDefaultInitialBankAddr));
	}

	// CD ROM RAM
	u8* pUnusedMem = pMemory->GetUnusedMemory();
	for (int i = 0, b = kBankCdRomRamStart; i < kNumCdRomRamBanks; i++, b++)
	{
		for (int d = 0; d < kNumBankSetIds; d++)
		{
			sprintf(bankName, "CD RAM %d%s", i, bankPostFix[d].c_str());
			BankSets[b].AddBankId(CodeAnalysis.CreateBank(bankName, 8, pUnusedMem, false /*bMachineROM*/, kDefaultInitialBankAddr));
		}
	}

	// move this to reset banks?
	// do I need this any more?
	for (int d = kBankUnusedStart; d < kNumBanks; d++)
		Banks[d] = &BankSets[kBankUnusedStart];

	// ROMs. Create with unused ram initially because the rom memory doesn't exist yet. 
	// The real memory gets set later after the game gets loaded. 
	for (int b = 0; b < kNumRomBanks; b++)
	{
		for (int d = 0; d < kNumBankSetIds; d++)
		{
			sprintf(bankName, "ROM %02d%s", b, bankPostFix[d].c_str());
			BankSets[b].AddBankId(CodeAnalysis.CreateBank(bankName, 8, pUnusedMem, false /*bMachineROM*/, kDefaultInitialBankAddr));
		}
	}

	// Unused banks. Intentionally adding one for each mpr slot.
	for (int d = 0; d < kNumMprSlots; d++)
	{
		sprintf(bankName, "UNUSED %02d", d);
		BankSets[kBankUnusedStart].AddBankId(CodeAnalysis.CreateBank(bankName, 8, pMemory->GetUnusedMemory(), false /*bMachineROM*/, kDefaultInitialBankAddr));
	}

	ResetBanks();

	// todo: move to loadproject
	//CheckMemoryMap();

	CodeAnalysis.Config.bShowBanks = true;
	CodeAnalysis.ViewState[0].Enabled = true;	// always have first view enabled
	
	// set supported bitmap formats
	CodeAnalysis.Config.bSupportedBitmapTypes[(int)EBitmapFormat::Bitmap_1Bpp] = true;
	CodeAnalysis.Config.bSupportedBitmapTypes[(int)EBitmapFormat::Sprite4Bpp_PCE] = true;
	
	for (int i = 0; i < FCodeAnalysisState::kNoViewStates; i++)
	{
		CodeAnalysis.ViewState[i].CurBitmapFormat = EBitmapFormat::Bitmap_1Bpp;
	}

	const FPCEConfig* pPCEConfig = GetPCEGlobalConfig();
	AddGamesList("Snapshot File", GetPCEGlobalConfig()->SnapshotFolder.c_str());

	LoadFont();

	// This is where we add the viewers we want
	pPCEViewer = new FPCEViewer(this);
	AddViewer(pPCEViewer);

	FOverviewViewer* pOverviewViewer = new FOverviewViewer(this);
	pOverviewViewer->SetRomOptionEnabled(false); // this enables showing the entire physical address range.
	AddViewer(pOverviewViewer);
	AddViewer(new FPCERegistersViewer(this));
	AddViewer(new FPaletteViewer(this));
	AddViewer(new FJoypadViewer(this));
	pSpriteViewer = new FSpriteViewer(this);
	AddViewer(pSpriteViewer);
	AddViewer(new FBackgroundViewer(this));
	pVRAMViewer = new FVRAMViewer(this);
	AddViewer(pVRAMViewer);
	pGraphicsViewer = new FPCEGraphicsViewer(this);
	AddViewer(pGraphicsViewer);
#ifndef NDEBUG
	pBatchGameLoadViewer = new FBatchGameLoadViewer(this);
	AddViewer(pBatchGameLoadViewer);
	AddViewer(new FDebugStatsViewer(this));
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
		CodeAnalysis.Init(this);
		
		// if we get here no game is loaded
		// so it doesnt make sense setting the item code.
		// there will be no code in the memory.
		
		// temp. DELETE ME
		/*const FAddressRef initialPC = GetPC();
		SetItemCode(CodeAnalysis, initialPC);*/
		CodeAnalysis.Debugger.SetPC(FAddressRef(MprBankId[0], 0));
		CodeAnalysis.Debugger.Break();
	}
	

	// Setup Debugger
	//FDebugger& debugger = CodeAnalysis.Debugger;
	//debugger.RegisterEventType((int)EEventType::ScreenPixWrite, "Screen Pixel Write", 0xff0000ff, nullptr, EventShowPixValue);
	//debugger.RegisterEventType((int)EEventType::ScreenAttrWrite, "Screen Attr Write", 0xff007fff, nullptr, EventShowAttrValue);
	//debugger.RegisterEventType((int)EEventType::KempstonJoystickRead, "Kempston Read", 0xff007f1f, IOPortEventShowAddress, IOPortEventShowValue);

	// Setup Memory Analyser
	//pMemoryAnalyser->AddROMArea(kROMStart, kROMEnd);
	//pMemoryAnalyser->SetScreenMemoryArea(kScreenPixMemStart, kScreenAttrMemEnd);

#ifndef NDEBUG
	std::chrono::duration<double, std::milli> ms_double = std::chrono::high_resolution_clock::now() - t1;
	LOGINFO("FPCEEmu::Init() took %.2f ms", ms_double);
#endif

	return true;
}

void FPCEEmu::ResetBanks()
{
	const bool bIsCdRom = pMedia->IsCDROM();
	const int romSize = bIsCdRom ? GG_BIOS_SYSCARD_SIZE : pMedia->GetROMSize();
	const int romBankCount = (romSize / 0x2000) + (romSize % 0x2000 ? 1 : 0);

#ifdef BANK_SWITCH_DEBUG
	BANK_LOG("ResetBanks()");
	BANK_LOG("Rom size is %d bytes. Bank count is %d", romSize, romBankCount);
#endif

	for (int bankNo = 0; bankNo < kNumBanks; bankNo++)
	{
		BankSets[bankNo].Reset();
		Banks[bankNo] = &BankSets[kBankUnusedStart];
	}

	if (pMemory->IsBackupRamEnabled())
	{
		Banks[kBankSaveRAM] = &BankSets[kBankSaveRAM];
	}
	
	std::string bankPostFix[8] = { "", " #2", " #3", " #4", " #5", " #6", " #7", " #8" };
	char bankName[32];
	
	if (bIsCdRom)
	{
		// Set cd rom ram banks
		constexpr int kNumCdRomRamEnd = kBankCdRomRamStart + kNumCdRomRamBanks;
		for (int i = 0, b = kBankCdRomRamStart; i < kNumCdRomRamBanks; i++, b++)
		{
			Banks[b] = &BankSets[b];
			uint8_t* pBankMemory = pMemory->GetCDROMRAM() + i * 0x2000;

			for (int d = 0; d < kNumBankSetIds; d++)
			{
				FCodeAnalysisBank* pBank = CodeAnalysis.GetBank(BankSets[b].GetBankId(d));
				pBank->Memory = pBankMemory;
			}
		}
	}

	Banks[kBankHWPage] = &BankSets[kBankHWPage];
	Banks[kBankWRAM0] = &BankSets[kBankWRAM0];
	Banks[kBankWRAM1] = &BankSets[kBankWRAM0];
	Banks[kBankWRAM2] = &BankSets[kBankWRAM0];
	Banks[kBankWRAM3] = &BankSets[kBankWRAM0];

	// Set initial rom banks.
	for (int bankNo = 0; bankNo < 128; bankNo++)
	{
		const int bankIndex = romBankCount ? pCore->GetMedia()->GetRomBankIndex(bankNo) : bankNo;
		Banks[bankNo] = &BankSets[bankIndex];
	}

	if (pMemory->GetCardRAMSize())
	{
		// Set card ram banks
		const uint8_t cardRamStart = pMemory->GetCardRAMStart();
		const uint8_t cardRamEnd = pMemory->GetCardRAMEnd();
		for (uint8_t r = cardRamStart; r <= cardRamEnd; r++)
		{
			uint8_t* pBankMemory = pMemory->GetMemoryMap()[r];
			for (int d = 0; d < kNumBankSetIds; d++)
			{
				FCodeAnalysisBank* pBank = CodeAnalysis.GetBank(BankSets[r].GetBankId(d));
				pBank->Memory = pBankMemory;
				
				sprintf(bankName, "CARD RAM %02d%s", r - cardRamStart, bankPostFix[d].c_str());
				pBank->Name = bankName;
			}
			Banks[r] = &BankSets[r];
		}
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

	// Reset banks for re-use.
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

	// todo: if any code analysis banks are marked as in use then set their primarymappedpage to the default
	
	// Set banks primary mapped page to mark them as in use.
	// They will get their actual mapped address set when they are mapped in.
	// We do this because we can't have any banks in use with PrimaryMappedPage of -1.
	BankSets[kBankHWPage].SetPrimaryMappedPage(CodeAnalysis, 0, 0);
	BankSets[kBankWRAM0].SetPrimaryMappedPage(CodeAnalysis, 0, kDefaultPrimaryMappedPage);
	BankSets[kBankSaveRAM].SetPrimaryMappedPage(CodeAnalysis, 0, kDefaultPrimaryMappedPage);

	// Patch in the rom memory into the rom banks.
	for (int b = 0; b < romBankCount; b++)
	{
		BankSets[b].SetPrimaryMappedPage(CodeAnalysis, 0, kDefaultPrimaryMappedPage);

		uint8_t* pBytes = bIsCdRom ? pMedia->GetSysCardBios() : pMedia->GetROM();
		uint8_t* pBankMemory = pBytes + b * 0x2000;
		for (int d = 0; d < kNumBankSetIds; d++)
		{
			FCodeAnalysisBank* pBank = CodeAnalysis.GetBank(BankSets[b].GetBankId(d));
			pBank->Memory = pBankMemory;

			sprintf(bankName, "%s %02d%s", bIsCdRom ? "BIOS" : "ROM", b, bankPostFix[d].c_str());
			pBank->Name = bankName;

			pBank->bMachineROM = bIsCdRom;
		}
	}

#ifdef BANK_SWITCH_DEBUG
	BANK_LOG("Mapping initial banks...");
	bDoneInitialBankMapping = false;
#endif

	// Go through each mpr slot and map a bank for each one
	for (int mprNum = 0; mprNum < 8; mprNum++)
	{
		MapMprBank(mprNum, pMemory->GetMpr(mprNum));
	}

#ifdef BANK_SWITCH_DEBUG
	bDoneInitialBankMapping = true;
	BANK_LOG("Done mapping initial banks");
#endif

#ifndef NDEBUG
	CheckDupeMprBankIds();
	CheckPhysicalMemoryRangeIsMapped();
#endif
}

void FPCEEmu::Shutdown()
{
	if (pCurrentProjectConfig != nullptr)
	{
		// Save Global Config - move to function?
		pGlobalConfig->LastGame = pCurrentProjectConfig->Name;
		SaveProject();
	}

	pGlobalConfig->Save(kGlobalConfigFilename);

	// destroy emulator here

	FEmuBase::Shutdown();
}

// todo: deal with resetting state when this function fails.
// if ImportAnalysisState() fails when booting up, the game save state will still load instead
// of the memory being reset. 
bool FPCEEmu::LoadProject(FProjectConfig* pGameConfig, bool bLoadGameData /* =  true*/)
{
	LOGINFO("Load Project '%s'. bLoadGameData = %s", pGameConfig->Name.c_str(), bLoadGameData ? "True" : "False");

	assert(pGameConfig != nullptr);
	FPCEGameConfig *pPCEGameConfig = (FPCEGameConfig*)pGameConfig;
	
	const std::string windowTitle = kAppTitle + " - " + pGameConfig->Name;
	SetWindowTitle(windowTitle.c_str());
	
	// Initialise code analysis
	CodeAnalysis.Init(this);
	
	GetGlobalsViewer()->Reset();
	pVRAMViewer->Reset();

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

		if (FileExists(analysisJsonFName.c_str()))
		{
			if (!ImportAnalysisJson(CodeAnalysis, analysisJsonFName.c_str()))
				return false;

			if (!ImportAnalysisState(CodeAnalysis, analysisStateFName.c_str()))
				return false;

			CheckPhysicalMemoryRangeIsMapped();
		}

		//pGraphicsViewer->LoadGraphicsSets(graphicsSetsJsonFName.c_str());
	}
	else
	{
		const bool bHasSnapshot = pGameConfig->EmulatorFile.FileName.empty() == false;
		if (bHasSnapshot)
		{
			/*/const FGameSnapshot* snapshot = &CurrentGameSnapshot;//GamesList.GetGame(RemoveFileExtension(pGameConfig->SnapshotFile.c_str()).c_str());
			if (snapshot == nullptr)
			{
				SetLastError("Could not find '%s%s'",pGlobalConfig->SnapshotFolder.c_str(), pGameConfig->SnapshotFile.c_str());
				return false;
			}*/
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

		// we only want to do this once when create the project
		
		uint32_t palette[32] = { 0 };
		// Create a palette entry for all the HW palettes
		for (int i = 0; i < 32; i++)
		{
			for (int c = 0; c < 16; c++)
			{
				palette[c] = i; 
			}
			// this wont create a new palette if the colours are the same.
			const int p = GetPaletteNo(palette, 16);
			LOGINFO("created palette %d", p);
		}
		LOGINFO("done");
	}

#if IMPORT_BIOS_ANALYSIS_JSON
	if (pMedia->IsCDROM())
	{
		if (FileExists(GetBundlePath(kBiosInfoJsonFile)))
			ImportAnalysisJson(CodeAnalysis, GetBundlePath(kBiosInfoJsonFile));
	}
#else
	AddBiosLabels();
#endif

	ReAnalyseCode(CodeAnalysis);
	GenerateGlobalInfo(CodeAnalysis);
	CodeAnalysis.SetAddressRangeDirty();


	// Add labels for the memory mapped registers. These are locations in the hardware page memory bank. 
	AddLabel(CodeAnalysis, FAddressRef(BankSets[kBankHWPage].GetBankId(0), 0x0), "VDC_AR_0000", ELabelType::Data);
	AddLabel(CodeAnalysis, FAddressRef(BankSets[kBankHWPage].GetBankId(0), 0x2), "VDC_DATA_LO_0002", ELabelType::Data);
	AddLabel(CodeAnalysis, FAddressRef(BankSets[kBankHWPage].GetBankId(0), 0x3), "VDC_DATA_HI_0002", ELabelType::Data);
	
	AddLabel(CodeAnalysis, FAddressRef(BankSets[kBankHWPage].GetBankId(0), 0x400), "VCE_CONTROL_0400", ELabelType::Data);
	AddLabel(CodeAnalysis, FAddressRef(BankSets[kBankHWPage].GetBankId(0), 0x402), "VCE_ADDR_LO_0402", ELabelType::Data);
	AddLabel(CodeAnalysis, FAddressRef(BankSets[kBankHWPage].GetBankId(0), 0x403), "VCE_ADDR_HI_0403", ELabelType::Data);
	AddLabel(CodeAnalysis, FAddressRef(BankSets[kBankHWPage].GetBankId(0), 0x404), "VCE_DATA_LO_0404", ELabelType::Data);
	AddLabel(CodeAnalysis, FAddressRef(BankSets[kBankHWPage].GetBankId(0), 0x405), "VCE_DATA_HI_0405", ELabelType::Data);

	AddLabel(CodeAnalysis, FAddressRef(BankSets[kBankHWPage].GetBankId(0), 0x800), "PSG_CHANSEL_0800", ELabelType::Data);
	AddLabel(CodeAnalysis, FAddressRef(BankSets[kBankHWPage].GetBankId(0), 0x801), "PSG_GLOBALVOL_0801", ELabelType::Data);
	AddLabel(CodeAnalysis, FAddressRef(BankSets[kBankHWPage].GetBankId(0), 0x802), "PSG_FREQLO_0802", ELabelType::Data);
	AddLabel(CodeAnalysis, FAddressRef(BankSets[kBankHWPage].GetBankId(0), 0x803), "PSG_FREQHI_0803", ELabelType::Data);
	AddLabel(CodeAnalysis, FAddressRef(BankSets[kBankHWPage].GetBankId(0), 0x804), "PSG_CHANCTRL_0804", ELabelType::Data);
	AddLabel(CodeAnalysis, FAddressRef(BankSets[kBankHWPage].GetBankId(0), 0x805), "PSG_CHANPAN_0805", ELabelType::Data);
	AddLabel(CodeAnalysis, FAddressRef(BankSets[kBankHWPage].GetBankId(0), 0x806), "PSG_CHANDATA_0806", ELabelType::Data);
	AddLabel(CodeAnalysis, FAddressRef(BankSets[kBankHWPage].GetBankId(0), 0x807), "PSG_NOISE_0807", ELabelType::Data);
	AddLabel(CodeAnalysis, FAddressRef(BankSets[kBankHWPage].GetBankId(0), 0x808), "PSG_LFOFREQ_0808", ELabelType::Data);
	AddLabel(CodeAnalysis, FAddressRef(BankSets[kBankHWPage].GetBankId(0), 0x809), "PSG_LFOCTRL_0809", ELabelType::Data);

	AddLabel(CodeAnalysis, FAddressRef(BankSets[kBankHWPage].GetBankId(0), 0x0C00), "TIMER_COUNTER_0C00", ELabelType::Data);
	AddLabel(CodeAnalysis, FAddressRef(BankSets[kBankHWPage].GetBankId(0), 0x0C01), "TIMER_CONTROL_0C01", ELabelType::Data);

	AddLabel(CodeAnalysis, FAddressRef(BankSets[kBankHWPage].GetBankId(0), 0x1402), "IRQ_DISABLE_1402", ELabelType::Data);
	AddLabel(CodeAnalysis, FAddressRef(BankSets[kBankHWPage].GetBankId(0), 0x1403), "IRQ_STATUS_1403", ELabelType::Data);

	AddLabel(CodeAnalysis, FAddressRef(BankSets[kBankHWPage].GetBankId(0), 0x1000), "JOYPAD_1000", ELabelType::Data);

	DebugStats.Reset();

	CodeAnalysis.Debugger.Break();
	PrevPC = p6280State->PC->GetValue();

	// some extra initialisation for creating new analysis from snapshot
	if(bLoadGameData == false)
	{
		FAddressRef initialPC = GetPC();
		SetItemCode(CodeAnalysis, initialPC);
		CodeAnalysis.Debugger.SetPC(initialPC);
	}

	//pGraphicsViewer->SetImagesRoot((pGlobalConfig->WorkspaceRoot + "/" + pGameConfig->Name + "/GraphicsSets/").c_str());

	pCurrentProjectConfig = pGameConfig;

	LoadLua();
	
	pGlobalConfig->AddProjectToRecents(GetProjectConfig()->Name);
	
	return true;
}

void FPCEEmu::AddBiosLabels()
{
	FCodeAnalysisState& state = GetCodeAnalysis();
	constexpr int kNumBiosFuncs = 77;
	const char* funcNames[kNumBiosFuncs] = {
		"CD_BOOT",		"CD_RESET",		"CD_BASE",		"CD_READ",		"CD_SEEK",		"CD_EXEC",		"CD_PLAY",		"CD_SEARCH",
		"CD_PAUSE",		"CD_STAT",		"CD_SUBQ",		"CD_DINFO",		"CD_CONTNTS",	"CD_SUBRD",		"CD_PCMRD",		"CD_FADE",
		"AD_RESET",		"AD_TRANS",		"AD_READ",		"AD_WRITE",		"AD_PLAY",		"AD_CPLAY",		"AD_STOP",		"AD_STAT",
		"BM_FORMAT",	"BM_FREE",		"BM_READ",		"BM_WRITE",		"BM_DELETE",	"BM_FILES",		"EX_GETVER",	"EX_SETVEC",
		"EX_GETFNT",	"EX_JOYSNS",	"EX_JOYREP",	"EX_SCRSIZ",	"EX_DOTMOD",	"EX_SCRMOD",	"EX_IMODE",		"EX_VMODE",
		"EX_HMODE",		"EX_VSYNC",		"EX_RCRON",		"EX_RCROFF",	"EX_IRQON",		"EX_IRQOFF",	"EX_BGON",		"EX_BGOFF",
		"EX_SPRON",		"EX_SPROFF",	"EX_DSPON",		"EX_DSPOFF",	"EX_DMAMOD",	"EX_SPRDMA",	"EX_SATCLR",	"EX_SPRPUT",
		"EX_SETRCR",	"EX_SETRED",	"EX_SETWRT",	"EX_SETDMA",	"EX_BINBCD",	"EX_BCDBIN",	"EX_RND",		"MA_MUL8U",
		"MA_MUL8S",		"MA_MUL16U",	"MA_DIV16S",	"MA_DIV16U",	"MA_SQRT",		"MA_SIN",		"MA_COS",		"MA_ATNI",
		"PSG_BIOS",		"GRP_BIOS",		"EX_MEMOPEN",	"PSG_DRIVER",	"EX_COLORCMD"
	};

	// Add labels for the jump table. This will be the same for all system card revisions.
	uint16_t baseAddr = 0xe000;
	for (int i = 0; i < kNumBiosFuncs; i++)
	{
		const FAddressRef addr = state.AddressRefFromPhysicalAddress(baseAddr + i * 3);
		SetItemCode(state, addr);
		AddLabel(state, addr, funcNames[i], ELabelType::Function);
	}

	// System Card 3.0 routine addresses.
	// Info taken from https://www.stum.de/2025/pcenginebiosoffsets/
	const uint16_t funcAddrs[kNumBiosFuncs] = {
		0xE0F3, 0xE8E3, 0xEB8F, 0xEC05, 0xEDCB, 0xEBEC, 0xEE10, 0xEF34,
		0xEF94, 0xF347, 0xEFBF, 0xEFF1, 0xF0A9, 0xF354, 0xF364, 0xF379,
		0xF37F, 0xF393, 0xF407, 0xF4D8, 0xF5C6, 0xF61F, 0xF6C1, 0xF6DB,
		0xF858, 0xF8B8, 0xF8E3, 0xF955, 0xFA1A, 0xFA72, 0xF02D, 0xF034,
		0xF124, 0xE49A, 0xE175, 0xE267, 0xE272, 0xE29D, 0xE382, 0xE391,
		0xE3A4, 0xE3B5, 0xE3C7, 0xE3CB, 0xE3CF, 0xE3D3, 0xE3E2, 0xE3E5,
		0xE3E8, 0xE3EB, 0xE3EE, 0xE3F3, 0xE3F8, 0xE40B, 0xE5DA, 0xE63C,
		0xE41F, 0xE42F, 0xE446, 0xE45D, 0xE621, 0xE600, 0xE67E, 0xFDC6,
		0xFDBF, 0xFDD4, 0xFDE2, 0xFDE9, 0xFDF0, 0xFDFE, 0xFDF7, 0xFE05,
		0xFE0C, 0xFE57, 0xFE92, 0xE6CF, 0xE509 
	};

	// Add labels for the routines themselves.
	// Games shouldn't call these directly.
	for (int i = 0; i < kNumBiosFuncs; i++)
	{
		const FAddressRef addr = state.AddressRefFromPhysicalAddress(funcAddrs[i]);
		SetItemCode(state, addr);
		const std::string name = std::string("_") + funcNames[i];
		AddLabel(state, addr, name.c_str(), ELabelType::Function);
	}
}

static const uint32_t kMachineStateMagic = 0xFaceCafe;

bool FPCEEmu::SaveMachineState(const char* fname)
{
	return pCore->SaveState(fname);
}

bool FPCEEmu::LoadMachineState(const char* fname)
{
	return pCore->LoadState(fname);
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
	const std::string saveStateFName = root + "/SaveState.bin";
	EnsureDirectoryExists(root.c_str());

	// set config values
	for (int i = 0; i < FCodeAnalysisState::kNoViewStates; i++)
	{
		const FCodeAnalysisViewState& viewState = CodeAnalysis.ViewState[i];
		FCodeAnalysisViewConfig& viewConfig = pCurrentProjectConfig->ViewConfigs[i];

		viewConfig.bEnabled = viewState.Enabled;
		viewConfig.ViewAddress = viewState.GetCursorItem().IsValid() ? viewState.GetCursorItem().AddressRef : FAddressRef();
	}

	AddGameConfig(pCurrentProjectConfig);
	SaveGameConfigToFile(*pCurrentProjectConfig, configFName.c_str());

	SaveMachineState(root.c_str()/*saveStateFName.c_str()*/);
	ExportAnalysisJson(CodeAnalysis, analysisJsonFName.c_str());
	ExportAnalysisState(CodeAnalysis, analysisStateFName.c_str());
	//pGraphicsViewer->SaveGraphicsSets(graphicsSetsJsonFName.c_str());

#if EXPORT_BIOS_ANALYSIS_JSON
	const std::string romJsonFName = GetBundlePath(kBiosInfoJsonFile);
	ExportAnalysisJson(CodeAnalysis, romJsonFName.c_str(), true);	// export ROMS only
#endif

	return true;
}

bool FPCEEmu::LoadEmulatorFile(const FEmulatorFile* pSnapshot)
{
	auto findIt = GamesLists.find(pSnapshot->ListName);
	if(findIt == GamesLists.end())
		return false;

	const std::string fileName = findIt->second.GetRootDir() + pSnapshot->FileName;

	switch (pSnapshot->Type)
	{
	case EEmuFileType::PCE:
	case EEmuFileType::CUE:
		return pCore->LoadMedia(fileName.c_str());
	default:
		return false;
	}
	return false;
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
		//pNewConfig->Spectrum128KGame = GetCurrentSpectrumModel() == ESpectrumModel::Spectrum128K;
		pNewConfig->EmulatorFile = snapshot;

		AddGameConfig(pNewConfig);
		SaveProject();

		return true;
	}
	return false;
}

void FPCEEmu::FileMenuAdditions(void)	
{
}

void FPCEEmu::SystemMenuAdditions(void)
{
}

void FPCEEmu::OptionsMenuAdditions(void)
{
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
	pVRAMViewer->Tick();

	FDebugger& debugger = CodeAnalysis.Debugger;
	if (debugger.IsStopped() == false)
	{
		CodeAnalysis.OnFrameStart();
		//CodeAnalysis.OnMachineFrameStart();

		int audioSampleCount = 0;
		pCore->RunToVBlank(pFrameBuffer, pAudioBuf, &audioSampleCount);
		
		CodeAnalysis.OnFrameEnd();
		//CodeAnalysis.OnMachineFrameStart();
	}

	UpdatePalettes();

	// Draw UI
	DrawDockingView();
}

void FPCEEmu::Reset()
{
	// this does a soft reset. it preserves the loaded rom.
	pCore->ResetMedia(false);

	// this does a hard reset. it erases the loaded rom
	// this crashed once so might not be stable.
	//pCore->GetMedia()->Reset();

	FPCEGameConfig* pEmptyConfig = (FPCEGameConfig*)GetGameConfigForName("No Project");

	if (pEmptyConfig == nullptr)
		pEmptyConfig = CreateNewEmptyConfig();

	LoadProject(pEmptyConfig, false);	// reset code analysis
}

void FPCEEmu::OnEnterEditMode(void)
{
	// todo call SaveState here with a buffer
	//pCore->SaveState(
}

void FPCEEmu::OnExitEditMode(void)
{
	// todo call LoadState here
}


void FPCEEmu::DrawEmulatorUI()
{
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

void FPCELaunchConfig::ParseCommandline(int argc, char** argv)
{
	/*
	FEmulatorLaunchConfig::ParseCommandline(argc,argv);	// call base class

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

// move this to it's own file?
void FPCEEmu::FBankSet::SetPrimaryMappedPage(FCodeAnalysisState& state, int bankSetIndex, uint16_t pageAddr)
{
	FCodeAnalysisBank* pBank = state.GetBank(Banks[bankSetIndex].BankId);
	assert(pBank);
	pBank->PrimaryMappedPage = pageAddr;
}

int16_t FPCEEmu::FBankSet::GetFreeBank(uint8_t mprSlot)
{
	for (int i = 0; i < Banks.size(); i++)
	{
		FBankSetEntry& entry = Banks[i];
		if (!entry.bMapped)
		{
			entry.bMapped = true;
			assert(SlotBankId[mprSlot] == -1);
			SlotBankId[mprSlot] = i;
			return entry.BankId;
		}
	}

	return -1;
}
	
void FPCEEmu::FBankSet::SetBankFreed(uint8_t mprSlot)
{
	assert(SlotBankId[mprSlot] != -1);
	Banks[SlotBankId[mprSlot]].bMapped = false;
	SlotBankId[mprSlot] = -1;
}
	
void FPCEEmu::FBankSet::Reset()
{
	for (int i = 0; i < kNumMprSlots; i++)
		SlotBankId[i] = -1;
	for (int i = 0; i < Banks.size(); i++)
		Banks[i].bMapped = false;
}
	
void FPCEEmu::FBankSet::AddBankId(int16_t bankId)
{
	Banks.push_back(FBankSetEntry({ bankId, false }));
}
	
int16_t FPCEEmu::FBankSet::GetBankId(int index) const
{
	assert(!Banks.empty());
	if (index >= Banks.size())
		return -1;

	return Banks[index].BankId;
}