#include "SpectrumTests.h"

#include "CodeAnalyser/Tests/CodeAnalyserTests.h"

#define SOKOL_IMPL
#include <sokol_audio.h>

#include <gtest/gtest.h>

// Demonstrate some basic assertions.
TEST(ZXSpectrumTest, BasicAssertions) 
{
	// Expect two strings not to be equal.
	EXPECT_STRNE("hello", "world");
	// Expect equality.
	EXPECT_EQ(7 * 6, 42);
}

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
	::testing::InitGoogleTest(&argc, argv);
	return RUN_ALL_TESTS();
}

