#include <imgui.h>

#include "PCEEmu.h"

#include <chrono>

#include "PCEConfig.h"
//#include "GameData.h"
#include "Util/FileUtil.h"
#include "Viewers/PCEViewer.h"
#include "Viewers/BatchGameLoadViewer.h"
#include "Viewers/PCERegistersViewer.h"
#include <geargrafx_core.h>
#include "Debug/DebugLog.h"

#include "App.h"
#include <CodeAnalyser/CodeAnalysisState.h>
#include "CodeAnalyser/CodeAnalysisJson.h"
#include "PCEGameConfig.h"

#include "optick/optick.h"

const char* kGlobalConfigFilename = "GlobalConfig.json";
const std::string kAppTitle = "PCE Analyser";

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

	return pCore->GetMemory()->Read(address, /* internal */ true);
}

uint16_t	FPCEEmu::ReadWord(uint16_t address) const 
{
	if (!pMedia->IsReady())
		return 0;

	return ReadByte(address) | (ReadByte(address + 1) << 8);
}

const uint8_t* FPCEEmu::GetMemPtr(uint16_t address) const 
{
	// this is needed for graphic related functionality.
	static uint8_t byteArray[65535] = { 0 };
	return byteArray;
}

void FPCEEmu::WriteByte(uint16_t address, uint8_t value)
{
	if (!pMedia->IsReady())
		return;

	return pCore->GetMemory()->Write(address, value);
}

FAddressRef	FPCEEmu::GetPC(void) 
{
	return CodeAnalysis.AddressRefFromPhysicalAddress(pCore->GetHuC6280()->GetState()->PC->GetValue());
} 

uint16_t	FPCEEmu::GetSP(void)
{
	return 0;
	//return ZXEmuState.cpu.sp;
}

ICPUEmulator* FPCEEmu::GetCPUEmulator(void) const
{
	return pPCE6502CPU;
}

// This is a geargfx specific version of FDebugger::Tick()
void OnGearGfxInstructionExecuted(void* pContext, uint16_t pc)
{
	//OPTICK_EVENT();

	FPCEEmu* pEmu = static_cast<FPCEEmu*>(pContext);
	FCodeAnalysisState& state = pEmu->GetCodeAnalysis();

	FAddressRef pcAddr = state.AddressRefFromPhysicalAddress(pc);
	state.Debugger.SetPC(pcAddr);

	RegisterCodeExecuted(state, pc, pc);

	// this is a hack. OnInstructionExecuted() is chips specific so we pass in a dummy pins value. 
	const uint64_t dummyPins = 0;
	const int trapId = state.Debugger.OnInstructionExecuted(dummyPins);

	if (trapId != kTrapId_None)
	{
		// This signals to geargfx to stop exection
		state.Debugger.Break();
	}
}

void OnMemoryRead(void* pContext, u16 pc, u16 dataAddr)
{
	//OPTICK_EVENT();

	FPCEEmu* pEmu = static_cast<FPCEEmu*>(pContext);
	RegisterDataRead(pEmu->GetCodeAnalysis(), pc, dataAddr);
}

void OnMemoryWritten(void* pContext, u16 pc, u16 dataAddr, u8 value)
{
	//OPTICK_EVENT();

	FPCEEmu* pEmu = static_cast<FPCEEmu*>(pContext);
	RegisterDataWrite(pEmu->GetCodeAnalysis(), pc, dataAddr, value);
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

	pEmu->MapMprBank(mprIndex, newBankIndex);
}

uint8_t gDummyMemory[0x2000];

std::string GetBankType(Memory* pMemory, uint8_t bankIndex)
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
}

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

void FPCEEmu::MapMprBank(uint8_t mprIndex, uint8_t newBankIndex)
{
	FCodeAnalysisState& state = CodeAnalysis;

	// Get the bank id of the bank we are about to map in.
	const uint16_t bankId = GetBankForMprSlot(newBankIndex, mprIndex);
	FCodeAnalysisBank* pInBank = GetCodeAnalysis().GetBank(bankId);
#ifdef BANK_SWITCH_DEBUG
	const FCodeAnalysisBank* pOutBank = GetCodeAnalysis().GetBank(MprBankId[mprIndex]);
#endif

	assert(pInBank);
	if (!pInBank)
		return;

	const uint16_t oldMappedAddress = pInBank->GetMappedAddress();
	const int oldPrimaryPage = pInBank->PrimaryMappedPage;
	const EBankAccess bankAccess = pMemory->GetMemoryMapWrite()[newBankIndex] ? EBankAccess::ReadWrite : EBankAccess::Read;
	state.MapBank(bankId, mprIndex * 8, bankAccess);
	pInBank->PrimaryMappedPage = mprIndex * 8;
	MprBankId[mprIndex] = bankId;

#ifdef BANK_SWITCH_DEBUG
	for (int i = 0; i < 8; i++)
	{
		if (pMemory->GetMpr(i) != 0xff)
		{
			const FCodeAnalysisBank* pBank = GetCodeAnalysis().GetBank(MprBankId[i]);
			//assert(pBank);
			if (pBank)
			{
				if (pBank->Memory != pMemory->GetMemoryMap()[pMemory->GetMpr(i)])
					BANK_ERROR("Mpr slot %d bank memory does not match geargfx", i);
			}
		}
	}

	BANK_LOG("IN: '%s' 0x%x->0x%x OUT: '%s'", pInBank->Name.c_str(), oldMappedAddress, pInBank->GetMappedAddress(), pOutBank ? pOutBank->Name.c_str() : "None");

	int b = 0;
	for (int addrVal = 0; addrVal < 0xffff; addrVal += 0x2000, b++)
	{
		FCodeAnalysisBank* pBank = CodeAnalysis.GetBank(GetCodeAnalysis().GetBankFromAddress(addrVal));
		if (pBank)
			BANK_LOG("%d Address 0x%04x: Bank Id %03d '%-7s'. %d%s", b, addrVal, pBank->Id, pBank->Name.c_str(), pBank->PrimaryMappedPage, b == mprIndex ? " <---" : "");
		else
			BANK_LOG("%d Address 0x%04x: no bank mapped",b, addrVal);
	}

	LogDupeMprBankIds();

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

// should this return bank ptr?
int16_t FPCEEmu::GetBankForMprSlot(uint8_t bankIndex, uint8_t mprIndex)
{
	if (Banks[bankIndex] == nullptr)
		return -1;

	// todo get this working
	// we need to decrement it somewhere.
	//Banks[bankIndex]->NumBanksInUse++;
	 
	// rename banks to #1 #2...?
	return Banks[bankIndex]->BankIds[mprIndex];
}

void FPCEEmu::LogDupeMprBankIds()
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
					}
					bDupe[i] = true;
				}
			}
		}
	}
}

void FPCEEmu::CheckPhysicalMemoryRangeIsMapped()
{
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
	pCore->SetInstructionExecutedCallback(OnGearGfxInstructionExecuted, this);
	pMemory->SetMemoryCallbacks(OnMemoryRead, OnMemoryWritten, BankChangeCallback, this);

	pMedia = pCore->GetMedia();

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
	//CodeAnalysis.Config.CharacterColourLUT = FZXGraphicsView::GetColourLUT();
	
	// Hardware page. (IO)
	// todo: hardware page is mpr slot 0xff. for now map some dummy memory until we figure out how to do it.
	for (int d = 0; d < kNumMprSlots; d++)
		BankSets[0xff].BankIds[d] = CodeAnalysis.CreateBank("HW PAGE", 8, gDummyMemory, false /*bMachineROM*/, 0x0);

	// Unused banks
	for (int d = 0; d < kNumMprSlots; d++)
		BankSets[0x80].BankIds[d] = CodeAnalysis.CreateBank("UNUSED", 8, pMemory->GetUnusedMemory(), false /*bMachineROM*/, 0x0);
	
	// Working RAM
	for (int d = 0; d < kNumMprSlots; d++)
		BankSets[0xf8].BankIds[d] = CodeAnalysis.CreateBank("WRAM", 8, pMemory->GetWorkingRAM(), false /*bMachineROM*/, 0x2000);
	
	// Save RAM
	for (int d = 0; d < kNumMprSlots; d++)
		BankSets[0xf7].BankIds[d] = CodeAnalysis.CreateBank("SAVE RAM", pMemory->GetBackupRAMSize() / 1024, pMemory->GetBackupRAM(), false /*bMachineROM*/, 0x0);
	
	for (int d = 0x80; d < kNumBanks; d++)
		Banks[d] = &BankSets[0x80];

	Banks[0xff] = &BankSets[0xff];
	Banks[0xf7] = &BankSets[0xf7];
	Banks[0xf8] = &BankSets[0xf8];
	Banks[0xf9] = &BankSets[0xf8];
	Banks[0xfa] = &BankSets[0xf8];
	Banks[0xfb] = &BankSets[0xf8];

	char bankName[32];
	u8* pUnusedMem = pMemory->GetUnusedMemory();

	for (int b = 0; b < kNumRomBanks; b++)
	{
		for (int d = 0; d < kNumMprSlots; d++)
		{
			sprintf(bankName, "ROM %02d", b);
			BankSets[b].BankIds[d] = CodeAnalysis.CreateBank(bankName, 8, pUnusedMem, false /*bMachineROM*/, 0x0000);
		}
	}

	ResetBanks();

	// todo: move to loadproject
	//CheckMemoryMap();

	CodeAnalysis.Config.bShowBanks = true;
	CodeAnalysis.ViewState[0].Enabled = true;	// always have first view enabled
	// set supported bitmap format
	/*CodeAnalysis.Config.bSupportedBitmapTypes[(int)EBitmapFormat::Bitmap_1Bpp] = true;
	for (int i = 0; i < FCodeAnalysisState::kNoViewStates; i++)
	{
		CodeAnalysis.ViewState[i].CurBitmapFormat = EBitmapFormat::Bitmap_1Bpp;
	}*/

	
	const FPCEConfig* pPCEConfig = GetPCEGlobalConfig();
	AddGamesList("Snapshot File", GetPCEGlobalConfig()->SnapshotFolder.c_str());

	LoadFont();

	// This is where we add the viewers we want
	pPCEViewer = new FPCEViewer(this);
	AddViewer(pPCEViewer);
#ifndef NDEBUG
	pBatchGameLoadViewer = new FBatchGameLoadViewer(this);
	AddViewer(pBatchGameLoadViewer);
#endif
	AddViewer(new FPCERegistersViewer(this));

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
	else
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
	const int romSize = pMedia->GetROMSize();
	const int romBankCount = (romSize / 0x2000) + (romSize % 0x2000 ? 1 : 0);

	// Patch in the rom memory into the rom banks
	for (int bankNo = 0; bankNo < romBankCount; bankNo++)
	{
		uint8_t* pMemory = pMedia->GetROMMap()[bankNo];
		for (int d = 0; d < 8; d++)
		{
			FCodeAnalysisBank* pBank = CodeAnalysis.GetBank(BankSets[bankNo].BankIds[d]);
			pBank->Memory = pMemory;
		}
	}

	// Set initial rom banks.
	// todo: explain non sequential nature of rom banks.
	for (int bankNo = 0; bankNo < 128; bankNo++)
	{
		const int bankIndex = romBankCount ? pCore->GetMedia()->GetRomBankIndex(bankNo) : bankNo;
		BankSets[bankNo].NumBanksInUse = 0;
		Banks[bankNo] = &BankSets[bankIndex];
	}

	for (auto& bank : CodeAnalysis.GetBanks())
	{
		bank.PrimaryMappedPage = -1;
		bank.Mapping = EBankAccess::None;
		bank.bEverBeenMapped = false;
	}

	// Go through each mpr slot and map a bank for each one
	for (int mprNum = 0; mprNum < 8; mprNum++)
	{
		MapMprBank(mprNum, pMemory->GetMpr(mprNum));
	}

#ifndef NDEBUG
	LogDupeMprBankIds();
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

bool FPCEEmu::LoadProject(FProjectConfig* pGameConfig, bool bLoadGameData /* =  true*/)
{
	assert(pGameConfig != nullptr);
	FPCEGameConfig *pPCEGameConfig = (FPCEGameConfig*)pGameConfig;
	
	const std::string windowTitle = kAppTitle + " - " + pGameConfig->Name;
	SetWindowTitle(windowTitle.c_str());
	
	// Initialise code analysis
	CodeAnalysis.Init(this);

	// Set options from config
	for (int i = 0; i < FCodeAnalysisState::kNoViewStates; i++)
	{
		CodeAnalysis.ViewState[i].Enabled = pGameConfig->ViewConfigs[i].bEnabled;
		CodeAnalysis.ViewState[i].GoToAddress(pGameConfig->ViewConfigs[i].ViewAddress);
	}

	bool bLoadSnapshot = pGameConfig->EmulatorFile.FileName.empty() == false;

	// Are we loading a previously saved game
	if (bLoadGameData)
	{
		const std::string root = pGlobalConfig->WorkspaceRoot;

		std::string analysisJsonFName = root + "AnalysisJson/" + pGameConfig->Name + ".json";
		std::string graphicsSetsJsonFName = root + "GraphicsSets/" + pGameConfig->Name + ".json";
		std::string analysisStateFName = root + "AnalysisState/" + pGameConfig->Name + ".astate";
		std::string saveStateFName = root + "SaveStates/" + pGameConfig->Name + ".state";

		// check for new location & adjust paths accordingly
		const std::string gameRoot = pGlobalConfig->WorkspaceRoot + pGameConfig->Name + "/";
		if (FileExists((gameRoot + "Config.json").c_str()))	
		{
			analysisJsonFName = gameRoot + "Analysis.json";
			graphicsSetsJsonFName = gameRoot + "GraphicsSets.json";
			analysisStateFName = gameRoot + "AnalysisState.bin";
			saveStateFName = gameRoot + "SaveState.bin";
		}

		if (LoadMachineState(saveStateFName.c_str()))
		{
			// if the game state loaded then we don't need the snapshot
			bLoadSnapshot = false;
		}

		if (FileExists(analysisJsonFName.c_str()))
		{
			ImportAnalysisJson(CodeAnalysis, analysisJsonFName.c_str());
			ImportAnalysisState(CodeAnalysis, analysisStateFName.c_str());
		}

		//pGraphicsViewer->LoadGraphicsSets(graphicsSetsJsonFName.c_str());
	}

	if (bLoadSnapshot)
	{
		// if the game state didn't load then reload the snapshot
		/*/const FGameSnapshot* snapshot = &CurrentGameSnapshot;//GamesList.GetGame(RemoveFileExtension(pGameConfig->SnapshotFile.c_str()).c_str());
		if (snapshot == nullptr)
		{
			SetLastError("Could not find '%s%s'",pGlobalConfig->SnapshotFolder.c_str(), pGameConfig->SnapshotFile.c_str());
			return false;
		}*/
		if (!LoadEmulatorFile(&pGameConfig->EmulatorFile))
		{
			return false;
		}
	}

	ResetBanks();

	ReAnalyseCode(CodeAnalysis);
	GenerateGlobalInfo(CodeAnalysis);
	CodeAnalysis.SetAddressRangeDirty();

	// decode whole screen
	//ZXDecodeScreen(&ZXEmuState);
	CodeAnalysis.Debugger.Break();

	//CodeAnalysis.Debugger.RegisterNewStackPointer(ZXEmuState.cpu.sp, FAddressRef());

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

static const uint32_t kMachineStateMagic = 0xFaceCafe;
//static c64_t g_SaveSlot;

bool FPCEEmu::SaveMachineState(const char* fname)
{
	/*
	// save game snapshot
	FILE* fp = fopen(fname, "wb");
	if (fp != nullptr)
	{
		const uint32_t versionNo = c64_save_snapshot(&C64Emu, &g_SaveSlot);
		fwrite(&kMachineStateMagic, sizeof(uint32_t), 1, fp);
		fwrite(&versionNo, sizeof(uint32_t), 1, fp);
		fwrite(&g_SaveSlot, sizeof(c64_t), 1, fp);

		// Cartridges
		CartridgeManager.SaveData(fp);

		fclose(fp);
		return true;
	}*/

	return false;
}

bool FPCEEmu::LoadMachineState(const char* fname)
{
	bool bSuccess = false;
	/*
	FILE* fp = fopen(fname, "rb");
	if (fp == nullptr)
		return false;

	uint32_t magic;
	uint32_t versionNo;
	fread(&magic, sizeof(uint32_t), 1, fp);
	if (magic == kMachineStateMagic)
	{
		fread(&versionNo, sizeof(uint32_t), 1, fp);
		fread(&g_SaveSlot, sizeof(c64_t), 1, fp);

		bSuccess = c64_load_snapshot(&C64Emu, versionNo, &g_SaveSlot);

		const ELoadDataResult res = CartridgeManager.LoadData(fp);
		switch (res)
		{
		case ELoadDataResult::OK:
			LoadedFileType = EC64FileType::Cartridge;
			break;
		case ELoadDataResult::NotFound:
			break;
		case ELoadDataResult::InvalidData:
			bSuccess = false;
			break;
		}
	}
	fclose(fp);*/
	return bSuccess;
}

// save config & data
bool FPCEEmu::SaveProject()
{
	if (pCurrentProjectConfig == nullptr)
		return false;

	const std::string root = pGlobalConfig->WorkspaceRoot + pCurrentProjectConfig->Name + "/";
	const std::string configFName = root + "Config.json";
	const std::string analysisJsonFName = root + "Analysis.json";
	const std::string graphicsSetsJsonFName = root + "GraphicsSets.json";
	const std::string analysisStateFName = root + "AnalysisState.bin";
	const std::string saveStateFName = root + "SaveState.bin";
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

	// The Future
	SaveMachineState(saveStateFName.c_str());
	ExportAnalysisJson(CodeAnalysis, analysisJsonFName.c_str());
	ExportAnalysisState(CodeAnalysis, analysisStateFName.c_str());
	//pGraphicsViewer->SaveGraphicsSets(graphicsSetsJsonFName.c_str());

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

	FDebugger& debugger = CodeAnalysis.Debugger;
	if (debugger.IsStopped() == false)
	{
		CodeAnalysis.OnFrameStart();

		int audioSampleCount = 0;
		pCore->RunToVBlank(pFrameBuffer, pAudioBuf, &audioSampleCount);
		
		CodeAnalysis.OnFrameEnd();
	}

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
}

void FPCEEmu::OnExitEditMode(void)
{
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
