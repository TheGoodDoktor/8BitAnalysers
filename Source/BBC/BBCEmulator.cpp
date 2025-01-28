#include "BBCEmulator.h"

#include "BBCConfig.h"
#include <Util/FileUtil.h>
#include <CodeAnalyser/UI/CodeAnalyserUI.h>
#include <imgui.h>
#include <CodeAnalyser/CodeAnalysisJson.h>
#include <CodeAnalyser/CodeAnalysisState.h>

#include "Graphics/BBCGraphicsViewer.h"

const char* kGlobalConfigFilename = "GlobalConfig.json";
const std::string kAppTitle = "BBC Analyser";


const char* kROMAnalysisFilename = "BBCRomsAnalysis.json";

uint32_t FBBCEmulator::ColourPalette[] =
{
	0xff000000,	// Black
	0xff0000ff,	// Red
	0xff00ff00,	// Green
	0xff00ffff,	// Yellow
	0xffff0000,	// Blue
	0xffff00ff,	// Magenta
	0xffffff00,	// Cyan
	0xffffffff,	// White

	// colours 8-15 are flashing, values here are the initial colour
	0xff000000,	// Black/White
	0xff0000ff,	// Red/Cyan
	0xff00ff00,	// Green/Magenta
	0xff00ffff,	// Yellow/Blue
	0xffff0000,	// Blue/Yellow
	0xffff00ff,	// Magenta/Green
	0xffffff00,	// Cyan/Red
	0xffffffff,	// White/Black
};

void SetWindowTitle(const char* pTitle);


void DebugCB(void* user_data, uint64_t pins)
{
	FBBCEmulator* pBBCEmu = (FBBCEmulator*)user_data;
	pBBCEmu->OnCPUTick(pins);
}

class F6502MemDescGenerator : public FMemoryRegionDescGenerator
{
public:
	F6502MemDescGenerator()
	{
		RegionMin = 0x100;
		RegionMax = 0x1ff;	// 16K after
		RegionBankId = -1;
	}

	const char* GenerateAddressString(FAddressRef addr) override
	{
		if (addr.Address == 0x100)
			snprintf(DescStr, sizeof(DescStr), "StackBottom");
		else if (addr.Address == 0x1FF)
			snprintf(DescStr, sizeof(DescStr), "StackTop");
		else
			snprintf(DescStr, sizeof(DescStr), "Stack + $%04X", addr.Address - 0x100);

		return DescStr;
	}
private:
	char DescStr[32] = { 0 };

};


FBBCEmulator::FBBCEmulator()
{
}

bool FBBCEmulator::Init(const FEmulatorLaunchConfig& launchConfig)
{
	if (FEmuBase::Init(launchConfig) == false)
		return false;

	SetWindowTitle(kAppTitle.c_str());
	//SetWindowIcon("SALogo.png");

	// Initialise Emulator
	pBBCConfig = new FBBCConfig();
	pGlobalConfig = pBBCConfig;
	pGlobalConfig->Load(kGlobalConfigFilename);
	CodeAnalysis.SetGlobalConfig(pGlobalConfig);

	LoadFont();

	LoadBBCProjectConfigs(this);

	// set supported bitmap format
	CodeAnalysis.Config.bSupportedBitmapTypes[(int)EBitmapFormat::Bitmap_1Bpp] = true;
	for (int i = 0; i < FCodeAnalysisState::kNoViewStates; i++)
	{
		CodeAnalysis.ViewState[i].CurBitmapFormat = EBitmapFormat::Bitmap_1Bpp;
	}

	// Setup BBC emulator
	bbc_desc_t desc;
	memset(&desc, 0, sizeof(desc));
	
	// setup debug hook
	desc.debug.callback.func = DebugCB;
	desc.debug.callback.user_data = this;
	desc.debug.stopped = CodeAnalysis.Debugger.GetDebuggerStoppedPtr();

	// Load ROM images
	desc.os_rom.ptr = LoadBinaryFile("Roms/OS-1.2.rom", desc.os_rom.size);

	for (int slot = 0; slot < BBC_NUM_ROM_SLOTS; slot++)
	{
		desc.roms[slot] = ROMSlots[slot].ROMData;
	}

	bbc_init(&BBCEmu, &desc);

	if (desc.os_rom.ptr)
		free(desc.os_rom.ptr);

	CPUType = ECPUType::M6502;
	SetNumberDisplayMode(ENumberDisplayMode::HexDollar);

	// Set up memory banks
	BankIds.RAM = CodeAnalysis.CreateBank("RAM", 32, BBCEmu.ram, false, 0x0000, true);					// RAM - $0000 - $7FFF - pages 0-31 - 32K
	LoadROM(pBBCConfig->BasicRom.c_str(), BasicROMSlot);	// top slot for Basic ROM
	BankIds.OSROM = CodeAnalysis.CreateBank("OS ROM", 16, BBCEmu.rom_os, true, 0xC000, true);			// OS ROM - $C000 - $FFFF - pages 48-63 - 16K

	// Setup ROMs

	int slot = 0;
	for (const auto& rom : pBBCConfig->AdditionalRoms)
	{
		if(slot == BasicROMSlot)	// don't write over the basic slot
			slot++;

		LoadROM(rom.c_str(), slot++);
		if (slot >= BBC_NUM_ROM_SLOTS)
			break;
	}

	// map in banks
	CodeAnalysis.MapBank(BankIds.RAM, 0, EBankAccess::ReadWrite);
	CodeAnalysis.MapBank(BankIds.OSROM, 48, EBankAccess::ReadWrite);
	SetROMSlot(BasicROMSlot);

	// setup code analysis
	CodeAnalysis.Init(this);
	CodeAnalysis.Config.bShowBanks = true;
	CodeAnalysis.ViewState[0].Enabled = true;	// always have first view enabled
	SetupCodeAnalysisLabels();
	AddMemoryRegionDescGenerator(new F6502MemDescGenerator());

	IOAnalysis.Init(this);

	pGraphicsViewer = new FBBCGraphicsViewer(this);
	AddViewer(pGraphicsViewer);

	CodeAnalysis.Debugger.Break();

	FBBCProjectConfig* pBasicConfig = (FBBCProjectConfig*)GetGameConfigForName("BBCBasic");

	if (pBasicConfig == nullptr)
		pBasicConfig = CreateNewBBCBasicConfig();

	LoadProject(pBasicConfig, true);	// load basic config initially

	return true;
}

// TODO: implement
bool FBBCEmulator::LoadROM(const char* pFileName, int slot)
{
	size_t romSize = 0;
	const std::string fname =	pBBCConfig->RomFolder + pFileName;

	void* pROMData = LoadBinaryFile(fname.c_str(), romSize);
	if (pROMData == nullptr)
		return false;

	if (romSize > 0x4000)
	{
		// error
		free(pROMData);
		return false;
	}

	memcpy(ROMSlots[slot].ROMData, pROMData, romSize);
	ROMSlots[slot].BankId = CodeAnalysis.CreateBank(pFileName, 16, ROMSlots[slot].ROMData, true, 0x8000, true);
	return true;
}

void FBBCEmulator::SetROMSlot(int slotNo)
{
	const int16_t bankId = ROMSlots[slotNo].BankId;
	if (CurrentROMBank != bankId)
	{
		CodeAnalysis.MapBank(bankId, 32, EBankAccess::ReadWrite);
		CurrentROMBank = bankId;
	}
}


void FBBCEmulator::SetupCodeAnalysisLabels()
{
	// Add labels for BBC HW addresses
	AddFredRegisterLabels(this);
	AddJimRegisterLabels(this);
	AddSheilaRegisterLabels(this);

	// Add Stack??
}

void FBBCEmulator::Shutdown()
{
	if (pCurrentProjectConfig != nullptr)
	{
		// Save Global Config - move to function?
		pGlobalConfig->LastGame = pCurrentProjectConfig->Name;
		SaveProject();
	}

	pGlobalConfig->Save(kGlobalConfigFilename);

	bbc_discard(&BBCEmu);

	FEmuBase::Shutdown();
}


void FBBCEmulator::DrawEmulatorUI()
{
}

void FBBCEmulator::Tick()
{
	FEmuBase::Tick();

	FDebugger& debugger = CodeAnalysis.Debugger;


	//TODO: Display.Tick();

	if (debugger.IsStopped() == false)
	{
		const float frameTime = (float)std::min(1000000.0f / ImGui::GetIO().Framerate, 32000.0f) * 1.0f;// speccyInstance.ExecSpeedScale;

		//CodeAnalysis.OnMachineFrameStart();	// TODO: tie to raster 
		CodeAnalysis.OnFrameStart();
		//StoreRegisters_6502(CodeAnalysis);

		bbc_exec(&BBCEmu, (uint32_t)std::max(static_cast<uint32_t>(frameTime), uint32_t(1)));

		CodeAnalysis.OnFrameEnd();
		//CodeAnalysis.OnMachineFrameEnd();	// TODO: tie to raster 
	}
	// Draw UI
	DrawDockingView();
}

void FBBCEmulator::Reset()
{
	FEmuBase::Reset();

	bbc_reset(&BBCEmu);
}

void FBBCEmulator::FixupAddressRefs()
{
}

void FBBCEmulator::FileMenuAdditions()
{
}

void FBBCEmulator::SystemMenuAdditions()
{
}

void FBBCEmulator::OptionsMenuAdditions()
{
}

void FBBCEmulator::WindowsMenuAdditions()
{
}

void FBBCEmulator::OnKeyUp(int keyCode)
{
	bbc_key_up(&BBCEmu, keyCode);
}	

void FBBCEmulator::OnKeyDown(int keyCode)
{
	bbc_key_down(&BBCEmu, keyCode);
}

void FBBCEmulator::OnChar(int charCode)
{
}

void FBBCEmulator::OnGamepadUpdated(int mask)
{
}	



bool FBBCEmulator::LoadEmulatorFile(const FEmulatorFile* pEmuFile)
{
	return false;
}

bool FBBCEmulator::NewProjectFromEmulatorFile(const FEmulatorFile& gameSnapshot)
{
	return false;
}


static const uint32_t kMachineStateMagic = 0xFaceCafe;
static bbc_t g_SaveSlot;

bool FBBCEmulator::SaveMachineState(const char* fname)
{
	// save game snapshot
	FILE* fp = fopen(fname, "wb");
	if (fp != nullptr)
	{
		const uint32_t versionNo = bbc_save_snapshot(&BBCEmu, &g_SaveSlot);
		fwrite(&kMachineStateMagic, sizeof(uint32_t), 1, fp);
		fwrite(&versionNo, sizeof(uint32_t), 1, fp);
		fwrite(&g_SaveSlot, sizeof(bbc_t), 1, fp);

		fclose(fp);
		return true;
	}

	return false;
}

bool FBBCEmulator::LoadMachineState(const char* fname)
{
	FILE* fp = fopen(fname, "rb");
	if (fp == nullptr)
		return false;

	bool bSuccess = false;
	uint32_t magic;
	uint32_t versionNo;
	fread(&magic, sizeof(uint32_t), 1, fp);
	if (magic == kMachineStateMagic)
	{
		fread(&versionNo, sizeof(uint32_t), 1, fp);
		fread(&g_SaveSlot, sizeof(bbc_t), 1, fp);

		bSuccess = bbc_load_snapshot(&BBCEmu, versionNo, &g_SaveSlot);
	}
	fclose(fp);
	return bSuccess;
}


bool FBBCEmulator::LoadProject(FProjectConfig* pProjectConfig, bool bLoadGameData)
{
	const std::string windowTitle = pProjectConfig != nullptr ? kAppTitle + " - " + pProjectConfig->Name : kAppTitle;
	SetWindowTitle(windowTitle.c_str());

	pCurrentProjectConfig = pProjectConfig;

	// Initialise code analysis
	CodeAnalysis.Init(this);

	//IOAnalysis.Reset();
	bool bLoadSnapshot = false;

	// Set options from config
	if (pProjectConfig)
	{
		for (int i = 0; i < FCodeAnalysisState::kNoViewStates; i++)
		{
			CodeAnalysis.ViewState[i].Enabled = pProjectConfig->ViewConfigs[i].bEnabled;
			CodeAnalysis.ViewState[i].GoToAddress(pProjectConfig->ViewConfigs[i].ViewAddress);
		}

		bLoadSnapshot = pProjectConfig->EmulatorFile.FileName.empty() == false;
	}

	if (bLoadGameData)
	{
		const std::string root = pGlobalConfig->WorkspaceRoot;
		const std::string dataFName = root + "GameData/" + pProjectConfig->Name + ".bin";

		std::string analysisJsonFName = root + "AnalysisJson/" + pProjectConfig->Name + ".json";
		std::string graphicsSetsJsonFName = root + "GraphicsSets/" + pProjectConfig->Name + ".json";
		std::string analysisStateFName = root + "AnalysisState/" + pProjectConfig->Name + ".astate";
		std::string saveStateFName = root + "SaveStates/" + pProjectConfig->Name + ".state";

		// check for new location & adjust paths accordingly
		const std::string gameRoot = pGlobalConfig->WorkspaceRoot + pProjectConfig->Name + "/";
		if (FileExists((gameRoot + "Config.json").c_str()))
		{
			analysisJsonFName = gameRoot + "Analysis.json";
			graphicsSetsJsonFName = gameRoot + "GraphicsSets.json";
			analysisStateFName = gameRoot + "AnalysisState.bin";
			saveStateFName = gameRoot + "SaveState.bin";
		}

		//if (LoadMachineState(saveStateFName.c_str()))
		//{
		//	bLoadSnapshot = false;
		//}


		if (FileExists(analysisJsonFName.c_str()))
		{
			ImportAnalysisJson(CodeAnalysis, analysisJsonFName.c_str());
			ImportAnalysisState(CodeAnalysis, analysisStateFName.c_str());
		}

		// Set memory banks
		//UpdateCodeAnalysisPages(C64Emu.cpu_port);

		FixupAddressRefs();

		// where do we want pokes to live?
		//LoadPOKFile(*pGameConfig, std::string(pGlobalConfig->PokesFolder + pGameConfig->Name + ".pok").c_str());
	}

	std::string romAnalysisFName;

	if(pGlobalConfig->ROMAnalysisPath.empty() == false)
		romAnalysisFName = pGlobalConfig->ROMAnalysisPath + kROMAnalysisFilename;
	else
		romAnalysisFName = pGlobalConfig->WorkspaceRoot + kROMAnalysisFilename;

	if (FileExists(romAnalysisFName.c_str()))
		ImportAnalysisJson(CodeAnalysis, romAnalysisFName.c_str());


	/*if (bLoadSnapshot)
	{
		// if the game state didn't load then reload the snapshot
		const FGameSnapshot* snapshot = GamesList.GetGame(RemoveFileExtension(pGameConfig->SnapshotFile.c_str()).c_str());
		if (snapshot == nullptr)
		{
			SetLastError("Could not find '%s%s'", pGlobalConfig->SnapshotFolder.c_str(), pGameConfig->SnapshotFile.c_str());
			return false;
		}
		if (!GameLoader.LoadSnapshot(*snapshot))
		{
			return false;
		}
	}*/
	ReAnalyseCode(CodeAnalysis);
	GenerateGlobalInfo(CodeAnalysis);
	CodeAnalysis.SetAddressRangeDirty();

	// Start in break mode so the memory will be in its initial state. 
	CodeAnalysis.Debugger.Break();

	CodeAnalysis.Debugger.RegisterNewStackPointer(BBCEmu.cpu.S, FAddressRef());

	// some extra initialisation for creating new analysis from snapshot
	if (bLoadGameData == false)
	{
		FAddressRef initialPC = CodeAnalysis.AddressRefFromPhysicalAddress(BBCEmu.cpu.PC);
		SetItemCode(CodeAnalysis, initialPC);
		CodeAnalysis.Debugger.SetPC(initialPC);
	}

	SetupCodeAnalysisLabels();

	//GraphicsViewer.SetImagesRoot((pGlobalConfig->WorkspaceRoot + "GraphicsSets/" + pGameConfig->Name + "/").c_str());

	return true;
}

bool FBBCEmulator::SaveProject(void)
{
	if (pCurrentProjectConfig == nullptr)
		return false;

	// save analysis
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

	//SaveMachineState(saveStateFName.c_str());
	ExportAnalysisJson(CodeAnalysis, analysisJsonFName.c_str());
	ExportAnalysisState(CodeAnalysis, analysisStateFName.c_str());

	std::string romAnalysisFName;
	if(pGlobalConfig->ROMAnalysisPath.empty() == false)
		romAnalysisFName = pGlobalConfig->ROMAnalysisPath + kROMAnalysisFilename;
	else
		romAnalysisFName = pGlobalConfig->WorkspaceRoot + kROMAnalysisFilename;
	ExportAnalysisJson(CodeAnalysis, romAnalysisFName.c_str(), true);	// Do this on a config?

	return false;
}




uint64_t FBBCEmulator::OnCPUTick(uint64_t pins)
{
	FCodeAnalysisState& state = CodeAnalysis;

	const uint16_t pc = GetPC().Address;

	const uint16_t addr = M6502_GET_ADDR(pins);
	const uint8_t val = M6502_GET_DATA(pins);
	bool irq = pins & M6502_IRQ;
	bool nmi = pins & M6502_NMI;
	bool rdy = pins & M6502_RDY;
	bool aec = pins & M6510_AEC;
	bool hitIrq = false;

	// register interrupt handlers
	if ((addr == 0xfffe || addr == 0xffff) && irq)
	{
		hitIrq = true;
		const uint16_t interruptHandler = ReadWord(0xfffe);
		InterruptHandlers.insert(CodeAnalysis.AddressRefFromPhysicalAddress(interruptHandler));
	}

	// TODO:
#if 1
	// trigger frame events on scanline pos
	static uint16_t lastScanlinePos = 0;
	const uint16_t scanlinePos = BBCEmu.crtc.v_ctr;
	if (scanlinePos != lastScanlinePos)
	{
		CodeAnalysis.Debugger.OnScanlineStart(scanlinePos);

		if (scanlinePos == 0)
			CodeAnalysis.OnMachineFrameStart();
		else if (scanlinePos == BBCEmu.crtc.v_total)    // last scanline
			CodeAnalysis.OnMachineFrameEnd();

		lastScanlinePos = scanlinePos;
	}
#endif
	const bool bReadingInstruction = addr == m6502_pc(&BBCEmu.cpu) - 1;

	if ((pins & M6502_SYNC) == 0) // not for instruction fetch
	{
		if (pins & M6502_RW)
		{
			if (state.bRegisterDataAccesses)
				RegisterDataRead(CodeAnalysis, pc, addr);   // this gives false positives on indirect addressing e.g. STA ($0a),y

			const uint32_t page = addr >> 8;

			if (page == kFredPage || page == kJimPage || page == kSheilaPage)
			{
				IOAnalysis.RegisterIORead(addr, GetPC());
			}
		}
		else
		{
			if (state.bRegisterDataAccesses)
			{
				RegisterDataWrite(CodeAnalysis, pc, addr, val);
			}

			FAddressRef pcRef = state.AddressRefFromPhysicalAddress(pc);
			FAddressRef addrRef = state.AddressRefFromPhysicalAddress(addr);
			state.SetLastWriterForAddress(addr, pcRef);

			const uint32_t page = addr >> 8;

			if (page == kFredPage || page == kJimPage || page == kSheilaPage)
			{
				IOAnalysis.RegisterIOWrite(addr, val, GetPC());
				IOMemBuffer[addr - (kFredPage<<8)] = val;
			}

			if (addr == 0xfe30)
			{
				SetROMSlot(val);
			}
		
			FCodeInfo* pCodeWrittenTo = CodeAnalysis.GetCodeInfoForAddress(addrRef);
			if (pCodeWrittenTo != nullptr && pCodeWrittenTo->bSelfModifyingCode == false)
				pCodeWrittenTo->bSelfModifyingCode = true;
		}
	}
	else
	{
		RegisterCodeExecuted(state, pc, PreviousPC);
		PreviousPC = pc;
	}

	CodeAnalysis.OnCPUTick(pins);

	return pins;
}