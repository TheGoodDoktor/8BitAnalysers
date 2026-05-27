# C/C++ Recompiler — Status & Continuation Guide

**Branch:** `cpp-recompiler` (do not merge to `master` until it produces useful output for real programs).
**As of:** commit `37ac3990` (Phase 1 slice 5). `master` base is `b004aceb`.
**Goal:** statically translate an analysed 8-bit game (Z80 first) into compilable, bit-accurate **C (default C99)** or **C++**.

Read alongside `Docs/CppRecompilationProposal.md` (feasibility/plan) and `Docs/Phase0_CFG_Design.md` (CFG design).

---

## 1. Current state — what works

A Z80 routine in the analyser can be exported to C/C++ that **compiles and runs** with correct, bit-accurate results.

- **Phase 0 — CFG reconstruction:** done, golden-tested. Builds basic blocks + edges from the analyser's per-instruction data.
- **Phase 1 — codegen:** the **entire unprefixed (main) page + the full CB page** are translated, with flag/ALU runtime helpers transcribed bit-for-bit from the vendored chips Z80 core (the differential-validation oracle).
- **Execution model:** **PC-dispatch** — each basic block is a function; a `z80_run` loop invokes blocks keyed on `cpu->PC`; `CALL`/`RET` push/pop the return PC on the Z80 stack. Calls, returns and computed jumps all work; full routines run.
- **Harness:** `bEmitHarness` makes a generated file self-contained (flat 64K memory + `Read8`/`Write8`/`In`/`Out`), runnable via `z80_call(cpu, entry)`.
- **Tests:** 9 `FCFGTest` cases pass (CFG golden + codegen substring + C++ target).
- **End-to-end verified** (MSVC `cl /TC`): a sum-bytes loop → `A=15 F=08` (bit-accurate); a `CALL` routine (callee adds 5, nested `RET`s) → `A=15 PC=FFFF`.

### Commits on the branch
```
37ac3990  slice 5: PC-dispatch execution model
a57d4445  slice 4: self-contained runtime harness
b0a3039d  slice 3: CB-prefixed opcode page
53e05465  slice 2: rest of the main Z80 opcode page
43f3f57e  slice 1: common opcode set
a8ec8f71  validate Phase 0 CFG with golden tests + headless-test fix
0e64e9c2  Phase 0 CFG + FCppExporter skeleton
```

---

## 2. Code map

| File | Role |
|---|---|
| `Source/Shared/CodeAnalyser/Recompiler/ControlFlowGraph.{h,cpp}` | Phase 0: CFG model (`FControlFlowGraph`/`FBasicBlock`/`FFlowEdge`), Z80 instruction classifier `ClassifyInstructionZ80`, `BuildCFGForAddressRange`/`BuildCFGForFunction`. `ClassifyInstruction6502` and `ResolveIndirectEdgesFromTraces` are stubs. |
| `Source/Shared/CodeAnalyser/Recompiler/CppExporter.{h,cpp}` | Codegen: `FCppExporter` (subclasses `FASMExporter`), opcode→C in `EmitInstructionSemanticsZ80`, flag/ALU runtime in `EmitRuntimeHelpers`, the PC-dispatch engine in `EmitDispatcher`, the standalone runtime in `EmitHarness`. Free entry `ExportCpp(pEmu, file, start, end, bEmitC=true, bEmitHarness=false)`. |
| `Source/Shared/CMakeShared.txt` | Globs `CodeAnalyser/Recompiler/*` into the build (new files there need a CMake re-run). |
| `Source/ZXSpectrum/Tests/ControlFlowGraphTests.cpp` | All recompiler tests (`FCFGTest` fixture). |

### How the generated output is structured (per program)
1. Header: includes + `Z80CpuState` struct + hook **declarations** (`Read8`/`Write8`/`In`/`Out`) + inline helpers (`z80_add8…`, rotates, `z80_push16`/`z80_pop16`, flag primitives) + an entry-point list comment.
2. One `static void L_XXXX(Z80CpuState* cpu)` per basic block; control transfers set `cpu->PC` and `return`.
3. The engine: `Z80BlockFn` typedef, `z80_lookup(pc)` switch, `z80_run(cpu)` loop, `z80_call(cpu, entry)`.
4. (Optional, `bEmitHarness`) flat `g_Z80Mem` + hook definitions.

---

## 3. Opcode coverage

**Done (main page):** NOP; `LD r,r'`/`LD r,n` (incl. `(HL)`); 8-bit ALU `A,r`/`A,n`; `INC`/`DEC r`; 16-bit `LD dd,nn`, `INC`/`DEC ss`, `ADD HL,ss`; A/HL↔memory (`LD (BC)/(DE)/(nn),…` and reverse); `PUSH`/`POP`; `RLCA`/`RRCA`/`RLA`/`RRA`; `DAA`/`CPL`/`SCF`/`CCF`; `EX DE,HL`/`EX AF,AF'`/`EXX`/`EX (SP),HL`; `LD SP,HL`; `IN A,(n)`/`OUT (n),A`; `DI`/`EI`. Control flow: `JP`/`JR`/`DJNZ`/`CALL`/`RET`/`RST`/`HALT`/`JP (HL)` (all conditional forms).

**Done (CB page):** `RLC`/`RRC`/`RL`/`RR`/`SLA`/`SRA`/`SLL`/`SRL`, `BIT`/`RES`/`SET` (register + `(HL)`).

**Not done:** `DD`/`FD` (IX/IY incl. `(IX+d)`, and `DD CB`/`FD CB`); `ED` (block ops `LDIR`/`CPIR`/…, 16-bit `ADC`/`SBC HL`, `NEG`, `IM`, `LD I/R,A`, `RRD`/`RLD`). 6502 codegen entirely. Unknown opcodes emit `/* TODO(Phase 1): semantics for opcode 0xNN */` and still build.

---

## 4. Known limitations / approximations
- **`BIT n,(HL)`** undocumented X/Y flags use the operand value, not the internal WZ register (WZ isn't modelled). Exact for register operands.
- **Self-modifying code:** detected and flagged in a block comment, but executed as fixed statements (no fallback interpreter yet).
- **Interrupts:** `DI`/`EI` set `IFF1/IFF2`; no IRQ/NMI delivery, no `IM` handling.
- **Timing:** no cycle counting (raster/contended-memory effects not modelled).
- **I/O:** harness `In`/`Out` are stubs.
- **Banking:** model is bank-aware (`FAddressRef`), but generated code assumes a flat 64K; paging not yet wired.

---

## 5. Build / test / run recipes

**Build the app (also compiles the exporter):**
```
cmake --build Source/ZXSpectrum/build --config Debug --target SpectrumAnalyser
```

**Run the tests** (off by default — `with_tests false` in `Source/ZXSpectrum/CMakeLists.txt`, 3 occurrences):
1. Flip all three to `true` (e.g. `sed -i 's/set( with_tests false )/set( with_tests true )/g' Source/ZXSpectrum/CMakeLists.txt`).
2. `cmake -S Source/ZXSpectrum -B Source/ZXSpectrum/build` (fetches GoogleTest via FetchContent — needs network).
3. `cmake --build Source/ZXSpectrum/build --config Debug --target SpectrumAnalyserTest`
4. From `Data/SpectrumAnalyser`: `..\..\Source\ZXSpectrum\build\bin\Debug\SpectrumAnalyserTest.exe --gtest_filter=FCFGTest.*`
5. **Revert `with_tests` to false** before committing (keeps CI/convention; the test file stays).

**End-to-end (compile & run generated C):** construct an `FCppExporter`, `Init(&std::string)`, `SetTargetLanguageC(true)`, `SetEmitHarness(true)`, `AddHeader()`, `ExportProgram(start,end)`, `Finish()`; append a `main()` that seeds memory/registers and calls `z80_call(&cpu, entry)`; compile with MSVC:
```
cmd /c "<vcvars64.bat> >nul 2>&1 && cd /d <dir> && cl /nologo /TC file.c /Fe:file.exe" ; .\file.exe
```
vcvars64 here: `C:\Program Files (x86)\Microsoft Visual Studio\2022\BuildTools\VC\Auxiliary\Build\vcvars64.bat`.

---

## 6. Gotchas (these have already bitten)
- **`FAddressRef` has no `operator>`** — only `<`, `<=`, `>=`, `==`, `!=`. Use swapped `<`.
- **Headless emulator tests** must call `ImGui::CreateContext()` before `pEmu->Init()` (Init touches ImGui). `SetWindowTitle`/`SetWindowIcon` are NULL-window-guarded (committed in `MainLoopGLFW_GL3.cpp`).
- **New files** under `Recompiler/` aren't picked up until CMake re-runs (the GLOB caveat from the root CLAUDE.md).
- **PowerShell + native exes:** don't pipe `2>&1` then test `$?` — it falsely reports failure. Use `$LASTEXITCODE`.
- **`EmitInstruction` gates on `flow.bEndsBlock`**, not "is last instruction in block" — a fall-through-terminated block's last ordinary instruction still needs its semantics emitted.
- **Call targets must be CFG leaders** (so the callee is a dispatch entry) — handled in the Pass-1 leader rule.

---

## 7. Recommended next steps (in priority order)
1. **Automated differential-execution harness** *(highest value, now unblocked)*: for a routine, run it on the chips emulator (oracle) and on the compiled generated C, then diff registers + memory. `cl` compilation at test time is proven to work. This turns correctness into continuous verification and de-risks all further opcode work.
2. **`ED` page**: block ops (`LDIR`/`LDDR`/`CPIR`…), 16-bit `ADC`/`SBC HL`, `NEG`, `IM n`, `LD I/R,A`, `RRD`/`RLD`. Self-contained and regular.
3. **`DD`/`FD` page**: IX/IY as HL with `(IX+d)`/`(IY+d)` displacement; the `DD CB`/`FD CB` bit-ops; `JP (IX)`/`(IY)` already classified as indirect.
4. Then: SMC fallback interpreter, banking, interrupts/timing — as needed by real targets.
5. Eventually: 6502 classifier + codegen (parallels the Z80 path; `ClassifyInstruction6502` stub exists).

When the recompiler can translate and run a non-trivial real game routine end-to-end (ideally with the differential harness green), open the PR `cpp-recompiler` → `master`.
