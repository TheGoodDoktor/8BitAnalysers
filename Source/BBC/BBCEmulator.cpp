#include "BBCEmulator.h"

#include "BBCConfig.h"
#include <Util/FileUtil.h>

const char* kGlobalConfigFilename = "GlobalConfig.json";
const std::string kAppTitle = "BBC Analyser";


const char* kROMAnalysisFilename = "BBCRomsAnalysis.json";

void SetWindowTitle(const char* pTitle);

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
	pGlobalConfig = new FBBCConfig();
	pGlobalConfig->Load(kGlobalConfigFilename);
	CodeAnalysis.SetGlobalConfig(pGlobalConfig);
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
	
	// Load ROM images
	desc.roms.os.ptr = LoadBinaryFile("Roms/OS-1.2.rom", desc.roms.os.size);
	desc.roms.basic.ptr = LoadBinaryFile("Roms/BASIC2.rom", desc.roms.basic.size);

	bbc_init(&BBCEmu, &desc);

	if (desc.roms.os.ptr)
		free(desc.roms.os.ptr);
	if (desc.roms.basic.ptr)
		free(desc.roms.basic.ptr);

	CPUType = ECPUType::M6502;
	SetNumberDisplayMode(ENumberDisplayMode::HexDollar);

	// Set up memory banks
	BankIds.RAM = CodeAnalysis.CreateBank("RAM", 32, BBCEmu.ram, false, 0x0000, true);					// RAM - $0000 - $7FFF - pages 0-31 - 32K
	BankIds.BasicROM = CodeAnalysis.CreateBank("Basic ROM", 16, BBCEmu.rom_basic, false, 0x8000, true);	// Basic ROM - $8000 - $BFFF - pages 32-47 - 16K
	BankIds.OSROM = CodeAnalysis.CreateBank("OS ROM", 16, BBCEmu.rom_os, false, 0xC000, true);			// OS ROM - $C000 - $FFFF - pages 48-63 - 16K

	// map in banks
	CodeAnalysis.MapBank(BankIds.RAM, 0, EBankAccess::ReadWrite);
	CodeAnalysis.MapBank(BankIds.BasicROM, 32, EBankAccess::ReadWrite);
	CodeAnalysis.MapBank(BankIds.OSROM, 48, EBankAccess::ReadWrite);

	// setup code analysis
	CodeAnalysis.Init(this);
	CodeAnalysis.Config.bShowBanks = true;
	CodeAnalysis.ViewState[0].Enabled = true;	// always have first view enabled

	return true;
}

void FBBCEmulator::Shutdown()
{
	FEmuBase::Shutdown();
}


void FBBCEmulator::DrawEmulatorUI()
{
}

void FBBCEmulator::Tick()
{
	FEmuBase::Tick();


	// Draw UI
	DrawDockingView();
}

void FBBCEmulator::Reset()
{
	FEmuBase::Reset();
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
}	

void FBBCEmulator::OnKeyDown(int keyCode)
{
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

bool FBBCEmulator::LoadProject(FProjectConfig* pConfig, bool bLoadGame)
{
	return false;
}

bool FBBCEmulator::SaveProject(void)
{
	return false;
}
