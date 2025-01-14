#include "BBCEmulator.h"

#include "BBCConfig.h"
#include <Util/FileUtil.h>
#include <imgui.h>

const char* kGlobalConfigFilename = "GlobalConfig.json";
const std::string kAppTitle = "BBC Analyser";


const char* kROMAnalysisFilename = "BBCRomsAnalysis.json";

void SetWindowTitle(const char* pTitle);


void DebugCB(void* user_data, uint64_t pins)
{
	FBBCEmulator* pBBCEmu = (FBBCEmulator*)user_data;
	pBBCEmu->OnCPUTick(pins);
}

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
	
	// setup debug hook
	desc.debug.callback.func = DebugCB;
	desc.debug.callback.user_data = this;
	desc.debug.stopped = CodeAnalysis.Debugger.GetDebuggerStoppedPtr();

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

	CodeAnalysis.Debugger.Break();

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

	FDebugger& debugger = CodeAnalysis.Debugger;

	//TODO: Display.Tick();

	if (debugger.IsStopped() == false)
	{
		const float frameTime = (float)std::min(1000000.0f / ImGui::GetIO().Framerate, 32000.0f) * 1.0f;// speccyInstance.ExecSpeedScale;

		CodeAnalysis.OnFrameStart();
		//StoreRegisters_6502(CodeAnalysis);

		bbc_exec(&BBCEmu, (uint32_t)std::max(static_cast<uint32_t>(frameTime), uint32_t(1)));

		CodeAnalysis.OnFrameEnd();
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
#if 0
	// trigger frame events on scanline pos
	static uint16_t lastScanlinePos = 0;
	const uint16_t scanlinePos = BBCEmu.vic.rs.v_count;
	if (scanlinePos != lastScanlinePos)
	{
		CodeAnalysis.Debugger.OnScanlineStart(scanlinePos);

		if (scanlinePos == 0)
			CodeAnalysis.OnMachineFrameStart();
		else if (scanlinePos == M6569_VTOTAL - 1)    // last scanline
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

			/*if (bIOMapped && (addr >> 12) == 0xd)
			{
				IOAnalysis.RegisterIORead(addr, GetPC());
				uint8_t readVal = 0;
				if (CartridgeManager.HandleIORead(addr, readVal))
				{
					M6502_SET_DATA(pins, readVal);
				}
			}*/
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

			/*if (bIOMapped && (addr >> 12) == 0xd)
			{
				IOAnalysis.RegisterIOWrite(addr, val, GetPC());
				IOMemBuffer[addr & 0xfff] = val;

				CartridgeManager.HandleIOWrite(addr, val);
			}*/

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