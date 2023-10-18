#include "C64Emulator.h"
#define NOMINMAX

//#define SOKOL_IMPL
#include "sokol_audio.h"
#include <ImGuiSupport/ImGuiTexture.h>

#include "c64-roms.h"
#include <Util/FileUtil.h>
#include <CodeAnalyser/UI/CodeAnalyserUI.h>

#include "C64Config.h"

const char* kGlobalConfigFilename = "GlobalConfig.json";
const std::string kAppTitle = "C64 Analyser";


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
    desc.audio.callback.user_data = nullptr;
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


bool FC64Emulator::Init()
{
	//SetWindowTitle(kAppTitle.c_str());
	//SetWindowIcon("SALogo.png");

	// Initialise Emulator
	pGlobalConfig = new FC64Config();
	pGlobalConfig->Load(kGlobalConfigFilename);
	CodeAnalysis.SetGlobalConfig(pGlobalConfig);

    //saudio_desc audiodesc;
    //memset(&audiodesc, 0, sizeof(saudio_desc));
    //saudio_setup(&audiodesc);

    Display.Init(&CodeAnalysis, this);

    // Setup C64 Emulator
    c64_joystick_type_t joy_type = C64_JOYSTICKTYPE_NONE;
    c64_desc_t desc = GenerateC64Desc(joy_type);
    c64_init(&C64Emu, &desc);

    // Setup C64 UI
    ui_c64_desc_t uiDesc;
    memset(&uiDesc, 0, sizeof(ui_c64_desc_t));
    uiDesc.c64 = &C64Emu;
    uiDesc.boot_cb = C64BootCallback;

    uiDesc.snapshot.load_cb = UISnapshotLoadCB;
    uiDesc.snapshot.save_cb = UISnapshotSaveCB;

    uiDesc.dbg_texture.create_cb = gfx_create_texture;
    uiDesc.dbg_texture.update_cb = gfx_update_texture;
    uiDesc.dbg_texture.destroy_cb = gfx_destroy_texture;
    uiDesc.dbg_keys.stop.keycode = ImGui::GetKeyIndex(ImGuiKey_Space);
    uiDesc.dbg_keys.stop.name = "F5";
    uiDesc.dbg_keys.cont.keycode = ImGuiKey_F5;
    uiDesc.dbg_keys.cont.name = "F5";
    uiDesc.dbg_keys.step_over.keycode = ImGuiKey_F6;
    uiDesc.dbg_keys.step_over.name = "F6";
    uiDesc.dbg_keys.step_into.keycode = ImGuiKey_F7;
    uiDesc.dbg_keys.step_into.name = "F7";
    uiDesc.dbg_keys.toggle_breakpoint.keycode = ImGuiKey_F9;
    uiDesc.dbg_keys.toggle_breakpoint.name = "F9";

	memset(&C64UI, 0, sizeof(C64UI));

    ui_c64_init(&C64UI, &uiDesc);

    CPUType = ECPUType::M6502;
    SetNumberDisplayMode(ENumberDisplayMode::HexDollar);

    // setup default memory configuration
    // RAM - $0000 - $9FFF - pages 0-39 - 40K

    // RAM - $C000 - $CFFF - pages 48-51 - 4k
    // IO System - %D000 - $DFFF - page 52-55 - 4k

    LowerRAMId = CodeAnalysis.CreateBank("LoRAM", 40, C64Emu.ram, true);
    HighRAMId = CodeAnalysis.CreateBank("HiRAM", 4, &C64Emu.ram[0xc000], true);
    IOAreaId = CodeAnalysis.CreateBank("IOArea", 4, IOMemBuffer, true);

    BasicROMId = CodeAnalysis.CreateBank("BasicROM", 8, C64Emu.rom_basic, true); // BASIC ROM - $A000-$BFFF - pages 40-47 - 8k
    RAMBehindBasicROMId = CodeAnalysis.CreateBank("RAMBehindBasicROM", 8, &C64Emu.ram[0xa000], true);

    KernelROMId = CodeAnalysis.CreateBank("KernelROM", 8, C64Emu.rom_kernal, true);   // Kernel ROM - $E000-$FFFF - pages 56-63 - 8k
    RAMBehindKernelROMId = CodeAnalysis.CreateBank("RAMBehindKernelROM", 8, &C64Emu.ram[0xe000], true);

    CharacterROMId = CodeAnalysis.CreateBank("CharacterROM", 4, C64Emu.rom_char, true); // Char ROM - %D000 - $DFFF - page 52-55 - 4k
    RAMBehindCharROMId = CodeAnalysis.CreateBank("RAMBehindCharROM", 4, &C64Emu.ram[0xd000], true);

    //ColourRAMId = CodeAnalysis.CreateBank("ColourRAM", 1, C64Emu.color_ram, true);  // Colour RAM - $D800

    // map in permanent banks
    CodeAnalysis.MapBank(LowerRAMId, 0);        // RAM - $0000 - $9FFF - pages 0-39 - 40K
    CodeAnalysis.MapBank(HighRAMId, 48);        // RAM - $C000 - $CFFF - pages 48-51 - 4k

    // TODO: Setup games list

    // TODO: Setup debugger

    // TODO: setup memory analyser

    IOAnalysis.Init(&CodeAnalysis);
    GraphicsViewer.Init(&CodeAnalysis, &C64Emu);

    // setup code analysis
    CodeAnalysis.Init(this);
	CodeAnalysis.Config.bShowBanks = true;
	CodeAnalysis.ViewState[0].Enabled = true;	// always have first view enabled
	SetupCodeAnalysisLabels();
	UpdateCodeAnalysisPages(0x7);
    
    GamesList.EnumerateGames(pGlobalConfig->PrgFolder.c_str());

    return true;
}

void FC64Emulator::SetupCodeAnalysisLabels()
{
    // Add IO Labels to code analysis
    FCodeAnalysisBank* pIOBank = CodeAnalysis.GetBank(IOAreaId);
    AddVICRegisterLabels(pIOBank->Pages[0]);  // Page $D000-$D3ff
    AddSIDRegisterLabels(pIOBank->Pages[1]);  // Page $D400-$D7ff
    pIOBank->Pages[2].SetLabelAtAddress("ColourRAM", ELabelType::Data, 0x0000);    // Colour RAM $D800
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
		CodeAnalysis.MapBank(RAMBehindBasicROMId, 40);          // RAM Under BASIC ROM - $A000-$BFFF - pages 40-47 - 8k
		CodeAnalysis.MapBank(RAMBehindCharROMId, 52);           // RAM Under Char ROM - %D000 - $DFFF - page 52-55 - 4k
		CodeAnalysis.MapBank(RAMBehindKernelROMId, 56);         // RAM Under Kernel ROM - $E000-$FFFF - pages 56-63 - 8k
    }
    else
    {
        /* A000..BFFF is either RAM-behind-BASIC-ROM or RAM */
        if ((cpuPort & (C64_CPUPORT_HIRAM | C64_CPUPORT_LORAM)) == (C64_CPUPORT_HIRAM | C64_CPUPORT_LORAM))
        {
			CodeAnalysis.MapBank(BasicROMId, 40);       // BASIC ROM - $A000-$BFFF - pages 40-47 - 8k
            bBasicROMMapped = true;
        }
        else
        {
			CodeAnalysis.MapBank(RAMBehindBasicROMId, 40);       // RAM Under BASIC ROM - $A000-$BFFF - pages 40-47 - 8k
        }

        /* E000..FFFF is either RAM-behind-KERNAL-ROM or RAM */
        if (cpuPort & C64_CPUPORT_HIRAM)
        {
            bKernelROMMapped = true;
			CodeAnalysis.MapBank(KernelROMId, 56);      // Kernel ROM - $E000-$FFFF - pages 56-63 - 8k
        }
        else
        {
			CodeAnalysis.MapBank(RAMBehindKernelROMId, 56);      // RAM Under Kernel ROM - $E000-$FFFF - pages 56-63 - 8k
        }

        /* D000..DFFF can be Char-ROM or I/O */
        if (cpuPort & C64_CPUPORT_CHAREN)
        {
            bIOMapped = true;
			CodeAnalysis.MapBank(IOAreaId, 52);         // IO System - %D000 - $DFFF - page 52-55 - 4k
		}
        else
        {
            bCharacterROMMapped = true;
			CodeAnalysis.MapBank(CharacterROMId, 52);       // Character ROM - %D000 - $DFFF - page 52-55 - 4k
        }
    }
}

bool FC64Emulator::LoadGame(const FGameInfo* pGameInfo)
{
    size_t fileSize;
    void* pGameData = LoadBinaryFile(pGameInfo->PRGFile.c_str(), fileSize);
    if (pGameData)
    {
        // FIXME: invalid function signature
        //c64_quickload(&C64Emu, (uint8_t*)pGameData, fileSize);
        chips_range_t data;
        data.ptr = pGameData;
        data.size = fileSize;
        c64_quickload(&C64Emu, data);
        free(pGameData);

        ResetCodeAnalysis();
        if (LoadCodeAnalysis(pGameInfo) == false)
        {
            SetupCodeAnalysisLabels();
        }
        GenerateGlobalInfo(CodeAnalysis);// Note this might not work because pages might not have been set up

        CurrentGame = pGameInfo;
        return true;
    }

    return false;
}

void FC64Emulator::ResetCodeAnalysis(void)
{
#if 0
    // Reset RAM pages
    for (int pageNo = 0; pageNo < 64; pageNo++)
        RAM[pageNo].Reset();

    // Reset IO
    for (int pageNo = 0; pageNo < 4; pageNo++)
        IOSystem[pageNo].Reset();

    // Reset Basic ROM
    for (int pageNo = 0; pageNo < 8; pageNo++)
        BasicROM[pageNo].Reset();

    // Reset Kernel ROM
    for (int pageNo = 0; pageNo < 8; pageNo++)
        KernelROM[pageNo].Reset();
#endif
    // Reset other analysers
    InterruptHandlers.clear();
    IOAnalysis.Reset();

}

bool FC64Emulator::SaveCodeAnalysis(const FGameInfo* pGameInfo)
{
    // TODO: Json save
    return false;
#if 0
    FMemoryBuffer saveBuffer;
    saveBuffer.Init();

    // Save RAM pages
    for (int pageNo = 0; pageNo < 64; pageNo++)
        RAM[pageNo].WriteToBuffer(saveBuffer);

    // Save IO
    for (int pageNo = 0; pageNo < 4; pageNo++)
        IOSystem[pageNo].WriteToBuffer(saveBuffer);

    // Save Basic ROM
    for (int pageNo = 0; pageNo < 8; pageNo++)
        BasicROM[pageNo].WriteToBuffer(saveBuffer);

    // Save Kernel ROM
    for (int pageNo = 0; pageNo < 8; pageNo++)
        KernelROM[pageNo].WriteToBuffer(saveBuffer);

    // Write to file
    char fileName[128];
    snprintf(fileName, sizeof(fileName), "AnalysisData/%s.bin", pGameInfo->Name.c_str());
    return saveBuffer.SaveToFile(fileName);
#endif
}

bool FC64Emulator::LoadCodeAnalysis(const FGameInfo* pGameInfo)
{
    // TODO: Json load
    return false;
#if 0
    char fileName[128];
    snprintf(fileName, sizeof(fileName), "AnalysisData/%s.bin", pGameInfo->Name.c_str());
    FMemoryBuffer loadBuffer;

    if (loadBuffer.LoadFromFile(fileName) == false)
        return false;

    // Load RAM pages
    for (int pageNo = 0; pageNo < 64; pageNo++)
        RAM[pageNo].ReadFromBuffer(loadBuffer);

    // Load IO
    for (int pageNo = 0; pageNo < 4; pageNo++)
        IOSystem[pageNo].ReadFromBuffer(loadBuffer);

    // Load Basic ROM
    for (int pageNo = 0; pageNo < 8; pageNo++)
        BasicROM[pageNo].ReadFromBuffer(loadBuffer);

    // Load Kernel ROM
    for (int pageNo = 0; pageNo < 8; pageNo++)
        KernelROM[pageNo].ReadFromBuffer(loadBuffer);

    // FIXME: Invalid method signature
    //CodeAnalysis.SetCodeAnalysisDirty();

    return true;
#endif
}


void FC64Emulator::Shutdown()
{
	pGlobalConfig->Save(kGlobalConfigFilename);

    if (CurrentGame != nullptr)
        SaveCodeAnalysis(CurrentGame);

    ui_c64_discard(&C64UI);
    c64_discard(&C64Emu);
}

bool FC64Emulator::DrawDockingView()
{
    //SCOPE_PROFILE_CPU("UI", "DrawUI", ProfCols::UI);

    static bool opt_fullscreen_persistant = true;
    bool opt_fullscreen = opt_fullscreen_persistant;
    //static ImGuiDockNodeFlags dockspace_flags = ImGuiDockNodeFlags_PassthruCentralNode;
    bool bOpen = false;
    ImGuiDockNodeFlags dockFlags = 0;

    // We are using the ImGuiWindowFlags_NoDocking flag to make the parent window not dockable into,
    // because it would be confusing to have two docking targets within each others.
    ImGuiWindowFlags window_flags = ImGuiWindowFlags_MenuBar | ImGuiWindowFlags_NoDocking;
    if (opt_fullscreen)
    {
        ImGuiViewport* viewport = ImGui::GetMainViewport();
        ImGui::SetNextWindowPos(viewport->Pos);
        ImGui::SetNextWindowSize(viewport->Size);
        ImGui::SetNextWindowViewport(viewport->ID);
        ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 0.0f);
        ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 0.0f);
        window_flags |= ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove;
        window_flags |= ImGuiWindowFlags_NoBringToFrontOnFocus | ImGuiWindowFlags_NoNavFocus;
    }

    // When using ImGuiDockNodeFlags_PassthruCentralNode, DockSpace() will render our background and handle the pass-thru hole, so we ask Begin() to not render a background.
    if (dockFlags & ImGuiDockNodeFlags_PassthruCentralNode)
        window_flags |= ImGuiWindowFlags_NoBackground;

    // Important: note that we proceed even if Begin() returns false (aka window is collapsed).
    // This is because we want to keep our DockSpace() active. If a DockSpace() is inactive, 
    // all active windows docked into it will lose their parent and become undocked.
    // We cannot preserve the docking relationship between an active window and an inactive docking, otherwise 
    // any change of dockspace/settings would lead to windows being stuck in limbo and never being visible.
    bool bQuit = false;
    ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0.0f, 0.0f));
    if (ImGui::Begin("DockSpace Demo", &bOpen, window_flags))
    {
        ImGui::PopStyleVar();

        if (opt_fullscreen)
            ImGui::PopStyleVar(2);

        // DockSpace
        ImGuiIO& io = ImGui::GetIO();
        if (io.ConfigFlags & ImGuiConfigFlags_DockingEnable)
        {
            const ImGuiID dockspaceId = ImGui::GetID("MyDockSpace");
            ImGui::DockSpace(dockspaceId, ImVec2(0.0f, 0.0f), dockFlags);
        }

        //bQuit = MainMenu();
        //DrawDebugWindows(uiState);
        DrawUI();
        ImGui::End();
    }
    else
    {
        ImGui::PopStyleVar();
        bQuit = true;
    }

    return bQuit;
}

void FC64Emulator::DrawUI()
{
    FCodeAnalysisViewState& viewState = CodeAnalysis.GetFocussedViewState();
    ui_c64_draw(&C64UI);

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

        // Temp
        ImGui::Text("Interrupt Handlers");
        for (auto& intHandler : InterruptHandlers)
        {
            ImGui::Text("$%04X:", intHandler);
            DrawAddressLabel(CodeAnalysis, viewState, intHandler);
        }
    }
    ImGui::End();


	if (ImGui::Begin("Debugger"))
	{
		CodeAnalysis.Debugger.DrawUI();
	}
	ImGui::End();

	if (ImGui::Begin("Memory Analyser"))
	{
		CodeAnalysis.MemoryAnalyser.DrawUI();
	}
	ImGui::End();

	if (ImGui::Begin("IO Analyser"))
	{
		CodeAnalysis.IOAnalyser.DrawUI();
	}
	ImGui::End();

	// Code analysis views
	for (int codeAnalysisNo = 0; codeAnalysisNo < FCodeAnalysisState::kNoViewStates; codeAnalysisNo++)
	{
		char name[32];
		sprintf(name, "Code Analysis %d", codeAnalysisNo + 1);
		if (CodeAnalysis.ViewState[codeAnalysisNo].Enabled)
		{
			if (ImGui::Begin(name, &CodeAnalysis.ViewState[codeAnalysisNo].Enabled))
			{
				DrawCodeAnalysisData(CodeAnalysis, codeAnalysisNo);
			}
			ImGui::End();
		}

	}

    if (ImGui::Begin("IO Analysis"))
    {
        IOAnalysis.DrawIOAnalysisUI();
    }
    ImGui::End();

    if (ImGui::Begin("Graphics Viewer"))
    {
        GraphicsViewer.DrawUI();
    }
    ImGui::End();

    if (ImGui::Begin("Games List"))
    {
        GamesList.DrawGameSelect();
        if (GamesList.GetSelectedGame() != -1 && ImGui::Button("Load"))
        {
            LoadGame(&GamesList.GetGameInfo(GamesList.GetSelectedGame()));

        }
    }
    ImGui::End();
}

void FC64Emulator::Tick()
{
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
    if ((addr == 0xfffe || addr == 0xffff) && irq)
    {
        hitIrq = true;
        const uint16_t interruptHandler = ReadWord(0xfffe);
        InterruptHandlers.insert(interruptHandler);
    }

    bool bReadingInstruction = addr == m6502_pc(&C64Emu.cpu) - 1;

    if ((pins & M6502_SYNC) == 0) // not for instruction fetch
    {
        if (pins & M6502_RW)
        {
            if (state.bRegisterDataAccesses)
                RegisterDataRead(CodeAnalysis, pc, addr);

            if (bIOMapped && (addr >> 12) == 0xd)
            {
                IOAnalysis.RegisterIORead(addr, pc);
            }
        }
        else
        {
            if (state.bRegisterDataAccesses)
            {
                // FIXME: Invalid parameter
                //RegisterDataWrite(CodeAnalysis, pc, addr);
            }

            // FIXME: parameter conversion for SetLastWriterForAddress
            FAddressRef pcRef = state.AddressRefFromPhysicalAddress(pc);
            FAddressRef addrRef = state.AddressRefFromPhysicalAddress(addr);
            state.SetLastWriterForAddress(addr, pcRef);

            if (bIOMapped && (addr >> 12) == 0xd)
            {
                IOAnalysis.RegisterIOWrite(addr, val, pc);
				IOMemBuffer[addr & 0xfff] = val;
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


    const bool bNeedMemUpdate = ((C64Emu.cpu_port ^ LastMemPort) & 7) != 0;
    if (bNeedMemUpdate)
    {
        UpdateCodeAnalysisPages(C64Emu.cpu_port);

        LastMemPort = C64Emu.cpu_port & 7;
    }

	CodeAnalysis.OnCPUTick(pins);

    return pins;
}
