#pragma once

// Differential-execution harness for the C/C++ recompiler.
//
// For a given Z80 program (raw byte array + initial register state + entry PC), we:
//   1. Run it on the vendored chips Z80 emulator (the bit-accurate oracle).
//   2. Generate C via FCppExporter (bEmitHarness=true), append a main() that seeds the
//      same initial state and calls z80_call(), compile with MSVC `cl /TC`, run the exe.
//   3. Read back the dumped state from the exe and assert it matches the oracle.
//
// Both runs terminate via the same convention as the generated z80_call: push 0xFFFF
// onto the stack as a sentinel return address; oracle places a HALT at 0xFFFF and stops
// when the HALT pin asserts; generated z80_run stops because 0xFFFF has no dispatch entry.
//
// The harness is Windows-only (uses `cl`). Tests should GTEST_SKIP() with a clear message
// when no MSVC toolchain is reachable; we don't want CI on other platforms to fail.

#include <cstdint>
#include <string>
#include <vector>

#include <gtest/gtest.h>

// Registers tracked for differential comparison. Layout mirrors the generated Z80CpuState
// header so the compiled-exe state dump can be read back field-by-field.
struct FZ80State
{
	uint8_t  A = 0, F = 0, B = 0, C = 0, D = 0, E = 0, H = 0, L = 0;
	uint8_t  A_ = 0, F_ = 0, B_ = 0, C_ = 0, D_ = 0, E_ = 0, H_ = 0, L_ = 0;	// shadow
	uint16_t IX = 0, IY = 0, SP = 0, PC = 0;
	uint8_t  I = 0, R = 0;
	uint8_t  IFF1 = 0, IFF2 = 0;	// bytes (not bool) so the dump format is unambiguous
	uint8_t  IM = 0;
};

struct FExecResult
{
	FZ80State State;
	std::vector<uint8_t> Memory;	// 64K snapshot (size 0x10000)
};

namespace DiffHarness
{
	// True if `cl.exe` is callable (either already on PATH, or via a vcvars64.bat we can
	// locate). When false, tests should GTEST_SKIP() rather than fail. The first call probes
	// and caches; subsequent calls are cheap.
	bool HasMsvcToolchain();

	// Prefix invocation that puts cl on PATH ("cl /TC ..." executes cleanly when prepended
	// with this). Empty string means cl is already on PATH. Valid only after HasMsvcToolchain
	// returned true.
	const std::string& MsvcInvokePrefix();

	// Run the program through the chips Z80 emulator. `initialMem` is a full 64K image (size
	// 0x10000); the harness places a HALT (0x76) at 0xFFFF and pushes 0xFFFF as the sentinel
	// return address before entering. Returns false if the cycle cap was hit before HALT.
	bool RunChipsOracle(const std::vector<uint8_t>& initialMem, const FZ80State& init,
		uint16_t entryPC, uint64_t cycleCap, FExecResult& outResult);

	// Generate C from the analyser's current state, append a main() that seeds the same
	// initial memory + registers and dumps the final state to a binary file, compile with
	// `cl /TC`, run it, parse the dump. Returns false (and fills outError) on any step
	// failing. `generatedC` must already include the harness preamble + program (bEmitHarness
	// true on the exporter).
	bool BuildAndRunGenerated(const std::string& generatedC,
		const std::vector<uint8_t>& initialMem, const FZ80State& init, uint16_t entryPC,
		const std::string& scratchDir, FExecResult& outResult, std::string& outError);

	// gtest predicates: return AssertionSuccess on match, AssertionFailure with the first
	// mismatch's details otherwise. Memory diff is scoped to [memStart,memEnd] so callers can
	// ignore stack scratch / unused ROM areas if needed.
	//
	// Register comparison defaults to "documented behaviour" mode: R (refresh counter, not
	// modelled in the generated code) is skipped entirely, and the undocumented XF/YF bits
	// in F (which depend on the internal WZ register, also not modelled) are masked out.
	// Both deviations are tracked in Docs/Recompiler_Status.md - flip bStrict to true once
	// WZ is modelled or to investigate a specific divergence.
	::testing::AssertionResult ExpectRegistersMatch(const FZ80State& oracle, const FZ80State& gen,
		bool bStrict = false);
	::testing::AssertionResult ExpectMemoryMatch(const std::vector<uint8_t>& oracle,
		const std::vector<uint8_t>& gen, uint16_t memStart, uint16_t memEnd);
}
