#include "RegisterView6502.h"
#include "../../CodeAnalyser.h"
#include "../CodeAnalyserUI.h"

#include <Util/Misc.h>
#include <imgui.h>
#include <CodeAnalyser/6502/CodeAnalyser6502.h>
#include <CodeAnalyser/UI/MemoryAccessGrid.h>
#include <ImGuiSupport/ImGuiScaling.h>

#include "../RegisterViewCommon.h"

class FZeroPageGrid : public FMemoryAccessGrid
{
	public:
		FZeroPageGrid(FCodeAnalysisState* pCodeAnalysis) :FMemoryAccessGrid(pCodeAnalysis, 16, 16)
		{
			//bDetailsToSide = true;
			bShowValues = true;
			bOutlineAllSquares = true;
			ZeroPageAddr = pCodeAnalysis->CPUInterface->CPUType == ECPUType::HuC6280 ? 0x2000 : 0; 
		}

		FAddressRef GetGridSquareAddress(int x, int y) override
		{
			return CodeAnalysis->AddressRefFromPhysicalAddress(ZeroPageAddr + x + (y * 16));
		}
		void OnDraw() override
		{
			const float imgScale = ImGui_GetScaling();
			GridSquareSize = 20.0f * imgScale;	// to fit an 8x8 square on a scaling screen image
		}
		uint16_t ZeroPageAddr = 0;
};

F6502DisplayRegisters::F6502DisplayRegisters(ICPUEmulator6502* pCPU)
{
	if (pCPU)
	{
		A  = pCPU->GetA();
		X  = pCPU->GetX();
		Y  = pCPU->GetY();
		S  = pCPU->GetS();
		P  = pCPU->GetP();
		PC = pCPU->GetPC();
	}

	CarryFlag            = P & ICPUEmulator6502::kFlagCarry;
	ZeroFlag             = P & ICPUEmulator6502::kFlagZero;
	InterruptDisableFlag = P & ICPUEmulator6502::kFlagInterrupt;
	DecimalModeFlag      = P & ICPUEmulator6502::kFlagDecimal;
	BreakFlag            = P & ICPUEmulator6502::kFlagBreak;
	OverflowFlag         = P & ICPUEmulator6502::kFlagOverflow;
	NegativeFlag         = P & ICPUEmulator6502::kFlagNegative;
}

static F6502DisplayRegisters g_OldRegs;

void StoreRegisters_6502(FCodeAnalysisState& state)
{
	g_OldRegs = F6502DisplayRegisters((ICPUEmulator6502*)state.CPUInterface->GetCPUEmulator());
}

const F6502DisplayRegisters& GetStoredRegisters_6502()
{
	return g_OldRegs;
}

void DrawRegisters_6502(FCodeAnalysisState& state)
{
	FCodeAnalysisViewState& viewState = state.GetFocussedViewState();
	ICPUEmulator6502* p6502CPU = (ICPUEmulator6502*)state.CPUInterface->GetCPUEmulator();

	const ImVec4 regNormalCol(1.0f, 1.0f, 1.0f, 1.0f);
	const ImVec4 regChangedCol(1.0f, 1.0f, 0.0f, 1.0f);
	ImVec4 regColour = regNormalCol;
	static FZeroPageGrid zeroPageGrid(&state);
	zeroPageGrid.FixupAddressRefs(); // hack

	F6502DisplayRegisters curRegs(p6502CPU);
	const F6502DisplayRegisters& oldRegs = g_OldRegs;

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
		DrawFlag(state, &curRegs.CarryFlag, oldRegs.CarryFlag);
		ImGui::TableSetColumnIndex(1);
		DrawFlag(state, &curRegs.ZeroFlag, oldRegs.ZeroFlag);
		ImGui::TableSetColumnIndex(2);
		DrawFlag(state, &curRegs.InterruptDisableFlag, oldRegs.InterruptDisableFlag);
		ImGui::TableSetColumnIndex(3);
		DrawFlag(state, &curRegs.DecimalModeFlag, oldRegs.DecimalModeFlag);
		ImGui::TableSetColumnIndex(4);
		DrawFlag(state, &curRegs.BreakFlag, oldRegs.BreakFlag);
		ImGui::TableSetColumnIndex(5);
		DrawFlag(state, &curRegs.OverflowFlag, oldRegs.OverflowFlag);
		ImGui::TableSetColumnIndex(6);
		DrawFlag(state, &curRegs.NegativeFlag, oldRegs.NegativeFlag);

		ImGui::EndTable();
	}

	ImGui::Separator();

	// A
	//ImGui::TextColored(curRegs.A != oldRegs.A ? regChangedCol : regNormalCol, "A:%s", NumStr(curRegs.A));
	DrawByteRegister(state, &curRegs.A, oldRegs.A, "A:%s");

	// X
	//ImGui::TextColored(curRegs.X != oldRegs.X ? regChangedCol : regNormalCol, "X:%s", NumStr(curRegs.X));
	DrawByteRegister(state, &curRegs.X, oldRegs.X, "X:%s");

	// Y
	//ImGui::TextColored(curRegs.Y != oldRegs.Y ? regChangedCol : regNormalCol, "Y:%s", NumStr(curRegs.Y));
	DrawByteRegister(state, &curRegs.Y, oldRegs.Y, "Y:%s");

	// Program counter
	ImGui::TextColored(curRegs.PC != oldRegs.PC ? regChangedCol : regNormalCol, "PC:%s", NumStr(curRegs.PC));
	DrawAddressLabel(state, viewState, curRegs.PC);

	// Stack pointer
	// Add 0x100 to give physical address
	const uint16_t StackPtr = state.CPUInterface->CPUType == ECPUType::HuC6280 ? 0x2000 : 0 + curRegs.S + 0x100;
	ImGui::TextColored(curRegs.S != oldRegs.S ? regChangedCol : regNormalCol, "SP:%s", NumStr(StackPtr));
	DrawAddressLabel(state, viewState, StackPtr);

	ImGui::Separator();
	const ImVec2 pos = ImGui::GetCursorScreenPos(); 
	zeroPageGrid.DrawAt(pos.x, pos.y);
	zeroPageGrid.OnDraw();
	ImGui::Text("Zero Page");

	// If we've edited any of the registers, write them back to the CPU.
	if (state.bAllowEditing)
	{
		// A
		if (curRegs.A != oldRegs.A)
			p6502CPU->SetA(curRegs.A);

		// X
		if (curRegs.X != oldRegs.X)
			p6502CPU->SetX(curRegs.X);

		// Y
		if (curRegs.Y != oldRegs.Y)
			p6502CPU->SetY(curRegs.Y);

		// Flags
		//uint8_t p = p6502CPU->GetP();
		//if (curRegs.CarryFlag != oldRegs.CarryFlag)
		//	p6502CPU->SetP((p6502CPU->GetP() & ~ICPUEmulator6502::kFlagCarry) | (curRegs.CarryFlag ? ICPUEmulator6502::kFlagCarry : 0));
	}
}