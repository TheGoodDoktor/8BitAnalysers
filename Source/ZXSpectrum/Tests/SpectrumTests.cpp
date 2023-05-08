#include "SpectrumTests.h"

#include "CodeAnalyser/Tests/CodeAnalyserTests.h"

bool RunSpectrumTests()
{
	// TODO: Run tests on spectrum emu & code analyser
	bool bSuccess = true;

	bSuccess &= RunCodeAnalyserTests();

	return bSuccess;
}

