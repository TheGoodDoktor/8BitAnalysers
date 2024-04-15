#include "C64Emulator.h"
#define NOMINMAX

//#define SOKOL_IMPL
#include "sokol_audio.h"
#include <ImGuiSupport/ImGuiTexture.h>

#include "c64-roms.h"
#include <Util/FileUtil.h>
#include <CodeAnalyser/UI/CodeAnalyserUI.h>

#include "C64Config.h"
#include <CodeAnalyser/CodeAnalysisJson.h>
#include <CodeAnalyser/CodeAnalysisState.h>
#include "CodeAnalyser/UI/CharacterMapViewer.h"
#include <Debug/DebugLog.h>

#include "FileLoaders/CRTFile.h"


const char* kGlobalConfigFilename = "GlobalConfig.json";
const std::string kAppTitle = "C64 Analyser";

const char* kROMAnalysisFilename = "C64RomsAnalysis.json";

void SetWindowTitle(const char* pTitle);


/* reboot callback */
static void C64BootCallback(c64_t* sys)
{
	// FIXME: no such struct member
	//FC64Emulator* pC64Emu = (FC64Emulator*)sys->user_data;
	//pC64Emu->OnBoot();
}

void* gfx_create_texture(int w, int h)
{
	return ImGui_CreateTextureRGBA(nullptr, w, h);
}

void gfx_update_texture(void* h, void* data, int data_byte_size)
{
	ImGui_UpdateTextureRGBA(h, (unsigned char*)data);
}

void gfx_destroy_texture(void* h)
{

}

/* audio-streaming callback */
static void push_audio(const float* samples, int num_samples, void* user_data)
{
	FC64Emulator* pC64Emu = (FC64Emulator*)user_data;
	if(pC64Emu->GetGlobalConfig()->bEnableAudio)
		saudio_push(samples, num_samples);
}

void DebugCB(void* user_data, uint64_t pins)
{
	FC64Emulator* pC64Emu = (FC64Emulator*)user_data;
	pC64Emu->OnCPUTick(pins);
}

/* get c64_desc_t struct based on joystick type */
c64_desc_t FC64Emulator::GenerateC64Desc(c64_joystick_type_t joy_type)
{
	c64_desc_t desc;
	memset(&desc, 0, sizeof(c64_desc_t));
	desc.joystick_type = joy_type;

	desc.audio.callback.func = push_audio;
	desc.audio.callback.user_data = this;
	desc.audio.sample_rate = saudio_sample_rate();
	//desc.audio.tape_sound = false;// sargs_boolean("tape_sound"),

	desc.roms.chars.ptr = dump_c64_char_bin;
	desc.roms.chars.size = sizeof(dump_c64_char_bin);
	desc.roms.basic.ptr = dump_c64_basic_bin;
	desc.roms.basic.size = sizeof(dump_c64_basic_bin);
	desc.roms.kernal.ptr = dump_c64_kernalv3_bin;
	desc.roms.kernal.size = sizeof(dump_c64_kernalv3_bin);

	// setup debug hook
	desc.debug.callback.func = DebugCB;
	desc.debug.callback.user_data = this;
	desc.debug.stopped = CodeAnalysis.Debugger.GetDebuggerStoppedPtr();

	desc.c1530_enabled = true;
	//desc.c1541_enabled = true;

	return desc;
}

// callback function to save snapshot to a numbered slot
void UISnapshotSaveCB(size_t slot_index)
{
}

// callback function to load snapshot from numbered slot
bool UISnapshotLoadCB(size_t slot_index)
{
	return true;
}


bool FC64Emulator::Init(const FEmulatorLaunchConfig& launchConfig)
{
	if(FEmuBase::Init(launchConfig) == false)
		return false;

	SetWindowTitle(kAppTitle.c_str());
	//SetWindowIcon("SALogo.png");

	// Initialise Emulator
	pGlobalConfig = new FC64Config();
	pGlobalConfig->Load(kGlobalConfigFilename);
	CodeAnalysis.SetGlobalConfig(pGlobalConfig);
	LoadC64ProjectConfigs(this);

	// set supported bitmap format
	CodeAnalysis.Config.bSupportedBitmapTypes[(int)EBitmapFormat::Bitmap_1Bpp] = true;
	CodeAnalysis.Config.bSupportedBitmapTypes[(int)EBitmapFormat::ColMapMulticolour_C64] = true;
	for (int i = 0; i < FCodeAnalysisState::kNoViewStates; i++)
	{
		CodeAnalysis.ViewState[i].CurBitmapFormat = EBitmapFormat::Bitmap_1Bpp;
	}

	// Setup C64 Emulator
	c64_joystick_type_t joy_type = C64_JOYSTICKTYPE_NONE;
	c64_desc_t desc = GenerateC64Desc(joy_type);
	c64_init(&C64Emu, &desc);

	Display.Init(&CodeAnalysis, this);


	CPUType = ECPUType::M6502;
	SetNumberDisplayMode(ENumberDisplayMode::HexDollar);

	// setup default memory configuration
	memset(IOMemBuffer,0,sizeof(IOMemBuffer));	// clear to 0

	BankIds.LowerRAM = CodeAnalysis.CreateBank("LoRAM", 32, C64Emu.ram,false, 0x0000,  true);				// RAM - $0000 - $7FFF - pages 0-31 - 32K
	BankIds.LowerRAM2 = CodeAnalysis.CreateBank("LoRAM2", 8, &C64Emu.ram[0x8000], false, 0x8000, true);	// RAM - $8000 - $9FFF - pages 32-39 - 8K
	BankIds.HighRAM = CodeAnalysis.CreateBank("HiRAM", 4, &C64Emu.ram[0xc000], false, 0xC000);			// RAM - $C000 - $CFFF - pages 48-51 - 4k
	BankIds.IOArea = CodeAnalysis.CreateBank("IOArea", 4, IOMemBuffer, false, 0xD000);					// IO System - %D000 - $DFFF - page 52-55 - 4k
	BankIds.BasicROM = CodeAnalysis.CreateBank("BasicROM", 8, C64Emu.rom_basic, true,0xA000);				// BASIC ROM - $A000-$BFFF - pages 40-47 - 8k
	BankIds.RAMBehindBasicROM = CodeAnalysis.CreateBank("RAMBehindBasicROM", 8, &C64Emu.ram[0xa000], false, 0xA000);
		   
	BankIds.KernelROM = CodeAnalysis.CreateBank("KernelROM", 8, C64Emu.rom_kernal, true, 0xE000);   // Kernel ROM - $E000-$FFFF - pages 56-63 - 8k
	BankIds.RAMBehindKernelROM = CodeAnalysis.CreateBank("RAMBehindKernelROM", 8, &C64Emu.ram[0xe000], false, 0xE000);
		   
	BankIds.CharacterROM = CodeAnalysis.CreateBank("CharacterROM", 4, C64Emu.rom_char, true,0xD000); // Char ROM - %D000 - $DFFF - page 52-55 - 4k
	BankIds.RAMBehindCharROM = CodeAnalysis.CreateBank("RAMBehindCharROM", 4, &C64Emu.ram[0xd000], false, 0xD000);

	//ColourRAMId = CodeAnalysis.CreateBank("ColourRAM", 1, C64Emu.color_ram, true);  // Colour RAM - $D800

	// map in permanent banks
	CodeAnalysis.MapBank(BankIds.LowerRAM, 0, EBankAccess::ReadWrite);	// RAM - $0000 - $7FFF - pages 0-31 - 32K
	CodeAnalysis.MapBank(BankIds.LowerRAM2, 32, EBankAccess::ReadWrite);	// RAM - $8000 - $9FFF - pages 32-39 - 8K
	CodeAnalysis.MapBank(BankIds.HighRAM, 48, EBankAccess::ReadWrite);	// RAM - $C000 - $CFFF - pages 48-51 - 4k

	// map RAM behind ROM banks a write
	CodeAnalysis.MapBank(BankIds.RAMBehindBasicROM, 40, EBankAccess::Write);
	CodeAnalysis.MapBank(BankIds.RAMBehindCharROM, 52, EBankAccess::Write);  // Map because VIC needs map address to be set - hack
	CodeAnalysis.MapBank(BankIds.RAMBehindKernelROM, 56, EBankAccess::Write);

	// Setup VIC Bank Mapping 16 * 4k pages
	VICBankMapping[0x0] = BankIds.LowerRAM;
	VICBankMapping[0x1] = BankIds.CharacterROM;
	VICBankMapping[0x2] = BankIds.LowerRAM;
	VICBankMapping[0x3] = BankIds.LowerRAM;
	VICBankMapping[0x4] = BankIds.LowerRAM;
	VICBankMapping[0x5] = BankIds.LowerRAM;
	VICBankMapping[0x6] = BankIds.LowerRAM;
	VICBankMapping[0x7] = BankIds.LowerRAM2;
	VICBankMapping[0x8] = BankIds.LowerRAM2;
	VICBankMapping[0x9] = BankIds.CharacterROM;
	VICBankMapping[0xa] = BankIds.RAMBehindBasicROM;
	VICBankMapping[0xb] = BankIds.RAMBehindBasicROM;
	VICBankMapping[0xc] = BankIds.HighRAM;
	VICBankMapping[0xd] = BankIds.RAMBehindCharROM;
	VICBankMapping[0xe] = BankIds.RAMBehindKernelROM;
	VICBankMapping[0xf] = BankIds.RAMBehindKernelROM;

	// Setup games lists
	AddGamesList("PRG File", GetC64GlobalConfig()->PrgFolder.c_str());
	AddGamesList("Tape File", GetC64GlobalConfig()->TapesFolder.c_str());
	AddGamesList("Crt File", GetC64GlobalConfig()->CrtFolder.c_str());

	// setup code analysis
	CodeAnalysis.Init(this);
	CodeAnalysis.Config.bShowBanks = true;
	CodeAnalysis.ViewState[0].Enabled = true;	// always have first view enabled
	SetupCodeAnalysisLabels();
	UpdateCodeAnalysisPages(0x7);
	
	IOAnalysis.Init(this);
	
	pCharacterMapViewer = new FCharacterMapViewer(this);
	AddViewer(pCharacterMapViewer);
	pGraphicsViewer = new FC64GraphicsViewer(this);
	AddViewer(pGraphicsViewer);
	
	// Set up cartridges
	CartridgeManager.Init(this);

	bool bLoadedGame = false;

	if (pGlobalConfig->LastGame.empty() == false)
	{
		bLoadedGame = FEmuBase::StartGameFromName(pGlobalConfig->LastGame.c_str(), true);
		SetupCodeAnalysisLabels();
	}
	
	return true;
}

void FC64Emulator::SetupCodeAnalysisLabels()
{
	// Add IO Labels to code analysis
	FCodeAnalysisBank* pIOBank = CodeAnalysis.GetBank(BankIds.IOArea);
	AddVICRegisterLabels(pIOBank->Pages[0]);  // Page $D000-$D3ff
	AddSIDRegisterLabels(pIOBank->Pages[1]);  // Page $D400-$D7ff
	pIOBank->Pages[2].SetLabelAtAddress("ColourRAM", ELabelType::Data, 0x0000,true);    // Colour RAM $D800
	AddCIARegisterLabels(pIOBank->Pages[3]);  // Page $DC00-$Dfff
}

void FC64Emulator::UpdateCodeAnalysisPages(uint8_t cpuPort)
{
	bBasicROMMapped = false;
	bKernelROMMapped = false;
	bCharacterROMMapped = false;
	bIOMapped = false;

	/* shortcut if HIRAM and LORAM is 0, everything is RAM */
	if ((cpuPort & (C64_CPUPORT_HIRAM | C64_CPUPORT_LORAM)) == 0)
	{
		// Map in all RAM
		CodeAnalysis.MapBank(BankIds.RAMBehindBasicROM, 40, EBankAccess::ReadWrite);          // RAM Under BASIC ROM - $A000-$BFFF - pages 40-47 - 8k
		CodeAnalysis.MapBank(BankIds.RAMBehindCharROM, 52, EBankAccess::ReadWrite);           // RAM Under Char ROM - %D000 - $DFFF - page 52-55 - 4k
		CodeAnalysis.MapBank(BankIds.RAMBehindKernelROM, 56, EBankAccess::ReadWrite);         // RAM Under Kernel ROM - $E000-$FFFF - pages 56-63 - 8k
	}
	else
	{
		/* A000..BFFF is either RAM-behind-BASIC-ROM or RAM */
		if ((cpuPort & (C64_CPUPORT_HIRAM | C64_CPUPORT_LORAM)) == (C64_CPUPORT_HIRAM | C64_CPUPORT_LORAM))
		{
			CodeAnalysis.MapBank(BankIds.BasicROM, 40, EBankAccess::Read);       // BASIC ROM - $A000-$BFFF - pages 40-47 - 8k
			bBasicROMMapped = true;
		}
		else
		{
			CodeAnalysis.MapBank(BankIds.RAMBehindBasicROM, 40, EBankAccess::Read);       // RAM Under BASIC ROM - $A000-$BFFF - pages 40-47 - 8k
		}

		/* E000..FFFF is either RAM-behind-KERNAL-ROM or RAM */
		if (cpuPort & C64_CPUPORT_HIRAM)
		{
			bKernelROMMapped = true;
			CodeAnalysis.MapBank(BankIds.KernelROM, 56, EBankAccess::Read);      // Kernel ROM - $E000-$FFFF - pages 56-63 - 8k
		}
		else
		{
			CodeAnalysis.MapBank(BankIds.RAMBehindKernelROM, 56, EBankAccess::Read);      // RAM Under Kernel ROM - $E000-$FFFF - pages 56-63 - 8k
		}

		/* D000..DFFF can be Char-ROM or I/O */
		if (cpuPort & C64_CPUPORT_CHAREN)
		{
			bIOMapped = true;
			CodeAnalysis.MapBank(BankIds.IOArea, 52, EBankAccess::ReadWrite);         // IO System - %D000 - $DFFF - page 52-55 - 4k
		}
		else
		{
			bCharacterROMMapped = true;
			CodeAnalysis.MapBank(BankIds.CharacterROM, 52, EBankAccess::Read);       // Character ROM - %D000 - $DFFF - page 52-55 - 4k
			CodeAnalysis.MapBank(BankIds.RAMBehindCharROM, 52, EBankAccess::Write);
		}
	}
}

// Note : can be passed nullptr on a reset
bool FC64Emulator::LoadProject(FProjectConfig* pProjectConfig, bool bLoadGameData)
{
	const std::string windowTitle = pProjectConfig != nullptr ? kAppTitle + " - " + pProjectConfig->Name : kAppTitle;
	SetWindowTitle(windowTitle.c_str());

	pCurrentProjectConfig = pProjectConfig;

	// Initialise code analysis
	CodeAnalysis.Init(this);
	CartridgeManager.ResetCartridgeBanks();

	//IOAnalysis.Reset();
	bool bLoadSnapshot = false;

	// Set options from config
	if(pProjectConfig)
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

		//GraphicsViewer.LoadGraphicsSets(graphicsSetsJsonFName.c_str());

		// Load machine state, if it fails, reload the prg file
		if (LoadMachineState(saveStateFName.c_str()))
		{
			// if the game state loaded then we don't need the snapshot
			bLoadSnapshot = false;
			/*
			const std::string snapshotFName = GetC64GlobalConfig()->PrgFolder + pGameConfig->SnapshotFile;
			chips_range_t snapshotData;
			snapshotData.ptr = LoadBinaryFile(snapshotFName.c_str(), snapshotData.size);
			if (snapshotData.ptr != nullptr)
			{
				c64_quickload(&C64Emu, snapshotData);
				free(snapshotData.ptr);
			}*/
		}

		if (FileExists(analysisJsonFName.c_str()))
		{
			ImportAnalysisJson(CodeAnalysis, analysisJsonFName.c_str());
			ImportAnalysisState(CodeAnalysis, analysisStateFName.c_str());
		}
		// Set memory banks
		UpdateCodeAnalysisPages(C64Emu.cpu_port);

		
		// where do we want pokes to live?
		//LoadPOKFile(*pGameConfig, std::string(pGlobalConfig->PokesFolder + pGameConfig->Name + ".pok").c_str());
	}

	if (FileExists(kROMAnalysisFilename))
		ImportAnalysisJson(CodeAnalysis, kROMAnalysisFilename);


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

	CodeAnalysis.Debugger.RegisterNewStackPointer(C64Emu.cpu.S, FAddressRef());

	// some extra initialisation for creating new analysis from snapshot
	if (bLoadGameData == false)
	{
		FAddressRef initialPC = CodeAnalysis.AddressRefFromPhysicalAddress(C64Emu.cpu.PC);
		SetItemCode(CodeAnalysis, initialPC);
		CodeAnalysis.Debugger.SetPC(initialPC);
	}

	SetupCodeAnalysisLabels();

	//GraphicsViewer.SetImagesRoot((pGlobalConfig->WorkspaceRoot + "GraphicsSets/" + pGameConfig->Name + "/").c_str());

	return true;
}

bool FC64Emulator::LoadEmulatorFile(const FEmulatorFile* pSnapshot)
{
	auto findIt = GamesLists.find(pSnapshot->ListName);
	if (findIt == GamesLists.end())
		return false;

	const std::string fileName = findIt->second.GetRootDir() + pSnapshot->FileName;
	//const char* pFileName = fileName.c_str();

	switch (pSnapshot->Type)
	{
	case EEmuFileType::PRG:
	{
		chips_range_t snapshotData;
		snapshotData.ptr = LoadBinaryFile(fileName.c_str(), snapshotData.size);
		if (snapshotData.ptr != nullptr)
		{
			const bool bSuccess = c64_quickload(&C64Emu, snapshotData);
			free(snapshotData.ptr);
			LoadedFileType = EC64FileType::PRG;
			return bSuccess;
		}
		else
		{
			return false;
		}
	}
	break;
	// C64 disks aren't implmented yet :(
	case EEmuFileType::D64:
	{
		chips_range_t diskData;
		diskData.ptr = LoadBinaryFile(fileName.c_str(), diskData.size);
		if (diskData.ptr != nullptr)
		{
			c1541_insert_disc(&C64Emu.c1541, diskData);
			free(diskData.ptr);
			LoadedFileType = EC64FileType::Disk;
			return true;
		}
		else
		{
			return false;
		}
	}
	break;
	case EEmuFileType::TAP:
	{
		chips_range_t tapeData;
		tapeData.ptr = LoadBinaryFile(fileName.c_str(), tapeData.size);
		if (tapeData.ptr != nullptr)
		{
			c64_insert_tape(&C64Emu, tapeData);
			free(tapeData.ptr);
			LoadedFileType = EC64FileType::Tape;
			return true;
		}
		else
		{
			return false;
		}
	}
	break;
	case EEmuFileType::CRT:
	{
		LoadedFileType = EC64FileType::Cartridge;
		return CartridgeManager.LoadCRTFile(fileName.c_str());
	}
	break;
	default:
		return false;
	}

}

bool FC64Emulator::NewProjectFromEmulatorFile(const FEmulatorFile& emuFile)
{
	// Remove any existing config 
	RemoveGameConfig(emuFile.DisplayName.c_str());
	FC64ProjectConfig* pNewConfig = CreateNewC64ProjectFromEmuFile(emuFile);

	if (pNewConfig != nullptr)
	{
		c64_desc_t desc = GenerateC64Desc(C64Emu.joystick_type);
		c64_init(&C64Emu, &desc);

		LoadProject(pNewConfig, false);
		AddGameConfig(pNewConfig);
		SaveProject();

		CodeAnalysis.Debugger.Continue();

		if (emuFile.Type == EEmuFileType::CRT)	// cartridge can start instantly
		{
			LoadEmulatorFile(&emuFile);
			FileLoadPhase = EFileLoadPhase::Run;
			CodeAnalysis.Debugger.Break();
		}
		else
		{
			FileLoadPhase = EFileLoadPhase::Reset;
		}

		return true;
	}
	return false;
}

#if 0
void FC64Emulator::ResetCartridgeBanks() 
{ 
	for(int slotNo = 0;slotNo<(int)ECartridgeSlot::Max;slotNo++)
	{
		FCartridgeSlot& slot = CartridgeSlots[slotNo];
		slot.Reset();
	}
	if(FirstCartridgeBankId != -1)
		CodeAnalysis.FreeBanksFrom(FirstCartridgeBankId);
}

ECartridgeSlot GetSlotFromAddress(uint16_t address);
FCartridgeBank& FC64Emulator::AddCartridgeBank(int bankNo, uint16_t address, uint32_t dataSize)
{
	ECartridgeSlot slot = GetSlotFromAddress(address);
	assert(slot != ECartridgeSlot::Unknown);
	FCartridgeSlot& cartridgeSlot = GetCartridgeSlot(slot);
	if(cartridgeSlot.Banks.size() <= bankNo)
		cartridgeSlot.Banks.resize(bankNo+1);
	//assert(cartridgeSlot.Banks.size() == bankNo);	// assume they get added in a linear way

	FCartridgeBank& bank = cartridgeSlot.Banks[bankNo];
	bank.BankNo = bankNo;
	bank.DataSize = dataSize;
	bank.Data = new uint8_t[bank.DataSize];
	bank.Address = address;

	// Create Analyser Bank
	char bankName[32];
	snprintf(bankName, 32, "CartBank%d", bankNo);
	bank.BankId = CodeAnalysis.CreateBank(bankName, bank.DataSize / 1024, bank.Data, false, bank.Address);

	//assert(ActiveCartridgeBanks == bankNo);
	//ActiveCartridgeBanks++;
	return bank;
}

void FC64Emulator::InitCartMapping(void)
{
	for(int slotNo=0;slotNo<(int)ECartridgeSlot::Max;slotNo++)
	{
		if(CartridgeSlots[slotNo].bActive == true)//Banks.empty() == false)
		{
			MapCartridgeBank((ECartridgeSlot)slotNo,0);//find first bank instead?
		}
	}
}

bool FC64Emulator::MapCartridgeBank(ECartridgeSlot slot, int bankNo)
{
	FCartridgeSlot& cartridgeSlot = GetCartridgeSlot(slot);
	FCartridgeBank& bank = cartridgeSlot.Banks[bankNo];

	// Map in memory page
	mem_map_rw(&C64Emu.mem_cpu, 0, bank.Address, bank.DataSize, bank.Data, &C64Emu.ram[bank.Address]);

	// Map in analysis
	CodeAnalysis.MapBank(bank.BankId, bank.Address / 1024, EBankAccess::Read);

	cartridgeSlot.CurrentBank = bankNo;
	cartridgeSlot.bActive = true;

	return true;
}

void	FC64Emulator::UnMapCartridge(ECartridgeSlot slot)
{
	FCartridgeSlot& cartridgeSlot = GetCartridgeSlot(slot);

	mem_map_ram(&C64Emu.mem_cpu, 0, cartridgeSlot.BaseAddress, cartridgeSlot.Size, &C64Emu.ram[cartridgeSlot.BaseAddress]);

	if(slot == ECartridgeSlot::Addr_8000)
		CodeAnalysis.MapBank(BankIds.LowerRAM2, cartridgeSlot.BaseAddress / 1024, EBankAccess::ReadWrite);
	else if (slot == ECartridgeSlot::Addr_A000)
		CodeAnalysis.MapBank(BankIds.RAMBehindBasicROM, cartridgeSlot.BaseAddress / 1024, EBankAccess::ReadWrite);
	else if (slot == ECartridgeSlot::Addr_E000)
		CodeAnalysis.MapBank(BankIds.RAMBehindKernelROM, cartridgeSlot.BaseAddress / 1024, EBankAccess::ReadWrite);

	cartridgeSlot.CurrentBank = -1;
	cartridgeSlot.bActive = false;
}
#endif


void FC64Emulator::ResetCodeAnalysis(void)
{
	// Reset other analysers
	InterruptHandlers.clear();
	IOAnalysis.Reset();
}

static const uint32_t kMachineStateMagic = 0xFaceCafe;
static c64_t g_SaveSlot;

bool FC64Emulator::SaveMachineState(const char* fname)
{
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
	}

	return false;
}

bool FC64Emulator::LoadMachineState(const char* fname)
{
	FILE* fp = fopen(fname, "rb");
	if(fp == nullptr)
		return false;

	bool bSuccess = false;
	uint32_t magic;
	uint32_t versionNo;
	fread(&magic, sizeof(uint32_t), 1, fp);
	if(magic == kMachineStateMagic)
	{
		fread(&versionNo, sizeof(uint32_t), 1, fp);
		fread(&g_SaveSlot, sizeof(c64_t), 1, fp);

		bSuccess = c64_load_snapshot(&C64Emu, versionNo, &g_SaveSlot);

		if(CartridgeManager.LoadData(fp))
			LoadedFileType = EC64FileType::Cartridge;
	}
	fclose(fp);
	return bSuccess;
}

bool FC64Emulator::SaveProject(void)
{
	if(pCurrentProjectConfig == nullptr)
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

	SaveMachineState(saveStateFName.c_str());
	ExportAnalysisJson(CodeAnalysis, analysisJsonFName.c_str());
	ExportAnalysisState(CodeAnalysis, analysisStateFName.c_str());

	ExportAnalysisJson(CodeAnalysis, kROMAnalysisFilename, true);	// Do this on a config?

	return true;
}

void FC64Emulator::Shutdown()
{
	if (pCurrentProjectConfig != nullptr)
	{
		// Save Global Config - move to function?
		pGlobalConfig->LastGame = pCurrentProjectConfig->Name;
		SaveProject();
	}

	pGlobalConfig->Save(kGlobalConfigFilename);

	//ui_c64_discard(&C64UI);
	c64_discard(&C64Emu);

	FEmuBase::Shutdown();
}



void FC64Emulator::DrawEmulatorUI()
{
	FCodeAnalysisViewState& viewState = CodeAnalysis.GetFocussedViewState();

	if (ImGui::Begin("C64 Screen"))
	{
		ImGui::Text("Mapped: ");
		if (bBasicROMMapped)
		{
			ImGui::SameLine();
			ImGui::Text("Basic ");
		}
		if (bKernelROMMapped)
		{
			ImGui::SameLine();
			ImGui::Text("Kernel ");
		}
		if (bIOMapped)
		{
			ImGui::SameLine();
			ImGui::Text("IO ");
		}
		if (bCharacterROMMapped)
		{
			ImGui::SameLine();
			ImGui::Text("CharROM ");
		}


		Display.DrawUI();

		// Scanline breakpoint
		if(CodeAnalysis.Debugger.GetScanlineBreakpoint() != -1)
		{
			ImGui::PushID("ScanlineBP");
			ImGui::Text("Scanline Breakpoint at line: %d", CodeAnalysis.Debugger.GetScanlineBreakpoint());
			ImGui::SameLine();
			if(ImGui::Button("Clear"))
				CodeAnalysis.Debugger.ClearScanlineBreakpoint();
			ImGui::PopID();
		}

		// Temp
		ImGui::Text("Interrupt Handlers");
		ImGui::SameLine();
		if(ImGui::Button("Clear"))
			InterruptHandlers.clear();

		for (auto& intHandler : InterruptHandlers)
		{
			//ImGui::Text("$%04X:", intHandler);
			ShowCodeAccessorActivity(CodeAnalysis, intHandler);
			ImGui::Text("   ");
			ImGui::SameLine();
			DrawCodeAddress(CodeAnalysis, viewState, intHandler);
		}
	}
	ImGui::End();

	CartridgeManager.DrawUI();
}

// Add C64 specific menu items
void	FC64Emulator::FileMenuAdditions(void) 
{
}

void	FC64Emulator::SystemMenuAdditions(void) 
{
}

void	FC64Emulator::OptionsMenuAdditions(void) 
{
	FC64Config* pC64Config = GetC64GlobalConfig();
	ImGui::MenuItem("Show H Counter", 0, &pC64Config->bShowHCounter);
	ImGui::MenuItem("Show VIC Overlay", 0, &pC64Config->bShowVICOverlay);
}

void	FC64Emulator::WindowsMenuAdditions(void) 
{
}


void FC64Emulator::Tick()
{
	FEmuBase::Tick();

	FDebugger& debugger = CodeAnalysis.Debugger;

	Display.Tick();

	if (debugger.IsStopped() == false)
	{
		const float frameTime = (float)std::min(1000000.0f / ImGui::GetIO().Framerate, 32000.0f) * 1.0f;// speccyInstance.ExecSpeedScale;
	
		CodeAnalysis.OnFrameStart();
		//StoreRegisters_6502(CodeAnalysis);

		c64_exec(&C64Emu, (uint32_t)std::max(static_cast<uint32_t>(frameTime), uint32_t(1)));

		CodeAnalysis.OnFrameEnd();
	}
	DrawDockingView();

	switch(FileLoadPhase)
	{
		case EFileLoadPhase::BasicReady:
			LoadEmulatorFile(&EmulatorFileToLoad);
			//GamesList.LoadGame(pCurrentGameConfig->Name.c_str());
			FileLoadPhase = EFileLoadPhase::Loaded;
			break;
		case EFileLoadPhase::Loaded:
			switch(LoadedFileType)
			{
				case EC64FileType::PRG:
					c64_basic_run(&C64Emu);
					FileLoadPhase = EFileLoadPhase::Run;
					break;
				case EC64FileType::Tape:
					c64_basic_load(&C64Emu);
					c64_tape_play(&C64Emu);
					FileLoadPhase = EFileLoadPhase::TapePlaying;
					break;
				default:
					break;
			}
			
			break;

		default:
			break;
	}

#if 0
	gfx_draw(c64_display_width(&c64), c64_display_height(&c64));
	const uint32_t load_delay_frames = 180;
	if (fs_ptr() && clock_frame_count() > load_delay_frames) {
		bool load_success = false;
		if (fs_ext("txt") || fs_ext("bas")) {
			load_success = true;
			keybuf_put((const char*)fs_ptr());
		}
		else if (fs_ext("tap")) {
			load_success = c64_insert_tape(&c64, fs_ptr(), fs_size());
		}
		else if (fs_ext("bin") || fs_ext("prg") || fs_ext("")) {
			load_success = c64_quickload(&c64, fs_ptr(), fs_size());
		}
		if (load_success) {
			if (clock_frame_count() > (load_delay_frames + 10)) {
				gfx_flash_success();
			}
			if (fs_ext("tap")) {
				c64_start_tape(&c64);
			}
			if (sargs_exists("input")) {
				keybuf_put(sargs_value("input"));
			}
			else if (fs_ext("tap")) {
				keybuf_put("LOAD\n");
			}
		}
		else {
			gfx_flash_error();
		}
		fs_free();
	}
	uint8_t key_code;
	if (0 != (key_code = keybuf_get()))
	{
		/* FIXME: this is ugly */
		c64_joystick_type_t joy_type = c64.joystick_type;
		c64.joystick_type = C64_JOYSTICKTYPE_NONE;
		c64_key_down(&c64, key_code);
		c64_key_up(&c64, key_code);
		c64.joystick_type = joy_type;
	}
#endif
}

void   FC64Emulator::Reset(void)
{
	FEmuBase::Reset();

	c64_reset(&C64Emu);

	// Set memory banks
	UpdateCodeAnalysisPages(C64Emu.cpu_port);
	if(LoadedFileType == EC64FileType::Cartridge)
		CartridgeManager.OnMachineReset();
}


int GetC64KeyFromKeyCode(int keyCode)
{
	int c = 0;
	bool bShift = false;
	switch (keyCode)
	{
	case ImGuiKey_Space:        c = 0x20; break;
	case ImGuiKey_LeftArrow:         c = 0x08; break;
	case ImGuiKey_RightArrow:        c = 0x09; break;
	case ImGuiKey_DownArrow:         c = 0x0A; break;
	case ImGuiKey_UpArrow:           c = 0x0B; break;
	case ImGuiKey_Enter:        c = 0x0D; break;
	case ImGuiKey_Backspace:           c = bShift ? 0x0C : 0x01; break;
	case ImGuiKey_Escape:       c = bShift ? 0x13 : 0x03; break;
	case ImGuiKey_F1:           c = 0xF1; break;
	case ImGuiKey_F2:           c = 0xF2; break;
	case ImGuiKey_F3:           c = 0xF3; break;
	case ImGuiKey_F4:           c = 0xF4; break;
	case ImGuiKey_F5:           c = 0xF5; break;
	case ImGuiKey_F6:           c = 0xF6; break;
	case ImGuiKey_F7:           c = 0xF7; break;
	case ImGuiKey_F8:           c = 0xF8; break;
	default:                        c = 0; break;
	}
	return c;
}

void FC64Emulator::OnKeyUp(int keyCode)
{
	c64_key_up(&C64Emu, GetC64KeyFromKeyCode(keyCode));
}

void FC64Emulator::OnKeyDown(int keyCode)
{
	c64_key_down(&C64Emu, GetC64KeyFromKeyCode(keyCode));
}

void FC64Emulator::OnGamepadUpdated(int mask)
{
	if (c64_joystick_type(&C64Emu) != C64_JOYSTICKTYPE_NONE)
	{
		c64_joystick(&C64Emu, mask, 0);
	}
}


void FC64Emulator::OnChar(int charCode)
{
	int c = charCode;

	if ((c > 0x20) && (c < 0x7F))
	{
		/* need to invert case (unshifted is upper caps, shifted is lower caps */
		if (isupper(c))
			c = tolower(c);

		else if (islower(c))
			c = toupper(c);

		c64_key_down(&C64Emu, c);
		c64_key_up(&C64Emu, c);
	}
}

// Emulator Event Handers
void    FC64Emulator::OnBoot(void)
{
	c64_desc_t desc = GenerateC64Desc(C64Emu.joystick_type);
	c64_init(&C64Emu, &desc);
}

// Don't think this is used anymore
// 
// pc points to instruction after the one just executed so we use the previous pc
int    FC64Emulator::OnCPUTrap(uint16_t pc, int ticks, uint64_t pins)
{
#if 0
	const uint16_t addr = M6502_GET_ADDR(pins);
	const bool bMemAccess = !!(pins & M6502_RDY);
	const bool bWrite = !!(pins & M6502_RW);

	bool bBreak = RegisterCodeExecuted(CodeAnalysis, LastPC, pc);
	FCodeInfo* pCodeInfo = CodeAnalysis.GetCodeInfoForAddress(LastPC);
	pCodeInfo->FrameLastExecuted = CodeAnalysis.CurrentFrameNo;

	// check for breakpointed code line
	if (bBreak)
		return UI_DBG_BP_BASE_TRAPID;

	LastPC = pc;
#endif
	return 0;
}

uint64_t FC64Emulator::OnCPUTick(uint64_t pins)
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

	// trigger frame events on scanline pos
	static uint16_t lastScanlinePos = 0;
	const uint16_t scanlinePos = C64Emu.vic.rs.v_count;
	if (scanlinePos != lastScanlinePos)
	{
		CodeAnalysis.Debugger.OnScanlineStart(scanlinePos);

		if(scanlinePos == 0)
			CodeAnalysis.OnMachineFrameStart();
		else if(scanlinePos == M6569_VTOTAL - 1)    // last scanline
			CodeAnalysis.OnMachineFrameEnd();

		lastScanlinePos = scanlinePos;
	}

	// TODO: scanline breakpoints

	bool bReadingInstruction = addr == m6502_pc(&C64Emu.cpu) - 1;

	if ((pins & M6502_SYNC) == 0) // not for instruction fetch
	{
		if (pins & M6502_RW)
		{
			if (state.bRegisterDataAccesses)
				RegisterDataRead(CodeAnalysis, pc, addr);   // this gives false positives on indirect addressing e.g. STA ($0a),y

			if (bIOMapped && (addr >> 12) == 0xd)
			{
				IOAnalysis.RegisterIORead(addr, GetPC());
				uint8_t readVal = 0;
				if (CartridgeManager.HandleIORead(addr, readVal))
				{
					M6502_SET_DATA(pins,readVal);
				}
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

			if (bIOMapped && (addr >> 12) == 0xd)
			{
				IOAnalysis.RegisterIOWrite(addr, val, GetPC());
				IOMemBuffer[addr & 0xfff] = val;
				
				CartridgeManager.HandleIOWrite(addr,val);
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

		if(FileLoadPhase == EFileLoadPhase::Reset)
		{
			if(pc == 0xE5CD)
				FileLoadPhase = EFileLoadPhase::BasicReady;
		}
	}


	const bool bNeedMemUpdate = ((C64Emu.cpu_port ^ LastMemPort) & 7) != 0;
	if (bNeedMemUpdate)
	{
		UpdateCodeAnalysisPages(C64Emu.cpu_port);

		LastMemPort = C64Emu.cpu_port & 7;
	}

	CodeAnalysis.OnCPUTick(pins);

	return pins;
}
