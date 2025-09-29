#include "MC6845Device.h"

#include <ui/ui_chip.h>
#include <Misc/EmuBase.h>

#include <imgui.h>

typedef struct {
	const char* title;          /* window title */
	mc6845_t* mc6845;           /* pointer to mc6845_t instance to track */
	int x, y;                   /* initial window pos */
	int w, h;                   /* initial window size, or 0 for default size */
	bool open;                  /* initial open state */
	ui_chip_desc_t chip_desc;   /* chip visualization desc */
} ui_mc6845_desc_t;

void ui_mc6845_init(ui_mc6845_t* win, const ui_mc6845_desc_t* desc);
void ui_mc6845_discard(ui_mc6845_t* win);
void ui_mc6845_draw(ui_mc6845_t* win);

static const ui_chip_pin_t _ui_mc6845_pins[] = {
	{ "D0",     0,      MC6845_D0 },
	{ "D1",     1,      MC6845_D1 },
	{ "D2",     2,      MC6845_D2 },
	{ "D3",     3,      MC6845_D3 },
	{ "D4",     4,      MC6845_D4 },
	{ "D5",     5,      MC6845_D5 },
	{ "D6",     6,      MC6845_D6 },
	{ "D7",     7,      MC6845_D7 },
	{ "CS",     9,      MC6845_CS },
	{ "RS",    10,      MC6845_RS },
	{ "RW",    11,      MC6845_RW },
	{ "DE",    13,      MC6845_DE },
	{ "VS",    14,      MC6845_VS },
	{ "HS",    15,      MC6845_HS },
	{ "MA0",   20,      MC6845_MA0 },
	{ "MA1",   21,      MC6845_MA1 },
	{ "MA2",   22,      MC6845_MA2 },
	{ "MA3",   23,      MC6845_MA3 },
	{ "MA4",   24,      MC6845_MA4 },
	{ "MA5",   25,      MC6845_MA5 },
	{ "MA6",   26,      MC6845_MA6 },
	{ "MA7",   27,      MC6845_MA7 },
	{ "MA8",   28,      MC6845_MA8 },
	{ "MA9",   29,      MC6845_MA9 },
	{ "MA10",  30,      MC6845_MA10 },
	{ "MA11",  31,      MC6845_MA11 },
	{ "MA12",  32,      MC6845_MA12 },
	{ "MA13",  33,      MC6845_MA13 },
	{ "RA0",   35,      MC6845_RA0 },
	{ "RA1",   36,      MC6845_RA1 },
	{ "RA2",   37,      MC6845_RA2 },
	{ "RA3",   38,      MC6845_RA3 },
	{ "RA4",   39,      MC6845_RA4 },
};


FMC6845Device::FMC6845Device()
{
	Name = "MC6845";
}

bool FMC6845Device::Init(const char* pName, FEmuBase* pEmulator, mc6845_t* pCRTC)
{
	Name = pName;
	pCRTCState = pCRTC;

	SetAnalyser(&pEmulator->GetCodeAnalysis());
	pEmulator->GetCodeAnalysis().IOAnalyser.AddDevice(this);

	ui_mc6845_desc_t desc = { 0 };
	desc.title = Name.c_str();
	desc.mc6845 = pCRTCState;
	desc.x = 0;
	desc.y = 0;
	UI_CHIP_INIT_DESC(&desc.chip_desc, "6845", 40, _ui_mc6845_pins);
	ui_mc6845_init(&UIState, &desc);


	return true;
}

void FMC6845Device::OnFrameTick()
{
}

void FMC6845Device::OnMachineFrameEnd()
{
}

void FMC6845Device::DrawCRTCStateUI(void)
{
	ui_mc6845_draw(&UIState);
}

void FMC6845Device::DrawDetailsUI()
{
	if (pCRTCState == nullptr)
		return;
	DrawCRTCStateUI();
}


// modified Chips code
#define CHIPS_ASSERT(c) assert(c)

void ui_mc6845_init(ui_mc6845_t* win, const ui_mc6845_desc_t* desc) {
	CHIPS_ASSERT(win && desc);
	CHIPS_ASSERT(desc->title);
	CHIPS_ASSERT(desc->mc6845);
	memset(win, 0, sizeof(ui_mc6845_t));
	win->title = desc->title;
	win->mc6845 = desc->mc6845;
	win->init_x = (float)desc->x;
	win->init_y = (float)desc->y;
	win->init_w = (float)((desc->w == 0) ? 460 : desc->w);
	win->init_h = (float)((desc->h == 0) ? 370 : desc->h);
	win->open = desc->open;
	win->valid = true;
	ui_chip_init(&win->chip, &desc->chip_desc);
}

void ui_mc6845_discard(ui_mc6845_t* win) {
	CHIPS_ASSERT(win && win->valid);
	win->valid = false;
}

static void _ui_mc6845_draw_state(ui_mc6845_t* win) {
	CHIPS_ASSERT(win && win->mc6845);
	mc6845_t* mc = win->mc6845;

	switch (mc->type) {
	case MC6845_TYPE_UM6845:    ImGui::Text("Type: UM6845"); break;
	case MC6845_TYPE_UM6845R:   ImGui::Text("Type: UM6845R"); break;
	case MC6845_TYPE_MC6845:    ImGui::Text("Type: MC6845"); break;
	default:                    ImGui::Text("Type: ???"); break;
	}
	ImGui::Separator();

	ImGui::Columns(2, "##regs", false);
	ImGui::SetColumnWidth(0, 124);
	ImGui::SetColumnWidth(1, 124);
	mc->h_total = ui_util_input_u8("R0 HTotal", mc->h_total); ImGui::NextColumn();
	mc->h_displayed = ui_util_input_u8("R1 HDisp", mc->h_displayed); ImGui::NextColumn();
	mc->h_sync_pos = ui_util_input_u8("R2 HSyncPos", mc->h_sync_pos); ImGui::NextColumn();
	mc->sync_widths = ui_util_input_u8("R3 SyncWidth", mc->sync_widths); ImGui::NextColumn();
	mc->v_total = ui_util_input_u8("R4 VTotal", mc->v_total); ImGui::NextColumn();
	mc->v_total_adjust = ui_util_input_u8("R5 VTotalAdj", mc->v_total_adjust); ImGui::NextColumn();
	mc->v_displayed = ui_util_input_u8("R6 VDisp", mc->v_displayed); ImGui::NextColumn();
	mc->v_sync_pos = ui_util_input_u8("R7 VSyncPos", mc->v_sync_pos); ImGui::NextColumn();
	mc->interlace_mode = ui_util_input_u8("R8 Interl", mc->interlace_mode); ImGui::NextColumn();
	mc->max_scanline_addr = ui_util_input_u8("R9 MaxScanl", mc->max_scanline_addr); ImGui::NextColumn();
	mc->cursor_start = ui_util_input_u8("R10 CursStart", mc->cursor_start); ImGui::NextColumn();
	mc->cursor_end = ui_util_input_u8("R11 CursEnd", mc->cursor_end); ImGui::NextColumn();
	mc->start_addr_hi = ui_util_input_u8("R12 AddrHi", mc->start_addr_hi); ImGui::NextColumn();
	mc->start_addr_lo = ui_util_input_u8("R13 AddrLo", mc->start_addr_lo); ImGui::NextColumn();
	mc->cursor_hi = ui_util_input_u8("R14 CursHi", mc->cursor_hi); ImGui::NextColumn();
	mc->cursor_lo = ui_util_input_u8("R15 CursLo", mc->cursor_lo); ImGui::NextColumn();
	mc->lightpen_hi = ui_util_input_u8("R16 LPenHi", mc->lightpen_hi); ImGui::NextColumn();
	mc->lightpen_lo = ui_util_input_u8("R17 LPenLo", mc->lightpen_lo); ImGui::NextColumn();
	ImGui::Columns();
	ImGui::Separator();
	ImGui::Text("Memory Addr: %04X  Row Start: %04X", mc->ma, mc->ma_row_start);
	ImGui::Text("Row Ctr:     %02X    Scanline Ctr: %02X", mc->v_ctr, mc->r_ctr);
	ImGui::Text("Hori Ctr:    %02X", mc->h_ctr);
	ImGui::Text("HSync Ctr:   %02X    VSync Ctr: %02X", mc->hsync_ctr, mc->vsync_ctr);
	ImGui::Text("Reg Sel:     R%d", mc->sel);
}

void ui_mc6845_draw(ui_mc6845_t* win) 
{
	CHIPS_ASSERT(win && win->valid && win->title && win->mc6845);
	
	ImGui::BeginChild("##chip", ImVec2(176, 0), true);
	ui_chip_draw(&win->chip, win->mc6845->pins);
	ImGui::EndChild();
	ImGui::SameLine();
	ImGui::BeginChild("##state", ImVec2(0, 0), true);
	_ui_mc6845_draw_state(win);
	ImGui::EndChild();
}