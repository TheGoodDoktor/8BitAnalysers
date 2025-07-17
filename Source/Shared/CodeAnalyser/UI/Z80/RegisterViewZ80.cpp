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
	B2 = pCPU->bc2 >> 8;
	C2 = pCPU->bc2 & 0xff;
	DE2 = pCPU->de2;
	D2 = pCPU->de2 >> 8;
	E2 = pCPU->de2 & 0xff;
	HL2 = pCPU->hl2;
	H2 = pCPU->hl2 >> 8;
	L2 = pCPU->hl2 & 0xff;
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

const ImVec4 g_RegNormalCol(1.0f, 1.0f, 1.0f, 1.0f);
const ImVec4 g_RegChangedCol(1.0f, 1.0f, 0.0f, 1.0f);

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

void EditByte(uint8_t* pByteData)
{
	const ENumberDisplayMode numMode = GetNumberDisplayMode();
	const float glyphWidth = ImGui::CalcTextSize("0").x;
	int flags = ImGuiInputTextFlags_EnterReturnsTrue;
	float width = glyphWidth * 2.0f;
	const char* format = "%02X";

	switch (numMode)
	{
	case ENumberDisplayMode::Decimal:
		width += glyphWidth;
		format = "%d";
		break;
	case ENumberDisplayMode::HexAitch:
	case ENumberDisplayMode::HexDollar:
	case ENumberDisplayMode::HexAmpersand:
		flags |= ImGuiInputTextFlags_CharsHexadecimal;
		break;
	default:
		break;
	}
	ImGui::PushID(pByteData);
	if (numMode == ENumberDisplayMode::HexDollar)
	{
		ImGui::Text("$");
		ImGui::SameLine(0, 0);
	}
	else if (numMode == ENumberDisplayMode::HexAmpersand)
	{
		ImGui::Text("&");
		ImGui::SameLine(0, 0);
	}
	ImGui::SetNextItemWidth((float)width);
	ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(0, 0));
	ImGui::SetItemAllowOverlap();	// allow controls
	ImGui::InputScalar("##byteinput", ImGuiDataType_U8, pByteData, NULL, NULL, format, ImGuiInputTextFlags_EnterReturnsTrue | ImGuiInputTextFlags_EnterReturnsTrue);
	if (numMode == ENumberDisplayMode::HexAitch)
	{
		ImGui::SameLine(0, 0);
		ImGui::Text("h");
	}
	ImGui::PopStyleVar();
	ImGui::PopID();
}

void EditWord(uint16_t* pWordData)
{
	const ENumberDisplayMode numMode = GetNumberDisplayMode();
	int flags = ImGuiInputTextFlags_EnterReturnsTrue;
	const float glyphWidth = ImGui::CalcTextSize("0").x;
	float width = glyphWidth * 4.0f;
	const char* format = "%04X";

	switch (numMode)
	{
	case ENumberDisplayMode::Decimal:
		width += glyphWidth;
		format = "%d";
		break;
	case ENumberDisplayMode::HexAitch:
	case ENumberDisplayMode::HexDollar:
	case ENumberDisplayMode::HexAmpersand:
		flags |= ImGuiInputTextFlags_CharsHexadecimal;
		break;
	default:
		break;
	}

	ImGui::PushID(pWordData);
	if (numMode == ENumberDisplayMode::HexDollar)
	{
		ImGui::Text("$");
		ImGui::SameLine(0, 0);
	}
	ImGui::SetNextItemWidth((float)width);
	ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(0, 0));
	ImGui::SetItemAllowOverlap();	// allow controls
	ImGui::InputScalar("##wordinput", ImGuiDataType_U16, pWordData, NULL, NULL, format, flags);
	
	if (numMode == ENumberDisplayMode::HexAitch)
	{
		ImGui::SameLine(0, 0);
		ImGui::Text("h");
	}
	ImGui::PopStyleVar();

	ImGui::PopID();
}

void DrawByteRegister(FCodeAnalysisState& state, uint8_t* curByte, uint8_t oldByte, const char* fmt)
{
	const ImVec4 col = *curByte != oldByte ? g_RegChangedCol : g_RegNormalCol;
	ImGui::BeginGroup();
	if (state.bAllowEditing)
	{
		ImGui::TextColored(col, fmt, "");
		ImGui::SameLine();
		ImGui::PushStyleColor(ImGuiCol_Text, col);
		EditByte(curByte);
		ImGui::PopStyleColor();
	}
	else
	{
		ImGui::TextColored(col, fmt, NumStr(*curByte));
	}
	ImGui::EndGroup();

	DoByteRegisterTooltip(*curByte);
}

void DrawWordRegister(FCodeAnalysisState& state, uint16_t* curWord, uint16_t oldWord, const char* fmt)
{
	const ImVec4 col = *curWord != oldWord ? g_RegChangedCol : g_RegNormalCol;

	ImGui::BeginGroup();
	if (state.bAllowEditing)
	{
		ImGui::Text(fmt, "");
		ImGui::SameLine();
		ImGui::PushStyleColor(ImGuiCol_Text, col);
		EditWord(curWord);
		ImGui::PopStyleColor();
	}
	else
	{
		ImGui::TextColored(col, fmt, NumStr(*curWord));
	}
	DrawAddressLabel(state, state.GetFocussedViewState(), *curWord);
	ImGui::EndGroup();
}

void DrawRegisterPair(FCodeAnalysisState& state, uint8_t* curLowByte, uint8_t* curHighByte, uint16_t* curWord, uint8_t oldLowByte, uint8_t oldHighByte, uint16_t oldWord, const char* tableName, const char* fmtLowByte, const char* fmtHighByte, const char* fmtWord)
{
	ImGuiTableFlags tableFlags = GetRegisterViewTableFlags();

	ImGui::BeginTable(tableName, 3, tableFlags);

	// Setup the columns so each byte register takes up half the space of the word register.
	ImGui::TableSetupColumn("RegLowByte", ImGuiTableColumnFlags_WidthStretch, 1.0f);
	ImGui::TableSetupColumn("RegHighByte", ImGuiTableColumnFlags_WidthStretch, 1.0f);
	ImGui::TableSetupColumn("RegWord", ImGuiTableColumnFlags_WidthStretch, 2.0f);

	// Low byte
	ImGui::TableNextColumn();

	DrawByteRegister(state, curLowByte, oldLowByte, fmtLowByte);
	
	// High byte
	ImGui::TableNextColumn();
	
	DrawByteRegister(state, curHighByte, oldHighByte, fmtHighByte);

	// Word register
	ImGui::TableNextColumn();

	DrawWordRegister(state, curWord, oldWord, fmtWord);
	
	ImGui::EndTable();
}

void DrawFlag(FCodeAnalysisState& state, bool* curFlag, bool oldFlag)
{
	const ImVec4 col = *curFlag != oldFlag ? g_RegChangedCol : g_RegNormalCol;
	ImGui::PushID(curFlag);
	if (state.bAllowEditing)
	{
		ImGui::PushStyleColor(ImGuiCol_CheckMark, col);
		ImGui::Checkbox("##flag", curFlag);
		ImGui::PopStyleColor();
	}
	else
	{
		ImGui::TextColored(col, "%s", *curFlag ? "Y" : "N");
	}
	ImGui::PopID();
}

void DrawRegisters_Z80(FCodeAnalysisState& state)
{
	FCodeAnalysisViewState& viewState = state.GetFocussedViewState();
	z80_t* pCPU = (z80_t*)state.CPUInterface->GetCPUEmulator();

	FZ80DisplayRegisters curRegs(pCPU);
	FZ80DisplayRegisters curRegsCopy = curRegs;
	const FZ80DisplayRegisters& oldRegs = g_OldRegs;

	ImGuiTableFlags tableFlags = GetRegisterViewTableFlags();

	// A
	ImGui::BeginTable("AFRegPair", 2, tableFlags);
	ImGui::TableNextColumn();
	DrawByteRegister(state, &curRegs.A, oldRegs.A, "A:%s");
	ImGui::SameLine();

	// F
	ImGui::TableNextColumn();
	DrawByteRegister(state, &curRegs.F, oldRegs.F, "F:%s");
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
		DrawFlag(state, &curRegs.CarryFlag, oldRegs.CarryFlag);

		ImGui::TableSetColumnIndex(1);
		DrawFlag(state, &curRegs.AddSubtractFlag, oldRegs.AddSubtractFlag);

		ImGui::TableSetColumnIndex(2);
		DrawFlag(state, &curRegs.ParityOverflowFlag, oldRegs.ParityOverflowFlag);

		ImGui::TableSetColumnIndex(3);
		DrawFlag(state, &curRegs.HalfCarryFlag, oldRegs.HalfCarryFlag);

		ImGui::TableSetColumnIndex(4);
		DrawFlag(state, &curRegs.ZeroFlag, oldRegs.ZeroFlag);

		ImGui::TableSetColumnIndex(5);
		DrawFlag(state, &curRegs.SignFlag, oldRegs.SignFlag);

		ImGui::EndTable();
	}

	// B & C
	DrawRegisterPair(state, &curRegs.B, &curRegs.C, &curRegs.BC, oldRegs.B, oldRegs.C, oldRegs.BC, "BCRegPair", "B:%s", "C:%s", "BC:%s");
	// D & E
	DrawRegisterPair(state, &curRegs.D, &curRegs.E, &curRegs.DE, oldRegs.D, oldRegs.E, oldRegs.DE, "DERegPair", "D:%s", "E:%s", "DE:%s");
	// H & L
	DrawRegisterPair(state, &curRegs.H, &curRegs.L, &curRegs.HL, oldRegs.H, oldRegs.L, oldRegs.HL, "HLRegPair", "H:%s", "L:%s", "HL:%s");


	// IX & IY
	
	// IX
	ImGui::BeginTable("IXIYRegs", 2, tableFlags);
	ImGui::TableNextColumn();
	DrawWordRegister(state, &curRegs.IX, oldRegs.IX, "IX:%s");

	// IY
	ImGui::TableNextColumn();
	DrawWordRegister(state, &curRegs.IY, oldRegs.IY, "IY:%s");
	ImGui::EndTable();

	// SP & PC
	ImGui::BeginTable("SPPCRegs", 2, tableFlags);
	ImGui::TableNextColumn();
	DrawWordRegister(state, &curRegs.SP, oldRegs.SP, "SP:%s");
	
	ImGui::TableNextColumn();
	DrawWordRegister(state, &curRegs.PC, oldRegs.PC, "PC:%s");
	ImGui::EndTable();


	// Shadow registers

	// A'
	ImGui::BeginTable("AF2RegPair", 2, tableFlags);
	ImGui::TableNextColumn();
	DrawByteRegister(state, &curRegs.A2, oldRegs.A2, "A':%s");


	// F'
	ImGui::TableNextColumn();
	DrawByteRegister(state, &curRegs.F2, oldRegs.F2, "F':%s");
	ImGui::EndTable();


	// B' & C'
	DrawRegisterPair(state, &curRegs.B2, &curRegs.C2, &curRegs.BC2, oldRegs.B2, curRegs.C2, curRegs.BC2, "BC2RegPair", "B':%s", "C':%s", "BC':%s");
	// D' & E'
	DrawRegisterPair(state, &curRegs.D2, &curRegs.E2, &curRegs.DE2, oldRegs.D2, curRegs.E2, curRegs.DE2, "DE2RegPair", "D':%s", "E':%s", "DE':%s");
	// H' & L'
	DrawRegisterPair(state, &curRegs.H2, &curRegs.L2, &curRegs.HL2, oldRegs.H2, curRegs.L2, curRegs.HL2, "HL2RegPair", "H':%s", "L':%s", "HL':%s");

	// R
	ImGui::BeginTable("RReg", 1, tableFlags);
	ImGui::TableNextColumn();
	DrawByteRegister(state, &curRegs.R, oldRegs.R, "R:%s");
	ImGui::EndTable();


	// Interrupt related
	// I
	ImGui::BeginTable("IRegs", 4, tableFlags);
	ImGui::TableNextColumn();
	DrawByteRegister(state, &curRegs.I, oldRegs.I, "I:%s");

	// IM
	ImGui::TableNextColumn();
	DrawByteRegister(state, &curRegs.IM, oldRegs.IM, "IM:%s");

	// IFF 1 & 2
	ImGui::TableNextColumn();
	ImGui::TextColored(curRegs.IFF1 != oldRegs.IFF1 ? g_RegChangedCol : g_RegNormalCol, "IFF1:%s", curRegs.IFF1 ? "Y " : "N ");

	ImGui::TableNextColumn();
	ImGui::TextColored(curRegs.IFF2 != oldRegs.IFF2 ? g_RegChangedCol : g_RegNormalCol, "IFF 2:%s", curRegs.IFF2 ? "Y" : "N");
	ImGui::EndTable();

	// If we've edited any of the registers, write them back to the CPU.
	if (state.bAllowEditing)
	{
		// A
		if (curRegs.A != curRegsCopy.A)
			pCPU->a = curRegs.A;

		// F
		if (curRegs.F != curRegsCopy.F)
			pCPU->f = curRegs.F;

		// BC
		if (curRegs.B != curRegsCopy.B)
			pCPU->b = curRegs.B;

		if (curRegs.C != curRegsCopy.C)
			pCPU->c = curRegs.C;

		if (curRegs.BC != curRegsCopy.BC)
			pCPU->bc = curRegs.BC;

		// DE
		if (curRegs.D != curRegsCopy.D)
			pCPU->d = curRegs.D;

		if (curRegs.E != curRegsCopy.E)
			pCPU->e = curRegs.E;

		if (curRegs.DE != curRegsCopy.DE)
			pCPU->de = curRegs.DE;

		// HL
		if (curRegs.H != curRegsCopy.H)
			pCPU->h = curRegs.H;

		if (curRegs.L != curRegsCopy.L)
			pCPU->l = curRegs.L;

		if (curRegs.HL != curRegsCopy.HL)
			pCPU->hl = curRegs.HL;

		// BC'
		if (curRegs.B2 != curRegsCopy.B2)
			pCPU->bc2 = (curRegs.B2 << 8) | pCPU->bc2 & 0xff;

		if (curRegs.C2 != curRegsCopy.C2)
			pCPU->bc2 = pCPU->bc2 & 0xff00 | curRegs.C2;

		if (curRegs.BC2 != curRegsCopy.BC2)
			pCPU->bc2 = curRegs.BC2;

		// DE'
		if (curRegs.D2 != curRegsCopy.D2)
			pCPU->de2 = (curRegs.D2 << 8) | pCPU->de2 & 0xff;

		if (curRegs.E2 != curRegsCopy.E2)
			pCPU->de2 = pCPU->de2 & 0xff00 | curRegs.E2;

		if (curRegs.DE2 != curRegsCopy.DE2)
			pCPU->de2 = curRegs.DE2;

		// HL'
		if (curRegs.H2 != curRegsCopy.H2)
			pCPU->hl2 = (curRegs.H2 << 8) | pCPU->hl2 & 0xff;

		if (curRegs.L2 != curRegsCopy.L2)
			pCPU->hl2 = pCPU->hl2 & 0xff00 | curRegs.L2;

		if (curRegs.HL2 != curRegsCopy.HL2)
			pCPU->hl2 = curRegs.HL2;

		// IX
		if (curRegs.IX != curRegsCopy.IX)
			pCPU->ix = curRegs.IX;

		// IY
		if (curRegs.IY != curRegsCopy.IY)
			pCPU->iy = curRegs.IY;

		// SP
		if (curRegs.SP != curRegsCopy.SP)
			pCPU->sp = curRegs.SP;

		// PC
		if (curRegs.PC != curRegsCopy.PC)
			pCPU->pc = curRegs.PC;

		// Flags
		if (curRegs.CarryFlag != curRegsCopy.CarryFlag)
			pCPU->f = (pCPU->f & ~Z80_CF) | (curRegs.CarryFlag ? Z80_CF : 0);

		if (curRegs.AddSubtractFlag != curRegsCopy.AddSubtractFlag)
			pCPU->f = (pCPU->f & ~Z80_NF) | (curRegs.AddSubtractFlag ? Z80_NF : 0);

		if (curRegs.ParityOverflowFlag != curRegsCopy.ParityOverflowFlag)
			pCPU->f = (pCPU->f & ~Z80_VF) | (curRegs.ParityOverflowFlag ? Z80_VF : 0);

		if (curRegs.HalfCarryFlag != curRegsCopy.HalfCarryFlag)
			pCPU->f = (pCPU->f & ~Z80_HF) | (curRegs.HalfCarryFlag ? Z80_HF : 0);

		if (curRegs.ZeroFlag != curRegsCopy.ZeroFlag)
			pCPU->f = (pCPU->f & ~Z80_ZF) | (curRegs.ZeroFlag ? Z80_ZF : 0);

		if (curRegs.SignFlag != curRegsCopy.SignFlag)
			pCPU->f = (pCPU->f & ~Z80_SF) | (curRegs.SignFlag ? Z80_SF : 0);
	}
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