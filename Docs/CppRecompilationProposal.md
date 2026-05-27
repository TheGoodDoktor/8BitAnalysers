# Technical Proposal: Static Recompilation of Analysed 8-Bit Games to C++

**Status:** Draft / feasibility study
**Author:** Generated for the 8BitAnalysers project (`ArcadeZ80` branch)
**Scope:** Z80 first (ZX Spectrum / CPC / Arcade Z80); 6502 a follow-on.

---

## 1. Verdict (TL;DR)

**Yes — converting an analysed game to compilable C++ is feasible, but "feasible" depends heavily on which kind of C++ you want.** There are two distinct targets, and they have very different risk profiles:

| Target | What it produces | Feasibility | Effort |
|---|---|---|---|
| **Tier A — Faithful translation** | C++ that *behaves* identically to the original (a specialised, statically-recompiled emulator). Reads like assembly-in-C. | **High.** Mechanical. Builds directly on existing infrastructure. | Medium |
| **Tier B — Idiomatic decompilation** | Human-readable C++ with structured control flow (`if`/`while`/`for`), typed variables and structs, real function signatures. | **Partial.** Possible for the "clean" subset of a game; undecidable in general; needs human/LLM in the loop. | High, incremental |

The recommendation is to **build Tier A first** (it is achievable, validatable, and immediately useful), then layer Tier B *structuring passes* on top for the portions of the program that permit it.

> **Output language note:** the backend targets **C99 by default**, with C++ as a selectable alternative (`FRecompilerConfig::bEmitC`). The translation model is identical for both — they differ only in accessor style (pointer/`->` vs reference/`.`), header set, and struct declaration. C is preferred for portability (it links into libretro cores, other emulators, and retro/embedded toolchains far more easily); nothing in either tier needs C++-only features. This project is unusually well-positioned for both, because it already owns (a) a rich per-instruction analysis database, (b) a cycle-accurate emulator to validate against, and (c) an LLM bridge (MCP) to drive the human-assisted parts.

This document explains why, grounded in the actual codebase and in a real analysed game (the ZX Spectrum *Dan Dare* workspace currently loaded in the live analyser).

---

## 2. What "convert to C++" actually means

"Decompile to C++" is a spectrum, not a single operation:

```
 faithful                                                         idiomatic
 <-------------------------------------------------------------------------->
 [1] CPU-in-C      [2] Block-per-       [3] Structured        [4] Typed,
     transliteration   function with        control flow         idiomatic C++
     (memory[] +       goto dispatch        (if/while/for)        (structs, real
      reg struct)                                                  signatures)
```

- **[1]–[2]** preserve behaviour exactly and are mechanically derivable from a disassembly. The output is essentially a *static binary translation*: a program-specific emulator with the instruction-decode loop unrolled into straight-line C++. This is **Tier A**.
- **[3]–[4]** require recovering structure that the original assembly discarded (control-flow regions, variable types, calling conventions). This is **Tier B** and is where the genuinely hard, sometimes undecidable, problems live.

Choosing the target up front matters because it determines the architecture. This proposal targets **[2] as the baseline deliverable** (faithful, with per-function granularity and resolved call/jump edges) and **[3]/[4] as opt-in, best-effort enhancements** per function.

---

## 3. Assets the analyser already provides

The good news: most of the expensive groundwork for a recompiler already exists in `Source/Shared/CodeAnalyser`. The analysis database is rich *at the item (per-instruction / per-byte) level*.

### 3.1 A per-instruction, per-byte annotated model

Memory is modelled in bank-aware 1 KB pages (`FCodeAnalysisPage`, `kPageSize = 1024`, `CodeAnalysisPage.h:24-45`), and each address carries structured metadata:

- **`FCodeInfo`** (`CodeAnalyserTypes.h:384-422`) per instruction: the decoded mnemonic text, **`OperandType`** (Pointer / JumpAddress / Decimal / Hex / etc.), the resolved **`OperandAddress`** (the actual target of a jump/call/data reference), per-instruction data-flow trackers **`Reads`** / **`Writes`** (`:414-415`), an **`bIsCall`** flag, execution counts, and — critically — a **`bSelfModifyingCode`** flag (`:402`).
- **`FDataInfo`** (`CodeAnalyserTypes.h:441-523`) per data byte: **`DataType`** (Byte/Word/Text/Bitmap/Struct/…), display type, struct membership (`StructId`), and read/write cross-references with `LastWriter`.
- **`FLabelInfo`** (`CodeAnalyserTypes.h:299-382`): typed, named labels (Data/Function/Code/Text), global vs local scope, and a reference tracker of every site that points at them.
- **`FAddressRef`** (`CodeAnalyserTypes.h:186-215`): a bank+offset address used everywhere, so the model is already paging-aware.

**Why this matters:** a recompiler's single hardest input — "for every branch/call/pointer, what address does it actually target?" — is *already computed and stored* as `FCodeInfo::OperandAddress`. We do not have to re-derive the control-flow edges from scratch for the statically-resolvable cases.

### 3.2 A working code generator to model the backend on

`AssemblerExport.cpp` is, in effect, a code generator already. `FASMExporter` (`AssemblerExport.h:25`) walks the address-ordered `ItemList` (`CodeAnalyser.h:452`) and emits per-item text — labels, instructions, typed data, comments — via a `printf`-style `Output()` sink, with per-dialect subclasses (sjasmplus, Maxam, SpectrumNext, SkoolKit) registered in a map (`g_AssemblerExporters`). A C++ backend is a **sibling of this class**: same walk, same item dispatch, different emission. We are not inventing a new traversal; we are adding an `FCppExporter`.

### 3.3 Function entry points, parameters, and runtime traces

`FunctionAnalyser` (`FunctionAnalyser.h:96-243`) records function entry points, call points (`FCPUFunctionCall`), exit points, and even **register-level parameter capture** with value histories (`FFunctionParam`). `EInstructionType` (`CodeAnalyserTypes.h:166-181`) already classifies the awkward instructions we care about: `FunctionCall`, `JumpToPointer`, `Halt`, `PortInput`/`PortOutput`, interrupt-mode changes.

### 3.4 A live emulator — i.e. a built-in correctness oracle

The analyser runs the game on a cycle-accurate `chips` CPU core, with execution counts per instruction and the ability to pause/step/inspect via the debugger and MCP. **This is the single biggest advantage this project has over a standalone decompiler:** we can validate generated C++ by running it in lockstep against the reference emulator (see §6).

### 3.5 An LLM bridge (MCP)

`Source/Shared/MCPServer` exposes the analysis state (function index, labels, disassembly, xrefs) and emulator control to an LLM. This is the natural driver for the human-assisted parts of Tier B — resolving jump tables from traces, naming variables, proposing struct layouts.

### 3.6 Concrete evidence: what the listing actually looks like

A real slice from the loaded game (`UpdateSentryGun`, `$AE5C`), straight from the analyser:

```asm
UpdateSentryGun:
    BIT  7,(IY+$00)
    JR   Z,.label_AE71          ; jump if high bit is zero
    CALL function_AEBC
.label_AE65:
    LD   A,(DE)
    CPL
    AND  (HL)
    LD   (HL),A
    INC  DE
    INC  H
    DJNZ .label_AE65
    RES  7,(IY+$00)
.label_AE71:
    CALL ApplySentryLaserVel
    CP   $20
    JR   NC,.label_AE7F
    ...
    LD   B,(IY+$01)
    LD   C,(IY+$02)
    CALL GetRoomFlagMapAddress
    BIT  6,(HL)
    JR   NZ,.label_AE7F
    ...
```

Note what is *already present* and machine-readable: resolved local branch targets (`.label_AE65`, `.label_AE71`), named call targets, a `DJNZ` counted loop (a textbook `for` candidate), flag-conditioned branches, and `(IY+n)` accesses that clearly index a per-entity struct (`+0` = flags byte, `+1`/`+2` = coordinates). The raw material for both Tier A *and* the Tier B structuring of this function is right here.

---

## 4. The hard problems (where the risk lives)

An honest proposal has to be equally clear about what is *not* in the database and what fundamentally resists static conversion.

### 4.1 Function extents and CFG are not stored — only item-level data is

Function records mostly capture **entry points and descriptions**; end addresses and a proper control-flow graph are runtime-derived and frequently incomplete. In the live session, `get_function_disassembly` for several functions returned only the first instruction (their stored `EndAddress == StartAddress`), and the `call-graph` resource was empty. The *full* instruction stream and the resolved edges live in the flat item list, not in tidy per-function CFGs.
**Implication:** we must add a **basic-block / CFG reconstruction pass** (Phase 0 below). The inputs exist (`OperandAddress`, `bIsCall`, instruction types); they just need to be assembled into a graph.

### 4.2 No per-instruction register/flag semantics in the analysis state

The disassembler (`Z80Disassembler.cpp`, built on the vendored `chips` `z80dasm.h`) produces **mnemonic text and operand values only** — not "this instruction reads B and HL, writes A, sets S/Z/H/P/N/C". Tier A *needs* exact semantics for every opcode, including the notoriously fiddly Z80 flag behaviour (half-carry, parity/overflow, the undocumented bits 3/5).
**Mitigation:** the semantics already exist, executable, inside the `chips` `z80.h` core. Two viable routes: (a) author a per-opcode C++ emission table once (≈250 base opcodes + CB/ED/DD/FD prefixes), validated against the core; or (b) generate the translation table programmatically from the core's own step function. Either way this is bounded, one-time work — and it is the bulk of Tier A's effort.

### 4.3 Self-modifying code (SMC)

SMC is common in 8-bit games (patched operands, decompressors, RAM-resident code). The analyser **detects and flags** it (`bSelfModifyingCode`, set at `CodeAnalyser.cpp:1110-1133`) but does not resolve the post-modification instruction stream.
**Implication for Tier A:** if code is compiled to fixed C++ statements, a write into that code is a correctness hole. Handled by: keeping the byte array authoritative and **guarding SMC regions with a fallback micro-interpreter**, or specialising the known patch sites (operand-patch SMC is usually a write to a single operand byte and can be modelled as a variable). Flagged regions tell us exactly where to apply this.

### 4.4 Computed / indirect jumps and jump tables

`JP (HL)`, `JP (IX)`, dispatch tables indexed by a state byte — these are classified (`JumpToPointer`) but their target sets are **not enumerated** statically.
**Mitigation:** this is precisely where the **live emulator pays off** — runtime traces (and execution counts) reveal the observed targets, and the MCP/LLM loop can confirm table bounds. Unresolved computed jumps degrade gracefully to an indirect dispatch through the address-keyed function/block map.

### 4.5 Overlapping and fall-through entry points

Real games reuse code tails: e.g. `AddAToHLAsDWordOffset` / `AddAToHLAsWordOffset` / `AddAToHLWithCarry` sit at consecutive addresses `$AEE3/$AEE4/$AEE5` — three named "functions" that are really three entry points falling through into one shared tail. The clean "one function → one C++ function" mapping breaks here.
**Mitigation:** generate at **basic-block granularity** with address-labelled blocks and allow control to enter mid-function; expose entry points as thin C++ wrappers that jump to the relevant block. This is why Tier A targets level [2] (block dispatch), not naive function-per-label.

### 4.6 Machine environment: banking, I/O, interrupts, timing

- **Banking/paging** (128K models, CPC, ROM overlays) — the model is already bank-aware (`FAddressRef`, `FCodeAnalysisBank`), so the generated memory accessors must route through a paging layer rather than a flat 64 KB array.
- **Hardware I/O** — port reads/writes are detected but `IOAnalyser` is largely a stub (`IOAnalyser.h:42-71`). The generated code must call out to a hardware model (ULA/AY/CRTC/…) or stubs; faithful audio/video requires porting or reusing the relevant `chips` device cores.
- **Interrupts** — IM1/IM2 frame interrupts typically drive the game loop; the recompiled program needs an interrupt dispatch entry consistent with the original mode.
- **Timing/cycle accuracy** — raster effects, contended memory and beam-synced tricks depend on exact cycle timing. A straight translation does **not** preserve cycles unless we carry a cycle counter and gate I/O/interrupts on it.

### 4.7 The undecidable core (Tier B only)

Perfect recovery of types, calling conventions, and structured control flow from arbitrary machine code is undecidable in general (it subsumes the halting problem for the code/data distinction and for indirect-target enumeration). Tier B must therefore be **best-effort and human-supervised**, not a "press button, get clean C++" promise. The analyser's annotations (data types, struct ids, named labels, parameter registers) shrink the unknowns dramatically, but cannot eliminate them.

---

## 5. Proposed approach (phased)

### Phase 0 — CFG / basic-block reconstruction *(new analysis, prerequisite)*
Add a pass that consumes the existing item list and `OperandAddress`/instruction-type data to build, per region: basic blocks, intra-procedural edges (fall-through, conditional/unconditional branches), call edges, and entry-point sets. This becomes shared infrastructure (also useful to the existing UI). Unresolved indirect edges are recorded as "open" and optionally back-filled from runtime traces.

### Phase 1 — Tier A backend: `FCppExporter` *(the core deliverable)*
A sibling of `FASMExporter` that emits compilable C++:
- A `Z80CpuState` struct (registers, flag bits, IX/IY, I/R, interrupt state) + a `Memory` abstraction (paging-aware; **the byte array stays authoritative**).
- One C++ function per basic block (or per function with internal `goto` labels), dispatched through an address→handler map so indirect/uresolved targets and fall-through entry points work.
- Per-instruction emission via the opcode→semantics table (§4.2), with exact Z80 flag handling validated against the `chips` core.
- Memory access routed through `read8/write8/in/out` hooks so I/O, banking, and SMC fallback are interception points, not special cases.
- A small runtime harness (reset vector, interrupt entry, frame loop) and a device-stub interface.

Deliverable: a generated C++ project that, linked against the harness, **reproduces the game's behaviour** and can be validated frame-for-frame (§6).

### Phase 2 — Tier B structuring passes *(opt-in, per function)*
On top of the CFG, for functions that qualify:
- **Control-flow structuring** (interval/structural analysis) to recover `if`/`else`/`while`/`for` — the `DJNZ` and conditional-branch patterns shown in §3.6 are direct targets.
- **Flag-condition lifting**: translate `CP $20` + `JR NC` into `if (a >= 0x20)` rather than emulating flag bits, where the flag's only consumer is the next branch.
- **Variable & struct typing** from the analyser's `FDataInfo` types and `StructId` — e.g. lift the `(IY+n)` accesses in §3.6 into `entity->flags` / `entity->x` / `entity->y`.
- **Function signatures** from `FFunctionParam` register capture.
Output for non-qualifying regions falls back cleanly to the Tier A translation, so the program always builds.

### Phase 3 — Human + LLM in the loop (MCP-driven)
Use the MCP bridge to: resolve computed-jump target sets from runtime traces, confirm jump-table bounds, name and type variables/structs, and mark SMC strategies. Tier B is a *collaboration*, not a fully automatic pipeline.

---

## 6. Validation strategy (the project's superpower)

Because the reference emulator is built in, correctness is testable by **differential execution**:

1. Snapshot CPU + RAM at a known point (the analyser can already do this).
2. Run N frames in the reference `chips` emulator; run the *same* N frames in the generated C++ against the same initial snapshot.
3. Diff register and memory state (the analyser already has memory-compare tooling). Any divergence pinpoints the first mistranslated instruction.

This turns recompiler development from "hope it works" into a tight, automatable regression loop, and it can gate every opcode in the §4.2 table from day one.

---

## 7. Effort, scope, and risk summary

**Recommended initial scope:** Z80, single-bank (48K-class) games, Tier A target [2], with differential validation. Defer 128K banking, audio fidelity, and cycle-exact raster effects to later milestones.

| Risk | Severity | Mitigation |
|---|---|---|
| Z80 flag/semantics correctness | High | Generate/validate opcode table against the `chips` core; differential testing per opcode |
| Self-modifying code | Medium–High | Authoritative byte array + guarded fallback interpreter on flagged regions |
| Computed jumps / jump tables | Medium | Runtime-trace target discovery via emulator + MCP; graceful indirect dispatch |
| Cycle/raster-exact timing | Medium | Carry a cycle counter; gate I/O/interrupts; accept non-exact for first milestone |
| I/O / audio-video fidelity | Medium | Reuse `chips` device cores or stub; out of scope for milestone 1 |
| Idiomatic output quality (Tier B) | Inherent | Position as best-effort + human/LLM assisted; always fall back to Tier A |

**Largest single cost:** the opcode→C++ semantics table (§4.2). It is bounded and one-time, and the validation oracle de-risks it.

---

## 8. Recommendation

1. **Proceed with Tier A.** It is feasible with the infrastructure already in place, produces an immediately verifiable artifact, and reuses the existing exporter pattern, the bank-aware memory model, the resolved-operand data, and the emulator-as-oracle.
2. **Build Phase 0 (CFG reconstruction) first** — it is the missing piece and is reusable beyond recompilation.
3. **Treat Tier B as an incremental, per-function enhancement** driven by the MCP/LLM loop, never as a guarantee of clean output.

The honest one-line answer to "can we convert an analysed game to C++?": **a behaviourally-faithful C++ port is clearly achievable and a natural extension of what 8BitAnalysers already does; a clean idiomatic C++ port is achievable only in part, and is best pursued as a human-assisted refinement layered on top.**

---

## Appendix A — Key code references

| Concept | Location |
|---|---|
| Item-ordered walk + emission (backend template) | `Source/Shared/CodeAnalyser/AssemblerExport.cpp:336-466`, `AssemblerExport.h:25` |
| Per-instruction info (operand type/addr, reads/writes, SMC flag) | `Source/Shared/CodeAnalyser/CodeAnalyserTypes.h:384-422` |
| Per-byte data info (types, struct id, xrefs) | `CodeAnalyserTypes.h:441-523` |
| Typed labels + reference tracking | `CodeAnalyserTypes.h:299-382` |
| Bank-aware address | `CodeAnalyserTypes.h:186-215` |
| Pages / banks | `CodeAnalysisPage.h:24-45`; `CodeAnalyser.h` (`FCodeAnalysisBank`) |
| Functions / params / call & exit points | `Source/Shared/CodeAnalyser/FunctionAnalyser.h:96-243` |
| Instruction classification (call, indirect jump, I/O, IRQ) | `CodeAnalyserTypes.h:166-181` |
| SMC detection | `Source/Shared/CodeAnalyser/CodeAnalyser.cpp:1110-1133` |
| I/O tracking (currently minimal) | `Source/Shared/CodeAnalyser/IOAnalyser.h:42-71` |
| Z80 disassembly (text only; built on `chips`) | `Source/Shared/CodeAnalyser/Z80/Z80Disassembler.cpp` |
| Executable opcode semantics to mine | `Source/Vendor/chips/z80.h` |
| LLM/automation bridge | `Source/Shared/MCPServer/` |

## Appendix B — Evidence base

- Codebase map of the analysis model and exporters (see references in Appendix A).
- Live analysed game: ZX Spectrum *Dan Dare* workspace, queried via the `AnalyserMCP` server (function index, labels, and the `UpdateSentryGun` / `AddAToHL*` listings quoted in §3.6 and §4.5), demonstrating both the richness of item-level annotation and the gaps in stored function-level structure.
