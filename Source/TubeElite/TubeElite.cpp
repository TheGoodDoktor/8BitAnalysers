#include "TubeElite.h"

#include "TubeEliteConfig.h"
#include "TubeCommands.h"
#include <Util/FileUtil.h>
#include <CodeAnalyser/UI/CodeAnalyserUI.h>
#include <imgui.h>
#include <CodeAnalyser/CodeAnalysisJson.h>
#include <CodeAnalyser/CodeAnalysisState.h>

#include <Debug/DebugLog.h>
#include <algorithm>

// TODO: Load Elite binaries
// use this a a guide: https://elite.bbcelite.com/6502sp/all/bcfs.html


const char* kGlobalConfigFilename = "GlobalConfig.json";
const std::string kAppTitle = "Tube Elite";

void SetWindowTitle(const char* pTitle);


void DebugCB(void* user_data, uint64_t pins)
{
	FTubeElite* pTubeElite = (FTubeElite*)user_data;
	pTubeElite->OnCPUTick(pins);
}


FTubeElite::FTubeElite()
{
}

bool FTubeElite::Init(const FEmulatorLaunchConfig& launchConfig)
{
	if (FEmuBase::Init(launchConfig) == false)
		return false;

	SetWindowTitle(kAppTitle.c_str());

	LaunchConfig = *((FTubeEliteLaunchConfig *)&launchConfig);

	// Initialise Emulator
	pConfig = new FTubeEliteConfig();
	pGlobalConfig = pConfig;
	pGlobalConfig->Load(kGlobalConfigFilename);
	CodeAnalysis.SetGlobalConfig(pGlobalConfig);

	LoadFont();

	// setup machine
	FTubeEliteMachineDesc desc = {};

	// setup debug hook
	desc.Debug.callback.func = DebugCB;
	desc.Debug.callback.user_data = this;
	desc.Debug.stopped = CodeAnalysis.Debugger.GetDebuggerStoppedPtr();
	desc.pTubeDataHandler = this;

	Machine.Init(desc);

	Display.Init(this);

	CPUType = ECPUType::M65C02;
	SetNumberDisplayMode(ENumberDisplayMode::HexAmpersand);

	// Set up memory banks
	RamBankId = CodeAnalysis.CreateBank("RAM", 64, Machine.RAM, false, 0x0000, true);					// RAM - $0000 - $FFFF - pages 0-63 - 64K

	// map in banks
	CodeAnalysis.MapBank(RamBankId, 0, EBankAccess::ReadWrite);

	// setup code analysis
	CodeAnalysis.Init(this);
	CodeAnalysis.Config.bShowBanks = false;
	CodeAnalysis.ViewState[0].Enabled = true;	// always have first view enabled

	CodeAnalysis.Debugger.Break();
	FTubeEliteProjectConfig* pTubeEliteConfig = nullptr;

	if (LaunchConfig.bBBCBasic)
	{
		// TODO: load BBC basic ROM binaries
		pTubeEliteConfig = CreateNewBBCBasicConfig();
	}
	else
	{
		// TODO: load in RAM image
		if (LoadBinaries() == false)
		{
			LOGERROR("Failed to load Tube Elite binaries.");
			return false;
		}
		pTubeEliteConfig = CreateNewTubeEliteConfig();
	}

	LoadProject(pTubeEliteConfig, true);

	// Execute from 0x10D4
	const uint16_t startAddress = 0x10D4; // start address for Tube Elite
	//Machine.cpu.PC = startAddress; // set the program counter to the start address
	//CodeAnalysis.Debugger.SetPC(CodeAnalysis.AddressRefFromPhysicalAddress(startAddress));

	// hack the reset vector
	//Machine.RAM[0xFFFC] = startAddress & 255; 
	//Machine.RAM[0xFFFD] = startAddress >> 8;

	// hack checksum routine
	Machine.RAM[0x6BFA] = 0xEA;
	Machine.RAM[0x6BFB] = 0xEA;

	Machine.Tube.HostWriteRegister(ETubeRegister::R2, 0x00);	// don't set high bit - language
	//Machine.Tube.HostWriteRegister(ETubeRegister::R2,0x80);	// set high bit - code
	return true;
}

void FTubeElite::SetupCodeAnalysisLabels()
{
}

#define LOAD_BIG_BINARY 1

std::vector<std::string> g_BinaryFiles = 
{
	"Bin/ELTA.bin",
	"Bin/ELTB.bin",
	"Bin/ELTC.bin",
	"Bin/ELTD.bin",
	"Bin/ELTE.bin",
	"Bin/ELTF.bin",
	"Bin/ELTG.bin",
	"Bin/ELTH.bin",
	"Bin/ELTI.bin",
	"Bin/ELTJ.bin",
	"Bin/WORDS.bin",
	"Bin/SHIPS.bin",
};

bool FTubeElite::LoadBinaries(void)
{
	uint16_t loadAddress = 0x1000; // default load address for Tube Elite binaries
	uint16_t shipsAddress = 0xd000; // default load address for Ships binary
	size_t size = 0;
	void* pData = nullptr;

#if LOAD_BIG_BINARY
	// Load a single big binary file
	const char* bigBinaryFile = "Bin/P.CODE.unprot.bin";
	const char* shipsBinaryFile = "Bin/SHIPS.bin";
	pData = LoadBinaryFile(GetBundlePath(bigBinaryFile), size);
	if (pData == nullptr || size == 0)
	{
		LOGERROR("Failed to load Tube Elite binary.");
		return false;
	}
	assert((loadAddress + size) <= 0x10000); // Ensure we don't overflow the RAM
	memcpy(Machine.RAM + loadAddress, pData, size); // copy binary data to RAM
	LOGINFO("Loaded Tube Elite binary: %s, size: %zu bytes, load address: $%04X - $%04X", bigBinaryFile,  size, loadAddress, loadAddress + size - 1);
	free(pData);

	// Load Ships binary
	pData = LoadBinaryFile(GetBundlePath(shipsBinaryFile), size);
	if (pData == nullptr || size == 0)
	{
		LOGERROR("Failed to load Tube Elite Ships binary.");
		return false;
	}
	assert((shipsAddress + size) <= 0x10000); // Ensure we don't overflow the RAM
	memcpy(Machine.RAM + shipsAddress, pData, size); // copy Ships binary data to RAM
	LOGINFO("Loaded Tube Ships binary: %s, size: %zu bytes, load address: $%04X - $%04X", shipsBinaryFile, size, shipsAddress, shipsAddress + size - 1);
	//loadAddress += (uint16_t)size; // increment load address for next binary
#else
	for (const std::string& fname : g_BinaryFiles)
	{
		pData = LoadBinaryFile(fname.c_str(), size);
		if (pData == nullptr || size == 0)
		{
			LOGERROR("Failed to load Tube Elite binary: %s", fname.c_str());
			return false;
		}
		
		if (size > 0)
		{
			memcpy(Machine.ram + loadAddress, pData, size); // copy binary data to RAM
			LOGINFO("Loaded Tube Elite binary: %s, size: %zu bytes, load address: $%04X - $%04X", fname.c_str(), size, loadAddress, loadAddress + size - 1);
			loadAddress += (uint16_t)size; // increment load address for next binary
		}

		free(pData);
	}
#endif
	// Load Tube ROM
	pData = LoadBinaryFile(GetBundlePath("6502Tube.rom"), size);
	if (pData == nullptr || size == 0)
	{
		LOGERROR("Failed to load Tube ROM.");
		return false;
	}

	assert((0xf800 + size) <= 0x10000); // Ensure we don't overflow the RAM
	memcpy(Machine.RAM + 0xf800, pData, size);
	free(pData);

	return true;
}

void FTubeElite::Shutdown()
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


void FTubeElite::DrawEmulatorUI()
{
	if (ImGui::Begin("Tube Elite Viewer"))
	{
		
	}
	ImGui::End();

	Display.DrawUI();
}

void FTubeElite::Tick()
{
	FEmuBase::Tick();

	FDebugger& debugger = CodeAnalysis.Debugger;
	
	if (debugger.IsStopped() == false)
	{
		const float frameTime = (float)std::min(1000000.0f / ImGui::GetIO().Framerate, 32000.0f) * 1.0f;// speccyInstance.ExecSpeedScale;

		CodeAnalysis.OnFrameStart();
		//StoreRegisters_6502(CodeAnalysis);

		Machine.Exec((uint32_t)std::max(static_cast<uint32_t>(frameTime), uint32_t(1)));

		CodeAnalysis.OnFrameEnd();
	}

	Display.Tick();

	// Draw UI
	DrawDockingView();
}

// Tube comms handling

bool FTubeElite::HandleIncomingByte(ETubeRegister reg, uint8_t val)
{
	switch (reg)
	{
		case ETubeRegister::R1:
			Display.ProcessVDUChar(val); // process the character for display
			break;
		case ETubeRegister::R2:
			LOGINFO("Received R2 data: 0x%02X", val);
			ProcessTubeCommandByte(val);
			break;

		case ETubeRegister::R3:
			LOGINFO("Received R3 data: 0x%02X", val);
			break;
		case ETubeRegister::R4:
			LOGINFO("Received R4 data: 0x%02X", val);
			break;
	}
	
	return true;
}

void FTubeElite::ProcessTubeCommandByte(uint8_t cmdByte)
{
	if (pCurrentCommand == nullptr)
	{
		pCurrentCommand = CreateTubeCommand(this, cmdByte);
	}
	else
	{
		pCurrentCommand->ReceiveParamByte(cmdByte);
	}
}

void FTubeElite::PollTubeCommand(void)
{
	if (pCurrentCommand != nullptr)
	{
		if (pCurrentCommand->IsReady())
		{
			pCurrentCommand->Execute();
		}

		if (pCurrentCommand->IsComplete())
		{
			delete pCurrentCommand;
			pCurrentCommand = nullptr;
		}
	}

}


void FTubeElite::Reset()
{
	FEmuBase::Reset();

	Machine.Reset();
}

void FTubeElite::FixupAddressRefs()
{
}

void FTubeElite::FileMenuAdditions()
{
}

void FTubeElite::SystemMenuAdditions()
{
}

void FTubeElite::OptionsMenuAdditions()
{
}

void FTubeElite::WindowsMenuAdditions()
{
}

void FTubeElite::OnKeyUp(int keyCode)
{
	//bbc_key_up(&BBCEmu, keyCode);
}

void FTubeElite::OnKeyDown(int keyCode)
{
	//bbc_key_down(&BBCEmu, keyCode);
}

void FTubeElite::OnChar(int charCode)
{
}

void FTubeElite::OnGamepadUpdated(int mask)
{
}



bool FTubeElite::LoadEmulatorFile(const FEmulatorFile* pEmuFile)
{
	return false;
}

bool FTubeElite::NewProjectFromEmulatorFile(const FEmulatorFile& gameSnapshot)
{
	return false;
}
#if 1

static const uint32_t kMachineStateMagic = 0xFaceCafe;

bool FTubeElite::SaveMachineState(const char* fname)
{
	// save game snapshot
	FILE* fp = fopen(fname, "wb");
	if (fp != nullptr)
	{
		const uint32_t versionNo = TUBE_ELITE_SNAPSHOT_VERSION;
		fwrite(&kMachineStateMagic, sizeof(uint32_t), 1, fp);
		fwrite(&versionNo, sizeof(uint32_t), 1, fp);
		Machine.SaveSnapshot(fp); // save the machine state
		//fwrite(&g_SaveSlot, sizeof(tube_elite_t), 1, fp);

		fclose(fp);
		return true;
	}

	return false;
}

bool FTubeElite::LoadMachineState(const char* fname)
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
#endif

bool FTubeElite::LoadProject(FProjectConfig* pProjectConfig, bool bLoadGameData)
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

	CodeAnalysis.Debugger.RegisterNewStackPointer(Machine.CPU.S, FAddressRef());

	// some extra initialisation for creating new analysis from snapshot
	if (bLoadGameData == false)
	{
		FAddressRef initialPC = CodeAnalysis.AddressRefFromPhysicalAddress(Machine.CPU.PC);
		SetItemCode(CodeAnalysis, initialPC);
		CodeAnalysis.Debugger.SetPC(initialPC);
	}

	SetupCodeAnalysisLabels();

	return true;
}

bool FTubeElite::SaveProject(void)
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

uint64_t FTubeElite::OnCPUTick(uint64_t pins)
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
#if 0
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
	const bool bReadingInstruction = addr == m65C02_pc(&Machine.CPU) - 1;

	if ((pins & M6502_SYNC) == 0) // not for instruction fetch
	{
		if (pins & M6502_RW)
		{
			if (state.bRegisterDataAccesses)
				RegisterDataRead(CodeAnalysis, pc, addr);   // this gives false positives on indirect addressing e.g. STA ($0a),y

			const uint32_t page = addr >> 8;
			// Tube registers
			if (addr >= 0xFEF8 && addr <= 0xFEFF)
			{
				// TODO: Register read	
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

			FCodeInfo* pCodeWrittenTo = CodeAnalysis.GetCodeInfoForAddress(addrRef);
			if (pCodeWrittenTo != nullptr && pCodeWrittenTo->bSelfModifyingCode == false)
				pCodeWrittenTo->bSelfModifyingCode = true;

			// Tube registers
			if (addr >= 0xFEF8 && addr <= 0xFEFF)
			{
				// TODO: Register write	
			}
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


// Keyboard stuff - move to a separate file?

struct FKeyVal
{
	int NoShift;
	int Shifted;
};

static std::map<ImGuiKey, FKeyVal> g_BBCKeysLUT =
{
	{ImGuiKey_Space,		{BBC_KEYCODE_SPACE, BBC_KEYCODE_SPACE}},
	{ImGuiKey_Enter,		{BBC_KEYCODE_ENTER, BBC_KEYCODE_ENTER}},
	{ImGuiKey_Escape,		{BBC_KEYCODE_ESCAPE, BBC_KEYCODE_ESCAPE}},
	{ImGuiKey_LeftCtrl,		{BBC_KEYCODE_CTRL, BBC_KEYCODE_CTRL}},
	{ImGuiKey_RightCtrl,	{BBC_KEYCODE_CTRL, BBC_KEYCODE_CTRL}},
	{ImGuiKey_LeftShift,	{BBC_KEYCODE_SHIFT, BBC_KEYCODE_SHIFT}},
	{ImGuiKey_RightShift,	{BBC_KEYCODE_SHIFT, BBC_KEYCODE_SHIFT}},
	{ImGuiKey_Backspace,	{BBC_KEYCODE_BACKSPACE, BBC_KEYCODE_BACKSPACE}},
	{ImGuiKey_LeftArrow,	{BBC_KEYCODE_CURSOR_LEFT, BBC_KEYCODE_CURSOR_LEFT}},
	{ImGuiKey_RightArrow,	{BBC_KEYCODE_CURSOR_RIGHT, BBC_KEYCODE_CURSOR_RIGHT}},
	{ImGuiKey_UpArrow,		{BBC_KEYCODE_CURSOR_UP, BBC_KEYCODE_CURSOR_UP}},
	{ImGuiKey_DownArrow,	{BBC_KEYCODE_CURSOR_DOWN, BBC_KEYCODE_CURSOR_DOWN}},
	{ImGuiKey_CapsLock,		{BBC_KEYCODE_CAPS_LOCK, BBC_KEYCODE_CAPS_LOCK}},
	{ImGuiKey_Apostrophe,	{'\'', '@'}},
	{ImGuiKey_Comma,		{',', '<'}},
	{ImGuiKey_Minus,		{'-', '_'}},
	{ImGuiKey_Period,		{'.', '>'}},
	{ImGuiKey_Slash,		{'/', '?'}},
	{ImGuiKey_Semicolon,	{';', ':'}},
	{ImGuiKey_Equal,		{'=', '+'}},
	{ImGuiKey_LeftBracket,	{'[', '{'}},
	{ImGuiKey_Backslash,	{'\\', '|'}},
	{ImGuiKey_RightBracket,	{']', '}'}},
	{ImGuiKey_GraveAccent,	{'`', '~'}},
};

int BBCKeyFromImGuiKey(ImGuiKey key)
{
	uint32_t bbcKey = 0;
	bool isShifted = ImGui::GetIO().KeyShift;

	if (key >= ImGuiKey_0 && key <= ImGuiKey_9)
	{
		if (isShifted)
		{
			// Handle shifted number keys (e.g., '!' for '1')
			switch (key)
			{
			case ImGuiKey_1: bbcKey = (uint8_t)'!'; break;
			case ImGuiKey_2: bbcKey = (uint8_t)'"'; break;
			case ImGuiKey_3: bbcKey = (uint8_t)'£'; break;
			case ImGuiKey_4: bbcKey = (uint8_t)'$'; break;
			case ImGuiKey_5: bbcKey = (uint8_t)'%'; break;
			case ImGuiKey_6: bbcKey = (uint8_t)'^'; break;
			case ImGuiKey_7: bbcKey = (uint8_t)'&'; break;
			case ImGuiKey_8: bbcKey = (uint8_t)'*'; break;
			case ImGuiKey_9: bbcKey = (uint8_t)'('; break;
			case ImGuiKey_0: bbcKey = (uint8_t)')'; break;
			}
		}
		else
		{
			bbcKey = '0' + (key - ImGuiKey_0);
		}
	}
	else if (key >= ImGuiKey_A && key <= ImGuiKey_Z)
	{
		bbcKey = isShifted ? 'A' + (key - ImGuiKey_A) : 'a' + (key - ImGuiKey_A);
	}
	else
	{
		auto keyIt = g_BBCKeysLUT.find(key);
		if (keyIt != g_BBCKeysLUT.end())
		{
			bbcKey = isShifted ? keyIt->second.Shifted : keyIt->second.NoShift;
		}
	}
	return bbcKey;
}
