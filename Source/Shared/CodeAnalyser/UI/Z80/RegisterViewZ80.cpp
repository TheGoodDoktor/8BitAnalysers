#include "RegisterViewZ80.h"

#include "../../CodeAnalyser.h"
#include "../CodeAnalyserUI.h"

#include <Util/Misc.h>
#include <chips/z80.h>
#include <imgui.h>
#include <CodeAnalyser/Z80/CodeAnalyserZ80.h>

// took these out of the chips debugger
uint16_t InputU16(const char* label, uint16_t val) 
{
	const int bufSize = 5;
	char buf[bufSize];
	for (int i = 0; i < 4; i++) {
		buf[i] = "0123456789ABCDEF"[val >> ((3 - i) * 4) & 0xF];
	}
	buf[4] = 0;
	const int flags = ImGuiInputTextFlags_CharsHexadecimal |
		ImGuiInputTextFlags_CharsUppercase |
		ImGuiInputTextFlags_EnterReturnsTrue;
	ImGui::PushItemWidth(38);
	if (ImGui::InputText(label, buf, sizeof(buf), flags)) {
		int res;
		if (sscanf(buf, "%X", &res) == 1) {
			val = (uint16_t)res;
		}
	}
	ImGui::PopItemWidth();
	return val;
}

uint8_t InputU8(const char* label, uint8_t val)
{
	const int bufSize = 3;
	char buf[bufSize];
	for (int i = 0; i < 2; i++) {
		buf[i] = "0123456789ABCDEF"[val >> ((1 - i) * 4) & 0xF];
	}
	buf[2] = 0;
	const int flags = ImGuiInputTextFlags_CharsHexadecimal |
		ImGuiInputTextFlags_CharsUppercase |
		ImGuiInputTextFlags_EnterReturnsTrue;
	ImGui::PushItemWidth(22);
	if (ImGui::InputText(label, buf, sizeof(buf), flags)) {
		int res;
		if (sscanf(buf, "%X", &res) == 1) {
			val = (uint8_t)res;
		}
	}
	ImGui::PopItemWidth();
	return val;
}

FZ80DisplayRegisters::FZ80DisplayRegisters(z80_t* pCPU)
{
	A = pCPU->a;
	F = pCPU->f;
	B = pCPU->b;
	C = pCPU->c;
	D = pCPU->d;
	E = pCPU->e;
	H = pCPU->h;
	L = pCPU->l;
	A2 = pCPU->af2 >> 8;
	F2 = pCPU->af2 & 0xff;
	BC = pCPU->bc;
	DE = pCPU->de;
	HL = pCPU->hl;
	BC2 = pCPU->bc2;
	DE2 = pCPU->de2;
	HL2 = pCPU->hl2;
	IX = pCPU->ix;
	IY = pCPU->iy;
	SP = pCPU->sp;
	PC = pCPU->pc;
	I = pCPU->i;
	R = pCPU->r;
	IM = pCPU->im;

	// Flags
	const uint8_t f = pCPU->f;
	CarryFlag = !!(f & Z80_CF);
	AddSubtractFlag = !!(f & Z80_NF);
	ParityOverflowFlag = !!(f & Z80_VF);
	HalfCarryFlag = !!(f & Z80_HF);
	ZeroFlag = !!(f & Z80_ZF);
	SignFlag = !!(f & Z80_SF);

	IFF1 = pCPU->iff1;
	IFF2 = pCPU->iff2;
}

FZ80DisplayRegisters g_OldRegs;

void StoreRegisters_Z80(FCodeAnalysisState& state)
{
	g_OldRegs = FZ80DisplayRegisters((z80_t*)state.CPUInterface->GetCPUEmulator());
}

const FZ80DisplayRegisters& GetStoredRegisters_Z80(void)
{
	return g_OldRegs;
}

void DoByteRegisterTooltip(uint8_t byteValue)
{
	if (ImGui::IsItemHovered())
	{
		ImGui::BeginTooltip();
		ImGui::Text("%s", NumStr(byteValue, ENumberDisplayMode::Decimal));
		ImGui::Text("%s", NumStr(byteValue, ENumberDisplayMode::Binary));
		ImGui::EndTooltip();
	}
}

ImGuiTableFlags GetRegisterViewTableFlags() 
{
	return ImGuiTableFlags_Borders | ImGuiTableFlags_RowBg | ImGuiTableFlags_PreciseWidths | ImGuiTableFlags_SizingStretchSame;
}

void DrawRegisterPair(FCodeAnalysisState& state, uint8_t curLowByte, uint8_t curHighByte, uint16_t curWord, uint8_t oldLowByte, uint8_t oldHighByte, uint16_t oldWord, ImVec4 regNormalCol, ImVec4 regChangedCol, const char* tableName, const char* fmtLowByte, const char* fmtHighByte, const char* fmtWord)
{
	FCodeAnalysisViewState& viewState = state.GetFocussedViewState();
	ImGuiTableFlags tableFlags = GetRegisterViewTableFlags();

	ImGui::BeginTable(tableName, 3, tableFlags);

	// Setup the columns so each byte register takes up half the space of the word register.
	ImGui::TableSetupColumn("RegLowByte", ImGuiTableColumnFlags_WidthStretch, 1.0f);
	ImGui::TableSetupColumn("RegHighByte", ImGuiTableColumnFlags_WidthStretch, 1.0f);
	ImGui::TableSetupColumn("RegWord", ImGuiTableColumnFlags_WidthStretch, 2.0f);

	ImGui::TableNextColumn();
	ImGui::TextColored(curLowByte != oldLowByte ? regChangedCol : regNormalCol, fmtLowByte, NumStr(curLowByte));
	DoByteRegisterTooltip(curLowByte);

	ImGui::TableNextColumn();
	ImGui::TextColored(curHighByte != oldHighByte ? regChangedCol : regNormalCol, fmtHighByte, NumStr(curHighByte));
	DoByteRegisterTooltip(curHighByte);

	ImGui::TableNextColumn();
	ImGui::TextColored(curWord != oldWord ? regChangedCol : regNormalCol, fmtWord, NumStr(curWord));
	DrawAddressLabel(state, viewState, curWord);
	ImGui::EndTable();
}

void DrawRegisters_Z80(FCodeAnalysisState& state)
{
	FCodeAnalysisViewState& viewState = state.GetFocussedViewState();
	z80_t* pCPU = (z80_t*)state.CPUInterface->GetCPUEmulator();
	const ImVec4 regNormalCol(1.0f, 1.0f, 1.0f, 1.0f);
	const ImVec4 regChangedCol(1.0f, 1.0f, 0.0f, 1.0f);
	ImVec4 regColour = regNormalCol;

	FZ80DisplayRegisters curRegs(pCPU);
	const FZ80DisplayRegisters& oldRegs = g_OldRegs;

	ImGuiTableFlags tableFlags = GetRegisterViewTableFlags();

	// A
	ImGui::BeginTable("AFRegPair", 2, tableFlags);
	ImGui::TableNextColumn();
	ImGui::TextColored(curRegs.A != oldRegs.A ? regChangedCol : regNormalCol, "A:%s", NumStr(curRegs.A));
	DoByteRegisterTooltip(curRegs.A);
	ImGui::SameLine();

	// F
	ImGui::TableNextColumn();
	ImGui::TextColored(curRegs.F != oldRegs.F ? regChangedCol : regNormalCol, "F:%s", NumStr(curRegs.F));
	DoByteRegisterTooltip(curRegs.F);
	ImGui::EndTable();

	
	// CPU flags
	static ImGuiTableFlags flags = ImGuiTableFlags_Borders | ImGuiTableFlags_RowBg;
	if (ImGui::BeginTable("z80flags", 6, flags))
	{
		ImGui::TableSetupColumn("Carry");
		ImGui::TableSetupColumn("Add/Sub");
		ImGui::TableSetupColumn("Par/Ovr");
		ImGui::TableSetupColumn("H-Carry");
		ImGui::TableSetupColumn("Zero");
		ImGui::TableSetupColumn("Sign");
		ImGui::TableHeadersRow();

		ImGui::TableNextRow();
		ImGui::TableSetColumnIndex(0);
		ImGui::TextColored(curRegs.CarryFlag != oldRegs.CarryFlag ? regChangedCol : regNormalCol, "%s", curRegs.CarryFlag ? "Y" : "N");
		ImGui::TableSetColumnIndex(1);
		ImGui::TextColored(curRegs.AddSubtractFlag != oldRegs.AddSubtractFlag ? regChangedCol : regNormalCol, "%s", curRegs.AddSubtractFlag ? "Y" : "N");
		ImGui::TableSetColumnIndex(2);
		ImGui::TextColored(curRegs.ParityOverflowFlag != oldRegs.ParityOverflowFlag ? regChangedCol : regNormalCol, "%s", curRegs.ParityOverflowFlag ? "Y" : "N");
		ImGui::TableSetColumnIndex(3);
		ImGui::TextColored(curRegs.HalfCarryFlag != oldRegs.HalfCarryFlag ? regChangedCol : regNormalCol, "%s", curRegs.HalfCarryFlag ? "Y" : "N");
		ImGui::TableSetColumnIndex(4);
		ImGui::TextColored(curRegs.ZeroFlag != oldRegs.ZeroFlag ? regChangedCol : regNormalCol, "%s", curRegs.ZeroFlag ? "Y" : "N");
		ImGui::TableSetColumnIndex(5);
		ImGui::TextColored(curRegs.SignFlag != oldRegs.SignFlag ? regChangedCol : regNormalCol, "%s", curRegs.SignFlag ? "Y" : "N");

		ImGui::EndTable();
	}

	// B & C
	DrawRegisterPair(state, curRegs.B, curRegs.C, curRegs.BC, oldRegs.B, oldRegs.C, oldRegs.BC, regNormalCol, regChangedCol, "BCRegPair", "B:%s", "C:%s", "BC:%s");
	// D & E
	DrawRegisterPair(state, curRegs.D, curRegs.E, curRegs.DE, oldRegs.D, oldRegs.E, oldRegs.DE, regNormalCol, regChangedCol, "DERegPair", "D:%s", "E:%s", "DE:%s");
	// H & L
	DrawRegisterPair(state, curRegs.H, curRegs.L, curRegs.HL, oldRegs.H, oldRegs.L, oldRegs.HL, regNormalCol, regChangedCol, "HLRegPair", "H:%s", "L:%s", "HL:%s");


	// IX & IY
	
	// IX
	ImGui::BeginTable("IXIYRegs", 2, tableFlags);
	ImGui::TableNextColumn();
	ImGui::TextColored(curRegs.IX != oldRegs.IX ? regChangedCol : regNormalCol, "IX:%s", NumStr(curRegs.IX));
	DrawAddressLabel(state, viewState, curRegs.IX);
	// IY
	ImGui::TableNextColumn();
	ImGui::TextColored(curRegs.IY != oldRegs.IY ? regChangedCol : regNormalCol, "IY:%s", NumStr(curRegs.IY));
	DrawAddressLabel(state, viewState, curRegs.IY);
	ImGui::EndTable();

	// SP & PC
	ImGui::BeginTable("SPPCRegs", 2, tableFlags);
	ImGui::TableNextColumn();
	ImGui::TextColored(curRegs.SP != oldRegs.SP ? regChangedCol : regNormalCol, "SP:%s", NumStr(curRegs.SP));
	DrawAddressLabel(state, viewState, curRegs.SP);
	
	ImGui::TableNextColumn();
	ImGui::TextColored(curRegs.PC != oldRegs.PC ? regChangedCol : regNormalCol, "PC:%s", NumStr(curRegs.PC));
	DrawAddressLabel(state, viewState, curRegs.PC);
	ImGui::EndTable();


	// Shadow registers

	// A'
	ImGui::BeginTable("AF2RegPair", 2, tableFlags);
	ImGui::TableNextColumn();
	ImGui::TextColored(curRegs.A2 != oldRegs.A2 ? regChangedCol : regNormalCol, "A':%s", NumStr(curRegs.A2));
	DoByteRegisterTooltip(curRegs.A2);

	// F'
	ImGui::TableNextColumn();
	ImGui::TextColored(curRegs.F2 != oldRegs.F2 ? regChangedCol : regNormalCol, "F':%s", NumStr(curRegs.F2));
	DoByteRegisterTooltip(curRegs.F2);
	ImGui::EndTable();


	// B' & C'
	DrawRegisterPair(state, curRegs.B2, curRegs.C2, curRegs.BC2, oldRegs.B2, curRegs.C2, curRegs.BC2, regNormalCol, regChangedCol, "BC2RegPair", "B':%s", "C':%s", "BC':%s");
	// D' & E'
	DrawRegisterPair(state, curRegs.D2, curRegs.E2, curRegs.DE2, oldRegs.D2, curRegs.E2, curRegs.DE2, regNormalCol, regChangedCol, "DE2RegPair", "D':%s", "E':%s", "DE':%s");
	// H' & L'
	DrawRegisterPair(state, curRegs.H2, curRegs.L2, curRegs.HL2, oldRegs.H2, curRegs.L2, curRegs.HL2, regNormalCol, regChangedCol, "HL2RegPair", "H':%s", "L':%s", "HL':%s");

	// R
	ImGui::BeginTable("RReg", 1, tableFlags);
	ImGui::TableNextColumn();
	ImGui::TextColored(curRegs.R != oldRegs.R ? regChangedCol : regNormalCol, "R:%s", NumStr(curRegs.R));
	ImGui::EndTable();


	// Interrupt related
	// I
	ImGui::BeginTable("IRegs", 4, tableFlags);
	ImGui::TableNextColumn();
	ImGui::TextColored(curRegs.I != oldRegs.I ? regChangedCol : regNormalCol, "I:%s", NumStr(curRegs.I));

	// IM
	ImGui::TableNextColumn();
	ImGui::TextColored(curRegs.IM != oldRegs.IM ? regChangedCol : regNormalCol, "IM:%s", NumStr(curRegs.IM));

	// IFF 1 & 2
	ImGui::TableNextColumn();
	ImGui::TextColored(curRegs.IFF1 != oldRegs.IFF1 ? regChangedCol : regNormalCol, "IFF1:%s", curRegs.IFF1 ? "Y " : "N ");

	ImGui::TableNextColumn();
	ImGui::TextColored(curRegs.IFF2 != oldRegs.IFF2 ? regChangedCol : regNormalCol, "IFF 2:%s", curRegs.IFF2 ? "Y" : "N");
	ImGui::EndTable();
}

void DrawMachineStateZ80(const FMachineState* pMachineStateBase, FCodeAnalysisState& state, FCodeAnalysisViewState& viewState)
{
	const FMachineStateZ80* pMachineState = static_cast<const FMachineStateZ80*>(pMachineStateBase);
	assert(state.CPUInterface->CPUType == ECPUType::Z80);

	// A
	ImGui::Text("A:%s", NumStr(pMachineState->A));

	ImGui::Separator();

	// CPU flags
	static ImGuiTableFlags flags = ImGuiTableFlags_Borders | ImGuiTableFlags_RowBg;
	if (ImGui::BeginTable("z80flags", 6, flags))
	{
		ImGui::TableSetupColumn("Carry");
		ImGui::TableSetupColumn("Add/Sub");
		ImGui::TableSetupColumn("Par/Ovr");
		ImGui::TableSetupColumn("H-Carry");
		ImGui::TableSetupColumn("Zero");
		ImGui::TableSetupColumn("Sign");
		ImGui::TableHeadersRow();

		ImGui::TableNextRow();
		ImGui::TableSetColumnIndex(0);
		ImGui::Text("%s", (pMachineState->F & Z80_CF) ? "Y" : "N");
		ImGui::TableSetColumnIndex(1);
		ImGui::Text("%s", (pMachineState->F & Z80_NF) ? "Y" : "N");
		ImGui::TableSetColumnIndex(2);
		ImGui::Text("%s", (pMachineState->F & Z80_VF) ? "Y" : "N");
		ImGui::TableSetColumnIndex(3);
		ImGui::Text("%s", (pMachineState->F & Z80_HF) ? "Y" : "N");
		ImGui::TableSetColumnIndex(4);
		ImGui::Text("%s", (pMachineState->F & Z80_ZF) ? "Y" : "N");
		ImGui::TableSetColumnIndex(5);
		ImGui::Text("%s", (pMachineState->F & Z80_SF) ? "Y" : "N");

		ImGui::EndTable();
	}

	ImGui::Separator();

	// B & C
	ImGui::Text("B:%s", NumStr(pMachineState->B));
	ImGui::SameLine();
	ImGui::Text("C:%s", NumStr(pMachineState->C));
	ImGui::SameLine();
	ImGui::Text("BC:%s", NumStr(pMachineState->BC));
	DrawAddressLabel(state, viewState, pMachineState->BC);

	ImGui::Separator();

	/*// D & E
	ImGui::TextColored(curRegs.D != oldRegs.D ? regChangedCol : regNormalCol, "D:%s", NumStr(curRegs.D));
	ImGui::SameLine();
	ImGui::TextColored(curRegs.E != oldRegs.E ? regChangedCol : regNormalCol, "E:%s", NumStr(curRegs.E));
	ImGui::SameLine();
	ImGui::TextColored(curRegs.DE != oldRegs.DE ? regChangedCol : regNormalCol, "DE:%s", NumStr(curRegs.DE));
	DrawAddressLabel(state, viewState, curRegs.DE);

	ImGui::Separator();
*/


}