#include "CodeAnalyserTests.h"

#include "CodeAnalyser/CodeAnalyserTypes.h"
#include "CodeAnalyser/CodeAnalysisPage.h"

#include <gtest/gtest.h>

TEST(CodeAnalyserTest, BasicAssertions)
{
	// Expect two strings not to be equal.
	EXPECT_STRNE("hello", "world");
	// Expect equality.
	EXPECT_EQ(7 * 6, 42);
}

TEST(CodeAnalyserTest, FAddressRef)
{
	EXPECT_EQ(sizeof(FAddressRef), 4);			// size
	EXPECT_EQ(FAddressRef().IsValid(),false);	// default to invalid
}

TEST(CodeAnalyserTest, Enums)
{
	EXPECT_EQ((int)ELabelType::Data, 0);
	EXPECT_EQ((int)ELabelType::Function, 1);
	EXPECT_EQ((int)ELabelType::Code, 2);
	EXPECT_EQ((int)ELabelType::Text, 3);
}

bool RunCodeAnalyserTests(void)
{
	return true;
}