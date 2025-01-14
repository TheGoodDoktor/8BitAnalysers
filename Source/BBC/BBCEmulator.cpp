#include "BBCEmulator.h"

#include "BBCConfig.h"

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
