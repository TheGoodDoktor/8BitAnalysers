#pragma once

// Phase 0 - Control-Flow Graph reconstruction.
// Builds an explicit basic-block graph from the per-instruction data the analyser
// already stores (FCodeInfo::OperandAddress / bIsCall / ByteSize, labels, function
// entries). See Docs/Phase0_CFG_Design.md.
//
// This is pure analysis: it produces no output. The C++ recompiler (FCppExporter)
// is the first consumer; a call-graph view and dead-code analysis could be others.

#include <cstdint>
#include <map>
#include <vector>

#include "CodeAnalyser/CodeAnalyserTypes.h"

class FCodeAnalysisState;
struct FFunctionInfo;

// How control leaves the *end* of a basic block.
enum class EBlockTerminator
{
	FallThrough,	// no control instruction; flows into the next block
	UncondBranch,	// JP nn / JR e
	CondBranch,		// JP cc,nn / JR cc,e / DJNZ e (two successors)
	Return,			// RET / RET cc / RETI / RETN
	IndirectJump,	// JP (HL) / JP (IX) / JP (IY) - target not statically known
	Call,			// only when a call is modelled as a block-ending tail call
	Halt,			// HALT
	Unresolved,		// could not be classified (treat conservatively)
};

// The kind of a single outgoing edge.
enum class EEdgeType
{
	FallThrough,	// sequential flow to the following block
	Branch,			// conditional branch taken-target
	Jump,			// unconditional jump target
	Call,			// CALL / RST target (call graph edge)
	Return,			// return edge (to caller - usually left implicit)
	IndirectJump,	// resolved indirect target (e.g. from a trace)
	Unresolved,		// indirect target that is not (yet) known
};

// A single edge between code addresses (block starts, except Call which targets a callee entry).
struct FFlowEdge
{
	FFlowEdge() = default;
	FFlowEdge(FAddressRef from, FAddressRef to, EEdgeType type, bool bCond)
		: From(from), To(to), Type(type), bConditional(bCond) {}

	FAddressRef	From;
	FAddressRef	To;				// invalid for Unresolved edges
	EEdgeType	Type = EEdgeType::FallThrough;
	bool		bConditional = false;
};

// A maximal straight-line run of instructions with a single entry and single (terminating) exit.
struct FBasicBlock
{
	FAddressRef	StartAddress;
	FAddressRef	EndAddress;			// address of the LAST byte of the last instruction
	FAddressRef	LastInstruction;	// address of the terminator instruction

	EBlockTerminator		Terminator = EBlockTerminator::FallThrough;
	std::vector<FFlowEdge>	Successors;
	std::vector<FAddressRef>	Predecessors;	// block-start addresses that flow here

	bool	bIsEntry = false;			// a function/label entry point
	bool	bContainsSelfModifyingCode = false;	// any instruction flagged bSelfModifyingCode
	int		InstructionCount = 0;
};

// The reconstructed graph for an address range or a function.
struct FControlFlowGraph
{
	void Clear()
	{
		Blocks.clear();
		EntryPoints.clear();
		UnresolvedEdges.clear();
	}

	FBasicBlock* GetBlockStartingAt(FAddressRef addr)
	{
		auto it = Blocks.find(addr);
		return it != Blocks.end() ? &it->second : nullptr;
	}

	// Find the block that contains addr (addr need not be the block start).
	FBasicBlock* GetBlockContaining(FAddressRef addr);

	std::map<FAddressRef, FBasicBlock>	Blocks;			// keyed by block start
	std::vector<FAddressRef>			EntryPoints;	// function entries / range start
	std::vector<FFlowEdge>				UnresolvedEdges;// indirect jumps awaiting resolution
};

// Result of classifying a single instruction's effect on control flow.
struct FInstructionFlow
{
	EBlockTerminator	Terminator = EBlockTerminator::FallThrough;
	bool				bConditional = false;	// branch/call/ret is conditional
	bool				bIsCall = false;		// CALL / RST (records a call edge)
	bool				bFallsThrough = true;	// execution can continue to the next instr
	bool				bEndsBlock = false;		// terminates the basic block
	FAddressRef			Target;					// direct target if statically known
	uint16_t			ByteSize = 1;
};

// --- Public API -------------------------------------------------------------

// CPU-dispatched classifier (keys off state.CPUType). Z80 implemented; 6502 stub.
FInstructionFlow	ClassifyInstruction(FCodeAnalysisState& state, FAddressRef addr);
FInstructionFlow	ClassifyInstructionZ80(FCodeAnalysisState& state, FAddressRef addr);
FInstructionFlow	ClassifyInstruction6502(FCodeAnalysisState& state, FAddressRef addr);

// Build a CFG for an inclusive address range.
bool	BuildCFGForAddressRange(FCodeAnalysisState& state, FAddressRef start, FAddressRef end, FControlFlowGraph& outCFG);

// Build a CFG for a single analysed function (uses its Start/End and exit points).
bool	BuildCFGForFunction(FCodeAnalysisState& state, const FFunctionInfo& function, FControlFlowGraph& outCFG);

// Optional: promote UnresolvedEdges to resolved targets using the emulator's
// observed execution history. Returns the number of edges resolved.
int		ResolveIndirectEdgesFromTraces(FCodeAnalysisState& state, FControlFlowGraph& cfg);
