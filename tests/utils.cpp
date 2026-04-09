#include "../src/utils.hpp"
#include <gtest/gtest.h>

TEST(beginsWith, returnsTrueForValidPrefix) {
  EXPECT_TRUE(beginsWith("abc", ""));
  EXPECT_TRUE(beginsWith("abc", "a"));
  EXPECT_TRUE(beginsWith("abc", "ab"));
  EXPECT_TRUE(beginsWith("abc", "abc"));
}

TEST(beginsWith, returnsFalseForInvalidPrefix) {
  EXPECT_FALSE(beginsWith("abc", "xyz"));
  EXPECT_FALSE(beginsWith("abc", "abcd"));
}

TEST(isInteger, returnsTrueForInts) {
  EXPECT_TRUE(isInteger("123"));
  EXPECT_TRUE(isInteger("0"));
  EXPECT_TRUE(isInteger("99999"));
}

TEST(isInteger, returnsFalseForNonInts) {
  EXPECT_FALSE(isInteger("abc"));
  EXPECT_FALSE(isInteger("a"));
  EXPECT_FALSE(isInteger("h8e90h89whruiow"));
}