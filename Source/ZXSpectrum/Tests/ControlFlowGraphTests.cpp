// Golden tests for Phase 0 control-flow-graph reconstruction.
//
// These build small, hand-assembled Z80 snippets into RAM, run the analyser's
// disassembly pass (so FCodeInfo + OperandAddress are populated exactly as in a real
// project), then assert the reconstructed CFG block/edge structure. The snippets mirror
// the two patterns called out in Docs/Phase0_CFG_Design.md:
//   1. a DJNZ counted loop with conditional branches (cf. UpdateSentryGun);
//   2. an overlapping fall-through entry-point chain (cf. AddAToHL* at $AEE3/4/5).
//
// Requires the test build (set with_tests true in Source/ZXSpectrum/CMakeLists.txt).

#include "../SpectrumEmu.h"

#include "CodeAnalyser/CodeAnalyser.h"
#include "CodeAnalyser/FunctionAnalyser.h"
#include "CodeAnalyser/Recompiler/ControlFlowGraph.h"

#include <imgui.h>
#include <gtest/gtest.h>

namespace
{
	const FBasicBlock* GetBlock(FControlFlowGraph& cfg, FCodeAnalysisState& state, uint16_t physAddr)
	{
		return cfg.GetBlockStartingAt(state.AddressRefFromPhysicalAddress(physAddr));
	}

	bool HasSuccessor(const FBasicBlock* pBlock, EEdgeType type, uint16_t toPhys)
	{
		if (pBlock == nullptr)
			return false;
		for (const FFlowEdge& edge : pBlock->Successors)
			if (edge.Type == type && edge.To.Address == toPhys)
				return true;
		return false;
	}

	bool HasPredecessor(const FBasicBlock* pBlock, uint16_t fromPhys)
	{
		if (pBlock == nullptr)
			return false;
		for (const FAddressRef& pred : pBlock->Predecessors)
			if (pred.Address == fromPhys)
				return true;
		return false;
	}
}

class FCFGTest : public ::testing::Test
{
protected:
	void SetUp() override
	{
		// FEmuBase::Init touches ImGui (font loading, io config); the real app creates the
		// context in the GLFW main loop, which the headless test harness never runs.
		ImGui::CreateContext();

		FSpectrumLaunchConfig config;
		config.SpecificGame = "ROM";	// don't load the last game
		pEmu = new FSpectrumEmu;
		pEmu->Init(config);
	}

	void TearDown() override
	{
		pEmu->Shutdown();
		delete pEmu;
		pEmu = nullptr;
		ImGui::DestroyContext();
	}

	// Write bytes to RAM and run linear disassembly so each instruction gets an FCodeInfo
	// with ByteSize and (for branches/calls) a resolved OperandAddress.
	void WriteAndAnalyse(uint16_t start, const uint8_t* pBytes, size_t count)
	{
		FCodeAnalysisState& state = pEmu->GetCodeAnalysis();

		for (size_t i = 0; i < count; i++)
			pEmu->WriteByte((uint16_t)(start + i), pBytes[i]);

		// the analysis state must observe the same bytes we wrote
		for (size_t i = 0; i < count; i++)
			ASSERT_EQ(state.ReadByte((uint16_t)(start + i)), pBytes[i]);

		const uint16_t end = (uint16_t)(start + count);
		uint16_t pc = start;
		while (pc < end)
		{
			const uint16_t next = WriteCodeInfoForAddress(state, pc);
			if (next <= pc)
				break;
			pc = next;
		}
	}

	FSpectrumEmu* pEmu = nullptr;
};

// Pattern 1: counted loop + conditional return + unconditional jump.
//
//   8000  LD B,5        06 05
//   8002  DEC A         3D            <- DJNZ target (loop head)
//   8003  DJNZ 8002     10 FD
//   8005  RET Z         C8            <- conditional return (falls through)
//   8006  JP 8000       C3 00 80
TEST_F(FCFGTest, DjnzLoopAndBranches)
{
	const uint16_t kStart = 0x8000;
	const uint8_t code[] = { 0x06, 0x05, 0x3D, 0x10, 0xFD, 0xC8, 0xC3, 0x00, 0x80 };
	WriteAndAnalyse(kStart, code, sizeof(code));

	FCodeAnalysisState& state = pEmu->GetCodeAnalysis();
	FControlFlowGraph cfg;
	ASSERT_TRUE(BuildCFGForAddressRange(state,
		state.AddressRefFromPhysicalAddress(0x8000),
		state.AddressRefFromPhysicalAddress(0x8008), cfg));

	EXPECT_EQ(cfg.Blocks.size(), 4u);
	EXPECT_TRUE(cfg.UnresolvedEdges.empty());

	const FBasicBlock* pEntry = GetBlock(cfg, state, 0x8000);	// LD B,5
	const FBasicBlock* pLoop  = GetBlock(cfg, state, 0x8002);	// DEC A / DJNZ
	const FBasicBlock* pRet   = GetBlock(cfg, state, 0x8005);	// RET Z
	const FBasicBlock* pJump  = GetBlock(cfg, state, 0x8006);	// JP 8000
	ASSERT_NE(pEntry, nullptr);
	ASSERT_NE(pLoop, nullptr);
	ASSERT_NE(pRet, nullptr);
	ASSERT_NE(pJump, nullptr);

	// Entry block falls through into the loop head (8002 is a leader = DJNZ target).
	EXPECT_EQ(pEntry->Terminator, EBlockTerminator::FallThrough);
	EXPECT_TRUE(HasSuccessor(pEntry, EEdgeType::FallThrough, 0x8002));

	// Loop block: conditional DJNZ -> back-edge to 8002, fall-through to 8005.
	EXPECT_EQ(pLoop->Terminator, EBlockTerminator::CondBranch);
	EXPECT_TRUE(HasSuccessor(pLoop, EEdgeType::Branch, 0x8002));
	EXPECT_TRUE(HasSuccessor(pLoop, EEdgeType::FallThrough, 0x8005));
	EXPECT_TRUE(HasPredecessor(pLoop, 0x8000));	// from entry fall-through
	EXPECT_TRUE(HasPredecessor(pLoop, 0x8002));	// self back-edge

	// Conditional RET: ends the block but still falls through to 8006.
	EXPECT_EQ(pRet->Terminator, EBlockTerminator::Return);
	EXPECT_TRUE(HasSuccessor(pRet, EEdgeType::FallThrough, 0x8006));

	// Unconditional JP back to the entry.
	EXPECT_EQ(pJump->Terminator, EBlockTerminator::UncondBranch);
	EXPECT_TRUE(HasSuccessor(pJump, EEdgeType::Jump, 0x8000));
	EXPECT_TRUE(HasPredecessor(pEntry, 0x8006));
}

// Pattern 2: three entry points falling through into a shared tail.
//
//   9000  ADD A,A   87   <- entry
//   9001  ADD A,A   87   <- entry
//   9002  ADD A,L   85   <- entry; shared tail begins here
//   9003  LD L,A    6F
//   9004  RET       C9
TEST_F(FCFGTest, FallThroughEntryChain)
{
	const uint16_t kStart = 0x9000;
	const uint8_t code[] = { 0x87, 0x87, 0x85, 0x6F, 0xC9 };
	WriteAndAnalyse(kStart, code, sizeof(code));

	FCodeAnalysisState& state = pEmu->GetCodeAnalysis();
	ASSERT_NE(state.pFunctions, nullptr);

	// Register the three overlapping entry points as functions so they become leaders.
	for (uint16_t addr = 0x9000; addr <= 0x9002; addr++)
	{
		FFunctionInfo fn;
		fn.StartAddress = state.AddressRefFromPhysicalAddress(addr);
		fn.EndAddress = fn.StartAddress;
		state.pFunctions->AddFunction(fn);
	}

	FControlFlowGraph cfg;
	ASSERT_TRUE(BuildCFGForAddressRange(state,
		state.AddressRefFromPhysicalAddress(0x9000),
		state.AddressRefFromPhysicalAddress(0x9004), cfg));

	EXPECT_EQ(cfg.Blocks.size(), 3u);
	EXPECT_TRUE(cfg.UnresolvedEdges.empty());

	const FBasicBlock* pE0 = GetBlock(cfg, state, 0x9000);
	const FBasicBlock* pE1 = GetBlock(cfg, state, 0x9001);
	const FBasicBlock* pTail = GetBlock(cfg, state, 0x9002);	// shared tail (9002..9004)
	ASSERT_NE(pE0, nullptr);
	ASSERT_NE(pE1, nullptr);
	ASSERT_NE(pTail, nullptr);

	// All three starts are entry points.
	EXPECT_TRUE(pE0->bIsEntry);
	EXPECT_TRUE(pE1->bIsEntry);
	EXPECT_TRUE(pTail->bIsEntry);

	// Fall-through chain 9000 -> 9001 -> 9002.
	EXPECT_TRUE(HasSuccessor(pE0, EEdgeType::FallThrough, 0x9001));
	EXPECT_TRUE(HasSuccessor(pE1, EEdgeType::FallThrough, 0x9002));
	EXPECT_TRUE(HasPredecessor(pE1, 0x9000));
	EXPECT_TRUE(HasPredecessor(pTail, 0x9001));

	// Shared tail ends in an unconditional RET (no flow successor).
	EXPECT_EQ(pTail->Terminator, EBlockTerminator::Return);
	EXPECT_EQ(pTail->EndAddress.Address, 0x9004);
	EXPECT_FALSE(HasSuccessor(pTail, EEdgeType::FallThrough, 0x9005));
}
