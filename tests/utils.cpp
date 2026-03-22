#include <gtest/gtest.h>
#include "../src/utils.hpp"

TEST(beginsWith, returnsTrueForValidPrefix)
{
  EXPECT_TRUE(beginsWith("abc", ""));
  EXPECT_TRUE(beginsWith("abc", "a"));
  EXPECT_TRUE(beginsWith("abc", "ab"));
  EXPECT_TRUE(beginsWith("abc", "abc"));
}

TEST(beginsWith, returnsFalseForInvalidPrefix)
{
  EXPECT_FALSE(beginsWith("abc", "xyz"));
  EXPECT_FALSE(beginsWith("abc", "abcd"));
}