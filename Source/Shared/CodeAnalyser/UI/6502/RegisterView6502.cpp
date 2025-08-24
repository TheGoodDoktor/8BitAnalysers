#include "../../CodeAnalyser.h"
#include "../CodeAnalyserUI.h"

#include <Util/Misc.h>
#include <imgui.h>
#include <CodeAnalyser/6502/CodeAnalyser6502.h>
#include <CodeAnalyser/UI/MemoryAccessGrid.h>
#include <ImGuiSupport/ImGuiScaling.h>

class FZeroPageGrid : public FMemoryAccessGrid
{
	public:
		FZeroPageGrid(FCodeAnalysisState* pCodeAnalysis) :FMemoryAccessGrid(pCodeAnalysis, 16, 16)
		{
			//bDetailsToSide = true;
			bShowValues = true;
			bOutlineAllSquares = true;
		}

		FAddressRef GetGridSquareAddress(int x, int y) override
		{
			return CodeAnalysis->AddressRefFromPhysicalAddress(x + (y * 16));
		}
		void OnDraw() override
		{
			const float imgScale = ImGui_GetScaling();
			GridSquareSize = 20.0f * imgScale;	// to fit an 8x8 square on a scaling screen image
		}
};

struct F6502DisplayRegisters
{
	F6502DisplayRegisters() {}
	F6502DisplayRegisters(ICPUEmulator6502* pCPU)
	{
		if (pCPU)
		{
			A = pCPU->GetA();
			X = pCPU->GetX();
			Y = pCPU->GetY();
			S = pCPU->GetS();
			P = pCPU->GetP();
			PC = pCPU->GetPC();
		}

		// set flags
		CarryFlag					= P & ICPUEmulator6502::kFlagCarry;
		ZeroFlag						= P & ICPUEmulator6502::kFlagZero;
		InterruptDisableFlag		= P & ICPUEmulator6502::kFlagInterrupt;
		DecimalModeFlag			= P & ICPUEmulator6502::kFlagDecimal;
		BreakFlag					= P & ICPUEmulator6502::kFlagBreak;
		OverflowFlag				= P & ICPUEmulator6502::kFlagOverflow;
		NegativeFlag				= P & ICPUEmulator6502::kFlagNegative;
	}

	uint8_t		A = 0;
	uint8_t		X = 0;
	uint8_t		Y = 0;
	uint8_t		S = 0;
	uint8_t		P = 0;
	uint16_t		PC = 0;

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
	g_OldRegs = F6502DisplayRegisters((ICPUEmulator6502*)state.CPUInterface->GetCPUEmulator());
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
		ImGui::TableSetColumnIndex(6);
		ImGui::TextColored(curRegs.NegativeFlag != oldRegs.NegativeFlag ? regChangedCol : regNormalCol, "%s", curRegs.NegativeFlag ? "Y" : "N");

		ImGui::EndTable();
	}

	ImGui::Separator();

	// A
	ImGui::TextColored(curRegs.A != oldRegs.A ? regChangedCol : regNormalCol, "A:%s", NumStr(curRegs.A));
	// X
	ImGui::TextColored(curRegs.X != oldRegs.X ? regChangedCol : regNormalCol, "X:%s", NumStr(curRegs.X));
	// Y
	ImGui::TextColored(curRegs.Y != oldRegs.Y ? regChangedCol : regNormalCol, "Y:%s", NumStr(curRegs.Y));

	// Program counter
	ImGui::TextColored(curRegs.PC != oldRegs.PC ? regChangedCol : regNormalCol, "PC:%s", NumStr(curRegs.PC));
	DrawAddressLabel(state, viewState, curRegs.PC);

	// Stack pointer
	// Add 0x100 to give physical address
	const uint16_t StackPtr = curRegs.S + 0x100;
	ImGui::TextColored(curRegs.S != oldRegs.S ? regChangedCol : regNormalCol, "SP:%s", NumStr(StackPtr));
	DrawAddressLabel(state, viewState, StackPtr);

	ImGui::Separator();
	ImGui::Text("Zero Page");
	const ImVec2 pos = ImGui::GetCursorScreenPos(); 
	zeroPageGrid.DrawAt(pos.x, pos.y);
	zeroPageGrid.OnDraw();
	StoreRegisters_6502(state);
}