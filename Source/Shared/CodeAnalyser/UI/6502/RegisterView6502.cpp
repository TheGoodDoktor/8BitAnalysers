#include "../../CodeAnalyser.h"
#include "../CodeAnalyserUI.h"

#include <Util/Misc.h>
#include <chips/m6502.h>
#include <imgui.h>
#include <CodeAnalyser/6502/CodeAnalyser6502.h>

struct F6502DisplayRegisters
{
	F6502DisplayRegisters() {}
	F6502DisplayRegisters(m6502_t* pCPU)
	{
		A = pCPU->A;
		X = pCPU->X;
		Y = pCPU->Y;
		S = pCPU->S;
		P = pCPU->P;
		PC = pCPU->PC;

		// set flags
		CarryFlag				= P & (1 << 0);
		ZeroFlag				= P & (1 << 1);
		InterruptDisableFlag	= P & (1 << 2);
		DecimalModeFlag			= P & (1 << 3);
		BreakFlag				= P & (1 << 4);
		OverflowFlag			= P & (1 << 6);
		NegativeFlag			= P & (1 << 7);
	}

	uint8_t		A;
	uint8_t		X;
	uint8_t		Y;
	uint8_t		S;
	uint8_t		P;
	uint16_t	PC;

	// flags
	bool	CarryFlag = false;
	bool	ZeroFlag = false;
	bool	InterruptDisableFlag = false;
	bool	DecimalModeFlag = false;
	bool	BreakFlag = false;
	bool	OverflowFlag = false;
	bool	NegativeFlag = false;
};

static F6502DisplayRegisters g_OldRegs;

void StoreRegisters_6502(FCodeAnalysisState& state)
{
	g_OldRegs = F6502DisplayRegisters((m6502_t*)state.CPUInterface->GetCPUEmulator());
}

void DrawRegisters_6502(FCodeAnalysisState& state)
{
	FCodeAnalysisViewState& viewState = state.GetFocussedViewState();
	m6502_t* pCPU = (m6502_t*)state.CPUInterface->GetCPUEmulator();
	const ImVec4 regNormalCol(1.0f, 1.0f, 1.0f, 1.0f);
	const ImVec4 regChangedCol(1.0f, 1.0f, 0.0f, 1.0f);
	ImVec4 regColour = regNormalCol;

	F6502DisplayRegisters curRegs(pCPU);
	const F6502DisplayRegisters& oldRegs = g_OldRegs;

	// A
	ImGui::TextColored(curRegs.A != oldRegs.A ? regChangedCol : regNormalCol, "A:%s", NumStr(curRegs.A));

	ImGui::Separator();

	// CPU flags
	static ImGuiTableFlags flags = ImGuiTableFlags_Borders | ImGuiTableFlags_RowBg;
	if (ImGui::BeginTable("6502flags", 7, flags))
	{
		ImGui::TableSetupColumn("Carry");
		ImGui::TableSetupColumn("Zero");
		ImGui::TableSetupColumn("Int Disable");
		ImGui::TableSetupColumn("Decimal");
		ImGui::TableSetupColumn("Break");
		ImGui::TableSetupColumn("Overflow");
		ImGui::TableSetupColumn("Negative");
		ImGui::TableHeadersRow();

		ImGui::TableNextRow();
		ImGui::TableSetColumnIndex(0);
		ImGui::TextColored(curRegs.CarryFlag != oldRegs.CarryFlag ? regChangedCol : regNormalCol, "%s", curRegs.CarryFlag ? "Y" : "N");
		ImGui::TableSetColumnIndex(1);
		ImGui::TextColored(curRegs.ZeroFlag != oldRegs.ZeroFlag ? regChangedCol : regNormalCol, "%s", curRegs.ZeroFlag ? "Y" : "N");
		ImGui::TableSetColumnIndex(2);
		ImGui::TextColored(curRegs.InterruptDisableFlag != oldRegs.InterruptDisableFlag ? regChangedCol : regNormalCol, "%s", curRegs.InterruptDisableFlag ? "Y" : "N");
		ImGui::TableSetColumnIndex(3);
		ImGui::TextColored(curRegs.DecimalModeFlag != oldRegs.DecimalModeFlag ? regChangedCol : regNormalCol, "%s", curRegs.DecimalModeFlag ? "Y" : "N");
		ImGui::TableSetColumnIndex(4);
		ImGui::TextColored(curRegs.BreakFlag != oldRegs.BreakFlag ? regChangedCol : regNormalCol, "%s", curRegs.BreakFlag ? "Y" : "N");
		ImGui::TableSetColumnIndex(5);
		ImGui::TextColored(curRegs.OverflowFlag != oldRegs.OverflowFlag ? regChangedCol : regNormalCol, "%s", curRegs.OverflowFlag ? "Y" : "N");
		ImGui::TableSetColumnIndex(5);
		ImGui::TextColored(curRegs.NegativeFlag != oldRegs.NegativeFlag ? regChangedCol : regNormalCol, "%s", curRegs.NegativeFlag ? "Y" : "N");

		ImGui::EndTable();
	}

	ImGui::Separator();

	ImGui::TextColored(curRegs.X != oldRegs.X ? regChangedCol : regNormalCol, "X:%s", NumStr(curRegs.X));
	ImGui::SameLine();
	ImGui::TextColored(curRegs.Y != oldRegs.Y ? regChangedCol : regNormalCol, "Y:%s", NumStr(curRegs.Y));

	ImGui::TextColored(curRegs.PC != oldRegs.PC ? regChangedCol : regNormalCol, "PC:%s", NumStr(curRegs.PC));
	DrawAddressLabel(state, viewState, curRegs.PC);

	const uint16_t StackPtr = curRegs.S + 0x100;
	ImGui::TextColored(curRegs.S != oldRegs.S ? regChangedCol : regNormalCol, "SP:%s", NumStr(StackPtr));
	DrawAddressLabel(state, viewState, StackPtr);

	ImGui::Text("TODO: Implement for 6502");
}