#pragma once

// Phase 1 (skeleton) - C / C++ recompilation backend.
//
// FCppExporter is a sibling of FASMExporter: it reuses the base class's Init/Output/
// Finish/data-emission plumbing, but instead of emitting assembly it emits a faithful
// ("Tier A") translation driven by the Phase 0 control-flow graph.
//
// Output language is selectable (see FRecompilerConfig): C99 (the default) or C++. The
// translation model - flat memory + CPU-state struct + goto-labelled basic blocks + hook
// calls - is identical for both; only the accessor style (pointer/`->` vs reference/`.`),
// the header set, and the struct declaration differ.
//
// What IS implemented here: the structural scaffolding - CPU-state struct + memory
// runtime declarations, per-block functions with goto/return/dispatch control flow,
// call emission, and the entry-point wiring derived from the CFG.
//
// What is deliberately a STUB (marked TODO): the per-opcode semantics (EmitInstruction),
// which is the bulk of the work and the subject of the next milestone. See
// Docs/CppRecompilationProposal.md (section 4.2) and Docs/Phase0_CFG_Design.md.

#include <string>

#include "CodeAnalyser/AssemblerExport.h"
#include "ControlFlowGraph.h"

// Output configuration for the recompiler backend.
struct FRecompilerConfig
{
	bool	bEmitC = true;			// true = C99, false = C++
	bool	bEmitHarness = false;	// also emit a self-contained runtime (memory/IO/dispatch)
};

class FCppExporter : public FASMExporter
{
public:
	// Emit a complete translation unit for the inclusive physical range.
	bool	ExportProgram(uint16_t startAddr, uint16_t endAddr);

	void	SetTargetLanguageC(bool bEmitC) { RecompilerConfig.bEmitC = bEmitC; }
	void	SetEmitHarness(bool bEmit) { RecompilerConfig.bEmitHarness = bEmit; }

	// Emits the runtime preamble (CPU state struct + memory/IO hook declarations).
	// Called via the base-class header mechanism (SetOutputToHeader + AddHeader).
	void	AddHeader(void) override;

private:
	void	EmitCpuStateStruct(void);
	void	EmitRuntimeDeclarations(void);
	void	EmitRuntimeHelpers(void);	// inline Z80 flag/ALU primitives (mirrors chips z80.h)
	void	EmitEntryPointDeclarations(void);
	void	EmitHarness(void);			// self-contained memory/IO/dispatch definitions

	void	EmitBasicBlock(const FBasicBlock& block);
	// Emits one instruction's comment + semantics. Control-flow instructions emit no inline
	// transfer (EmitTerminator handles that); ordinary instructions emit their semantics.
	void	EmitInstruction(FAddressRef addr);
	void	EmitInstructionSemanticsZ80(FAddressRef addr);	// Phase 1 opcode subset
	void	EmitTerminator(const FBasicBlock& block);

	// C boolean expression for the condition of a conditional Z80 instruction at addr
	// (JR/JP/CALL/RET cc, and DJNZ which also decrements B). Empty if unconditional.
	std::string	ConditionExpr(FAddressRef addr);

	std::string	BlockLabel(FAddressRef addr) const;	// e.g. "L_AE5C"
	std::string	FunctionName(FAddressRef addr) const;	// label name or "func_AE5C"
	std::string	DisassemblyText(FAddressRef addr);		// best-effort mnemonic for comments

	// Target-language helpers - the only points where C and C++ output diverge.
	const char*	CpuArgDecl(void) const { return RecompilerConfig.bEmitC ? "Z80CpuState* cpu" : "Z80CpuState& cpu"; }
	const char*	Acc(void) const { return RecompilerConfig.bEmitC ? "cpu->" : "cpu."; }	// member-access prefix
	const char*	CpuPtr(void) const { return RecompilerConfig.bEmitC ? "cpu" : "&cpu"; }	// pass-by-pointer to helpers

	FRecompilerConfig	RecompilerConfig;
	FControlFlowGraph	CFG;
};

// Entry point mirroring ExportAssembler(): builds, emits, and finishes a file.
// bEmitC selects C99 (default) or C++ output; bEmitHarness appends a self-contained
// runtime so the file compiles and runs standalone (leaf routines).
bool	ExportCpp(class FEmuBase* pEmu, const char* pFilename, uint16_t startAddr, uint16_t endAddr, bool bEmitC = true, bool bEmitHarness = false);
