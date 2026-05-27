# Phase 0 — Control-Flow Graph Reconstruction: Design

**Prerequisite for:** C++ recompilation (`FCppExporter`), see `CppRecompilationProposal.md`.
**Status:** Design + skeleton.
**Location of code:** `Source/Shared/CodeAnalyser/Recompiler/` (`ControlFlowGraph.{h,cpp}`).

---

## 1. Why this phase exists

The proposal established that the analyser stores rich data **per instruction/byte** (`FCodeInfo`, `FDataInfo`, `FLabelInfo`) but does **not** store a usable control-flow graph: function records mostly hold entry points and runtime-derived (often incomplete) end addresses, and the call-graph resource can be empty. A recompiler — even the faithful Tier-A one — needs an explicit graph of basic blocks and edges so it can:

- emit code at **basic-block granularity** (the only way to handle overlapping / fall-through entry points like `AddAToHLAsDWordOffset`/`AddAToHLAsWordOffset`/`AddAToHLWithCarry` at `$AEE3/4/5`);
- resolve every branch/call target into an explicit successor edge;
- know which instructions **terminate** a block, **fall through**, or **branch two ways**;
- isolate the unresolved cases (indirect jumps, jump tables, SMC) for trace-driven or human-assisted resolution.

Phase 0 builds this graph purely from data the analyser already computes. It introduces no emission — it is reusable analysis (the disassembly UI, a future call-graph view, and dead-code detection can all consume it).

## 2. Goals / non-goals

**Goals**
- Reconstruct basic blocks and intra-/inter-procedural edges for an address range or a function.
- Classify each block terminator (fall-through, unconditional/conditional branch, call, return, indirect/unresolved jump).
- Record unresolved edges (e.g. `JP (HL)`) as first-class data, optionally back-fillable from runtime traces.
- Bank-aware throughout (`FAddressRef`, never raw `uint16_t` for stored references).

**Non-goals (later phases)**
- Structuring into `if`/`while`/`for` (Phase 2).
- Type/variable recovery (Phase 2).
- Opcode → C++ semantics (Phase 1 / `FCppExporter`).
- Cross-bank flow following paging writes (milestone 2).

## 3. Inputs (all already in the model)

| Input | Source | Used for |
|---|---|---|
| Instruction extent (`ByteSize`) | `FCodeInfo::ByteSize` | walking the stream |
| Resolved branch/call/pointer target | `FCodeInfo::OperandAddress` | edge targets |
| "is a call" | `FCodeInfo::bIsCall` | call edges |
| Instruction class | `GetInstructionType()` / `GetInstructionTypeZ80()` (`CodeAnalyserZ80.cpp:463`) | indirect jump, halt, ret detection assist |
| Opcode bytes | `FCodeAnalysisState::ReadByte()` | precise terminator classification |
| Labels / function entries | `GetLabelForAddress()`, `pFunctions->GetFunctions()` | forced block leaders |
| Address-ordered items | `FCodeAnalysisState::ItemList` | range iteration |

## 4. CFG data model

(See `ControlFlowGraph.h` for the concrete declarations.)

```
FControlFlowGraph
 ├─ std::map<FAddressRef, FBasicBlock>  Blocks      // keyed by block start
 ├─ std::vector<FAddressRef>            EntryPoints // function entries / range start
 └─ std::vector<FFlowEdge>              UnresolvedEdges

FBasicBlock
 ├─ FAddressRef StartAddress, EndAddress           // [start, end] inclusive of last instr
 ├─ FAddressRef LastInstruction                    // the terminator instruction
 ├─ EBlockTerminator Terminator                    // how the block ends
 ├─ std::vector<FFlowEdge> Successors              // outgoing (incl. fall-through)
 ├─ std::vector<FAddressRef> Predecessors          // incoming block starts
 └─ bool bIsEntry                                  // a function/label entry point

FFlowEdge { FAddressRef From, To; EEdgeType Type; bool bConditional; }

EEdgeType        { FallThrough, Branch, Jump, Call, Return, IndirectJump, Unresolved }
EBlockTerminator { FallThrough, UncondBranch, CondBranch, Return, IndirectJump,
                   Call /*tail*/, Halt, Unresolved }
```

Calls do **not** split the block by default (control returns), but they emit a `Call` edge for the call-graph; the instruction *after* a `CALL` is still marked a leader so it is a clean join point if anything else jumps there.

## 5. Algorithm

Standard two-pass basic-block construction, adapted for 8-bit reality.

### Pass 1 — Leader identification
An address is a **leader** (starts a block) if any of:
1. it is the range start or a function/global-label entry (`bIsEntry`);
2. it is the `OperandAddress` target of any branch/jump within range;
3. it immediately follows a **block-terminating** instruction (branch/jump/ret/halt);
4. it immediately follows a `CALL` (join point for the return).

Leaders are collected into a sorted set of `FAddressRef`. Walking uses `ByteSize` to step instruction-by-instruction and `AdvanceAddressRef` to stay bank-correct. Bytes typed as **data** inside the range are skipped (they are not instructions) — this is where `FDataInfo`/`EItemType::Data` matters, so we never disassemble inline tables as code.

### Pass 2 — Block construction & edge classification
For each leader, accumulate instructions until the next leader or a terminator. Classify the terminator with `ClassifyInstructionZ80()` (see §6) and add successor edges:

| Terminator | Successors |
|---|---|
| FallThrough | `{ next, FallThrough }` |
| UncondBranch (`JP/JR nn`) | `{ target, Jump }` |
| CondBranch (`JP cc / JR cc / DJNZ`) | `{ target, Branch (conditional) }`, `{ next, FallThrough }` |
| Return (`RET/RET cc/RETI/RETN`) | conditional RET also adds `{ next, FallThrough }` |
| IndirectJump (`JP (HL)/(IX)/(IY)`) | recorded in `UnresolvedEdges` |
| Call (only if treated as tail) | `{ target, Call }` |
| Halt | none (or self-loop, configurable) |

`RST p` is modelled as a `Call` edge to the fixed address `p` (it is a call to a ROM/page-0 routine) and falls through.

### Pass 3 — Predecessor linking & entry marking
Walk all successor edges to populate `Predecessors`, and flag blocks whose start is a function entry or has incoming `Call` edges as `bIsEntry`.

### Optional Pass 4 — Trace back-fill (later)
For each `UnresolvedEdges` entry, query observed targets from the live emulator's execution history / frame trace (the analyser already records `ExecutionCount`, and the MCP exposes a frame trace). Resolved targets are promoted to real `IndirectJump` edges; the rest remain `Unresolved` and degrade to an indirect dispatch in generated code.

## 6. Z80 terminator classification

`ClassifyInstructionZ80(state, addr) -> FInstructionFlow` inspects opcode bytes (the same technique as `GetInstructionTypeZ80`, `CodeAnalyserZ80.cpp:463`). Summary of the opcodes that affect flow:

| Opcode(s) | Meaning | Terminator | Cond | Target source |
|---|---|---|---|---|
| `C3` | `JP nn` | UncondBranch | no | `OperandAddress` |
| `C2 CA D2 DA E2 EA F2 FA` | `JP cc,nn` | CondBranch | yes | `OperandAddress` |
| `18` | `JR e` | UncondBranch | no | `OperandAddress` |
| `20 28 30 38` | `JR cc,e` | CondBranch | yes | `OperandAddress` |
| `10` | `DJNZ e` | CondBranch | yes | `OperandAddress` |
| `E9` (also `DD E9`,`FD E9`) | `JP (HL/IX/IY)` | IndirectJump | no | unresolved |
| `CD` | `CALL nn` | (call, falls through) | no | `OperandAddress` |
| `C4 CC D4 DC E4 EC F4 FC` | `CALL cc,nn` | (cond call, falls through) | yes | `OperandAddress` |
| `C9` | `RET` | Return | no | — |
| `C0 C8 D0 D8 E0 E8 F0 F8` | `RET cc` | Return | yes | — |
| `ED 4D / ED 45` | `RETI / RETN` | Return | no | — |
| `C7 CF D7 DF E7 EF F7 FF` | `RST p` | (call, falls through) | no | fixed `p` |
| `76` | `HALT` | Halt | — | — |
| everything else | — | FallThrough | — | — |

The `6502` variant (`JMP/JSR/RTS/RTI/branches/JMP (ind)`) is a parallel function added when 6502 recompilation is tackled; the dispatch is keyed off `state.CPUType`.

## 7. Hard cases & how the CFG represents them

- **Fall-through / overlapping entry points** — handled natively: every entry label is a leader, so a shared tail becomes its own block reachable from multiple entry blocks. No code is duplicated; entries become thin wrappers in emission.
- **Indirect jumps / jump tables** — recorded as `UnresolvedEdges`; never silently dropped. Trace back-fill (Pass 4) or MCP/human annotation resolves them; unresolved ones become a runtime indirect dispatch.
- **Self-modifying code** — `FCodeInfo::bSelfModifyingCode` flagged blocks are tagged so emission can route them through the fallback interpreter rather than fixed statements.
- **Data embedded in code** — skipped in Pass 1 using `EItemType::Data` / `FDataInfo`, so tables between routines aren't mis-decoded.
- **Cross-bank flow** — `FAddressRef` keeps bank identity; targets in other banks produce edges flagged for the paging layer (following them is milestone 2).

## 8. Public API & integration

```cpp
// ControlFlowGraph.h
bool BuildCFGForAddressRange(FCodeAnalysisState& state, FAddressRef start, FAddressRef end,
                             FControlFlowGraph& outCFG);
bool BuildCFGForFunction(FCodeAnalysisState& state, const FFunctionInfo& fn,
                         FControlFlowGraph& outCFG);
FInstructionFlow ClassifyInstruction(FCodeAnalysisState& state, FAddressRef addr); // CPU-dispatched
```

- **Build wiring:** new files live in `Source/Shared/CodeAnalyser/Recompiler/`; `Source/Shared/CMakeShared.txt` is extended with a glob for that directory (a new directory is *not* picked up automatically — re-run CMake after adding files).
- **Consumers:** `FCppExporter` (Phase 1); optionally a call-graph UI view and dead-code analysis.

## 9. Validation

- **Coverage check:** every instruction in the input range belongs to exactly one block (no gaps, no overlaps) — assert during build.
- **Edge soundness:** every non-indirect terminator has at least one resolved successor; conditional terminators have exactly two.
- **Differential cross-check:** the set of reachable instructions from entry points must be a superset of instructions the emulator actually executed (`ExecutionCount > 0`); any executed instruction not reachable in the static CFG flags an unresolved indirect edge to investigate.
- **Golden tests:** hand-built CFGs for the `UpdateSentryGun` region (a clean function with a `DJNZ` loop and several conditional branches) and the `AddAToHL*` fall-through chain, added under the existing GoogleTest harness (`Source/Shared/CodeAnalyser/Tests/`).
