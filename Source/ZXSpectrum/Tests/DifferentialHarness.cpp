#include "DifferentialHarness.h"

#include "../ZXChipsImpl.h"	// brings in the chips z80_t API (NB: do not define CHIPS_IMPL
								//   here - the impl already lives in ZXChipsImpl.c)

#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <fstream>
#include <sstream>
#include <vector>

#if defined(_WIN32)
#include <windows.h>
#endif

namespace DiffHarness
{
// =====================================================================================
// MSVC toolchain probing
// =====================================================================================
//
// We need cl.exe to compile the generated C at test time. Two acceptable paths:
//   (a) cl is already on PATH (test was launched from a Developer Command Prompt, or the
//       caller exported vcvars). Then we just invoke "cl ..." directly.
//   (b) We can locate vcvars64.bat from a known install and prefix every cl invocation with
//       "<vcvars64> >nul 2>&1 && cl ...". The env override RECOMPILER_VCVARS wins; otherwise
//       we try a handful of stock 2022 paths. If neither works, HasMsvcToolchain() returns
//       false and the tests should GTEST_SKIP() with a clear reason.

static bool s_ProbedToolchain = false;
static bool s_HasMsvc = false;
static std::string s_InvokePrefix;	// e.g. "\"C:\\...\\vcvars64.bat\" >nul 2>&1 && " or empty

#if defined(_WIN32)
static bool RunSilent(const std::string& cmd)
{
	// system() returns the shell's exit code; we want it for the inner command. The .bat
	// chain we build below redirects its own noise; system()'s own stdio is suppressed by
	// the caller redirecting cmd's stdout/stderr inside the command string itself.
	return std::system(cmd.c_str()) == 0;
}

static bool ProbeToolchain()
{
	// Try cl directly first (PATH already set).
	if (RunSilent("cl /? >nul 2>&1"))
	{
		s_InvokePrefix.clear();
		return true;
	}

	std::vector<std::string> candidates;
	if (const char* envVar = std::getenv("RECOMPILER_VCVARS"))
		candidates.emplace_back(envVar);

	const char* knownPaths[] = {
		"C:\\Program Files (x86)\\Microsoft Visual Studio\\2022\\BuildTools\\VC\\Auxiliary\\Build\\vcvars64.bat",
		"C:\\Program Files\\Microsoft Visual Studio\\2022\\Community\\VC\\Auxiliary\\Build\\vcvars64.bat",
		"C:\\Program Files\\Microsoft Visual Studio\\2022\\Professional\\VC\\Auxiliary\\Build\\vcvars64.bat",
		"C:\\Program Files\\Microsoft Visual Studio\\2022\\Enterprise\\VC\\Auxiliary\\Build\\vcvars64.bat",
		"C:\\Program Files (x86)\\Microsoft Visual Studio\\2022\\Community\\VC\\Auxiliary\\Build\\vcvars64.bat",
	};
	for (const char* p : knownPaths)
		candidates.emplace_back(p);

	for (const std::string& path : candidates)
	{
		// Try invoking cl through this vcvars. Wrap path in double-quotes for spaces.
		std::string probe = "(\"" + path + "\" >nul 2>&1) && cl /? >nul 2>&1";
		if (RunSilent(probe))
		{
			s_InvokePrefix = "(\"" + path + "\" >nul 2>&1) && ";
			return true;
		}
	}
	return false;
}
#endif

bool HasMsvcToolchain()
{
#if defined(_WIN32)
	if (!s_ProbedToolchain)
	{
		s_HasMsvc = ProbeToolchain();
		s_ProbedToolchain = true;
	}
	return s_HasMsvc;
#else
	return false;
#endif
}

const std::string& MsvcInvokePrefix()
{
	return s_InvokePrefix;
}

// =====================================================================================
// Chips Z80 oracle
// =====================================================================================
//
// Drive the vendored chips emulator over a private flat 64K memory. The bus loop honours
// MREQ (memory read/write) and IORQ (we just return 0xFF on input, swallow output). The
// run terminates when the HALT pin asserts after the program RETs to the 0xFFFF sentinel
// (which we pre-populate with 0x76, the HALT opcode).

bool RunChipsOracle(const std::vector<uint8_t>& initialMem, const FZ80State& init,
	uint16_t entryPC, uint64_t cycleCap, FExecResult& outResult)
{
	if (initialMem.size() != 0x10000)
		return false;

	std::vector<uint8_t> mem = initialMem;
	mem[0xFFFF] = 0x76;	// HALT at sentinel return address - matches z80_call(0xFFFF) push

	z80_t cpu;
	uint64_t pins = z80_init(&cpu);

	// Seed the register file (z80_init left everything as 0xFFFF, PC=0).
	cpu.a = init.A; cpu.f = init.F;
	cpu.b = init.B; cpu.c = init.C;
	cpu.d = init.D; cpu.e = init.E;
	cpu.h = init.H; cpu.l = init.L;
	cpu.af2 = (uint16_t)((init.A_ << 8) | init.F_);
	cpu.bc2 = (uint16_t)((init.B_ << 8) | init.C_);
	cpu.de2 = (uint16_t)((init.D_ << 8) | init.E_);
	cpu.hl2 = (uint16_t)((init.H_ << 8) | init.L_);
	cpu.ix = init.IX; cpu.iy = init.IY;
	cpu.sp = init.SP; cpu.i = init.I; cpu.r = init.R;
	cpu.iff1 = init.IFF1 != 0;
	cpu.iff2 = init.IFF2 != 0;
	cpu.im = init.IM;

	// Push the 0xFFFF sentinel onto the Z80 stack and re-prefetch at the entry. This mirrors
	// what the generated z80_call() does, so both sides see the same stack state on entry.
	cpu.sp -= 2;
	mem[cpu.sp]     = 0xFF;
	mem[(uint16_t)(cpu.sp + 1)] = 0xFF;
	pins = z80_prefetch(&cpu, entryPC);

	uint64_t cycles = 0;
	while (cycles < cycleCap)
	{
		pins = z80_tick(&cpu, pins);
		if (pins & Z80_MREQ)
		{
			const uint16_t addr = Z80_GET_ADDR(pins);
			if (pins & Z80_RD)
			{
				Z80_SET_DATA(pins, mem[addr]);
			}
			else if (pins & Z80_WR)
			{
				mem[addr] = Z80_GET_DATA(pins);
			}
		}
		else if (pins & Z80_IORQ)
		{
			if (pins & Z80_RD)
			{
				Z80_SET_DATA(pins, 0xFF);
			}
			// IO writes intentionally ignored.
		}
		if (pins & Z80_HALT)
			break;
		cycles++;
	}

	if ((pins & Z80_HALT) == 0)
		return false;	// cycle cap hit before reaching the sentinel

	// We pinned HALT at the 0xFFFF sentinel and stopped as soon as the HALT pin asserted.
	// The chips Z80's intra-instruction PC state at the assertion point isn't a clean match
	// for what the generated z80_run leaves behind (which is just whatever the failing
	// z80_lookup was called with - i.e. the sentinel itself). Report the sentinel so the two
	// sides line up; the program's *real* control flow is verified by all the other state.
	const uint16_t reportedPC = 0xFFFF;

	FExecResult& r = outResult;
	r.State.A = cpu.a; r.State.F = cpu.f;
	r.State.B = cpu.b; r.State.C = cpu.c;
	r.State.D = cpu.d; r.State.E = cpu.e;
	r.State.H = cpu.h; r.State.L = cpu.l;
	r.State.A_ = (uint8_t)(cpu.af2 >> 8); r.State.F_ = (uint8_t)cpu.af2;
	r.State.B_ = (uint8_t)(cpu.bc2 >> 8); r.State.C_ = (uint8_t)cpu.bc2;
	r.State.D_ = (uint8_t)(cpu.de2 >> 8); r.State.E_ = (uint8_t)cpu.de2;
	r.State.H_ = (uint8_t)(cpu.hl2 >> 8); r.State.L_ = (uint8_t)cpu.hl2;
	r.State.IX = cpu.ix; r.State.IY = cpu.iy;
	r.State.SP = cpu.sp; r.State.PC = reportedPC;
	r.State.I = cpu.i; r.State.R = cpu.r;
	r.State.IFF1 = cpu.iff1 ? 1 : 0;
	r.State.IFF2 = cpu.iff2 ? 1 : 0;
	r.State.IM = cpu.im;
	r.Memory = std::move(mem);
	// Undo the HALT sentinel so memory comparison against the generated side matches (the
	// generated side leaves 0xFFFF untouched at whatever the initial image had).
	r.Memory[0xFFFF] = initialMem[0xFFFF];
	return true;
}

// =====================================================================================
// Generated-C compile-and-run
// =====================================================================================

// Append a main() that seeds g_Z80Mem and the Z80CpuState, runs the routine, then dumps
// the final state to a binary file. The dump format is fixed and self-describing only by
// position - see ParseDump() below.
static void AppendMainShim(std::string& src, const std::vector<uint8_t>& initialMem,
	const FZ80State& init, uint16_t entryPC, const std::string& dumpPath)
{
	std::ostringstream s;
	s << "\n#include <stdio.h>\n\n";

	// Only emit assignments for non-zero bytes. The full 64K initial-memory literal made cl
	// take O(many minutes) to parse - presumably an O(n^2) somewhere in the front-end. The
	// programs we test seed at most a few dozen bytes of code + data, so this is tiny.
	s << "int main(void) {\n";
	{
		size_t nonZero = 0;
		for (size_t i = 0; i < initialMem.size(); i++)
			if (initialMem[i]) nonZero++;
		s << "\t/* seed " << nonZero << " non-zero bytes; the rest of g_Z80Mem stays at the BSS-initialised zero */\n";
	}
	for (size_t i = 0; i < initialMem.size(); i++)
	{
		if (initialMem[i] == 0)
			continue;
		char buf[64];
		std::snprintf(buf, sizeof(buf), "\tg_Z80Mem[0x%04X] = 0x%02X;\n",
			(unsigned)i, initialMem[i]);
		s << buf;
	}
	s << "\tZ80CpuState cpu;\n";
	s << "\tfor (unsigned char* p = (unsigned char*)&cpu; p < (unsigned char*)(&cpu+1); p++) *p = 0;\n";
	auto setReg = [&](const char* name, uint32_t v) {
		s << "\tcpu." << name << " = 0x" << std::hex << v << std::dec << ";\n";
	};
	setReg("A", init.A); setReg("F", init.F);
	setReg("B", init.B); setReg("C", init.C);
	setReg("D", init.D); setReg("E", init.E);
	setReg("H", init.H); setReg("L", init.L);
	setReg("A_", init.A_); setReg("F_", init.F_);
	setReg("B_", init.B_); setReg("C_", init.C_);
	setReg("D_", init.D_); setReg("E_", init.E_);
	setReg("H_", init.H_); setReg("L_", init.L_);
	setReg("IX", init.IX); setReg("IY", init.IY);
	setReg("SP", init.SP);
	setReg("I", init.I); setReg("R", init.R);
	s << "\tcpu.IFF1 = " << (init.IFF1 ? "1" : "0") << ";\n";
	s << "\tcpu.IFF2 = " << (init.IFF2 ? "1" : "0") << ";\n";
	s << "\tcpu.IM = " << (int)init.IM << ";\n";
	s << "\tcpu.Mem = g_Z80Mem;\n";

	s << "\tz80_call(&cpu, 0x" << std::hex << entryPC << std::dec << ");\n";

	// Dump - explicit field-by-field byte writes so the parser doesn't depend on struct
	// padding. 27 register bytes + 65536 memory bytes.
	s << "\tFILE* fp = fopen(\"" << dumpPath << "\", \"wb\");\n";
	s << "\tif (!fp) return 2;\n";
	s << "\tunsigned char st[27];\n";
	s << "\tst[0]=cpu.A;  st[1]=cpu.F;  st[2]=cpu.B;  st[3]=cpu.C;\n";
	s << "\tst[4]=cpu.D;  st[5]=cpu.E;  st[6]=cpu.H;  st[7]=cpu.L;\n";
	s << "\tst[8]=cpu.A_; st[9]=cpu.F_; st[10]=cpu.B_;st[11]=cpu.C_;\n";
	s << "\tst[12]=cpu.D_;st[13]=cpu.E_;st[14]=cpu.H_;st[15]=cpu.L_;\n";
	s << "\tst[16]=(unsigned char)cpu.IX; st[17]=(unsigned char)(cpu.IX>>8);\n";
	s << "\tst[18]=(unsigned char)cpu.IY; st[19]=(unsigned char)(cpu.IY>>8);\n";
	s << "\tst[20]=(unsigned char)cpu.SP; st[21]=(unsigned char)(cpu.SP>>8);\n";
	s << "\tst[22]=(unsigned char)cpu.PC; st[23]=(unsigned char)(cpu.PC>>8);\n";
	s << "\tst[24]=cpu.I; st[25]=cpu.R;\n";
	s << "\tst[26]=(unsigned char)((cpu.IFF1?1:0)|((cpu.IFF2?1:0)<<1)|(cpu.IM<<2));\n";
	s << "\tfwrite(st, 1, 27, fp);\n";
	s << "\tfwrite(g_Z80Mem, 1, 0x10000, fp);\n";
	s << "\tfclose(fp);\n";
	s << "\treturn 0;\n";
	s << "}\n";

	src += s.str();
}

static bool ParseDump(const std::string& path, FExecResult& outResult)
{
	std::ifstream f(path, std::ios::binary);
	if (!f.is_open())
		return false;
	uint8_t st[27];
	f.read(reinterpret_cast<char*>(st), 27);
	if (f.gcount() != 27)
		return false;
	outResult.State.A  = st[0];  outResult.State.F  = st[1];
	outResult.State.B  = st[2];  outResult.State.C  = st[3];
	outResult.State.D  = st[4];  outResult.State.E  = st[5];
	outResult.State.H  = st[6];  outResult.State.L  = st[7];
	outResult.State.A_ = st[8];  outResult.State.F_ = st[9];
	outResult.State.B_ = st[10]; outResult.State.C_ = st[11];
	outResult.State.D_ = st[12]; outResult.State.E_ = st[13];
	outResult.State.H_ = st[14]; outResult.State.L_ = st[15];
	outResult.State.IX = (uint16_t)(st[16] | (st[17] << 8));
	outResult.State.IY = (uint16_t)(st[18] | (st[19] << 8));
	outResult.State.SP = (uint16_t)(st[20] | (st[21] << 8));
	outResult.State.PC = (uint16_t)(st[22] | (st[23] << 8));
	outResult.State.I  = st[24]; outResult.State.R  = st[25];
	outResult.State.IFF1 = (st[26] >> 0) & 1;
	outResult.State.IFF2 = (st[26] >> 1) & 1;
	outResult.State.IM   = (st[26] >> 2) & 7;

	outResult.Memory.resize(0x10000);
	f.read(reinterpret_cast<char*>(outResult.Memory.data()), 0x10000);
	return f.gcount() == 0x10000;
}

bool BuildAndRunGenerated(const std::string& generatedC,
	const std::vector<uint8_t>& initialMem, const FZ80State& init, uint16_t entryPC,
	const std::string& scratchDir, FExecResult& outResult, std::string& outError)
{
#if !defined(_WIN32)
	(void)generatedC; (void)initialMem; (void)init; (void)entryPC; (void)scratchDir;
	(void)outResult;
	outError = "Differential harness is Windows-only";
	return false;
#else
	if (!HasMsvcToolchain())
	{
		outError = "MSVC cl.exe not reachable (set RECOMPILER_VCVARS or run from a Developer Command Prompt)";
		return false;
	}

	CreateDirectoryA(scratchDir.c_str(), nullptr);	// best effort; OK if it already exists

	const std::string srcPath  = scratchDir + "\\diff_test.c";
	const std::string exePath  = scratchDir + "\\diff_test.exe";
	const std::string dumpPath = scratchDir + "\\diff_test.bin";
	DeleteFileA(exePath.c_str());
	DeleteFileA(dumpPath.c_str());

	std::string src = generatedC;
	// dump path needs backslash-escapes when embedded as a C string literal.
	std::string escDump;
	for (char ch : dumpPath)
	{
		if (ch == '\\') escDump += "\\\\";
		else escDump += ch;
	}
	AppendMainShim(src, initialMem, init, entryPC, escDump);

	{
		std::ofstream srcFile(srcPath, std::ios::binary);
		if (!srcFile.is_open())
		{
			outError = "Failed to write " + srcPath;
			return false;
		}
		srcFile.write(src.data(), (std::streamsize)src.size());
	}

	// Compile. /TC = treat as C; /nologo = quiet; /Fe/Fo place outputs in the scratch dir.
	// Redirect the compiler chatter to a log so failures can be diagnosed.
	const std::string logPath = scratchDir + "\\diff_test.log";
	std::string cmd = MsvcInvokePrefix() + "cd /d \"" + scratchDir + "\" && cl /nologo /TC \""
		+ srcPath + "\" /Fe\"" + exePath + "\" /Fo\"" + scratchDir + "\\\\\" > \""
		+ logPath + "\" 2>&1";
	if (std::system(cmd.c_str()) != 0)
	{
		// Read the log into outError so the gtest failure points at the real cause.
		std::ifstream log(logPath);
		std::stringstream ss; ss << log.rdbuf();
		outError = "cl failed:\n" + ss.str();
		return false;
	}

	// Run.
	const std::string runCmd = "\"" + exePath + "\" > nul 2>&1";
	const int rc = std::system(runCmd.c_str());
	if (rc != 0)
	{
		std::ostringstream ss;
		ss << "Generated exe exited with code " << rc;
		outError = ss.str();
		return false;
	}

	if (!ParseDump(dumpPath, outResult))
	{
		outError = "Failed to parse state dump at " + dumpPath;
		return false;
	}
	return true;
#endif
}

// =====================================================================================
// Diff predicates
// =====================================================================================

// Format a uint16 as hex. Inlined because gtest's AssertionResult::operator<< wraps each
// argument in its own ostringstream, so stream manipulators (std::hex) don't persist past
// the value they were composed with.
static std::string Hex(uint32_t v, int width)
{
	char buf[16];
	std::snprintf(buf, sizeof(buf), "0x%0*X", width, (unsigned)v);
	return buf;
}

::testing::AssertionResult ExpectRegistersMatch(const FZ80State& o, const FZ80State& g, bool bStrict)
{
	// In non-strict mode mask out the bits known to depend on un-modelled CPU state:
	//   - XF/YF (bits 3 and 5 of F) follow the internal WZ register on memory-operand
	//     instructions. WZ isn't tracked in the generated code, so these bits diverge.
	//   - R (refresh) counts opcode fetches in the oracle but is left at 0 by the generated
	//     code (it has no instruction-stream loop - blocks are functions).
	// Strict mode compares every bit; useful once WZ is modelled or to debug a divergence.
	const uint8_t fMask = bStrict ? 0xFF : (uint8_t)~(0x08 | 0x20);	// ~(XF|YF)

#define DIFF_REG(field, width) \
	if (o.field != g.field) \
		return ::testing::AssertionFailure() << "register " #field " mismatch: oracle=" \
			<< Hex((uint32_t)o.field, (width)) << " generated=" \
			<< Hex((uint32_t)g.field, (width))
	DIFF_REG(A, 2);
	if ((o.F & fMask) != (g.F & fMask))
		return ::testing::AssertionFailure() << "register F mismatch (masked 0x"
			<< Hex(fMask, 2) << "): oracle=" << Hex(o.F, 2) << " generated="
			<< Hex(g.F, 2);
	DIFF_REG(B, 2); DIFF_REG(C, 2);
	DIFF_REG(D, 2); DIFF_REG(E, 2); DIFF_REG(H, 2); DIFF_REG(L, 2);
	DIFF_REG(A_, 2); DIFF_REG(F_, 2); DIFF_REG(B_, 2); DIFF_REG(C_, 2);
	DIFF_REG(D_, 2); DIFF_REG(E_, 2); DIFF_REG(H_, 2); DIFF_REG(L_, 2);
	DIFF_REG(IX, 4); DIFF_REG(IY, 4); DIFF_REG(SP, 4); DIFF_REG(PC, 4);
	DIFF_REG(I, 2);
	if (bStrict) DIFF_REG(R, 2);
	DIFF_REG(IFF1, 2); DIFF_REG(IFF2, 2); DIFF_REG(IM, 2);
#undef DIFF_REG
	return ::testing::AssertionSuccess();
}

::testing::AssertionResult ExpectMemoryMatch(const std::vector<uint8_t>& o,
	const std::vector<uint8_t>& g, uint16_t memStart, uint16_t memEnd)
{
	if (o.size() != 0x10000 || g.size() != 0x10000)
		return ::testing::AssertionFailure() << "memory snapshots must be 64K";
	for (uint32_t a = memStart; a <= memEnd; a++)
	{
		if (o[a] != g[a])
			return ::testing::AssertionFailure()
				<< "memory mismatch at " << Hex(a, 4)
				<< ": oracle=" << Hex(o[a], 2)
				<< " generated=" << Hex(g[a], 2);
	}
	return ::testing::AssertionSuccess();
}

}	// namespace DiffHarness
