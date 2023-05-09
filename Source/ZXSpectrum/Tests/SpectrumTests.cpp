#include "SpectrumTests.h"

#include "CodeAnalyser/Tests/CodeAnalyserTests.h"

#define SOKOL_IMPL
#include <sokol_audio.h>

void SetWindowTitle(const char* pTitle)
{
}

void SetWindowIcon(const char* pIconFile)
{
}

bool RunSpectrumTests()
{
	// TODO: Run tests on spectrum emu & code analyser
	bool bSuccess = true;

	bSuccess &= RunCodeAnalyserTests();

	return bSuccess;
}

int main(int argc, char* argv[])
{

	return 0;
}

