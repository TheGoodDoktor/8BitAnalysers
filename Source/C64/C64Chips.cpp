/*
    UI implementation for c64.c, this must live in a .cc file.
*/
//#include "common.h"
//#define CHIPS_IMPL

#define NOMINMAX

#include "imgui.h"
#include "chips/chips_common.h"
#include "chips/z80.h"
#include "chips/m6502.h"
#include "chips/m6526.h"
#include "chips/m6569.h"
#include "chips/m6581.h"
#include "chips/beeper.h"
#include "chips/kbd.h"
#include "chips/mem.h"
#include "chips/clk.h"
#include <chips/m6502.h>
#include <chips/m6522.h>
#include <chips/m6569.h>
#include <systems/c1530.h>
#include <systems/c1541.h>
#include <systems/c64.h>

#define UI_DASM_USE_Z80
#define UI_DASM_USE_M6502
//#include "ui.h"
#include "util/m6502dasm.h"
#include "util/z80dasm.h"

#define CHIPS_UI_IMPL
#define UI_DBG_USE_M6502

#include "ui/ui_util.h"
#include "ui/ui_chip.h"
#include "ui/ui_memedit.h"
#include "ui/ui_memmap.h"
#include "ui/ui_dasm.h"
#include "ui/ui_dbg.h"
#include "ui/ui_snapshot.h"
#include "ui/ui_m6502.h"
#include "ui/ui_m6526.h"
#include "ui/ui_m6581.h"
#include "ui/ui_m6569.h"
#include "ui/ui_audio.h"
#include "ui/ui_kbd.h"
#include "ui/ui_c64.h"

#include "c64-roms.h"

#ifdef __clang__
#pragma clang diagnostic ignored "-Wmissing-field-initializers"
#endif
#include <ImGuiSupport/ImGuiTexture.h>
#include <backends/imgui_impl_win32.h>

#define SOKOL_IMPL
#include "sokol_audio.h"

#include "CodeAnalyser/CodeAnalyser.h"
#include "CodeAnalyser/UI/CodeAnalyserUI.h"
#include "Util/MemoryBuffer.h"
#include "Util/FileUtil.h"
#include "IOAnalysis/C64IOAnalysis.h"
#include "GraphicsViewer/C64GraphicsViewer.h"
#include "C64Display.h"
#include "C64GamesList.h"
#include <Util/Misc.h>
#include <algorithm>

class FC64Emulator : public ICPUInterface
{
public:

    bool    Init();
    void    Shutdown();
    void    Tick();

    // Begin IInputEventHandler interface implementation
    void	OnKeyUp(int keyCode);
    void	OnKeyDown(int keyCode);
    void	OnChar(int charCode);
    void    OnGamepadUpdated(int mask);
    // End IInputEventHandler interface implementation

    // Begin ICPUInterface interface implementation
    uint8_t		ReadByte(uint16_t address) const override
    {
        return mem_rd(const_cast<mem_t*>(&C64Emu.mem_cpu), address);
    }
    uint16_t	ReadWord(uint16_t address) const override
    {
        return mem_rd16(const_cast<mem_t*>(&C64Emu.mem_cpu), address);
    }
    const uint8_t* GetMemPtr(uint16_t address) const override
    {
        return mem_readptr(const_cast<mem_t*>(&C64Emu.mem_cpu), address);
    }

    void WriteByte(uint16_t address, uint8_t value) override
    {
        mem_wr(&C64Emu.mem_cpu, address, value);
    }

    FAddressRef GetPC() override
    {
        uint16_t address = m6502_pc(&C64Emu.cpu);
        FAddressRef result(0, address);

        return result;
    }

    uint16_t	GetSP(void) override
    {
        return m6502_s(&C64Emu.cpu) + 0x100;    // stack begins at 0x100
    }
        
    // End ICPUInterface interface implementation

    c64_desc_t GenerateC64Desc(c64_joystick_type_t joy_type);
    void SetupCodeAnalysisLabels(void);
    void UpdateCodeAnalysisPages(uint8_t cpuPort);
    bool LoadGame(const FGameInfo* pGameInfo);
    void ResetCodeAnalysis(void);
    bool SaveCodeAnalysis(const FGameInfo* pGameInfo);
    bool LoadCodeAnalysis(const FGameInfo* pGameInfo);

    // Emulator Event Handlers
    void    OnBoot(void);
    int     OnCPUTrap(uint16_t pc, int ticks, uint64_t pins);
    uint64_t    OnCPUTick(uint64_t pins);


private:
    c64_t       C64Emu;
    ui_c64_t    C64UI;
    double      ExecTime;

    FC64GamesList       GamesList;
    const FGameInfo*    CurrentGame = nullptr;

    FC64Display         Display;
 
    FCodeAnalysisState  CodeAnalysis;

    // Analysis pages
    FCodeAnalysisPage   KernelROM[8];       // 8K Kernel ROM
    FCodeAnalysisPage   BasicROM[8];        // 8K Basic ROM
    FCodeAnalysisPage   CharacterROM[4];    // 4K Character ROM
    FCodeAnalysisPage   IOSystem[4];        // 4K IO System
    FCodeAnalysisPage   RAM[64];            // 64K RAM

    uint8_t             LastMemPort = 0x7;  // Default startup
    uint16_t            LastPC = 0;

    FC64IOAnalysis      IOAnalysis;
    FC64GraphicsViewer  GraphicsViewer;
    std::set<uint16_t>  InterruptHandlers;

    // Mapping status
    bool                bBasicROMMapped = true;
    bool                bKernelROMMapped = true;
    bool                bCharacterROMMapped = false;
    bool                bIOMapped = true;

    // Bank Ids
    uint16_t            LowerRAMId = -1;
	uint16_t            HighRAMId = -1;
	uint16_t            IOAreaId = -1;
	uint16_t            BasicROMId = -1;
	uint16_t            RAMBehindBasicROMId = -1;
	uint16_t            KernelROMId = -1;
	uint16_t            RAMBehindKernelROMId = -1;
	uint16_t            CharacterROMId = -1;
	uint16_t            RAMBehindCharROMId = -1;
	uint16_t            ColourRAMId = -1;
};

FC64Emulator g_C64Emu;

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


/* reboot callback */
static void C64BootCallback(c64_t* sys) 
{
    // FIXME: no such struct member
    //FC64Emulator* pC64Emu = (FC64Emulator*)sys->user_data;
    //pC64Emu->OnBoot();
}

#if 0
static int CPUTrapCallback(uint16_t pc, int ticks, uint64_t pins, void* user_data)
{
    FC64Emulator* pC64Emu = (FC64Emulator*)user_data;
    return pC64Emu->OnCPUTrap(pc, ticks, pins);
}

uint64_t CPUTickCallback(uint64_t pins, void* user_data)
{
    c64_t* sys = (c64_t*)user_data;

    // FIXME: no such struct field
    return 0;
    //FC64Emulator* pC64Emu = (FC64Emulator*)sys->user_data;
    //return pC64Emu->OnCPUTick(pins);
}
#endif

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

    // FIXME: No such fields in c64_desc_t
    //desc.pixel_buffer = Display.GetPixelBuffer();
    //desc.pixel_buffer_size = Display.GetPixelBufferSize();

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
    saudio_desc audiodesc;
    memset(&audiodesc, 0, sizeof(saudio_desc));
    saudio_setup(&audiodesc);

    Display.Init(&CodeAnalysis, &C64Emu);

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
    
    ui_c64_init(&C64UI, &uiDesc);

    CPUType = ECPUType::M6502;
    SetNumberDisplayMode(ENumberDisplayMode::HexDollar);

    // setup default memory configuration
    // RAM - $0000 - $9FFF - pages 0-39 - 40K
    
    // RAM - $C000 - $CFFF - pages 48-51 - 4k
    // IO System - %D000 - $DFFF - page 52-55 - 4k
    
	LowerRAMId = CodeAnalysis.CreateBank("LoRAM", 40, C64Emu.ram, true);
    HighRAMId = CodeAnalysis.CreateBank("HiRAM", 4, &C64Emu.ram[0xc000], true);
	IOAreaId = CodeAnalysis.CreateBank("IOArea", 4, nullptr, true);

	BasicROMId = CodeAnalysis.CreateBank("BasicROM", 8, C64Emu.rom_basic, true); // BASIC ROM - $A000-$BFFF - pages 40-47 - 8k
    RAMBehindBasicROMId = CodeAnalysis.CreateBank("RAMBehindBasicROM", 8, &C64Emu.ram[0xa000], true);

    KernelROMId = CodeAnalysis.CreateBank("KernelROM", 8, C64Emu.rom_kernal, true);   // Kernel ROM - $E000-$FFFF - pages 56-63 - 8k
    RAMBehindKernelROMId = CodeAnalysis.CreateBank("RAMBehindKernelROM", 8, &C64Emu.ram[0xe000], true);

    CharacterROMId = CodeAnalysis.CreateBank("CharacterROM", 4, C64Emu.rom_char, true);
    RAMBehindCharROMId = CodeAnalysis.CreateBank("RAMBehindCharROM", 4, &C64Emu.ram[0xd000], true);

    ColourRAMId = CodeAnalysis.CreateBank("ColourRAM", 1, C64Emu.color_ram, true);
	
    // initial config
	CodeAnalysis.MapBank(LowerRAMId, 0);        // RAM - $0000 - $9FFF - pages 0-39 - 40K
    CodeAnalysis.MapBank(BasicROMId, 40);       // BASIC ROM - $A000-$BFFF - pages 40-47 - 8k
	CodeAnalysis.MapBank(HighRAMId, 48);        // RAM - $C000 - $CFFF - pages 48-51 - 4k
	CodeAnalysis.MapBank(IOAreaId, 52);         // IO System - %D000 - $DFFF - page 52-55 - 4k
	CodeAnalysis.MapBank(KernelROMId, 56);      // Kernel ROM - $E000-$FFFF - pages 56-63 - 8k

    for (int pageNo = 0; pageNo < 64; pageNo++)
    {
        char pageName[16];
        // Initialise RAM Pages

        // FIXME: no parameterized constructor
        RAM[pageNo].Initialise();
        //RAM[pageNo].Initialise(pageNo * FCodeAnalysisPage::kPageSize);

        snprintf(pageName, sizeof(pageName), "RAM[%02X]", pageNo);

        // FIXME:: RegisterPage is a private member
        //CodeAnalysis.RegisterPage(&RAM[pageNo], pageName);
        
        // Initialise Basic ROM
        if (pageNo >= 40 && pageNo < 48)
        {
            const int pageIndex = pageNo - 40;
            // FIXME: no parameterized constructor
            BasicROM[pageIndex].Initialise();
            //BasicROM[pageIndex].Initialise(pageNo * FCodeAnalysisPage::kPageSize);

            snprintf(pageName, sizeof(pageName), "BasicROM[%02X]", pageIndex);

            // FIXME: FCodeAnalysisState::RegisterPage is a private member
            //CodeAnalysis.RegisterPage(&BasicROM[pageIndex], pageName);
        }

        // Initialise IO System & character RAM
        if (pageNo >= 52 && pageNo < 56)
        {
            const int pageIndex = pageNo - 52;
            snprintf(pageName, sizeof(pageName), "IO[%02X]", pageIndex);
            // FIXME: no parametrized constructor in FCodeAnalysisPage
            IOSystem[pageIndex].Initialise();
            //IOSystem[pageIndex].Initialise(pageNo* FCodeAnalysisPage::kPageSize);

            // FIXME: FCodeAnalysisState::RegisterPage is a private member
            //CodeAnalysis.RegisterPage(&IOSystem[pageIndex], pageName);

            snprintf(pageName, sizeof(pageName), "CharROM[%02X]", pageIndex);
            // FIXME: no parametrized constructor in FCodeAnalysisPage
            CharacterROM[pageIndex].Initialise();
            //CharacterROM[pageIndex].Initialise(pageNo* FCodeAnalysisPage::kPageSize);
            // FIXME: FCodeAnalysisState::RegisterPage is a private member
            //CodeAnalysis.RegisterPage(&CharacterROM[pageIndex], pageName);
        }

        // Initialise Kernel ROM
        if (pageNo >= 56)
        {
            const int pageIndex = pageNo - 56;
            snprintf(pageName, sizeof(pageName), "Kernel[%02X]", pageIndex);

            // FIXME: FCodeAnalysisState constructor has no parameters
            KernelROM[pageIndex].Initialise();
            //KernelROM[pageIndex].Initialise(pageNo * FCodeAnalysisPage::kPageSize);

            // FIXME: FCodeAnalysisState::RegisterPage is a private member
            //CodeAnalysis.RegisterPage(&KernelROM[pageIndex], pageName);
        }
    }
    
    // Map permanent regions
    for (int pageNo = 0; pageNo < 64; pageNo++)
    {
        // Map bottom 40K to RAM as it's always RAM
        // FIXME: FCodeAnalysisState::SetCodeAnalysisRWPage and FCodeAnalysisState::SetCodeAnalysisWritePage are private members
        /*
        if (pageNo < 40)
            CodeAnalysis.SetCodeAnalysisRWPage(pageNo, &RAM[pageNo], &RAM[pageNo]);
        else if(pageNo > 47 && pageNo < 52)    // 0xc000 4k region
            CodeAnalysis.SetCodeAnalysisRWPage(pageNo, &RAM[pageNo], &RAM[pageNo]);
        else
        {
            if (pageNo < 52 || pageNo > 55)
                CodeAnalysis.SetCodeAnalysisWritePage(pageNo, &RAM[pageNo]);
        }
        */
    }
    
    SetupCodeAnalysisLabels();
    UpdateCodeAnalysisPages(0x7);
    IOAnalysis.Init(&CodeAnalysis);
    GraphicsViewer.Init(&CodeAnalysis,&C64Emu);

    // FIXME: missing InitialiseCodeAnalysis declaration - uncommitted changes?
    //InitialiseCodeAnalysis(CodeAnalysis, this);

    GamesList.EnumerateGames();

    return true;
}

void FC64Emulator::SetupCodeAnalysisLabels()
{
    // Add IO Labels to code analysis
    AddVICRegisterLabels(IOSystem[0]);  // Page $D000-$D3ff
    AddSIDRegisterLabels(IOSystem[1]);  // Page $D400-$D7ff
    IOSystem[2].SetLabelAtAddress("ColourRAM", ELabelType::Data, 0x0000);    // Colour RAM $D800
    AddCIARegisterLabels(IOSystem[3]);  // Page $DC00-$Dfff
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
        //mem_map_ram(&sys->mem_cpu, 0, 0xA000, 0x6000, sys->ram + 0xA000);
        for (int pageNo = 41; pageNo < 64; pageNo++)
        {
            // FIXME: 'SetCodeAnalysisRWPage' is a private member of 'FCodeAnalysisState'
            //CodeAnalysis.SetCodeAnalysisRWPage(pageNo, &RAM[pageNo], &RAM[pageNo]);
        }
    }
    else
    {
        /* A000..BFFF is either RAM-behind-BASIC-ROM or RAM */
        if ((cpuPort & (C64_CPUPORT_HIRAM | C64_CPUPORT_LORAM)) == (C64_CPUPORT_HIRAM | C64_CPUPORT_LORAM))
        {
            bBasicROMMapped = true;

            // Map Basic ROM Code Analysis pages to  
            for (int pageNo = 40; pageNo < 48; pageNo++)
            {
                // FIXME: 'SetCodeAnalysisRWPage' is a private member of 'FCodeAnalysisState'
                //CodeAnalysis.SetCodeAnalysisRWPage(pageNo, &BasicROM[pageNo - 40], &RAM[pageNo]);
            }
        }
        else
        {
            for (int pageNo = 40; pageNo < 48; pageNo++)
            {
                // FIXME: 'SetCodeAnalysisRWPage' is a private member of 'FCodeAnalysisState'
                //CodeAnalysis.SetCodeAnalysisRWPage(pageNo, &RAM[pageNo], &RAM[pageNo]);
            }
        }
        //mem_map_rw(&sys->mem_cpu, 0, 0xA000, 0x2000, read_ptr, sys->ram + 0xA000);

        /* E000..FFFF is either RAM-behind-KERNAL-ROM or RAM */
        if (cpuPort & C64_CPUPORT_HIRAM)
        {
            bKernelROMMapped = true;
            //read_ptr = sys->rom_kernal;
            for (int pageNo = 56; pageNo < 64; pageNo++)
            {
                // FIXME: 'SetCodeAnalysisRWPage' is a private member of 'FCodeAnalysisState'
                //CodeAnalysis.SetCodeAnalysisRWPage(pageNo, &KernelROM[pageNo - 56], &RAM[pageNo]);
            }
        }
        else
        {
            //read_ptr = sys->ram + 0xE000;
            for (int pageNo = 56; pageNo < 64; pageNo++)
            {
                // FIXME: 'SetCodeAnalysisRWPage' is a private member of 'FCodeAnalysisState'
                //CodeAnalysis.SetCodeAnalysisRWPage(pageNo, &RAM[pageNo], &RAM[pageNo]);
            }
        }
        //mem_map_rw(&sys->mem_cpu, 0, 0xE000, 0x2000, read_ptr, sys->ram + 0xE000);

        /* D000..DFFF can be Char-ROM or I/O */
        if (cpuPort & C64_CPUPORT_CHAREN)
        {
            bIOMapped = true;
            //sys->io_mapped = true;
            for (int pageNo = 52; pageNo < 56; pageNo++)
            {
                // FIXME: 'SetCodeAnalysisRWPage' is a private member of 'FCodeAnalysisState'
                //CodeAnalysis.SetCodeAnalysisRWPage(pageNo, &IOSystem[pageNo - 52], &IOSystem[pageNo - 52]);
            }
        }
        else
        {
            bCharacterROMMapped = true;

            //mem_map_rw(&sys->mem_cpu, 0, 0xD000, 0x1000, sys->rom_char, sys->ram + 0xD000);
            for (int pageNo = 52; pageNo < 56; pageNo++)
            {
                // FIXME: 'SetCodeAnalysisRWPage' is a private member of 'FCodeAnalysisState'
                //CodeAnalysis.SetCodeAnalysisRWPage(pageNo, &CharacterROM[pageNo - 52], &RAM[pageNo]);
            }
        }
    }

    // FIXME: invalid signature for method SetCodeAnalysisDirty
    //CodeAnalysis.SetCodeAnalysisDirty();
}

bool FC64Emulator::LoadGame(const FGameInfo* pGameInfo)
{
    size_t fileSize;
    const std::string fileName = std::string("Games/") + pGameInfo->PRGFile;
    void* pGameData = LoadBinaryFile(fileName.c_str(), fileSize);
    if (pGameData)
    {
        // FIXME: invalid function signature
        //c64_quickload(&C64Emu, (uint8_t*)pGameData, fileSize);
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

bool FC64Emulator::LoadCodeAnalysis(const FGameInfo *pGameInfo)
{
    // TODO: Json load
    return false;
#if 0
    char fileName[128];
    snprintf(fileName, sizeof(fileName), "AnalysisData/%s.bin",pGameInfo->Name.c_str());
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
    if(CurrentGame != nullptr)
        SaveCodeAnalysis(CurrentGame);

    ui_c64_discard(&C64UI);
    c64_discard(&C64Emu);
}

void FC64Emulator::Tick()
{
    const float frameTime = (float)std::min(1000000.0f / ImGui::GetIO().Framerate, 32000.0f) * 1.0f;// speccyInstance.ExecSpeedScale;
    FCodeAnalysisViewState& viewState =  CodeAnalysis.GetFocussedViewState();

    c64_exec(&C64Emu, (uint32_t)std::max(static_cast<uint32_t>(frameTime), uint32_t(1)));

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

    if (ImGui::Begin("Code Analysis"))
    {
        DrawCodeAnalysisData(CodeAnalysis, 0);
    }
    ImGui::End();

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

// pc points to instruction after the one just executed so we use the previous pc
int    FC64Emulator::OnCPUTrap(uint16_t pc, int ticks, uint64_t pins)
{
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
    return 0;
}

uint64_t FC64Emulator::OnCPUTick(uint64_t pins)
{
    static uint16_t lastPC = m6502_pc(&C64Emu.cpu);
    const uint16_t pc = C64Emu.cpu.PC;// g_M6502PC;// m6502_pc(&C64Emu.cpu);// pc after execution?
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

            if (CodeAnalysis.bRegisterDataAccesses)
                RegisterDataRead(CodeAnalysis, pc, addr);

            if (bIOMapped && (addr >> 12) == 0xd)
            {
                IOAnalysis.RegisterIORead(addr, pc);
            }
        }
        else
        {
            if (CodeAnalysis.bRegisterDataAccesses)
            {
                // FIXME: Invalid parameter
                //RegisterDataWrite(CodeAnalysis, pc, addr);
            }

            // FIXME: parameter conversion for SetLastWriterForAddress
            FAddressRef pcRef(0, pc);
            CodeAnalysis.SetLastWriterForAddress(addr, pcRef);

            if (bIOMapped && (addr >> 12) == 0xd)
            {
                IOAnalysis.RegisterIOWrite(addr, val, pc);
            }

            FCodeInfo* pCodeWrittenTo = CodeAnalysis.GetCodeInfoForAddress(addr);
            if (pCodeWrittenTo != nullptr && pCodeWrittenTo->bSelfModifyingCode == false)
                pCodeWrittenTo->bSelfModifyingCode = true;
        }
    }

    lastPC = m6502_pc(&C64Emu.cpu);

    const bool bNeedMemUpdate = ((C64Emu.cpu_port ^ LastMemPort) & 7) != 0;
    if (bNeedMemUpdate)
    {
        UpdateCodeAnalysisPages(C64Emu.cpu_port);

        LastMemPort = C64Emu.cpu_port & 7;
    }

    // FIXME: no such method
    //return OldTickCB(pins, &C64Emu);
    return 0;
}


void C64ChipsInit()
{
    g_C64Emu.Init();
}

/* per frame stuff, tick the emulator, handle input, decode and draw emulator display */
void C64ChipsTick(void) 
{
    g_C64Emu.Tick();
}

/* application cleanup callback */
void C64ChipsShutdown(void) 
{
    g_C64Emu.Shutdown();
}
#if 0
/* keyboard input handling */
void app_input(const sapp_event* event) 
{
    if (ui_input(event)) {
        /* input was handled by UI */
        return;
    }

    const bool shift = event->modifiers & SAPP_MODIFIER_SHIFT;
    switch (event->type) {
        int c;
    case SAPP_EVENTTYPE_CHAR:
        c = (int)event->char_code;
        if ((c > 0x20) && (c < 0x7F)) {
            /* need to invert case (unshifted is upper caps, shifted is lower caps */
            if (isupper(c)) {
                c = tolower(c);
            }
            else if (islower(c)) {
                c = toupper(c);
            }
            c64_key_down(&c64, c);
            c64_key_up(&c64, c);
        }
        break;
    case SAPP_EVENTTYPE_KEY_DOWN:
    case SAPP_EVENTTYPE_KEY_UP:
        switch (event->key_code) {
        case SAPP_KEYCODE_SPACE:        c = 0x20; break;
        case SAPP_KEYCODE_LEFT:         c = 0x08; break;
        case SAPP_KEYCODE_RIGHT:        c = 0x09; break;
        case SAPP_KEYCODE_DOWN:         c = 0x0A; break;
        case SAPP_KEYCODE_UP:           c = 0x0B; break;
        case SAPP_KEYCODE_ENTER:        c = 0x0D; break;
        case SAPP_KEYCODE_BACKSPACE:    c = shift ? 0x0C : 0x01; break;
        case SAPP_KEYCODE_ESCAPE:       c = shift ? 0x13 : 0x03; break;
        case SAPP_KEYCODE_F1:           c = 0xF1; break;
        case SAPP_KEYCODE_F2:           c = 0xF2; break;
        case SAPP_KEYCODE_F3:           c = 0xF3; break;
        case SAPP_KEYCODE_F4:           c = 0xF4; break;
        case SAPP_KEYCODE_F5:           c = 0xF5; break;
        case SAPP_KEYCODE_F6:           c = 0xF6; break;
        case SAPP_KEYCODE_F7:           c = 0xF7; break;
        case SAPP_KEYCODE_F8:           c = 0xF8; break;
        default:                        c = 0; break;
        }
        if (c) {
            if (event->type == SAPP_EVENTTYPE_KEY_DOWN) {
                c64_key_down(&c64, c);
            }
            else {
                c64_key_up(&c64, c);
            }
        }
        break;
    case SAPP_EVENTTYPE_TOUCHES_BEGAN:
        sapp_show_keyboard(true);
        break;
    default:
        break;
    }
}
#endif


