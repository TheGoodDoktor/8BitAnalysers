#include <imgui.h>

#include "PCEEmu.h"
//#include <cstdint>

#include "PCEConfig.h"
//#include "GameData.h"
//#include <ImGuiSupport/ImGuiTexture.h>
#include "Util/FileUtil.h"

//#include "CodeAnalyser/CodeAnalyser.h"
//#include "CodeAnalyser/UI/CodeAnalyserUI.h"

//#include "Debug/DebugLog.h"
//#include "Debug/ImGuiLog.h"
//#include <cassert>
//#include <Util/Misc.h>

#include "App.h"
#include <CodeAnalyser/CodeAnalysisState.h>
#include "CodeAnalyser/CodeAnalysisJson.h"
#include "PCEGameConfig.h"

const char* kGlobalConfigFilename = "GlobalConfig.json";
const std::string kAppTitle = "PCE Analyser";

uint8_t		FPCEEmu::ReadByte(uint16_t address) const
{
	return 0;
	//return mem_rd(const_cast<mem_t*>(&ZXEmuState.mem), address);
}

uint16_t	FPCEEmu::ReadWord(uint16_t address) const 
{
	return ReadByte(address) | (ReadByte(address + 1) << 8);
}

const uint8_t* FPCEEmu::GetMemPtr(uint16_t address) const 
{
	static uint8_t zeroByte = 0;
	return &zeroByte;
	//return mem_readptr(const_cast<mem_t*>(&ZXEmuState.mem), address);
}


void FPCEEmu::WriteByte(uint16_t address, uint8_t value)
{
	//mem_wr(&ZXEmuState.mem, address, value);
}


FAddressRef	FPCEEmu::GetPC(void) 
{
	return 0;
	//return CodeAnalysis.Debugger.GetPC();
} 

uint16_t	FPCEEmu::GetSP(void)
{
	return 0;
	//return ZXEmuState.cpu.sp;
}

void* FPCEEmu::GetCPUEmulator(void) const
{
	return 0;
	//return (void *)&ZXEmuState.cpu;
}

/*void* gfx_create_texture(int w, int h)
{
	return ImGui_CreateTextureRGBA(nullptr, w, h);
}

void gfx_update_texture(void* h, void* data, int data_byte_size)
{
	ImGui_UpdateTextureRGBA(h, (unsigned char *)data);
}

void gfx_destroy_texture(void* h)
{
	
}*/


bool FPCEEmu::Init(const FEmulatorLaunchConfig& config)
{
	FEmuBase::Init(config);
	
	const FPCELaunchConfig& PCELaunchConfig = (const FPCELaunchConfig&)config;
    
	SetWindowTitle(kAppTitle.c_str());
	SetWindowIcon(GetBundlePath("PCELogo.png"));

	// Initialise Emulator
	pGlobalConfig = new FPCEConfig();
    pGlobalConfig->Init();
	pGlobalConfig->Load(kGlobalConfigFilename);
	CodeAnalysis.SetGlobalConfig(pGlobalConfig);
	SetHexNumberDisplayMode(pGlobalConfig->NumberDisplayMode);
	SetNumberDisplayMode(pGlobalConfig->NumberDisplayMode);
	//CodeAnalysis.Config.CharacterColourLUT = FZXGraphicsView::GetColourLUT();
	
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
	//AddViewer(new FOverviewViewer(this));	
	//pGraphicsViewer = new FZXGraphicsViewer(this);
	//AddViewer(pGraphicsViewer);

	//PCEViewer.Init(this);

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
	}
	
	// Setup Debugger
	//FDebugger& debugger = CodeAnalysis.Debugger;
	//debugger.RegisterEventType((int)EEventType::ScreenPixWrite, "Screen Pixel Write", 0xff0000ff, nullptr, EventShowPixValue);
	//debugger.RegisterEventType((int)EEventType::ScreenAttrWrite, "Screen Attr Write", 0xff007fff, nullptr, EventShowAttrValue);
	//debugger.RegisterEventType((int)EEventType::KempstonJoystickRead, "Kempston Read", 0xff007f1f, IOPortEventShowAddress, IOPortEventShowValue);

	// Setup Memory Analyser
	//pMemoryAnalyser->AddROMArea(kROMStart, kROMEnd);
	//pMemoryAnalyser->SetScreenMemoryArea(kScreenPixMemStart, kScreenAttrMemEnd);

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

	// Add data regions for screen memory
	/*FDataRegion screenPixRegion;
	screenPixRegion.StartAddress = CodeAnalysis.AddressRefFromPhysicalAddress(kScreenPixMemStart);
	screenPixRegion.EndAddress = CodeAnalysis.AddressRefFromPhysicalAddress(kScreenPixMemEnd);
	CodeAnalysis.pDataRegions->AddRegion(screenPixRegion);

	FDataRegion screenAttrRegion;
	screenAttrRegion.StartAddress = CodeAnalysis.AddressRefFromPhysicalAddress(kScreenAttrMemStart);
	screenAttrRegion.EndAddress = CodeAnalysis.AddressRefFromPhysicalAddress(kScreenAttrMemEnd);
	CodeAnalysis.pDataRegions->AddRegion(screenAttrRegion);
*/
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
		FAddressRef initialPC = CodeAnalysis.AddressRefFromPhysicalAddress(0/*ZXEmuState.cpu.pc*/);
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
	const char* pFileName = fileName.c_str();

	/*switch (pSnapshot->Type)
	{
	case EEmuFileType::Z80:
		return LoadZ80File(this, pFileName);
	case EEmuFileType::SNA:
		return LoadSNAFile(this, pFileName);
	case EEmuFileType::TAP:
		return LoadTAPFile(this, pFileName);
	case EEmuFileType::TZX:
		return LoadTZXFile(this, pFileName);
	default:
		return false;
	}*/
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
	FEmuBase::Tick();

	FDebugger& debugger = CodeAnalysis.Debugger;

	//SpectrumViewer.Tick();

	if (debugger.IsStopped() == false)
	{
		const float frameTime = std::min(1000000.0f / ImGui::GetIO().Framerate, 32000.0f) /* * ExecSpeedScale*/;
		const uint32_t microSeconds = std::max(static_cast<uint32_t>(frameTime), uint32_t(1));

		CodeAnalysis.OnFrameStart();
		//StoreRegisters_Z80(CodeAnalysis);

		//ZXExeEmu(&ZXEmuState, microSeconds);
		
		CodeAnalysis.OnFrameEnd();
	}

	// Draw UI
	DrawDockingView();
}

void FPCEEmu::Reset()
{
	// todo
}

void FPCEEmu::OnEnterEditMode(void)
{
}

void FPCEEmu::OnExitEditMode(void)
{
}


void FPCEEmu::DrawEmulatorUI()
{
	const double timeMS = 1000.0f / ImGui::GetIO().Framerate;
	
	// show PCE window
	if (ImGui::Begin("PCE View"))
	{
		//SpectrumViewer.Draw();
	}
	ImGui::End();
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

/*ImTextureID	FPCEEmu::GetMachineSnapshotThumbnail(int snapshotNo) const
{
	if (snapshotNo > 0 && snapshotNo < kNoSnapshots)
	{
		const FSnapshot& snapshot = Snapshots[snapshotNo];
		if (snapshot.bValid == true)
			return snapshot.Thumbnail;
	}

	return 0;
}*/


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
