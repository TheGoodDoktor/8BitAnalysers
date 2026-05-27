#include "ControlFlowGraph.h"

#include "CodeAnalyser/CodeAnalyser.h"
#include "CodeAnalyser/FunctionAnalyser.h"

#include <set>

// =====================================================================================
// Phase 0 - CFG reconstruction. See Docs/Phase0_CFG_Design.md.
// This is a skeleton: the Z80 terminator classifier is implemented; the 6502 variant
// and the runtime-trace back-fill are stubs marked TODO.
// =====================================================================================

// -------------------------------------------------------------------------------------
// Instruction classification
// -------------------------------------------------------------------------------------

FInstructionFlow ClassifyInstructionZ80(FCodeAnalysisState& state, FAddressRef addr)
{
	FInstructionFlow flow;

	const FCodeInfo* pCodeInfo = state.GetCodeInfoForAddress(addr);
	flow.ByteSize = pCodeInfo ? pCodeInfo->ByteSize : 1;

	// The analyser has already resolved direct branch/call targets into OperandAddress.
	const FAddressRef target = pCodeInfo ? pCodeInfo->OperandAddress : FAddressRef();
	const uint8_t op = state.ReadByte(addr);

	switch (op)
	{
		// --- Unconditional jumps -----------------------------------------------------
		case 0xC3:	// JP nn
		case 0x18:	// JR e
			flow.Terminator = EBlockTerminator::UncondBranch;
			flow.bEndsBlock = true;
			flow.bFallsThrough = false;
			flow.Target = target;
			break;

		// --- Conditional branches (two successors) -----------------------------------
		case 0xC2: case 0xCA: case 0xD2: case 0xDA:	// JP cc,nn
		case 0xE2: case 0xEA: case 0xF2: case 0xFA:
		case 0x20: case 0x28: case 0x30: case 0x38:	// JR cc,e
		case 0x10:									// DJNZ e
			flow.Terminator = EBlockTerminator::CondBranch;
			flow.bConditional = true;
			flow.bEndsBlock = true;
			flow.bFallsThrough = true;
			flow.Target = target;
			break;

		// --- Indirect jump (target not statically known) -----------------------------
		case 0xE9:	// JP (HL)
			flow.Terminator = EBlockTerminator::IndirectJump;
			flow.bEndsBlock = true;
			flow.bFallsThrough = false;
			break;

		// --- Calls (end the block: control goes to the callee; the return address is
		//     the next instruction, which becomes a leader / dispatch resume point) ----
		case 0xCD:	// CALL nn
			flow.Terminator = EBlockTerminator::Call;
			flow.bIsCall = true;
			flow.bEndsBlock = true;
			flow.bFallsThrough = false;
			flow.Target = target;
			break;
		case 0xC4: case 0xCC: case 0xD4: case 0xDC:	// CALL cc,nn
		case 0xE4: case 0xEC: case 0xF4: case 0xFC:
			flow.Terminator = EBlockTerminator::Call;
			flow.bIsCall = true;
			flow.bConditional = true;
			flow.bEndsBlock = true;
			flow.bFallsThrough = true;	// the not-taken path continues at the next instruction
			flow.Target = target;
			break;

		// --- RST p (call to a fixed page-0 address) ----------------------------------
		case 0xC7: case 0xCF: case 0xD7: case 0xDF:
		case 0xE7: case 0xEF: case 0xF7: case 0xFF:
			flow.Terminator = EBlockTerminator::Call;
			flow.bIsCall = true;
			flow.bEndsBlock = true;
			flow.bFallsThrough = false;
			flow.Target = target.IsValid() ? target : state.AddressRefFromPhysicalAddress(op & 0x38);
			break;

		// --- Returns -----------------------------------------------------------------
		case 0xC9:	// RET
			flow.Terminator = EBlockTerminator::Return;
			flow.bEndsBlock = true;
			flow.bFallsThrough = false;
			break;
		case 0xC0: case 0xC8: case 0xD0: case 0xD8:	// RET cc (can fall through)
		case 0xE0: case 0xE8: case 0xF0: case 0xF8:
			flow.Terminator = EBlockTerminator::Return;
			flow.bConditional = true;
			flow.bEndsBlock = true;
			flow.bFallsThrough = true;
			break;

		// --- HALT --------------------------------------------------------------------
		case 0x76:
			flow.Terminator = EBlockTerminator::Halt;
			flow.bEndsBlock = true;
			flow.bFallsThrough = false;
			break;

		// --- Prefixed opcodes --------------------------------------------------------
		case 0xDD: case 0xFD:	// IX / IY prefix - only JP (IX)/(IY) affects flow
		{
			FAddressRef a2 = addr;
			state.AdvanceAddressRef(a2, 1);
			if (state.ReadByte(a2) == 0xE9)	// JP (IX) / JP (IY)
			{
				flow.Terminator = EBlockTerminator::IndirectJump;
				flow.bEndsBlock = true;
				flow.bFallsThrough = false;
			}
			break;
		}
		case 0xED:	// extended - RETI (0x4D) / RETN (0x45) are returns
		{
			FAddressRef a2 = addr;
			state.AdvanceAddressRef(a2, 1);
			const uint8_t op2 = state.ReadByte(a2);
			if (op2 == 0x4D || op2 == 0x45)
			{
				flow.Terminator = EBlockTerminator::Return;
				flow.bEndsBlock = true;
				flow.bFallsThrough = false;
			}
			break;
		}

		default:
			// Everything else is straight-line: flow.Terminator stays FallThrough.
			break;
	}

	return flow;
}

FInstructionFlow ClassifyInstruction6502(FCodeAnalysisState& state, FAddressRef addr)
{
	// TODO(Phase 0, 6502): classify JMP/JMP(ind)/JSR/RTS/RTI/Bcc/BRK.
	// Until implemented, treat every instruction as fall-through so a 6502 range
	// degrades to a single block rather than producing wrong edges.
	FInstructionFlow flow;
	const FCodeInfo* pCodeInfo = state.GetCodeInfoForAddress(addr);
	flow.ByteSize = pCodeInfo ? pCodeInfo->ByteSize : 1;
	return flow;
}

FInstructionFlow ClassifyInstruction(FCodeAnalysisState& state, FAddressRef addr)
{
	switch (state.CPUInterface->CPUType)
	{
		case ECPUType::Z80:		return ClassifyInstructionZ80(state, addr);
		case ECPUType::M6502:
		case ECPUType::M65C02:	return ClassifyInstruction6502(state, addr);
		default:				return FInstructionFlow();	// fall-through default
	}
}

// -------------------------------------------------------------------------------------
// CFG construction
// -------------------------------------------------------------------------------------

namespace
{
	struct FInstrRecord
	{
		FAddressRef			Addr;
		FAddressRef			NextAddr;	// address of the following instruction
		FInstructionFlow	Flow;
		bool				bSelfModifying = false;
	};

	bool InRange(FAddressRef a, FAddressRef start, FAddressRef end)
	{
		return a.IsValid() && a >= start && a <= end;
	}

	bool IsEntryLabel(FCodeAnalysisState& state, FAddressRef addr)
	{
		const FLabelInfo* pLabel = state.GetLabelForAddress(addr);
		if (pLabel != nullptr && (pLabel->Global || pLabel->LabelType == ELabelType::Function))
			return true;
		return state.pFunctions != nullptr && state.pFunctions->GetFunctionAtAddress(addr) != nullptr;
	}
}

bool BuildCFGForAddressRange(FCodeAnalysisState& state, FAddressRef start, FAddressRef end, FControlFlowGraph& outCFG)
{
	outCFG.Clear();
	if (start.IsValid() == false || end < start)
		return false;

	// --- Pass 1: walk instructions, collect records + leaders -------------------------
	std::vector<FInstrRecord> instrs;
	std::set<FAddressRef> leaders;
	leaders.insert(start);

	FAddressRef addr = start;
	while (InRange(addr, start, end))
	{
		const FCodeInfo* pCodeInfo = state.GetCodeInfoForAddress(addr);
		if (pCodeInfo == nullptr)
		{
			// Not a decoded instruction (inline data / undecoded). Skip a byte.
			if (state.AdvanceAddressRef(addr, 1) == false)
				break;
			continue;
		}

		FInstrRecord rec;
		rec.Addr = addr;
		rec.Flow = ClassifyInstruction(state, addr);
		rec.bSelfModifying = pCodeInfo->bSelfModifyingCode;

		FAddressRef next = addr;
		state.AdvanceAddressRef(next, rec.Flow.ByteSize);
		rec.NextAddr = next;
		instrs.push_back(rec);

		// Leader: any entry label/function start.
		if (IsEntryLabel(state, addr))
			leaders.insert(addr);

		// Leader: a (resolved) branch / jump / call target inside the range. Call targets
		// matter for the PC-dispatch model - the callee entry must be its own block so the
		// dispatcher can resume there.
		if ((rec.Flow.Terminator == EBlockTerminator::UncondBranch ||
			 rec.Flow.Terminator == EBlockTerminator::CondBranch ||
			 rec.Flow.Terminator == EBlockTerminator::Call) &&
			InRange(rec.Flow.Target, start, end))
		{
			leaders.insert(rec.Flow.Target);
		}

		// Leader: the instruction following a block-terminating instruction.
		if (rec.Flow.bEndsBlock && InRange(next, start, end))
			leaders.insert(next);

		addr = next;
	}

	if (instrs.empty())
		return false;

	// --- Pass 2: group records into blocks, build successor edges ---------------------
	size_t i = 0;
	while (i < instrs.size())
	{
		FBasicBlock block;
		block.StartAddress = instrs[i].Addr;
		block.bIsEntry = IsEntryLabel(state, block.StartAddress);

		// Accumulate until terminator or until the next record is a leader.
		size_t termIdx = i;
		bool bSMC = false;
		int count = 0;
		while (termIdx < instrs.size())
		{
			bSMC |= instrs[termIdx].bSelfModifying;
			count++;

			const bool bLastRecord = (termIdx + 1 == instrs.size());
			const bool bNextIsLeader = !bLastRecord && leaders.count(instrs[termIdx + 1].Addr) > 0;
			if (instrs[termIdx].Flow.bEndsBlock || bLastRecord || bNextIsLeader)
				break;
			termIdx++;
		}

		const FInstrRecord& term = instrs[termIdx];
		FAddressRef lastByte = term.Addr;
		if (term.Flow.ByteSize > 1)
			state.AdvanceAddressRef(lastByte, term.Flow.ByteSize - 1);

		block.EndAddress = lastByte;
		block.LastInstruction = term.Addr;
		block.Terminator = term.Flow.bEndsBlock ? term.Flow.Terminator : EBlockTerminator::FallThrough;
		block.InstructionCount = count;
		block.bContainsSelfModifyingCode = bSMC;

		// Flow successors based on the terminator.
		switch (block.Terminator)
		{
			case EBlockTerminator::UncondBranch:
				if (term.Flow.Target.IsValid())
					block.Successors.emplace_back(block.StartAddress, term.Flow.Target, EEdgeType::Jump, false);
				else
					outCFG.UnresolvedEdges.emplace_back(block.StartAddress, FAddressRef(), EEdgeType::Unresolved, false);
				break;

			case EBlockTerminator::CondBranch:
				if (term.Flow.Target.IsValid())
					block.Successors.emplace_back(block.StartAddress, term.Flow.Target, EEdgeType::Branch, true);
				block.Successors.emplace_back(block.StartAddress, term.NextAddr, EEdgeType::FallThrough, false);
				break;

			case EBlockTerminator::Call:
				// Call edge to the callee; the return address (next instr) is the resume point.
				if (term.Flow.Target.IsValid())
					block.Successors.emplace_back(block.StartAddress, term.Flow.Target, EEdgeType::Call, term.Flow.bConditional);
				block.Successors.emplace_back(block.StartAddress, term.NextAddr, EEdgeType::FallThrough, false);
				break;

			case EBlockTerminator::Return:
				if (term.Flow.bConditional)	// conditional RET can fall through
					block.Successors.emplace_back(block.StartAddress, term.NextAddr, EEdgeType::FallThrough, false);
				break;

			case EBlockTerminator::IndirectJump:
				outCFG.UnresolvedEdges.emplace_back(block.StartAddress, FAddressRef(), EEdgeType::Unresolved, false);
				break;

			case EBlockTerminator::Halt:
				break;	// no successor

			case EBlockTerminator::FallThrough:
			default:
				if (term.NextAddr.IsValid())
					block.Successors.emplace_back(block.StartAddress, term.NextAddr, EEdgeType::FallThrough, false);
				break;
		}

		outCFG.Blocks[block.StartAddress] = block;
		i = termIdx + 1;
	}

	// --- Pass 3: predecessors + entry marking ----------------------------------------
	for (auto& blockPair : outCFG.Blocks)
	{
		for (const FFlowEdge& edge : blockPair.second.Successors)
		{
			if (edge.To.IsValid() == false)
				continue;
			FBasicBlock* pTarget = outCFG.GetBlockStartingAt(edge.To);
			if (pTarget == nullptr)
				continue;	// edge leaves the analysed range (e.g. ROM / other bank)
			pTarget->Predecessors.push_back(blockPair.first);
			if (edge.Type == EEdgeType::Call)
				pTarget->bIsEntry = true;	// called => an entry point
		}
	}

	// Entry points: the range start plus every block flagged as an entry.
	outCFG.EntryPoints.push_back(start);
	for (const auto& blockPair : outCFG.Blocks)
	{
		if (blockPair.second.bIsEntry && blockPair.first != start)
			outCFG.EntryPoints.push_back(blockPair.first);
	}

	return true;
}

bool BuildCFGForFunction(FCodeAnalysisState& state, const FFunctionInfo& function, FControlFlowGraph& outCFG)
{
	// NOTE: FFunctionInfo::EndAddress is runtime-derived and often == StartAddress for
	// functions the emulator hasn't fully traced (see Docs/CppRecompilationProposal.md).
	// Guard against a zero-length range by extending to the next function entry.
	FAddressRef start = function.StartAddress;
	FAddressRef end = function.EndAddress;

	if (end <= start && state.pFunctions != nullptr)
	{
		// Find the next function entry and stop just before it.
		FAddressRef best;
		for (const auto& fnPair : state.pFunctions->GetFunctions())
		{
			if (start < fnPair.first && (best.IsValid() == false || fnPair.first < best))
				best = fnPair.first;
		}
		if (best.IsValid())
		{
			end = best;
			state.AdvanceAddressRef(end, -1);	// last byte before next entry
		}
	}

	return BuildCFGForAddressRange(state, start, end, outCFG);
}

int ResolveIndirectEdgesFromTraces(FCodeAnalysisState& /*state*/, FControlFlowGraph& /*cfg*/)
{
	// TODO(Phase 0, pass 4): for each entry in cfg.UnresolvedEdges, gather the targets
	// the emulator actually jumped to from that site (execution history / frame trace,
	// exposed via the debugger and MCP), and promote them to resolved IndirectJump edges.
	// Unresolved remainders stay unresolved and become a runtime indirect dispatch.
	return 0;
}

// -------------------------------------------------------------------------------------

FBasicBlock* FControlFlowGraph::GetBlockContaining(FAddressRef addr)
{
	if (Blocks.empty())
		return nullptr;

	auto it = Blocks.upper_bound(addr);
	if (it == Blocks.begin())
		return nullptr;
	--it;
	if (addr >= it->second.StartAddress && addr <= it->second.EndAddress)
		return &it->second;
	return nullptr;
}
