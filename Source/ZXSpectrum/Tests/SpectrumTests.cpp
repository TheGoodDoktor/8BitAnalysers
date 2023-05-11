#include "SpectrumTests.h"

#include "../SpectrumEmu.h"

#define SOKOL_IMPL
#include <sokol_audio.h>

#include <gtest/gtest.h>
#include "../SnapshotLoaders/SNALoader.h"

// Demonstrate some basic assertions.
TEST(ZXSpectrumTest, BasicAssertions) 
{
	// Expect two strings not to be equal.
	EXPECT_STRNE("hello", "world");
	// Expect equality.
	EXPECT_EQ(7 * 6, 42);
}

class FSpectrumEmuTest : public ::testing::Test
{
protected:
	void SetUp() override
	{
		FSpectrumConfig config;
		pEmu = new FSpectrumEmu;
		pEmu->Init(config);
	}

	void TearDown() override 
	{
		pEmu->Shutdown();
		delete pEmu;
		pEmu = nullptr;
	}

	FSpectrumEmu* pEmu = nullptr;
};

TEST_F(FSpectrumEmuTest, InitialisationTest)
{
	ASSERT_NE(pEmu, nullptr);
	EXPECT_EQ(pEmu->IsInitialised(), true);
};

TEST_F(FSpectrumEmuTest, SnapshotLoaderTest)
{
	ASSERT_NE(pEmu, nullptr);
	const bool bLoaded = LoadSNAFile(pEmu, "Tests/testminimal.sna");
	EXPECT_EQ(bLoaded, true);
};


// needed to get it compiling
void SetWindowTitle(const char* pTitle) {}
void SetWindowIcon(const char* pIconFile) {}

int main(int argc, char* argv[])
{
	::testing::InitGoogleTest(&argc, argv);
	return RUN_ALL_TESTS();
}

