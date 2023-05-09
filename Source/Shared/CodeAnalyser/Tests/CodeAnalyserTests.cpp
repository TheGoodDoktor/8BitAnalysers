#include "CodeAnalyserTests.h"

#include "CodeAnalyser/CodeAnalyserTypes.h"

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

bool RunCodeAnalyserTests(void)
{
	return true;
}