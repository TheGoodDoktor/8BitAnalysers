#include "Disassembler.h"
#include "imgui.h"
#include "util/z80dasm.h"
#include "util/m6502dasm.h"
#include "ui/ui_util.h"

#ifndef DASM_ASSERT
#include <assert.h>
#define DASM_ASSERT(c) assert(c)
#endif

#define DASM_USE_Z80
//#define DASM_USE_M6502



// initialise an instance of the disassembler
void DasmInit(FDasmState* pDasmState, const FDasmDesc* pDasmDesc)
{
	DASM_ASSERT(pDasmState && pDasmDesc);

	pDasmState->Title = pDasmDesc->Title;
	pDasmState->CPUType = pDasmDesc->CPUType;
	pDasmState->ReadCB = pDasmDesc->ReadCB;
	pDasmState->StartAddress = pDasmDesc->StartAddress;
	pDasmState->pUserData = pDasmDesc->pUserData;

	// window related - remove
	pDasmState->init_x = (float)pDasmDesc->x;
	pDasmState->init_y = (float)pDasmDesc->y;
	pDasmState->init_w = (float)((pDasmDesc->w == 0) ? 400 : pDasmDesc->w);
	pDasmState->init_h = (float)((pDasmDesc->h == 0) ? 256 : pDasmDesc->h);
	pDasmState->open = pDasmDesc->open;


	pDasmState->HighlightColour = 0xFF30FF30;

	// setup layers
	for (int i = 0; i < DASM_MAX_LAYERS; i++) 
	{
		if (pDasmDesc->LayerNames[i].empty() == false)
		{
			pDasmState->NumLayers++;
			pDasmState->LayerNames[i] = pDasmDesc->LayerNames[i];
		}
		else
			break;
	}
	
	pDasmState->valid = true;
}

void DasmDiscard(FDasmState* pDasmState)
{
	DASM_ASSERT(pDasmState && pDasmState->valid);
	pDasmState->valid = false;
}


/* disassembler callback to fetch the next instruction byte */
static uint8_t _ui_dasm_in_cb(void* pUserData)
{
	FDasmState* pDasm = (FDasmState*)pUserData;

	const uint8_t val = pDasm->ReadCB(pDasm->CurLayer, pDasm->CurrentAddress++, pDasm->pUserData);
	
	if (pDasm->bin_pos < DASM_MAX_BINLEN) 
		pDasm->bin_buf[pDasm->bin_pos++] = val;
	
	return val;
}

/* disassembler callback to output a character */
static void _ui_dasm_out_cb(char c, void* user_data)
{
	FDasmState* win = (FDasmState*)user_data;
	if ((win->str_pos + 1) < DASM_MAX_STRLEN) {
		win->str_buf[win->str_pos++] = c;
		win->str_buf[win->str_pos] = 0;
	}
}

/* disassemble the next instruction */
static void DasmDissassemble(FDasmState* pDasmState)
{
	pDasmState->str_pos = 0;
	pDasmState->bin_pos = 0;
#if defined(DASM_USE_Z80) && defined(DASM_USE_M6502)
	if (pDasmState->CPUType == DasmCPUType::Z80) {
		z80dasm_op(pDasmState->CurrentAddress, _ui_dasm_in_cb, _ui_dasm_out_cb, pDasmState);
	}
	else {
		m6502dasm_op(pDasmState->CurrentAddress, _ui_dasm_in_cb, _ui_dasm_out_cb, pDasmState);
	}
#elif defined(DASM_USE_Z80)
	z80dasm_op(pDasmState->CurrentAddress, _ui_dasm_in_cb, _ui_dasm_out_cb, pDasmState);
#else
	m6502dasm_op(win->cur_addr, _ui_dasm_in_cb, _ui_dasm_out_cb, win);
#endif
}

/* check if the current Z80 or m6502 instruction contains a jump target */
static bool _ui_dasm_jumptarget(FDasmState* pDasmState, uint16_t pc, uint16_t* out_addr)
{
	if (pDasmState->CPUType == DasmCPUType::Z80)
	{
		if (pDasmState->bin_pos == 3)
		{
			switch (pDasmState->bin_buf[0])
			{
				/* CALL nnnn */
			case 0xCD:
				/* CALL cc,nnnn */
			case 0xDC: case 0xFC: case 0xD4: case 0xC4:
			case 0xF4: case 0xEC: case 0xE4: case 0xCC:
				/* JP nnnn */
			case 0xC3:
				/* JP cc,nnnn */
			case 0xDA: case 0xFA: case 0xD2: case 0xC2:
			case 0xF2: case 0xEA: case 0xE2: case 0xCA:
				*out_addr = (pDasmState->bin_buf[2] << 8) | pDasmState->bin_buf[1];
				return true;
			}
		}
		else if (pDasmState->bin_pos == 2)
		{
			switch (pDasmState->bin_buf[0])
			{
				/* DJNZ d */
			case 0x10:
				/* JR d */
			case 0x18:
				/* JR cc,d */
			case 0x38: case 0x30: case 0x20: case 0x28:
				*out_addr = pc + (int8_t)pDasmState->bin_buf[1];
				return true;
			}
		}
		else if (pDasmState->bin_pos == 1) 
		{
			switch (pDasmState->bin_buf[0])
			{
				/* RST */
			case 0xC7:  *out_addr = 0x00; return true;
			case 0xCF:  *out_addr = 0x08; return true;
			case 0xD7:  *out_addr = 0x10; return true;
			case 0xDF:  *out_addr = 0x18; return true;
			case 0xE7:  *out_addr = 0x20; return true;
			case 0xEF:  *out_addr = 0x28; return true;
			case 0xF7:  *out_addr = 0x30; return true;
			case 0xFF:  *out_addr = 0x38; return true;
			}
		}
	}
	else 
	{
		/* M6502 CPU */
		if (pDasmState->bin_pos == 3) 
		{
			uint8_t l, h;
			uint16_t addr;
			switch (pDasmState->bin_buf[0])
			{
				/* JSR/JMP abs */
			case 0x20: case 0x4C:
				*out_addr = (pDasmState->bin_buf[2] << 8) | pDasmState->bin_buf[1];
				return true;
				/* JMP ind */
			case 0x6C:
				addr = (pDasmState->bin_buf[2] << 8) | pDasmState->bin_buf[1];
				l = pDasmState->ReadCB(pDasmState->CurLayer, addr++, pDasmState->pUserData);
				h = pDasmState->ReadCB(pDasmState->CurLayer, addr++, pDasmState->pUserData);
				*out_addr = (h << 8) | l;
				return true;
			}
		}
		else if (pDasmState->bin_pos == 2)
		{
			switch (pDasmState->bin_buf[0])
			{
				/* relative branch */
			case 0x10: case 0x30: case 0x50: case 0x70:
			case 0x90: case 0xB0: case 0xD0: case 0xF0:
				*out_addr = pc + (int8_t)pDasmState->bin_buf[1];
				return true;
			}
		}
	}
	return false;
}

/* push an address on the bookmark stack */
static void _ui_dasm_stack_push(FDasmState* pDasmState, uint16_t addr)
{
	if (pDasmState->StackNum < DASM_MAX_STACK)
	{
		/* ignore if the same address is already on top of stack */
		if ((pDasmState->StackNum > 0) && (addr == pDasmState->Stack[pDasmState->StackNum - 1])) 
		{
			return;
		}
		pDasmState->StackPos = pDasmState->StackNum;
		pDasmState->Stack[pDasmState->StackNum++] = addr;
	}
}

/* return current address on stack, and set pos to previous */
static bool _ui_dasm_stack_back(FDasmState* pDasmState, uint16_t* addr)
{
	if (pDasmState->StackNum > 0) 
	{
		*addr = pDasmState->Stack[pDasmState->StackPos];
		if (pDasmState->StackPos > 0)
			pDasmState->StackPos--;
		return true;
	}
	*addr = 0;
	return false;
}

/* goto to address, op address on stack */
static void _ui_dasm_goto(FDasmState* pDasmState, uint16_t addr)
{
	pDasmState->StartAddress = addr;
}

/* draw the address entry field and layer combo */
static void _ui_dasm_draw_controls(FDasmState* pDasmState)
{
	pDasmState->StartAddress = ui_util_input_u16("##addr", pDasmState->StartAddress);
	ImGui::SameLine();
	uint16_t addr = 0;

	if (ImGui::ArrowButton("##back", ImGuiDir_Left)) 
	{
		if (_ui_dasm_stack_back(pDasmState, &addr)) {
			_ui_dasm_goto(pDasmState, addr);
		}
	}

	if (ImGui::IsItemHovered() && (pDasmState->StackNum > 0))
	{
		ImGui::SetTooltip("Goto %04X", pDasmState->Stack[pDasmState->StackPos]);
	}
	
	ImGui::SameLine();
	ImGui::SameLine();

	// TODO: alternative combo??
	//ImGui::PushItemWidth(ImGui::GetContentRegionAvailWidth());
	//ImGui::Combo("##layer", &pDasmState->cur_layer, pDasmState->layers, pDasmState->num_layers);
	//ImGui::PopItemWidth();
}

/* draw the disassembly column */
static void DasmDrawDisassembly(FDasmState* pDasmState)
{
	ImGui::BeginChild("##dasmbox", ImVec2(0, 0), true);
	_ui_dasm_draw_controls(pDasmState);

	ImGui::BeginChild("##dasm", ImVec2(0, 0), false);
	ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(0, 0));
	ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(0, 0));
	const float line_height = ImGui::GetTextLineHeight();
	const float glyph_width = ImGui::CalcTextSize("F").x;
	const float cell_width = 3 * glyph_width;
	ImGuiListClipper clipper(DASM_NUM_LINES, line_height);

	/* skip hidden lines */
	pDasmState->CurrentAddress = pDasmState->StartAddress;
	for (int line_i = 0; (line_i < clipper.DisplayStart) && (line_i < DASM_NUM_LINES); line_i++) 
	{
		DasmDissassemble(pDasmState);
	}

	/* visible items */
	for (int line_i = clipper.DisplayStart; line_i < clipper.DisplayEnd; line_i++) 
	{
		const uint16_t op_addr = pDasmState->CurrentAddress;
		DasmDissassemble(pDasmState);
		const int num_bytes = pDasmState->bin_pos;

		/* highlight current hovered address */
		bool highlight = false;
		if (pDasmState->HighlightAddr == op_addr) 
		{
			ImGui::PushStyleColor(ImGuiCol_Text, pDasmState->HighlightAddr);
			highlight = true;
		}

		/* address */
		ImGui::Text("%04X: ", op_addr);
		ImGui::SameLine();

		/* instruction bytes */
		const float line_start_x = ImGui::GetCursorPosX();
		for (int n = 0; n < num_bytes; n++) {
			ImGui::SameLine(line_start_x + cell_width * n);
			ImGui::Text("%02X ", pDasmState->bin_buf[n]);
		}

		/* disassembled instruction */
		ImGui::SameLine(line_start_x + cell_width * 4 + glyph_width * 2);
		ImGui::Text("%s", pDasmState->str_buf);

		if (highlight) {
			ImGui::PopStyleColor();
		}

		/* check for jump instruction and draw an arrow  */
		uint16_t jump_addr = 0;
		if (_ui_dasm_jumptarget(pDasmState, pDasmState->CurrentAddress, &jump_addr)) 
		{
			ImGui::SameLine(line_start_x + cell_width * 4 + glyph_width * 2 + glyph_width * 20);
			ImGui::PushID(line_i);

			if (ImGui::ArrowButton("##btn", ImGuiDir_Right)) {
				ImGui::SetScrollY(0);
				_ui_dasm_goto(pDasmState, jump_addr);
				_ui_dasm_stack_push(pDasmState, op_addr);
			}

			if (ImGui::IsItemHovered()) 
			{
				ImGui::SetTooltip("Goto %04X", jump_addr);
				pDasmState->HighlightAddr = jump_addr;
			}
			ImGui::PopID();
		}
	}
	clipper.End();
	ImGui::PopStyleVar(2);
	ImGui::EndChild();
	ImGui::EndChild();
}

/* draw the stack */
static void DasmDrawStack(FDasmState* pDasmState)
{
	ImGui::BeginChild("##stackbox", ImVec2(72, 0), true);

	if (ImGui::Button("Clear")) 
		pDasmState->StackNum = 0;

	char buf[5] = { 0 };

	if (ImGui::ListBoxHeader("##stack", ImVec2(-1, -1))) 
	{

		for (int i = 0; i < pDasmState->StackNum; i++)
		{
			snprintf(buf, sizeof(buf), "%04X", pDasmState->Stack[i]);
			ImGui::PushID(i);

			if (ImGui::Selectable(buf, i == pDasmState->StackPos)) 
			{
				pDasmState->StackPos = i;
				_ui_dasm_goto(pDasmState, pDasmState->Stack[i]);
			}
			
			if (ImGui::IsItemHovered()) 
			{
				ImGui::SetTooltip("Goto %04X", pDasmState->Stack[i]);
				pDasmState->HighlightAddr = pDasmState->Stack[i];
			}
			
			ImGui::PopID();
		}
		ImGui::ListBoxFooter();
	}
	
	ImGui::EndChild();
}


void DasmDraw(FDasmState* pDasmState)
{
	DASM_ASSERT(pDasmState && pDasmState->valid);
	if (!pDasmState->open) {
		return;
	}
	ImGui::SetNextWindowPos(ImVec2(pDasmState->init_x, pDasmState->init_y), ImGuiCond_Once);
	ImGui::SetNextWindowSize(ImVec2(pDasmState->init_w, pDasmState->init_h), ImGuiCond_Once);
	if (ImGui::Begin(pDasmState->Title.c_str(), &pDasmState->open)) 
	{
		DasmDrawStack(pDasmState);
		ImGui::SameLine();
		DasmDrawDisassembly(pDasmState);
	}
	ImGui::End();
}