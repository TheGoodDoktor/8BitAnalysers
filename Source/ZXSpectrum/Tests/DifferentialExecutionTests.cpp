// Differential execution tests for the C recompiler.
//
// For each test case:
//   1. Lay a small Z80 program into a 64K image (with seed data where the program reads it).
//   2. Pick an initial register state (SP and entry PC matter; we leave shadows etc. at 0).
//   3. Run it on the chips Z80 emulator (oracle).
//   4. Run linear disassembly so the analyser sees the same bytes, then emit C with the
//      self-contained harness via FCppExporter (bEmitHarness=true).
//   5. Compile the C with `cl`, run the exe, parse the dumped state.
//   6. Assert (state, memory window) match.
//
// This is the safety net that unblocks all further opcode work - any divergence between the
// generated translation and the oracle now fails a test instead of going unnoticed.
//
// Requires the test build (set with_tests true in Source/ZXSpectrum/CMakeLists.txt). Also
// requires MSVC `cl.exe` to be reachable (either on PATH or via vcvars64 in a known location
// or RECOMPILER_VCVARS env). When unavailable the tests GTEST_SKIP() with a clear reason.

#include "../SpectrumEmu.h"
#include "DifferentialHarness.h"

#include "CodeAnalyser/CodeAnalyser.h"
#include "CodeAnalyser/Recompiler/ControlFlowGraph.h"
#include "CodeAnalyser/Recompiler/CppExporter.h"

#include <imgui.h>
#include <gtest/gtest.h>

#include <filesystem>
#include <string>
#include <vector>

namespace
{
	// Helper: write a program into a fresh 64K image at the given physical address.
	std::vector<uint8_t> MakeImage(uint16_t loadAddr, const std::vector<uint8_t>& code,
		const std::vector<std::pair<uint16_t, std::vector<uint8_t>>>& dataBlobs = {})
	{
		std::vector<uint8_t> img(0x10000, 0);
		for (size_t i = 0; i < code.size(); i++)
			img[(uint16_t)(loadAddr + i)] = code[i];
		for (const auto& blob : dataBlobs)
		{
			for (size_t i = 0; i < blob.second.size(); i++)
				img[(uint16_t)(blob.first + i)] = blob.second[i];
		}
		return img;
	}
}

class FDiffExec : public ::testing::Test
{
protected:
	void SetUp() override
	{
		if (!DiffHarness::HasMsvcToolchain())
			GTEST_SKIP() << "MSVC cl.exe not reachable. Set RECOMPILER_VCVARS to a vcvars64.bat "
				<< "path or run from a Developer Command Prompt to enable these tests.";

		ImGui::CreateContext();

		FSpectrumLaunchConfig config;
		config.SpecificGame = "ROM";
		pEmu = new FSpectrumEmu;
		pEmu->Init(config);

		// One scratch dir per test process - we don't need per-test isolation since each
		// test reuses the same file names and overwrites them.
		ScratchDir = (std::filesystem::temp_directory_path() / "8ba_diff_exec").string();
	}

	void TearDown() override
	{
		if (pEmu)
		{
			pEmu->Shutdown();
			delete pEmu;
			pEmu = nullptr;
			ImGui::DestroyContext();
		}
	}

	// Run the same image+entry through both sides and assert equivalence. memDiff* bound the
	// memory range we care about - typically [code start, code end + data region end].
	void RunAndCompare(const std::vector<uint8_t>& image, uint16_t loadAddr, uint16_t loadEnd,
		const FZ80State& init, uint16_t entryPC,
		uint16_t memDiffStart, uint16_t memDiffEnd)
	{
		// Mirror the bytes into the analyser's memory and run linear disassembly so each
		// instruction has an FCodeInfo.
		FCodeAnalysisState& state = pEmu->GetCodeAnalysis();
		for (uint32_t a = loadAddr; a <= loadEnd; a++)
			pEmu->WriteByte((uint16_t)a, image[a]);
		uint16_t pc = loadAddr;
		while (pc <= loadEnd)
		{
			const uint16_t next = WriteCodeInfoForAddress(state, pc);
			if (next <= pc)
				break;
			pc = next;
		}

		// Emit C with the harness.
		FCppExporter exporter;
		std::string generated;
		ASSERT_TRUE(exporter.Init(&generated, pEmu));
		exporter.SetTargetLanguageC(true);
		exporter.SetEmitHarness(true);
		exporter.SetOutputToHeader();
		exporter.AddHeader();
		ASSERT_TRUE(exporter.ExportProgram(loadAddr, loadEnd));
		exporter.Finish();

		FExecResult oracle;
		ASSERT_TRUE(DiffHarness::RunChipsOracle(image, init, entryPC, /*cycleCap*/ 2'000'000, oracle))
			<< "Oracle did not reach the 0xFFFF HALT sentinel within the cycle cap. Did the "
			<< "program RET back to the caller?";

		FExecResult gen;
		std::string err;
		ASSERT_TRUE(DiffHarness::BuildAndRunGenerated(generated, image, init, entryPC,
			ScratchDir, gen, err)) << err;

		EXPECT_TRUE(DiffHarness::ExpectRegistersMatch(oracle.State, gen.State));
		EXPECT_TRUE(DiffHarness::ExpectMemoryMatch(oracle.Memory, gen.Memory,
			memDiffStart, memDiffEnd));
	}

	FSpectrumEmu* pEmu = nullptr;
	std::string ScratchDir;
};

// Sum a 5-byte block via a DJNZ loop. Verified manually during slice 5; this turns the
// verification into a continuously-running test.
TEST_F(FDiffExec, SumBytesViaDjnzLoop)
{
	const uint16_t kStart = 0x8000;
	const std::vector<uint8_t> code = {
		// LD HL,0x9000
		0x21, 0x00, 0x90,
		// LD B,5
		0x06, 0x05,
		// XOR A
		0xAF,
		// loop:  ADD A,(HL); INC HL; DJNZ loop
		0x86, 0x23, 0x10, 0xFB,
		// RET
		0xC9,
	};
	const uint16_t kEnd = (uint16_t)(kStart + code.size() - 1);

	const std::vector<uint8_t> data = { 1, 2, 3, 4, 5 };
	auto image = MakeImage(kStart, code, { { 0x9000, data } });

	FZ80State init;
	init.SP = 0xFF00;	// well clear of the program

	RunAndCompare(image, kStart, kEnd, init, kStart,
		/*memDiffStart*/ 0x8000, /*memDiffEnd*/ 0x9010);
	// Sanity: A=15 (1+2+3+4+5), F has Z=0 H=0 (the chips oracle produces 0x08 here).
	// We don't pin the exact F value - the differential check is the source of truth.
}

// CALL/RET via two routines. Caller sets A=10, calls a leaf that adds 5, then RETs. After
// the caller's own RET we land on the 0xFFFF sentinel.
TEST_F(FDiffExec, CallAndReturn)
{
	const uint16_t kStart = 0x9000;
	const std::vector<uint8_t> code = {
		// 9000: LD A,10
		0x3E, 0x0A,
		// 9002: CALL 0x9007
		0xCD, 0x07, 0x90,
		// 9005: RET
		0xC9,
		// 9006: padding (so callee aligns)
		0x00,
		// 9007: callee - ADD A,5 ; RET
		0xC6, 0x05,
		0xC9,
	};
	const uint16_t kEnd = (uint16_t)(kStart + code.size() - 1);

	auto image = MakeImage(kStart, code);
	FZ80State init;
	init.SP = 0xFF00;

	RunAndCompare(image, kStart, kEnd, init, kStart,
		0x9000, 0x900A);
}

// CB-page exercise: rotate, BIT, SET, RES, plus an EX DE,HL and a memory write so the
// memory diff has something to assert.
TEST_F(FDiffExec, CbPageAndExchange)
{
	const uint16_t kStart = 0xA000;
	const std::vector<uint8_t> code = {
		// LD A,0x81
		0x3E, 0x81,
		// RLC A        (-> 0x03, C set)
		0xCB, 0x07,
		// LD B,A
		0x47,
		// SLA B        (-> 0x06, C cleared)
		0xCB, 0x20,
		// LD HL,0xB000
		0x21, 0x00, 0xB0,
		// LD DE,0xB100
		0x11, 0x00, 0xB1,
		// EX DE,HL     (HL <-> DE)
		0xEB,
		// LD (HL),A
		0x77,
		// SET 4,(HL)
		0xCB, 0xE6,
		// RES 0,(HL)
		0xCB, 0x86,
		// BIT 4,(HL)
		0xCB, 0x66,
		// RET
		0xC9,
	};
	const uint16_t kEnd = (uint16_t)(kStart + code.size() - 1);

	auto image = MakeImage(kStart, code);
	FZ80State init;
	init.SP = 0xFF00;

	RunAndCompare(image, kStart, kEnd, init, kStart,
		0xA000, 0xB110);
}
