#include "ArcadeZ80.h"

#define NOMINMAX

#include "ArcadeZ80Config.h"
#include <Util/FileUtil.h>
#include <CodeAnalyser/UI/CodeAnalyserUI.h>
#include <imgui.h>
#include <CodeAnalyser/CodeAnalysisJson.h>
#include <CodeAnalyser/CodeAnalysisState.h>

#include <Debug/DebugLog.h>
#include <algorithm>
#include "SaveGame.h"
#include "CodeAnalyser/UI/OverviewViewer.h"
#include "CodeAnalyser/UI/CharacterMapViewer.h"
#include "CodeAnalyser/UI/GlobalsViewer.h"
#include "TimePilot/TimePilotMachine.h"
#include "ArcadeZ80MCP.h"


// TODO: Load Arcade Z80 binaries

extern void DoM65C02Test();	// bit of a hack to run the M65C02 test suite

const char* kGlobalConfigFilename = "GlobalConfig.json";
const std::string kAppTitle = "Arcade Z80 Analyser";

void SetWindowTitle(const char* pTitle);

bool InitAsmExporters(FArcadeZ80* Emu);


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

	// This is where we add the viewers we want
	AddViewer(new FOverviewViewer(this));	// TODO: put in base class?
	pCharacterMapViewer = new FCharacterMapViewer(this);
	AddViewer(pCharacterMapViewer);
	pCharacterMapViewer->SetGridSize(32, 32);
	pCharacterMapViewer->SetGridStride(32);

	// setup machine
	FArcadeZ80MachineDesc desc = {};

	desc.pCodeAnalysis = &CodeAnalysis;

	// setup debug hook
	desc.Debug.callback.func = DebugCB;
	desc.Debug.callback.user_data = this;
	desc.Debug.stopped = CodeAnalysis.Debugger.GetDebuggerStoppedPtr();

	pMachine = new FTimePilotMachine;
	pMachine->Init(desc);
	pMachine->SetupCodeAnalysisForMachine();

	//Display.Init(this);

	CPUType = ECPUType::Z80;
	SetNumberDisplayMode(ENumberDisplayMode::HexAitch);

	// For ASM exporting - ROM area of TimePilot
	InitAsmExporters(this);
	ExportStartAddress = 0x0000;
	ExportEndAddress = 0x5FFF;

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

		pArcadeZ80Config = new FArcadeZ80ProjectConfig;

		pArcadeZ80Config->Name = "TimePilot";

		const std::string root = pGlobalConfig->WorkspaceRoot + pArcadeZ80Config->Name + "/";
		const std::string configFName = root + "Config.json";
		LoadGameConfigFromFile(*pArcadeZ80Config, configFName.c_str());
	}

	LoadProject(pArcadeZ80Config, true);

	// Setup Debugger
	FDebugger& debugger = CodeAnalysis.Debugger;
	//debugger.RegisterEventType((int)EEventType::None, "None", 0);
	debugger.RegisterEventType((int)EEventType::WriteVideoRAM, "Video RAM Write", 0xff0000ff, nullptr, nullptr);
	debugger.RegisterEventType((int)EEventType::WriteColorRAM, "Color RAM Write", 0xff00ffff, nullptr, nullptr);
	debugger.RegisterEventType((int)EEventType::WriteSpriteRAM, "Sprite RAM Write", 0xffff00ff, nullptr, nullptr);
	debugger.RegisterEventType((int)EEventType::ReadVideoScanline, "Video Scanline Read", 0xffffff00, nullptr, nullptr);
	debugger.RegisterEventType((int)EEventType::ReadDSW2, "DSW2 Read", 0xffff00ff, nullptr, nullptr);
	debugger.RegisterEventType((int)EEventType::ReadIN0, "IN0 Read", 0xff00ff00, nullptr, nullptr);
	debugger.RegisterEventType((int)EEventType::ReadIN1, "IN1 Read", 0xff0000ff, nullptr, nullptr);
	debugger.RegisterEventType((int)EEventType::ReadIN2, "IN2 Read", 0xff00ffff, nullptr, nullptr);
	debugger.RegisterEventType((int)EEventType::ReadDSW1, "DSW1 Read", 0xffffff00, nullptr, nullptr);
	debugger.RegisterEventType((int)EEventType::SendAudioCommand, "Send Audio Command", 0xffff00ff, nullptr, nullptr);
	debugger.RegisterEventType((int)EEventType::WatchdogReset, "Watchdog Reset", 0xff00ff00, nullptr, nullptr);
	debugger.RegisterEventType((int)EEventType::InterruptEnable, "Interrupt Enable", 0xff0000ff, nullptr, nullptr);
	debugger.RegisterEventType((int)EEventType::FlipScreen, "Flip Screen", 0xff00ffff, nullptr, nullptr);
	debugger.RegisterEventType((int)EEventType::TriggerAudioInterrupt, "Trigger Audio Interrupt", 0xffffff00, nullptr, nullptr);
	debugger.RegisterEventType((int)EEventType::VideoEnable, "Video Enable", 0xffff00ff, nullptr, nullptr);
	debugger.RegisterEventType((int)EEventType::CoinCounter1, "Coin Counter 1", 0xff00ff00, nullptr, nullptr);
	debugger.RegisterEventType((int)EEventType::CoinCounter2, "Coin Counter 2", 0xff0000ff, nullptr, nullptr);

	// Because all Arcade games are ROM based, enable ROM-based globals by default
	pGlobalsViewer->ShowROMLabels(true);

	InitMCPServer();

	return true;
}

void FArcadeZ80::SetupCodeAnalysisLabels()
{
}



bool FArcadeZ80::LoadBinaries(void)
{
	// TODO: load ROM binaries

	return true;
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

	pMachine->Shutdown();
	delete pMachine;

	FEmuBase::Shutdown();
}

void DrawDebugUI(FArcadeZ80Debug& debugInfo)
{
}

void FArcadeZ80::DrawEmulatorUI()
{
	if (ImGui::Begin("ArcadeZ80 Viewer"))
	{
		pMachine->DrawUI();
		DrawDebugUI(Debug);
	}
	ImGui::End();

	if (ImGui::Begin("ArcadeZ80 Debug"))
	{
		pMachine->DrawDebugUI();
	}
	ImGui::End();

}

void FArcadeZ80::Tick()
{
	FEmuBase::Tick();
		
	//Display.Tick();
	pMachine->Update();

	//CodeAnalysis.OnMachineFrameStart();
	
	FDebugger& debugger = CodeAnalysis.Debugger;
	
	if (debugger.IsStopped() == false)
	{
		const float frameTime = (float)std::min(1000000.0f / ImGui::GetIO().Framerate, 32000.0f) * 1.0f;// speccyInstance.ExecSpeedScale;

		CodeAnalysis.OnFrameStart();
		//StoreRegisters_6502(CodeAnalysis);

		pMachine->Exec((uint32_t)std::max(static_cast<uint32_t>(frameTime), uint32_t(1)));

		CodeAnalysis.OnFrameEnd();
	}

	//CodeAnalysis.OnMachineFrameEnd();

	// Draw UI
	DrawDockingView();

	UpdateMCPServer();
}



void FArcadeZ80::Reset()
{
	FEmuBase::Reset();

	pMachine->Reset();
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
		pMachine->SaveSnapshot(fp); // save the machine state

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
		bSuccess = pMachine->LoadSnapshot(fp, versionNo); // load the machine state
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

	CodeAnalysis.Debugger.RegisterNewStackPointer(pMachine->CPU.sp, FAddressRef());

	// some extra initialisation for creating new analysis from snapshot
	//if (bLoadGameData == false)
	{
		FAddressRef initialPC = CodeAnalysis.AddressRefFromPhysicalAddress(pMachine->CPU.pc);
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
	ExportAnalysisJson(CodeAnalysis, analysisJsonFName.c_str(), EExportAnalysis::Both);
	ExportAnalysisState(CodeAnalysis, analysisStateFName.c_str(), true);

	return false;
}

uint64_t FArcadeZ80::OnCPUTick(uint64_t pins)
{
	FCodeAnalysisState& state = CodeAnalysis;
	FDebugger& debugger = CodeAnalysis.Debugger;
	z80_t& cpu = pMachine->CPU;
	const uint16_t pc = GetPC().Address;
	static uint64_t lastTickPins = 0;
	const uint64_t risingPins = pins & (pins ^ lastTickPins);
	lastTickPins = pins;
	static uint16_t lastScanlinePos = 0;

	const uint16_t scanlinePos = (uint16_t)pMachine->VCounter;
	
	
	// trigger frame events on scanline pos
	if (scanlinePos != lastScanlinePos)
	{
		if (scanlinePos == 0)	// first scanline
			CodeAnalysis.OnMachineFrameStart();
		if (scanlinePos == 256)	// last scanline
			CodeAnalysis.OnMachineFrameEnd();
	}
	

	lastScanlinePos = scanlinePos;

	/* memory and IO requests */
	if (pins & Z80_MREQ)
	{
		/* a memory request machine cycle
			FIXME: 'contended memory' accesses should inject wait states
		*/
		const uint16_t addr = Z80_GET_ADDR(pins);
		const uint8_t value = Z80_GET_DATA(pins);
		if (pins & Z80_RD)
		{
			if (risingPins & Z80_INT)	// check if in interrupt - could this be done in the shared code analysis?
			{
				// TODO: read is to fetch interrupt handler address
				//LOGINFO("Interrupt Handler at: %x", value);
				const uint8_t im = cpu.im;

				if (im == 2)
				{
					const uint8_t i = cpu.i;	// I register has high byte of interrupt vector
					const uint16_t interruptVector = (i << 8) | 0xff;//value;
					const uint16_t interruptHandler = state.CPUInterface->ReadWord(interruptVector);
					bHasInterruptHandler = true;
					InterruptHandlerAddress = interruptHandler;
				}

			}
			else
			{
				if (state.bRegisterDataAccesses)
					RegisterDataRead(state, pc, addr);

				if(addr == 0xC000)
					debugger.RegisterEvent((uint8_t)EEventType::ReadVideoScanline, state.AddressRefFromPhysicalAddress(pc), addr, value, scanlinePos);
				else if (addr == 0xC200)
					debugger.RegisterEvent((uint8_t)EEventType::ReadDSW2, state.AddressRefFromPhysicalAddress(pc), addr, value, scanlinePos);
				else if (addr == 0xC300)
					debugger.RegisterEvent((uint8_t)EEventType::ReadIN0, state.AddressRefFromPhysicalAddress(pc), addr, value, scanlinePos);
				else if (addr == 0xC320)
					debugger.RegisterEvent((uint8_t)EEventType::ReadIN1, state.AddressRefFromPhysicalAddress(pc), addr, value, scanlinePos);
				else if (addr == 0xC340)
					debugger.RegisterEvent((uint8_t)EEventType::ReadIN2, state.AddressRefFromPhysicalAddress(pc), addr, value, scanlinePos);
				else if (addr == 0xC360)
					debugger.RegisterEvent((uint8_t)EEventType::ReadDSW1, state.AddressRefFromPhysicalAddress(pc), addr, value, scanlinePos);

			}
		}
		else if (pins & Z80_WR)
		{
			if (state.bRegisterDataAccesses)
				RegisterDataWrite(state, pc, addr, value);
			const FAddressRef addrRef = state.AddressRefFromPhysicalAddress(addr);
			const FAddressRef pcAddrRef = state.AddressRefFromPhysicalAddress(pc);
			state.SetLastWriterForAddress(addr, pcAddrRef);

			if (addr >= 0xA400 && addr <= 0xA7FF)
				debugger.RegisterEvent((uint8_t)EEventType::WriteVideoRAM, pcAddrRef, addr, value, scanlinePos);
			else if (addr >= 0xA000 && addr < 0xA3FF)
				debugger.RegisterEvent((uint8_t)EEventType::WriteColorRAM, pcAddrRef, addr, value, scanlinePos);
			else if (addr >= 0xB000 && addr < 0xB100)
				debugger.RegisterEvent((uint8_t)EEventType::WriteSpriteRAM, pcAddrRef, addr, value, scanlinePos);
			else if (addr >= 0xB400 && addr < 0xB500)
				debugger.RegisterEvent((uint8_t)EEventType::WriteSpriteRAM, pcAddrRef, addr, value, scanlinePos);
			else if(addr == 0xC000)
				debugger.RegisterEvent((uint8_t)EEventType::SendAudioCommand, pcAddrRef, addr, value, scanlinePos);
			else if (addr == 0xC200)
				debugger.RegisterEvent((uint8_t)EEventType::WatchdogReset, pcAddrRef, addr, value, scanlinePos);
			else if (addr == 0xC300)
				debugger.RegisterEvent((uint8_t)EEventType::InterruptEnable, pcAddrRef, addr, value, scanlinePos);
			else if (addr == 0xC302)
				debugger.RegisterEvent((uint8_t)EEventType::FlipScreen, pcAddrRef, addr, value, scanlinePos);
			else if (addr == 0xC304)
				debugger.RegisterEvent((uint8_t)EEventType::TriggerAudioInterrupt, pcAddrRef, addr, value, scanlinePos);
			else if (addr == 0xC308)
				debugger.RegisterEvent((uint8_t)EEventType::VideoEnable, pcAddrRef, addr, value, scanlinePos);
			else if (addr == 0xC30A)
				debugger.RegisterEvent((uint8_t)EEventType::CoinCounter1, pcAddrRef, addr, value, scanlinePos);
			else if (addr == 0xC30C)
				debugger.RegisterEvent((uint8_t)EEventType::CoinCounter2, pcAddrRef, addr, value, scanlinePos);
		}
	}

	// Handle IO operations
	if (pins & Z80_IORQ)
	{
		const FAddressRef pcAddrRef = state.AddressRefFromPhysicalAddress(pc);
		const uint8_t data = Z80_GET_DATA(pins);
		const uint16_t addr = Z80_GET_ADDR(pins);

		//IOAnalysis.IOHandler(pc, pins);

		if (pins & Z80_RD)
		{
			// TODO: Port reads
		}
		else if (pins & Z80_WR)
		{
			// TODO: Port writes
			
		}
	}

	InstructionsTicks++;

	const bool bNewOp = z80_opdone(&pMachine->CPU);

	if (bNewOp)
	{
		OnInstructionExecuted(InstructionsTicks, pins);
		InstructionsTicks = 0;
	}

	CodeAnalysis.OnCPUTick(pins);

	return pins;
}

void	FArcadeZ80::OnInstructionExecuted(int ticks, uint64_t pins)
{
	FCodeAnalysisState& state = CodeAnalysis;
	const uint16_t pc = pins & 0xffff;	// set PC to pc of instruction just executed

	RegisterCodeExecuted(state, pc, PreviousPC);
	//MemoryHandlerTrapFunction(pc, ticks, pins, this);

	PreviousPC = pc;
}


