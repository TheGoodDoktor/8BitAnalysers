#include "ArcadeZ80.h"

#include "ArcadeZ80Config.h"
#include <Util/FileUtil.h>
#include <CodeAnalyser/UI/CodeAnalyserUI.h>
#include <imgui.h>
#include <CodeAnalyser/CodeAnalysisJson.h>
#include <CodeAnalyser/CodeAnalysisState.h>

#include <Debug/DebugLog.h>
#include <algorithm>
#include "SaveGame.h"

// TODO: Load Arcade Z80 binaries

extern void DoM65C02Test();	// bit of a hack to run the M65C02 test suite

const char* kGlobalConfigFilename = "GlobalConfig.json";
const std::string kAppTitle = "Arcade Z80 Analyser";

void SetWindowTitle(const char* pTitle);


void DebugCB(void* user_data, uint64_t pins)
{
	FArcadeZ80* pArcadeZ80 = (FArcadeZ80*)user_data;
	pArcadeZ80->OnCPUTick(pins);
}


FArcadeZ80::FArcadeZ80()
{
}

bool FArcadeZ80::Init(const FEmulatorLaunchConfig& launchConfig)
{
	if (FEmuBase::Init(launchConfig) == false)
		return false;

	SetWindowTitle(kAppTitle.c_str());

	LaunchConfig = *((FArcadeZ80LaunchConfig *)&launchConfig);

	// Initialise Emulator
	pConfig = new FArcadeZ80Config();
	pGlobalConfig = pConfig;
	pGlobalConfig->Load(kGlobalConfigFilename);
	CodeAnalysis.SetGlobalConfig(pGlobalConfig);

	LoadFont();

	// setup machine
	FArcadeZ80MachineDesc desc = {};

	// setup debug hook
	desc.Debug.callback.func = DebugCB;
	desc.Debug.callback.user_data = this;
	desc.Debug.stopped = CodeAnalysis.Debugger.GetDebuggerStoppedPtr();

	Machine.Init(desc);

	//Display.Init(this);

	CPUType = ECPUType::Z80;
	SetNumberDisplayMode(ENumberDisplayMode::HexAitch);

	// Set up memory banks
	RamBankId = CodeAnalysis.CreateBank("RAM", 64, Machine.RAM, false, 0x0000, true);					// RAM - $0000 - $FFFF - pages 0-63 - 64K

	// map in banks
	CodeAnalysis.MapBank(RamBankId, 0, EBankAccess::ReadWrite);

	// setup code analysis
	CodeAnalysis.Init(this);
	CodeAnalysis.Config.bShowBanks = false;
	CodeAnalysis.ViewState[0].Enabled = true;	// always have first view enabled

	CodeAnalysis.Debugger.Break();
	FArcadeZ80ProjectConfig* pArcadeZ80Config = nullptr;

	{
		// TODO: load in RAM image
		if (LoadBinaries() == false)
		{
			LOGERROR("Failed to load Arcade Z80 binaries.");
			return false;
		}
		pArcadeZ80Config = CreateNewArcadeZ80Config();
	}

	LoadProject(pArcadeZ80Config, true);


	return true;
}

void FArcadeZ80::SetupCodeAnalysisLabels()
{
}



bool FArcadeZ80::LoadBinaries(void)
{
	// TODO: load ROM binaries

	return false;
}

void FArcadeZ80::Shutdown()
{
	if (pCurrentProjectConfig != nullptr)
	{
		// Save Global Config - move to function?
		pGlobalConfig->LastGame = pCurrentProjectConfig->Name;
		SaveProject();
	}

	pGlobalConfig->Save(kGlobalConfigFilename);

	Machine.Shutdown();

	FEmuBase::Shutdown();
}

void DrawDebugUI(FArcadeZ80Debug& debugInfo)
{
}

void FArcadeZ80::DrawEmulatorUI()
{
	if (ImGui::Begin("ArcadeZ80 Viewer"))
	{
		DrawDebugUI(Debug);
		const uint16_t kLastSaveAddr = 0x1019; // last save game address
		const uint16_t kCurrentStatusAddr = 0x08A4;
	}
	ImGui::End();

	//Display.DrawUI();

}

void FArcadeZ80::Tick()
{
	FEmuBase::Tick();
		
	//Display.Tick();
	
	FDebugger& debugger = CodeAnalysis.Debugger;
	
	if (debugger.IsStopped() == false)
	{
		const float frameTime = (float)std::min(1000000.0f / ImGui::GetIO().Framerate, 32000.0f) * 1.0f;// speccyInstance.ExecSpeedScale;

		CodeAnalysis.OnFrameStart();
		//StoreRegisters_6502(CodeAnalysis);

		Machine.Exec((uint32_t)std::max(static_cast<uint32_t>(frameTime), uint32_t(1)));

		CodeAnalysis.OnFrameEnd();
	}



	// Draw UI
	DrawDockingView();
}



void FArcadeZ80::Reset()
{
	FEmuBase::Reset();

	Machine.Reset();
}

void FArcadeZ80::FixupAddressRefs()
{
}

void FArcadeZ80	::FileMenuAdditions()
{
}

void FArcadeZ80::SystemMenuAdditions()
{
}

void FArcadeZ80::OptionsMenuAdditions()
{
}

void FArcadeZ80::WindowsMenuAdditions()
{
}

void FArcadeZ80::OnKeyUp(int keyCode)
{
	//bbc_key_up(&BBCEmu, keyCode);
}

void FArcadeZ80::OnKeyDown(int keyCode)
{
	//bbc_key_down(&BBCEmu, keyCode);
}

void FArcadeZ80::OnChar(int charCode)
{
}

void FArcadeZ80::OnGamepadUpdated(int mask)
{
}



bool FArcadeZ80::LoadEmulatorFile(const FEmulatorFile* pEmuFile)
{
	return false;
}

bool FArcadeZ80::NewProjectFromEmulatorFile(const FEmulatorFile& gameSnapshot)
{
	return false;
}

static const uint32_t kMachineStateMagic = 0xFaceCafe;

bool FArcadeZ80::SaveMachineState(const char* fname)
{
	// save game snapshot
	FILE* fp = fopen(fname, "wb");
	if (fp != nullptr)
	{
		const uint32_t versionNo = ARCADEZ80_SNAPSHOT_VERSION;
		fwrite(&kMachineStateMagic, sizeof(uint32_t), 1, fp);
		fwrite(&versionNo, sizeof(uint32_t), 1, fp);
		Machine.SaveSnapshot(fp); // save the machine state

		fclose(fp);
		return true;
	}

	return false;
}

bool FArcadeZ80::LoadMachineState(const char* fname)
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
		bSuccess = Machine.LoadSnapshot(fp, versionNo); // load the machine state
	}
	fclose(fp);
	return bSuccess;
}

bool FArcadeZ80::LoadProject(FProjectConfig* pProjectConfig, bool bLoadGameData)
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

		FixupAddressRefs();
	}

	ReAnalyseCode(CodeAnalysis);
	GenerateGlobalInfo(CodeAnalysis);
	CodeAnalysis.SetAddressRangeDirty();

	// Start in break mode so the memory will be in its initial state. 
	CodeAnalysis.Debugger.Break();

	CodeAnalysis.Debugger.RegisterNewStackPointer(Machine.CPU.sp, FAddressRef());

	// some extra initialisation for creating new analysis from snapshot
	if (bLoadGameData == false)
	{
		FAddressRef initialPC = CodeAnalysis.AddressRefFromPhysicalAddress(Machine.CPU.pc);
		SetItemCode(CodeAnalysis, initialPC);
		CodeAnalysis.Debugger.SetPC(initialPC);
	}

	SetupCodeAnalysisLabels();

	return true;
}

bool FArcadeZ80::SaveProject(void)
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

	return false;
}

uint64_t FArcadeZ80::OnCPUTick(uint64_t pins)
{
	FCodeAnalysisState& state = CodeAnalysis;

	const uint16_t pc = GetPC().Address;

	// TODO: Z80 instruction analysis

	CodeAnalysis.OnCPUTick(pins);

	return pins;
}


